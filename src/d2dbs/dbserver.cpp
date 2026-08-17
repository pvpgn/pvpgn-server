/*
 * Copyright (C) 2001		sousou	(liupeng.cs@263.net)
 *
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
#define SERVER_INTERNAL_ACCESS
#include "setup.h"
#include "dbserver.h"

#include <cstring>
#include <ctime>

#ifdef WIN32
# include <conio.h>
#endif

#include "compat/psock.h"
#include "compat/strerror.h"

#include "common/addr.h"
#include "common/eventlog.h"
#include "common/d2dbs_d2gs_protocol.h"
#include "common/fdwatch.h"
#include "common/network.h"
#include "common/packet.h"
#include "common/xalloc.h"

#include "connection.h"
#include "charlock.h"
#include "d2ladder.h"
#include "handle_d2gs.h"
#include "handle_init.h"
#include "handle_signal.h"
#include "pgsid.h"
#include "prefs.h"
#include "version.h"

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_WS2TCPIP_H
# include <Ws2tcpip.h>
#endif

#include "common/setup_after.h"

#ifdef WIN32
extern int g_ServiceStatus;
#endif

namespace pvpgn
{

	namespace d2dbs
	{

		static std::time_t curr_exittime;

		/* dbs_server_main
		 * The module's driver function -- we just call other functions and
		 * interpret their results.
		 */

		static int dbs_handle_timed_events(void);

		static int handle_accept(void* data, t_fdwatch_type rw);
		static int handle_tcp(void* data, t_fdwatch_type rw);

		static void _server_mainloop(t_addrlist* laddrs);
		static void dbs_check_timeout();
		static void dbs_keepalive();


		static char const* laddr_type_get_str(t_laddr_type laddr_type)
		{
			switch (laddr_type)
			{
			case laddr_type_d2gs:
				return "d2gs";
			default:
				return "UNKNOWN";
			}
		}


		static int sd_accept(t_addr const* curr_laddr, t_laddr_info const* laddr_info, int ssocket)
		{
			char               tempa[32];
			int                csocket;
			struct sockaddr_in caddr;
			psock_t_socklen    caddr_len;
			unsigned int       raddr;
			unsigned short     rport;

			if (!addr_get_addr_str(curr_laddr, tempa, sizeof(tempa)))
				std::strcpy(tempa, "x.x.x.x:x");

			/* accept the connection */
			std::memset(&caddr, 0, sizeof(caddr)); /* not sure if this is needed... modern systems are ok anyway */
			caddr_len = sizeof(caddr);
			if ((csocket = psock_accept(ssocket, (struct sockaddr*)&caddr, &caddr_len)) < 0)
			{
				/* BSD, POSIX error for aborted connections, SYSV often uses EAGAIN or EPROTO */
				if (
#ifdef PSOCK_EWOULDBLOCK
					psock_errno() == PSOCK_EWOULDBLOCK ||
#endif
#ifdef PSOCK_ECONNABORTED
					psock_errno() == PSOCK_ECONNABORTED ||
#endif
#ifdef PSOCK_EPROTO
					psock_errno() == PSOCK_EPROTO ||
#endif
					0)
					eventlog(eventlog_level_error, __FUNCTION__, "client aborted connection on {} (psock_accept: {})", tempa, pstrerror(psock_errno()));
				else /* EAGAIN can mean out of resources _or_ connection aborted :( */
					if (
#ifdef PSOCK_EINTR
						psock_errno() != PSOCK_EINTR &&
#endif
						1)
						eventlog(eventlog_level_error, __FUNCTION__, "could not accept new connection on {} (psock_accept: {})", tempa, pstrerror(psock_errno()));
				return -1;
			}

			/* dont accept new connections while shutting down */
			if (curr_exittime)
			{
				psock_shutdown(csocket, PSOCK_SHUT_RDWR);
				psock_close(csocket);
				return 0;
			}

			char addrstr[INET_ADDRSTRLEN] = {};
			inet_ntop(AF_INET, &(caddr.sin_addr), addrstr, sizeof(addrstr));

			eventlog(eventlog_level_info, __FUNCTION__, "[{}] accepted connection from {} on {}", csocket, addr_num_to_addr_str(ntohl(caddr.sin_addr.s_addr), ntohs(caddr.sin_port)), tempa);

			{
				int optval = 1;
				psock_t_socklen	optlen = sizeof(optval);
				if (psock_setsockopt(csocket, PSOCK_SOL_SOCKET, PSOCK_SO_KEEPALIVE, &optval, optlen))
				{
					eventlog(eventlog_level_info, __FUNCTION__, "[{}] could not set socket option SO_KEEPALIVE (psock_setsockopt: {})", csocket, pstrerror(psock_errno()));
				}
				else
				{
					eventlog(eventlog_level_info, __FUNCTION__, "[{}] set KEEPALIVE option", csocket);
				}
			}

			{
				struct sockaddr_in rsaddr;
				psock_t_socklen    rlen;

				std::memset(&rsaddr, 0, sizeof(rsaddr)); /* not sure if this is needed... modern systems are ok anyway */
				rlen = sizeof(rsaddr);
				if (psock_getsockname(csocket, (struct sockaddr*)&rsaddr, &rlen) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "[{}] unable to determine real local port (psock_getsockname: {})", csocket, pstrerror(psock_errno()));
					/* not a fatal error */
					raddr = addr_get_ip(curr_laddr);
					rport = addr_get_port(curr_laddr);
				}
				else
				{
					if (rsaddr.sin_family != PSOCK_AF_INET)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "local address returned with bad address family {}", (int)rsaddr.sin_family);
						/* not a fatal error */
						raddr = addr_get_ip(curr_laddr);
						rport = addr_get_port(curr_laddr);
					}
					else
					{
						raddr = ntohl(rsaddr.sin_addr.s_addr);
						rport = ntohs(rsaddr.sin_port);
					}
				}
			}

			if (psock_ctl(csocket, PSOCK_NONBLOCK) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] could not set TCP socket to non-blocking mode (closing connection) (psock_ctl: {})", csocket, pstrerror(psock_errno()));
				psock_close(csocket);
				return -1;
			}

			{
				t_d2dbs_connection* c;
				if (!(c = conn_create(csocket, raddr, rport, addr_get_ip(curr_laddr), addr_get_port(curr_laddr), ntohl(caddr.sin_addr.s_addr), ntohs(caddr.sin_port))))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "[{}] unable to create new connection (closing connection)", csocket);
					psock_close(csocket);
					return -1;
				}

				if (conn_add_fdwatch(c, handle_tcp) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "[{}] unable to add socket to fdwatch pool (max connections?)", csocket);
					conn_set_state(c, conn_state_destroy);
					return -1;
				}

				eventlog(eventlog_level_debug, __FUNCTION__, "[{}] client connected to a {} listening address", csocket, laddr_type_get_str(laddr_info->type));
			}

			return 0;
		}


		static int sd_tcpinput(t_d2dbs_connection* c)
		{
			t_packet* packet = nullptr;
			auto csocket = c->sd;
			auto currsize = conn_get_in_size(c);

			if (!conn_get_in_queue(c))
			{
				switch (conn_get_class(c))
				{
				case conn_class_init:
					if (!(packet = packet_create(packet_class_init)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not allocate init packet for input");
						return -1;
					}
					break;
				case conn_class_d2gs:
					if (!(packet = packet_create(packet_class_d2dbs_d2gs)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not allocate d2dbs_d2gs packet for input");
						return -1;
					}
					break;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "[{}] connection has bad class (closing connection)", c->sd);
					conn_close_read(c);
					return -2;
				}

				conn_put_in_queue(c, packet);
				currsize = 0;
			}

			packet = conn_get_in_queue(c);
			switch (net_recv_packet(csocket, packet, &currsize))
			{
			case -1:
				eventlog(eventlog_level_debug, __FUNCTION__, "[{}] read returned -1 (closing connection)", csocket);
				conn_close_read(c);
				return -2;

			case 0: /* still working on it */
				/* eventlog(eventlog_level_debug,__FUNCTION__,"[{}] still reading \"{}\" packet ({} of {} bytes so far)",conn_get_socket(c),packet_get_class_str(packet),conn_get_in_size(c),packet_get_size(packet)); */
				conn_set_in_size(c, currsize);
				break;

			case 1: /* done reading */
			{
				conn_put_in_queue(c, nullptr);

				int ret;
				switch (conn_get_class(c))
				{
				case conn_class_init:
					ret = handle_init_packet(c, packet);
					break;
				case conn_class_d2gs:
					ret = handle_d2gs_packet(c, packet);
					break;
				default:
					ret = -1;
				}

				packet_del_ref(packet);
				if (ret < 0)
				{
					conn_close_read(c);
					return -2;
				}

				conn_set_in_size(c, 0);
			}
			}

			return 0;
		}


		static int sd_tcpoutput(t_d2dbs_connection* c)
		{
			unsigned int totsize = 0;
			auto csocket = c->sd;

			for (;;)
			{
				auto currsize = conn_get_out_size(c);

				t_packet* packet = conn_peek_outqueue(c);
				if (packet == nullptr)
				{
					return -2;
				}

				switch (net_send_packet(csocket, packet, &currsize)) /* avoid warning */
				{
				case -1:
					/* marking connection as "destroyed", memory will be freed later */
					conn_clear_outqueue(c);
					conn_set_state(c, conn_state_destroy);
					return -2;

				case 0: /* still working on it */
					conn_set_out_size(c, currsize);
					return 0; /* bail out */

				case 1: /* done sending */
					packet = conn_pull_outqueue(c);
					packet_del_ref(packet);
					conn_set_out_size(c, 0);

					/* stop if out of packets or EWOULDBLOCK) */
					if (!conn_peek_outqueue(c))
					{
						return 0;
					}

					totsize += currsize;

					break;
				}
			}

			/* not reached */
		}


		static int handle_accept(void* data, t_fdwatch_type rw)
		{
			t_laddr_info* laddr_info = (t_laddr_info*)addr_get_data((t_addr*)data).p;

			return sd_accept((t_addr*)data, laddr_info, laddr_info->ssocket);
		}


		static int handle_tcp(void* data, t_fdwatch_type rw)
		{
			switch (rw)
			{
			case fdwatch_type_read: return sd_tcpinput((t_d2dbs_connection*)data);
			case fdwatch_type_write: return sd_tcpoutput((t_d2dbs_connection*)data);
			default:
				return -1;
			}
		}


		static int _setup_add_addrs(t_addrlist** pladdrs, const char* str, unsigned int defaddr, unsigned short defport, t_laddr_type type)
		{
			t_addr* curr_laddr;
			t_addr_data     laddr_data;
			t_laddr_info* laddr_info;
			t_elem const* acurr;

			if (*pladdrs == NULL)
			{
				*pladdrs = addrlist_create(str, defaddr, defport);
				if (*pladdrs == NULL) return -1;
			}
			else if (addrlist_append(*pladdrs, str, defaddr, defport)) return -1;

			/* Mark all these laddrs for being classic Battle.net service */
			LIST_TRAVERSE_CONST(*pladdrs, acurr)
			{
				curr_laddr = (t_addr*)elem_get_data(acurr);
				if (addr_get_data(curr_laddr).p)
					continue;
				laddr_info = (t_laddr_info*)xmalloc(sizeof(t_laddr_info));
				laddr_info->ssocket = -1;
				laddr_info->type = type;
				laddr_data.p = laddr_info;
				if (addr_set_data(curr_laddr, laddr_data) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not set address data");
					if (laddr_info->ssocket != -1)
					{
						psock_close(laddr_info->ssocket);
						laddr_info->ssocket = -1;
					}
					return -1;
				}
			}

			return 0;
		}


		static int _set_reuseaddr(int sock)
		{
			int val = 1;

			return psock_setsockopt(sock, PSOCK_SOL_SOCKET, PSOCK_SO_REUSEADDR, &val, (psock_t_socklen)sizeof(val));
		}


		static int _bind_socket(int sock, unsigned addr, short port)
		{
			struct sockaddr_in saddr;

			std::memset(&saddr, 0, sizeof(saddr));
			saddr.sin_family = PSOCK_AF_INET;
			saddr.sin_port = htons(port);
			saddr.sin_addr.s_addr = htonl(addr);
			return psock_bind(sock, (struct sockaddr*)&saddr, (psock_t_socklen)sizeof(saddr));
		}


		static int _setup_listensock(t_addrlist* laddrs)
		{
			t_addr* curr_laddr;
			t_laddr_info* laddr_info;
			t_elem const* acurr;
			char            tempa[32];
			int		    fidx;

			LIST_TRAVERSE_CONST(laddrs, acurr)
			{
				curr_laddr = (t_addr*)elem_get_data(acurr);
				if (!(laddr_info = (t_laddr_info*)addr_get_data(curr_laddr).p))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "NULL address info");
					goto err;
				}

				if (!addr_get_addr_str(curr_laddr, tempa, sizeof(tempa)))
					std::strcpy(tempa, "x.x.x.x:x");

				laddr_info->ssocket = psock_socket(PSOCK_PF_INET, PSOCK_SOCK_STREAM, PSOCK_IPPROTO_TCP);
				if (laddr_info->ssocket < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not create a {} listening socket (psock_socket: {})", laddr_type_get_str(laddr_info->type), pstrerror(psock_errno()));
					goto err;
				}

				if (_set_reuseaddr(laddr_info->ssocket) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not set option SO_REUSEADDR on {} socket {} (psock_setsockopt: {})", laddr_type_get_str(laddr_info->type), laddr_info->ssocket, pstrerror(psock_errno()));
				/* not a fatal error... */

				if (_bind_socket(laddr_info->ssocket, addr_get_ip(curr_laddr), addr_get_port(curr_laddr)) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not bind {} socket to address {} TCP (psock_bind: {})", laddr_type_get_str(laddr_info->type), tempa, pstrerror(psock_errno()));
					goto errsock;
				}

				/* tell socket to listen for connections */
				if (psock_listen(laddr_info->ssocket, LISTEN_QUEUE) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not set {} socket {} to listen (psock_listen: {})", laddr_type_get_str(laddr_info->type), laddr_info->ssocket, pstrerror(psock_errno()));
					goto errsock;
				}

				if (psock_ctl(laddr_info->ssocket, PSOCK_NONBLOCK) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not set {} TCP listen socket to non-blocking mode (psock_ctl: {})", laddr_type_get_str(laddr_info->type), pstrerror(psock_errno()));

				/* index not stored persisently because we dont need to refer to it later */
				fidx = fdwatch_add_fd(laddr_info->ssocket, fdwatch_type_read, handle_accept, curr_laddr);
				if (fidx < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not add listening socket {} to fdwatch pool (max sockets?)", laddr_info->ssocket);
					goto errsock;
				}

				eventlog(eventlog_level_info, __FUNCTION__, "listening for {} connections on {} TCP", laddr_type_get_str(laddr_info->type), tempa);
			}

			return 0;

		errsock:
			psock_close(laddr_info->ssocket);
			laddr_info->ssocket = -1;

		err:
			return -1;
		}


		static void _server_mainloop(t_addrlist* laddrs)
		{
			while (1)
			{

#ifdef WIN32
				if (g_ServiceStatus < 0 && kbhit() && getch() == 'q')
					d2dbs_signal_quit_wrapper();
				if (g_ServiceStatus == 0) d2dbs_signal_quit_wrapper();

				while (g_ServiceStatus == 2) Sleep(1000);
#endif

				if (d2dbs_handle_signal() < 0)
				{
					eventlog(eventlog_level_info, __FUNCTION__, "the server is shutting down ({} connections left)", connlist().size());
					break;
				}

				dbs_handle_timed_events();


				/* no need to populate the fdwatch structures as they are populated on the fly
				 * by sd_accept, conn_push_outqueue, conn_pull_outqueue, conn_destory */

				 /* find which sockets need servicing */
				switch (fdwatch(D2DBS_POLL_INTERVAL))
				{
				case -1: /* error */
					if (
#ifdef PSOCK_EINTR
						psock_errno() != PSOCK_EINTR &&
#endif
						1)
						eventlog(eventlog_level_error, __FUNCTION__, "fdwatch() failed (errno: {})", pstrerror(psock_errno()));
				case 0: /* timeout... no sockets need checking */
					continue;
				}

				/* cycle through the ready sockets and handle them */
				fdwatch_handle();

				/* reap dead connections */
				connlist_reap();

			}
		}


		static void _shutdown_addrs(t_addrlist* laddrs)
		{
			t_addr* curr_laddr;
			t_laddr_info* laddr_info;
			t_elem const* acurr;

			LIST_TRAVERSE_CONST(laddrs, acurr)
			{
				curr_laddr = (t_addr*)elem_get_data(acurr);

				if ((laddr_info = (t_laddr_info*)addr_get_data(curr_laddr).p))
				{
					if (laddr_info->ssocket != -1)
					{
						psock_close(laddr_info->ssocket);
					}

					xfree(laddr_info);
				}
			}

			addrlist_destroy(laddrs);
		}


		int pre_server_startup()
		{
			eventlog(eventlog_level_info, __FUNCTION__, D2DBS_VERSION);

			if (d2dbs_d2ladder_init() == -1)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "d2dbs_d2ladder_init() failed");
				return STATUS_D2LADDER_FAILURE;
			}

			if (cl_init(DEFAULT_HASHTBL_LEN, DEFAULT_GS_MAX) == -1)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "cl_init() failed");
				return STATUS_CHARLOCK_FAILURE;
			}

			if (fdwatch_init(D2DBS_FDWATCH_MAX_CONNECTIONS))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "error initilizing fdwatch");
				return STATUS_FDWATCH_FAILURE;
			}

			return 0;
		}


		bool server_process()
		{
			t_addrlist* laddrs = nullptr;
			if (_setup_add_addrs(&laddrs, d2dbs_prefs_get_servaddrs(), INADDR_ANY, DEFAULT_LISTEN_PORT, laddr_type_d2gs))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not create {} server address list from \"{}\"", laddr_type_get_str(laddr_type_d2gs), d2dbs_prefs_get_servaddrs());
				return false;
			}

			if (_setup_listensock(laddrs))
			{
				_shutdown_addrs(laddrs);
				return false;
			}

			_server_mainloop(laddrs);

			// cleanup for server shutdown
			connlist_destroy(); // equivalent to pvpgn::bnetd::_shutdown_conns()
			_shutdown_addrs(laddrs);

			return true;
		}


		void post_server_shutdown(int status)
		{
			switch (status)
			{
			case 0:
				fdwatch_close();
			case STATUS_FDWATCH_FAILURE:
				cl_destroy();
			case STATUS_CHARLOCK_FAILURE:
				d2dbs_d2ladder_destroy();
			case STATUS_D2LADDER_FAILURE:
				pgsid_destroy();
				break;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "got bad status \"{}\" during shutdown", status);
			}
		}


		static int dbs_handle_timed_events(void)
		{
			static	std::time_t		prev_ladder_save_time = 0;
			static	std::time_t		prev_keepalive_save_time = 0;
			static  std::time_t		prev_timeout_checktime = 0;
			std::time_t			now;

			now = std::time(NULL);
			if (now - prev_ladder_save_time > (signed)prefs_get_laddersave_interval()) {
				d2ladder_saveladder();
				prev_ladder_save_time = now;
			}
			if (now - prev_keepalive_save_time > (signed)prefs_get_keepalive_interval()) {
				dbs_keepalive();
				prev_keepalive_save_time = now;
			}
			if (now - prev_timeout_checktime > (signed)d2dbs_prefs_get_timeout_checkinterval()) {
				dbs_check_timeout();
				prev_timeout_checktime = now;
			}
			return 0;
		}


		static void dbs_check_timeout()
		{
			std::time_t now = std::time(nullptr);
			unsigned int timeout = d2dbs_prefs_get_idletime();

			for (auto c : connlist())
			{
				if (!c)
				{
					continue;
				}

				if (now - c->last_active > timeout)
				{
					eventlog(eventlog_level_debug, __FUNCTION__, "[{}] connection {} timed out", c->sd, c->serverid);

					conn_set_state(c, conn_state_destroy);
				}
			}
		}

		static void dbs_keepalive()
		{
			for (auto c : connlist())
			{
				if (!c)
				{
					continue;
				}

				t_packet* rpacket = packet_create(packet_class_d2dbs_d2gs);
				if (!rpacket)
				{
					continue;
				}

				packet_set_size(rpacket, sizeof(t_d2dbs_d2gs_echorequest));
				packet_set_type(rpacket, D2DBS_D2GS_ECHOREQUEST);

				bn_int_set(&rpacket->u.d2dbs_d2gs_echorequest.h.seqno, 0);

				conn_push_outqueue(c, rpacket);

				packet_del_ref(rpacket);
			}
		}

	}

}
