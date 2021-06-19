/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "common/setup_before.h"
#include "setup.h"
#define CONNECTION_INTERNAL_ACCESS
#include "connection.h"

#include <algorithm>
#include <cstring>
#include <list>

#include "compat/psock.h"

#include "common/addr.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "common/network.h"
#include "common/xalloc.h"

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_WS2TCPIP_H
# include <Ws2tcpip.h>
#endif

#include "charlock.h"
#include "pgsid.h"
#include "prefs.h"

#include "common/setup_after.h"


namespace pvpgn
{

	namespace d2dbs
	{

		static std::list<t_d2dbs_connection*> conn_head = {};
		static std::list<t_d2dbs_connection*> conn_dead = {};


		t_d2dbs_connection* conn_create(int sock, unsigned int real_local_addr, unsigned short real_local_port, unsigned int local_addr, unsigned short local_port, unsigned int addr, unsigned short port)
		{
			t_d2dbs_connection* conn = (t_d2dbs_connection*)xmalloc(sizeof(t_d2dbs_connection));
			if (!conn)
			{
				return nullptr;
			}
			std::memset(conn, 0, sizeof(t_d2dbs_connection));

			conn->sd = sock;
			conn->ipaddr = addr;
			conn->fdw_idx = -1;
			conn->major = 0;
			conn->minor = 0;
			conn->type = 0;
			conn->stats = 0;
			conn->serverid = pgsid_get_id(addr);
			conn->verified = 0;
			{
				std::memset(conn->serverip, 0, sizeof(conn->serverip));
				struct in_addr in = {};
				in.s_addr = htonl(addr);
				char addrstr[INET_ADDRSTRLEN] = {};
				inet_ntop(AF_INET, &(in), addrstr, sizeof(addrstr));
				std::strncpy((char*)conn->serverip, addrstr, sizeof(conn->serverip) - 1);
			}
			conn->last_active = std::time(nullptr);
			conn->cclass = conn_class_init;
			conn->state = conn_state_initial;
			conn->queues.outqueue = nullptr;
			conn->queues.outsize = 0;
			conn->queues.outsizep = 0;
			conn->queues.inqueue = nullptr;
			conn->queues.insize = 0;

			conn_head.push_back(conn);

			eventlog(eventlog_level_info, __FUNCTION__, "[{}] created connection serverip={} serverid={}", conn->sd, conn->serverip, conn->serverid);

			return conn;
		}

		// Caller is responsible for removing connection from desired list
		static void conn_destroy(t_d2dbs_connection* c)
		{
			if (c == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			eventlog(eventlog_level_info, __FUNCTION__, "[{}] unlock all characters on gs {}({})", c->sd, c->serverip, c->serverid);
			eventlog_step(prefs_get_logfile_gs(), eventlog_level_info, __FUNCTION__, "unlock all characters on gs %s(%d)", c->serverip, c->serverid);
			eventlog_step(prefs_get_logfile_gs(), eventlog_level_info, __FUNCTION__, "close connection to gs on socket %d", c->sd);
			cl_unlock_all_char_by_gsid(c->serverid);

			// make sure the connection is closed
			if (c->sd != -1)
			{
				// -1 means that the socket was already closed by conn_close()
				fdwatch_del_fd(c->fdw_idx);
				psock_shutdown(c->sd, PSOCK_SHUT_RDWR);
				psock_close(c->sd);
			}

			// clear out the packet queues
			if (c->queues.inqueue)
			{
				packet_del_ref(c->queues.inqueue);
			}

			queue_clear(&c->queues.outqueue);

			eventlog(eventlog_level_info, __FUNCTION__, "[{}] closed {} connection", c->sd, conn_class_get_str(conn_get_class(c)));

			xfree(c);
		}


		const char* conn_class_get_str(t_conn_class cclass)
		{
			switch (cclass)
			{
			case conn_class_init:
				return "init";
			case conn_class_d2gs:
				return "d2gs";
			default:
				return "UNKNOWN";
			}
		}

		t_conn_class conn_get_class(t_d2dbs_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return conn_class_empty;
			}

			return c->cclass;
		}

		void conn_set_class(t_d2dbs_connection* c, t_conn_class cclass)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			c->cclass = cclass;
		}


		const char* conn_state_get_str(t_conn_state state)
		{
			switch (state)
			{
			case conn_state_empty:
				return "empty";
			case conn_state_initial:
				return "initial";
			case conn_state_loggedin:
				return "loggedin";
			case conn_state_destroy:
				return "destroy";
			default:
				return "UNKNOWN";
			}
		}

		t_conn_state conn_get_state(t_d2dbs_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return conn_state_empty;
			}

			return c->state;
		}

		void conn_set_state(t_d2dbs_connection* c, t_conn_state state)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			// special case for destroying connections, add them to conn_dead list
			if (state == conn_state_destroy && c->state != conn_state_destroy)
			{
				conn_dead.push_back(c);
			}
			else if (state != conn_state_destroy && c->state == conn_state_destroy)
			{
				auto it = std::find(conn_dead.begin(), conn_dead.end(), c);
				if (it != conn_dead.end())
				{
					conn_dead.erase(it);
				}
				else
				{
					eventlog(eventlog_level_error, __FUNCTION__, "[{}] could not find connection in conn_dead", c->sd);
				}
			}

			c->state = state;
		}


		void conn_clear_outqueue(t_d2dbs_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			queue_clear(&c->queues.outqueue);
		}
		
		t_packet* conn_peek_outqueue(t_d2dbs_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return nullptr;
			}

			if (c->queues.outqueue)
			{
				return queue_peek_packet((t_queue const* const*)&c->queues.outqueue);
			}
			else
			{
				return nullptr;
			}
		}

		t_packet* conn_pull_outqueue(t_d2dbs_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return nullptr;
			}

			if (c->queues.outsizep)
			{
				if (!(--c->queues.outsizep)) fdwatch_update_fd(c->fdw_idx, fdwatch_type_read);
				return queue_pull_packet((t_queue**)&c->queues.outqueue);
			}

			return nullptr;
		}


		t_packet* conn_get_in_queue(t_d2dbs_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return nullptr;
			}

			return c->queues.inqueue;
		}

		void conn_put_in_queue(t_d2dbs_connection* c, t_packet* packet)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			c->queues.inqueue = packet;
		}


		unsigned int conn_get_in_size(t_d2dbs_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->queues.insize;
		}

		void conn_set_in_size(t_d2dbs_connection* c, unsigned int size)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			c->queues.insize = size;
		}


		unsigned int conn_get_out_size(t_d2dbs_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}

			return c->queues.outsize;
		}


		void conn_set_out_size(t_d2dbs_connection* c, unsigned int size)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			c->queues.outsize = size;
		}


		int conn_push_outqueue(t_d2dbs_connection* c, t_packet* packet)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (!packet)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
				return -1;
			}

			queue_push_packet(&c->queues.outqueue, packet);
			if (!c->queues.outsizep++)
			{
				fdwatch_update_fd(c->fdw_idx, fdwatch_type_read | fdwatch_type_write);
			}

			return 0;
		}


		int conn_add_fdwatch(t_d2dbs_connection* c, fdwatch_handler handle)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			c->fdw_idx = fdwatch_add_fd(c->sd, fdwatch_type_read, handle, c);
			return c->fdw_idx;
		}


		void conn_close_read(t_d2dbs_connection* c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			conn_set_state(c, conn_state_destroy);

			/* only if we still got output packets remove the read availability
			 * from fdwatch, we are NOT allowed to remove all availability or
			 * remove it completely from fdwatch while handling read, also
			 * if the connection has no output packets is ok to leave it
			 * in read availability check cause it will be closed immediately
			 * in connlist_reap() anyway
			 */
			if (conn_peek_outqueue(c))
			{
				fdwatch_update_fd(c->fdw_idx, fdwatch_type_write);
			}
		}



		const std::list<t_d2dbs_connection*>& connlist()
		{
			return conn_head;
		}

		void connlist_reap()
		{
			for (auto it = conn_dead.begin(); it != conn_dead.end(); )
			{
				t_d2dbs_connection* c = *it;
				if (!conn_peek_outqueue(c))
				{
					conn_destroy(c); // also removes from fdwatch

					conn_head.remove(c);
					it = conn_dead.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

		void connlist_destroy()
		{
			for (auto it = conn_head.begin(); it != conn_head.end(); )
			{
				t_d2dbs_connection* c = *it;
				if (!conn_peek_outqueue(c))
				{
					conn_destroy(c); // also removes from fdwatch

					conn_dead.remove(c);
					it = conn_head.erase(it);
				}
				else
				{
					++it;
				}
			}
		}

	}

}