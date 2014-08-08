/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
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
#include "setup.h"
#include "connection.h"

#include <limits>
#include <cctype>
#include <cstring>
#include <ctime>
#include <cassert>

#include "compat/psock.h"
#include "compat/strcasecmp.h"
#include "common/eventlog.h"
#include "common/introtate.h"
#include "common/addr.h"
#include "common/xalloc.h"
#include "common/network.h"
#include "prefs.h"
#include "game.h"
#include "net.h"
#include "handle_init.h"
#include "handle_bnetd.h"
#include "handle_d2cs.h"
#include "handle_d2gs.h"
#include "d2gs.h"
#include "s2s.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace d2cs
	{

		static t_hashtable 	* connlist_head = NULL;
		static t_hashtable	* conn_charname_list_head = NULL;
		static t_list		* connlist_dead = NULL;
		static unsigned int	total_connection = 0;

		static int conn_handle_connecting(t_connection * c);
		static t_packet * conn_create_packet(t_connection * c);
		static int conn_handle_packet(t_connection * c, t_packet * packet);
		static int conn_handle_read(t_connection * c);
		static int conn_handle_write(t_connection * c);
		static unsigned int conn_charname_hash(char const * charname);
		static unsigned int conn_sessionnum_hash(unsigned int sessionnum);

		static unsigned int conn_sessionnum_hash(unsigned int sessionnum)
		{
			return sessionnum;
		}

		static unsigned int conn_charname_hash(char const * charname)
		{
			unsigned int hash;
			unsigned int i, len, pos;
			unsigned int ch;

			ASSERT(charname, 0);
			len = std::strlen(charname);
			for (hash = 0, i = 0, pos = 0; i < len; i++) {
				if (isascii((int)charname[i])) {
					ch = (unsigned int)(unsigned char)std::tolower((int)charname[i]);
				}
				else {
					ch = (unsigned int)(unsigned char)charname[i];
				}
				hash ^= ROTL(ch, pos, (std::numeric_limits<unsigned int>::digits));
				pos += (std::numeric_limits<unsigned char>::digits) - 1;
			}
			return hash;
		}

		extern t_hashtable * d2cs_connlist(void)
		{
			return connlist_head;
		}

		extern int d2cs_connlist_create(void)
		{
			if (!(connlist_head = hashtable_create(200))) return -1;
			if (!(conn_charname_list_head = hashtable_create(200))) return -1;
			return 0;
		}

		extern int d2cs_connlist_destroy(void)
		{
			t_connection 	* c;
			t_elem		* curr;


			if (connlist_dead) {
				d2cs_connlist_reap();
				if (list_destroy(connlist_dead))
					eventlog(eventlog_level_error, __FUNCTION__, "error destroy conndead list");
				connlist_dead = NULL;
			}

			BEGIN_HASHTABLE_TRAVERSE_DATA(connlist_head, c, t_connection)
			{
				d2cs_conn_destroy(c, &curr);
			}
			END_HASHTABLE_TRAVERSE_DATA()

			if (hashtable_destroy(connlist_head) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error destroy connection list");
				return -1;
			}
			connlist_head = NULL;

			if (hashtable_destroy(conn_charname_list_head) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error destroy connection charname list");
				return -1;
			}
			conn_charname_list_head = NULL;
			return 0;
		}

		extern int d2cs_connlist_reap(void)
		{
			t_connection 	* c;

			if (!connlist_dead) return 0;
			BEGIN_LIST_TRAVERSE_DATA(connlist_dead, c, t_connection)
			{
				d2cs_conn_destroy(c, &curr_elem_);
			}
			END_LIST_TRAVERSE_DATA()

				return 0;
		}

		extern int conn_check_multilogin(t_connection const * c, char const * charname)
		{
			t_connection * conn;

			ASSERT(charname, -1);
			if (!prefs_check_multilogin()) return 0;
			if (gamelist_find_character(charname)) {
				return -1;
			}
			conn = d2cs_connlist_find_connection_by_charname(charname);
			if (conn && conn != c) {
				return -1;
			}
			return 0;
		}

		extern t_connection * d2cs_connlist_find_connection_by_sessionnum(unsigned int sessionnum)
		{
			t_connection 	* c;
			t_entry		* curr;
			unsigned int	hash;

			hash = conn_sessionnum_hash(sessionnum);
			HASHTABLE_TRAVERSE_MATCHING(connlist_head, curr, hash)
			{
				if (!(c = (t_connection*)entry_get_data(curr))) {
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection in list");
				}
				else if (c->sessionnum == sessionnum) {
					hashtable_entry_release(curr);
					return c;
				}
			}
			return NULL;
		}

		extern t_connection * d2cs_connlist_find_connection_by_charname(char const * charname)
		{
			t_entry		* curr;
			t_connection 	* c;
			unsigned int	hash;

			hash = conn_charname_hash(charname);
			HASHTABLE_TRAVERSE_MATCHING(connlist_head, curr, hash)
			{
				if (!(c = (t_connection*)entry_get_data(curr))) {
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection in list");
				}
				else {
					if (!c->charname) continue;
					if (!strcmp_charname(c->charname, charname)) {
						hashtable_entry_release(curr);
						return c;
					}
				}
			}
			return NULL;
		}

		static t_packet * conn_create_packet(t_connection * c)
		{
			t_packet	* packet;

			switch (c->cclass) {
				CASE(conn_class_init, packet = packet_create(packet_class_init));
				CASE(conn_class_d2cs, packet = packet_create(packet_class_d2cs));
				CASE(conn_class_d2gs, packet = packet_create(packet_class_d2gs));
				CASE(conn_class_bnetd, packet = packet_create(packet_class_d2cs_bnetd));
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "got bad connection class %d", c->cclass);
				return NULL;
			}
			if (!packet) {
				eventlog(eventlog_level_error, __FUNCTION__, "error create packet");
				return NULL;
			}
			d2cs_conn_set_in_queue(c, packet);
			return packet;
		}

		static int conn_handle_connecting(t_connection * c)
		{
			int	retval;

			if (net_check_connected(c->sock) < 0) {
				eventlog(eventlog_level_warn, __FUNCTION__, "can not connect to %s", addr_num_to_addr_str(c->addr, c->port));
				return -1;
			}
			eventlog(eventlog_level_info, __FUNCTION__, "connected to %s", addr_num_to_addr_str(c->addr, c->port));
			c->state = conn_state_init;
			/* this is a kind of hack to not update fd but updating breaks kqueue
			 * and the clean fix would require a cache a userland copy of the kernel
			 * kqueue fds, considering that it also doesnt brake anything else should do
			 * for the moment
			 fdwatch_update_fd(c->sock, fdwatch_type_read); */
			switch (c->cclass) {
			case conn_class_bnetd:
				retval = handle_bnetd_init(c);
				break;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "got bad connection class %d", c->cclass);
				return -1;
			}
			return retval;
		}

		static int conn_handle_packet(t_connection * c, t_packet * packet)
		{
			int	retval;

			switch (c->cclass) {
				CASE(conn_class_init, retval = d2cs_handle_init_packet(c, packet));
				CASE(conn_class_d2cs, retval = d2cs_handle_d2cs_packet(c, packet));
				CASE(conn_class_d2gs, retval = handle_d2gs_packet(c, packet));
				CASE(conn_class_bnetd, retval = handle_bnetd_packet(c, packet));
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "got bad connection class %d (close connection)", c->cclass);
				retval = -1;
				break;
			}
			return retval;
		}

		static int conn_handle_read(t_connection * c)
		{
			t_packet	* packet;
			int		retval;

			packet = d2cs_conn_get_in_queue(c);
			if (!packet) {
				packet = conn_create_packet(c);
				if (!packet) return -1;
				c->insize = 0;
			}

			switch (net_recv_packet(c->sock, packet, &c->insize)) {
			case -1:
				retval = -1;
				break;
			case 0:
				retval = 0;
				break;
			case 1:
				c->insize = 0;
				d2cs_conn_set_in_queue(c, NULL);
				retval = conn_handle_packet(c, packet);
				packet_del_ref(packet);
				break;
			default:
				retval = 0;
				break;
			}
			return retval;
		}

		static int conn_handle_write(t_connection * c)
		{
			t_packet	* packet;
			int		retval;

			if (c->state == conn_state_connecting) {
				return conn_handle_connecting(c);
			}

			if (!(packet = conn_peek_outqueue(c))) return 0;

			switch (net_send_packet(c->sock, packet, &c->outsize)) {
			case -1:
				retval = -1;
				break;
			case 0:
				retval = 0;
				break;
			case 1:
				c->outsize = 0;
				packet = conn_pull_outqueue(c);
				packet_del_ref(packet);
				retval = 0;
				break;
			default:
				retval = -1;
			}
			return retval;
		}

		extern int conn_handle_socket(t_connection * c)
		{
			std::time_t	now;

			ASSERT(c, -1);
			now = std::time(NULL);
			if (c->socket_flag & SOCKET_FLAG_READ) {
				if (conn_handle_read(c) < 0) return -1;
				c->last_active = now;
			}
			if (c->socket_flag & SOCKET_FLAG_WRITE) {
				if (conn_handle_write(c) < 0) return -1;
				c->last_active = now;
			}
			c->socket_flag = 0;
			return 0;
		}

		extern int connlist_check_timeout(void)
		{
			t_connection	* c;
			std::time_t		now;

			now = std::time(NULL);
			BEGIN_HASHTABLE_TRAVERSE_DATA(connlist_head, c, t_connection)
			{
				switch (c->cclass) {
				case conn_class_d2cs:
					if (prefs_get_idletime() && (now - c->last_active > prefs_get_idletime())) {
						eventlog(eventlog_level_info, __FUNCTION__, "client %d idled too long std::time, destroy it", c->sessionnum);
						d2cs_conn_set_state(c, conn_state_destroy);
					}
					break;
				case conn_class_d2gs:
					if (prefs_get_s2s_idletime() && now - c->last_active > prefs_get_s2s_idletime()) {
						eventlog(eventlog_level_info, __FUNCTION__, "server %d timed out", c->sessionnum);
						d2cs_conn_set_state(c, conn_state_destroy);
					}
					break;
				case conn_class_bnetd:
					break;
				default:
					break;
				}
			}
			END_HASHTABLE_TRAVERSE_DATA()
				return 0;
		}

		extern t_connection * d2cs_conn_create(int sock, unsigned int local_addr, unsigned short local_port,
			unsigned int addr, unsigned short port)
		{
			static unsigned int	sessionnum = 1;
			t_connection		* c;

			if (sock < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad socket");
				return NULL;
			}
			c = (t_connection*)xmalloc(sizeof(t_connection));
			c->charname = NULL;
			c->account = NULL;
			c->sock = sock;
			c->fdw_idx = -1;
			c->socket_flag = 0;
			c->local_addr = local_addr;
			c->local_port = local_port;
			c->addr = addr;
			c->port = port;
			c->cclass = conn_class_init;
			c->state = conn_state_init;
			c->sessionnum = sessionnum++;
			c->outqueue = NULL;
			c->inqueue = NULL;
			c->outsize = 0;
			c->outsizep = 0;
			c->insize = 0;
			c->charinfo = NULL;
			c->d2gs_id = 0;
			c->gamequeue = NULL;
			c->last_active = std::time(NULL);
			c->sessionnum_hash = conn_sessionnum_hash(c->sessionnum);
			c->bnetd_sessionnum = 0;
			c->charname_hash = 0;
			if (hashtable_insert_data(connlist_head, c, c->sessionnum_hash) < 0) {
				xfree(c);
				eventlog(eventlog_level_error, __FUNCTION__, "error add connection to list");
				return NULL;
			}
			total_connection++;
			eventlog(eventlog_level_info, __FUNCTION__, "created session=%d socket=%d (%d current connections)", c->sessionnum, sock, total_connection);
			return c;
		}

		extern int d2cs_conn_destroy(t_connection * c, t_elem ** curr)
		{
			t_elem * elem;

			ASSERT(c, -1);
			if (c->state == conn_state_destroying) return 0;
			if (hashtable_remove_data(connlist_head, c, c->sessionnum_hash) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error remove connection from list");
				return -1;
			}
			c->state = conn_state_destroying;
			if (c->d2gs_id && c->cclass == conn_class_d2gs) {
				d2gs_deactive(d2gslist_find_gs(c->d2gs_id), c);
			}
			if (c->cclass == conn_class_bnetd) {
				s2s_destroy(c);
			}
			if (c->gamequeue) {
				gq_destroy(c->gamequeue, &elem);
			}
			if (c->account) xfree((void *)c->account);
			if (c->charinfo) xfree((void *)c->charinfo);
			if (c->charname) d2cs_conn_set_charname(c, NULL);
			if (c->inqueue) packet_del_ref(c->inqueue);
			queue_clear(&c->outqueue);

			if (connlist_dead) list_remove_data(connlist_dead, c, curr);
			fdwatch_del_fd(c->fdw_idx);
			psock_shutdown(c->sock, PSOCK_SHUT_RDWR);
			psock_close(c->sock);

			total_connection--;
			eventlog(eventlog_level_info, __FUNCTION__, "[%d] closed connection %d (%d left)", c->sock, c->sessionnum, total_connection);
			xfree(c);
			return 0;
		}

		extern int d2cs_conn_get_socket(t_connection const * c)
		{
			ASSERT(c, -1);
			return c->sock;
		}

		extern t_conn_state d2cs_conn_get_state(t_connection const * c)
		{
			ASSERT(c, conn_state_none);
			return c->state;
		}

		extern int d2cs_conn_set_state(t_connection * c, t_conn_state state)
		{
			t_elem * curr;

			ASSERT(c, -1);
			/* special case for destroying connections, add them to connlist_dead list */
			if (state == conn_state_destroy && c->state != conn_state_destroy) {
				if (!connlist_dead)
					connlist_dead = list_create();
				list_append_data(connlist_dead, c);
			}
			else if (state != conn_state_destroy && c->state == conn_state_destroy) {
				if (list_remove_data(connlist_dead, c, &curr)) {
					eventlog(eventlog_level_error, __FUNCTION__, "could not remove dead connection");
					return -1;
				}
			}
			c->state = state;
			return 0;
		}

		extern t_conn_class d2cs_conn_get_class(t_connection const * c)
		{
			ASSERT(c, conn_class_none);
			return c->cclass;
		}

		extern int d2cs_conn_set_class(t_connection * c, t_conn_class cclass)
		{
			ASSERT(c, -1);
			c->cclass = cclass;
			return 0;
		}

		extern t_packet * d2cs_conn_get_in_queue(t_connection const * c)
		{
			ASSERT(c, NULL);
			return c->inqueue;
		}

		extern void d2cs_conn_set_in_queue(t_connection * c, t_packet *packet)
		{
			assert(c);
			c->inqueue = packet;
		}

		extern unsigned int d2cs_conn_get_out_size(t_connection const * c)
		{
			ASSERT(c, 0);
			return c->outsize;
		}

		extern t_queue * * d2cs_conn_get_out_queue(t_connection const * c)
		{
			ASSERT(c, NULL);
			return (t_queue * *)&c->outqueue;
		}

		extern unsigned int d2cs_conn_get_in_size(t_connection const * c)
		{
			ASSERT(c, 0);
			return c->insize;
		}

		extern int conn_push_outqueue(t_connection * c, t_packet * packet)
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

			queue_push_packet((t_queue * *)&c->outqueue, packet);
			if (!c->outsizep++) fdwatch_update_fd(c->fdw_idx, fdwatch_type_read | fdwatch_type_write);

			return 0;
		}

		extern t_packet * conn_peek_outqueue(t_connection * c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			return queue_peek_packet((t_queue const * const *)&c->outqueue);
		}

		extern t_packet * conn_pull_outqueue(t_connection * c)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return NULL;
			}

			if (c->outsizep) {
				if (!(--c->outsizep)) fdwatch_update_fd(c->fdw_idx, fdwatch_type_read);
				return queue_pull_packet((t_queue * *)&c->outqueue);
			}

			return NULL;
		}

		extern int conn_add_socket_flag(t_connection * c, unsigned int flag)
		{
			ASSERT(c, -1);
			c->socket_flag |= flag;
			return 0;
		}

		extern int conn_process_packet(t_connection * c, t_packet * packet, t_packet_handle_table * table,
			unsigned int table_size)
		{
			unsigned int     type;
			unsigned int     size;

			ASSERT(c, -1);
			ASSERT(packet, -1);
			ASSERT(table, -1);
			type = packet_get_type(packet);
			size = packet_get_size(packet);

			if (type >= table_size || !table[type].size) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad packet type %d (class %d)", type, packet_get_class(packet));
				return -1;
			}
			if (size < table[type].size) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad packet size %d (type %d class %d)", size, type, packet_get_class(packet));
				return -1;
			}
			if (!(c->state & table[type].state)) {
				eventlog(eventlog_level_error, __FUNCTION__, "connection %d state mismatch for packet type %d (class %d)", c->sessionnum,
					type, packet_get_class(packet));
				return -1;
			}
			if (!table[type].handler) {
				eventlog(eventlog_level_error, __FUNCTION__, "missing handler for packet type %d (class %d)", type, packet_get_class(packet));
				return -1;
			}
			return table[type].handler(c, packet);
		}

		extern int d2cs_conn_set_account(t_connection * c, char const * account)
		{
			ASSERT(c, -1);
			if (!account) {
				if (c->account) xfree((void *)c->account);
				c->account = NULL;
			}
			if (c->account) xfree((void *)c->account);
			c->account = xstrdup(account);

			return 0;
		}

		extern char const * d2cs_conn_get_account(t_connection const * c)
		{
			ASSERT(c, NULL);
			return c->account;
		}

		extern int d2cs_conn_set_charname(t_connection * c, char const * charname)
		{
			char const	* temp;

			ASSERT(c, -1);
			temp = NULL;
			if (charname) temp = xstrdup(charname);
			if (c->charname) {
				if (hashtable_remove_data(conn_charname_list_head, c, c->charname_hash) < 0) {
					eventlog(eventlog_level_error, __FUNCTION__, "error remove charname %s from list", charname);
					if (temp) xfree((void *)temp);
					return -1;
				}
				hashtable_purge(conn_charname_list_head);
				xfree((void *)c->charname);
			}
			if (charname) {
				c->charname = temp;
				c->charname_hash = conn_charname_hash(charname);
				if (hashtable_insert_data(conn_charname_list_head, c, c->charname_hash) < 0) {
					eventlog(eventlog_level_error, __FUNCTION__, "error insert charname %s to list", charname);
					xfree((void *)c->charname);
					c->charname = NULL;
					return -1;
				}
			}
			else {
				c->charname = NULL;
				c->charname_hash = 0;
			}
			return 0;
		}

		extern char const * d2cs_conn_get_charname(t_connection const * c)
		{
			ASSERT(c, NULL);
			return c->charname;
		}

		extern unsigned int d2cs_conn_get_sessionnum(t_connection const * c)
		{
			ASSERT(c, 0);
			return c->sessionnum;
		}

		extern unsigned int conn_get_charinfo_ladder(t_connection const * c)
		{
			ASSERT(c, 0);
			return d2charinfo_get_ladder(c->charinfo);
		}

		extern unsigned int conn_get_charinfo_expansion(t_connection const * c)
		{
			ASSERT(c, 0);
			return d2charinfo_get_expansion(c->charinfo);
		}

		extern unsigned int conn_get_charinfo_hardcore(t_connection const * c)
		{
			ASSERT(c, 0);
			return d2charinfo_get_hardcore(c->charinfo);
		}

		extern unsigned int conn_get_charinfo_dead(t_connection const * c)
		{
			ASSERT(c, 0);
			return d2charinfo_get_dead(c->charinfo);
		}

		extern unsigned int conn_get_charinfo_difficulty(t_connection const * c)
		{
			ASSERT(c, 0);
			return d2charinfo_get_difficulty(c->charinfo);
		}

		extern unsigned int conn_get_charinfo_level(t_connection const * c)
		{
			ASSERT(c, 0);
			return d2charinfo_get_level(c->charinfo);
		}

		extern unsigned int conn_get_charinfo_class(t_connection const * c)
		{
			ASSERT(c, 0);
			return d2charinfo_get_class(c->charinfo);
		}

		extern int conn_set_charinfo(t_connection * c, t_d2charinfo_summary const * charinfo)
		{
			ASSERT(c, -1);
			if (!charinfo) {
				if (c->charinfo) xfree((void *)c->charinfo);
				c->charinfo = NULL;
				return 0;
			}
			if (c->charinfo) xfree((void *)c->charinfo);
			c->charinfo = (t_d2charinfo_summary*)xmalloc(sizeof(t_d2charinfo_summary));
			std::memcpy((void*)c->charinfo, charinfo, sizeof(t_d2charinfo_summary));
			return 0;
		}

		extern int conn_set_d2gs_id(t_connection * c, unsigned int d2gs_id)
		{
			ASSERT(c, -1)
				c->d2gs_id = d2gs_id;
			return 0;
		}

		extern unsigned int conn_get_d2gs_id(t_connection const * c)
		{
			ASSERT(c, 0)
				return c->d2gs_id;
		}

		extern unsigned int d2cs_conn_get_addr(t_connection const * c)
		{
			ASSERT(c, 0);
			return c->addr;
		}

		extern unsigned short d2cs_conn_get_port(t_connection const * c)
		{
			ASSERT(c, 0);
			return c->port;
		}

		extern t_gq * conn_get_gamequeue(t_connection const * c)
		{
			ASSERT(c, NULL);
			return c->gamequeue;
		}

		extern int conn_set_gamequeue(t_connection * c, t_gq * gq)
		{
			ASSERT(c, -1);
			c->gamequeue = gq;
			return 0;
		}

		extern int conn_set_bnetd_sessionnum(t_connection * c, unsigned int sessionnum)
		{
			ASSERT(c, -1);
			c->bnetd_sessionnum = sessionnum;
			return 0;
		}

		extern unsigned int conn_get_bnetd_sessionnum(t_connection const * c)
		{
			ASSERT(c, 0);
			return c->bnetd_sessionnum;
		}

		extern int conn_add_fd(t_connection * c, t_fdwatch_type rw, fdwatch_handler handler)
		{
			assert(c);
			c->fdw_idx = fdwatch_add_fd(c->sock, rw, handler, c);
			return c->fdw_idx;
		}

	}

}
