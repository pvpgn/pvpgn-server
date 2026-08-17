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
#ifndef INCLUDED_CONNECTION_H
#define INCLUDED_CONNECTION_H

#include <ctime>
#include <list>

#include "common/queue.h"
#include "common/hashtable.h"
#include "common/packet.h"
#include "common/fdwatch.h"


namespace pvpgn
{

	namespace d2dbs
	{

		enum t_conn_class
		{
			conn_class_empty,
			conn_class_init,
			conn_class_d2gs
		};

		enum t_conn_state
		{
			conn_state_empty,
			conn_state_initial,
			conn_state_loggedin,
			conn_state_destroy
		};

		typedef struct
		{
			int				sd; // tcp_sock
			unsigned int	ipaddr; // tcp_addr
			int				fdw_idx;
			unsigned char	major;
			unsigned char	minor;
			unsigned char	type;
			unsigned char	stats;
			unsigned int	serverid;
			unsigned int	verified;
			unsigned char	serverip[16];
			std::time_t		last_active;
#ifdef CONNECTION_INTERNAL_ACCESS
			t_conn_class	cclass;
			t_conn_state	state;
			struct
			{
				t_queue* outqueue; /* packets waiting to be sent */
				unsigned int outsize; /* amount sent from the current output packet */
				unsigned int outsizep;
				t_packet* inqueue; /* packet waiting to be processed */
				unsigned int insize; /* amount received into the current input packet */
			} queues; /* network queues and related data */
#endif
		} t_d2dbs_connection;


		t_d2dbs_connection* conn_create(int sock, unsigned int real_local_addr, unsigned short real_local_port, unsigned int local_addr, unsigned short local_port, unsigned int addr, unsigned short port);

		const char* conn_class_get_str(t_conn_class cclass);
		t_conn_class conn_get_class(t_d2dbs_connection* c);
		void conn_set_class(t_d2dbs_connection* c, t_conn_class cclass);

		const char* conn_state_get_str(t_conn_state state);
		t_conn_state conn_get_state(t_d2dbs_connection* c);
		void conn_set_state(t_d2dbs_connection* c, t_conn_state state);

		void conn_clear_outqueue(t_d2dbs_connection* c);
		t_packet* conn_peek_outqueue(t_d2dbs_connection* c);
		t_packet* conn_pull_outqueue(t_d2dbs_connection* c);

		t_packet* conn_get_in_queue(t_d2dbs_connection* c);
		void conn_put_in_queue(t_d2dbs_connection* c, t_packet* packet);

		unsigned int conn_get_in_size(t_d2dbs_connection* c);
		void conn_set_in_size(t_d2dbs_connection* c, unsigned int size);

		unsigned int conn_get_out_size(t_d2dbs_connection* c);
		void conn_set_out_size(t_d2dbs_connection* c, unsigned int size);

		int conn_push_outqueue(t_d2dbs_connection* c, t_packet* packet);

		int conn_add_fdwatch(t_d2dbs_connection* c, fdwatch_handler handle);

		void conn_close_read(t_d2dbs_connection* c);


		const std::list<t_d2dbs_connection*>& connlist();
		void connlist_reap();
		void connlist_destroy();
	}

}

#endif
