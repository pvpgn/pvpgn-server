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
#include "d2gs.h"

#include <limits>
#include <cstdlib>
#include <ctime>
#include <cstring>

#include "compat/psock.h"
#include "common/addr.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/introtate.h"
#include "prefs.h"
#include "game.h"
#include "net.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace d2cs
	{

		static t_list		* d2gslist_head = NULL;
		static unsigned int	d2gs_id = 0;
		static unsigned int	total_d2gs = 0;

		extern t_list *	d2gslist(void)
		{
			return d2gslist_head;
		}

		extern int d2gslist_create(void)
		{
			d2gslist_head = list_create();
			return d2gslist_reload(prefs_get_d2gs_list());
		}

		extern int d2gslist_reload(char const * gslist)
		{
			t_addrlist	* gsaddrs;
			t_d2gs		* gs;

			if (!d2gslist_head) return -1;

			BEGIN_LIST_TRAVERSE_DATA(d2gslist_head, gs, t_d2gs)
			{
				BIT_CLR_FLAG(gs->flag, D2GS_FLAG_VALID);
			}
			END_LIST_TRAVERSE_DATA()

				gsaddrs = addrlist_create(gslist, INADDR_ANY, 0);
			if (gsaddrs) {
				t_elem const *acurr;
				t_addr * curr_laddr;

				LIST_TRAVERSE_CONST(gsaddrs, acurr)
				{
					curr_laddr = (t_addr*)elem_get_data(acurr);
					if (!curr_laddr) {
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL value in gslist");
						continue;
					}
					if (!(gs = d2gslist_find_gs_by_ip(addr_get_ip(curr_laddr))))
						gs = d2gs_create(addr_num_to_ip_str(addr_get_ip(curr_laddr)));
					if (gs) BIT_SET_FLAG(gs->flag, D2GS_FLAG_VALID);
				}
				addrlist_destroy(gsaddrs);
			}

			BEGIN_LIST_TRAVERSE_DATA(d2gslist_head, gs, t_d2gs)
			{
				if (!BIT_TST_FLAG(gs->flag, D2GS_FLAG_VALID)) {
					d2gs_destroy(gs, &curr_elem_);
				}
			}
			END_LIST_TRAVERSE_DATA()
				return 0;
		}

		extern int d2gslist_destroy(void)
		{
			t_d2gs	* gs;

			BEGIN_LIST_TRAVERSE_DATA_CONST(d2gslist_head, gs, t_d2gs)
			{
				d2gs_destroy(gs, (t_elem **)&curr_elem_);
			}
			END_LIST_TRAVERSE_DATA_CONST()
				d2cs_connlist_reap();

			if (list_destroy(d2gslist_head) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error destroy d2gs list");
				return -1;
			}
			d2gslist_head = NULL;
			return 0;
		}

		extern t_d2gs * d2gslist_find_gs_by_ip(unsigned int ip)
		{
			t_d2gs	* gs;

			BEGIN_LIST_TRAVERSE_DATA_CONST(d2gslist_head, gs, t_d2gs)
			{
				if (gs->ip == ip) return gs;
			}
			END_LIST_TRAVERSE_DATA_CONST()
				return NULL;
		}

		extern t_d2gs * d2gslist_find_gs(unsigned int id)
		{
			t_d2gs	* gs;

			BEGIN_LIST_TRAVERSE_DATA_CONST(d2gslist_head, gs, t_d2gs)
			{
				if (gs->id == id) return gs;
			}
			END_LIST_TRAVERSE_DATA_CONST()
				return NULL;
		}

		extern t_d2gs * d2gs_create(char const * ipaddr)
		{
			t_d2gs 	* gs;
			unsigned int	ip;

			ASSERT(ipaddr, NULL);
			if ((ip = net_inet_addr(ipaddr)) == ~0U) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad ip address %s", ipaddr);
				return NULL;
			}
			if (d2gslist_find_gs_by_ip(ntohl(ip))) {
				eventlog(eventlog_level_error, __FUNCTION__, "game server %s already in list", ipaddr);
				return NULL;
			}
			gs = (t_d2gs*)xmalloc(sizeof(t_d2gs));
			gs->ip = ntohl(ip);
			gs->id = ++d2gs_id;
			gs->active = 0;
			gs->token = 0;
			gs->state = d2gs_state_none;
			gs->gamenum = 0;
			gs->maxgame = 0;
			gs->connection = NULL;

			if (list_append_data(d2gslist_head, gs) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error add gs to list");
				xfree(gs);
				return NULL;
			}
			eventlog(eventlog_level_info, __FUNCTION__, "added game server %s (id: %d) to list", ipaddr, gs->id);
			return gs;
		}

		extern int d2gs_destroy(t_d2gs * gs, t_elem ** curr)
		{
			ASSERT(gs, -1);
			if (list_remove_data(d2gslist_head, gs, curr) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error remove gs from list");
				return -1;
			}
			if (gs->active && gs->connection) {
				d2cs_conn_set_state(gs->connection, conn_state_destroy);
				d2gs_deactive(gs, gs->connection);
			}
			eventlog(eventlog_level_info, __FUNCTION__, "removed game server %s (id: %d) from list", addr_num_to_ip_str(gs->ip), gs->id);
			xfree(gs);
			return 0;
		}

		extern t_d2gs * d2gslist_get_server_by_id(unsigned int id)
		{
			t_d2gs	* gs;

			BEGIN_LIST_TRAVERSE_DATA_CONST(d2gslist_head, gs, t_d2gs)
			{
				if (gs->id == id) return gs;
			}
			END_LIST_TRAVERSE_DATA_CONST()
				return NULL;
		}

		extern t_d2gs * d2gslist_choose_server(void)
		{
			t_d2gs			* gs;
			t_d2gs			* ogs;
			unsigned int		percent;
			unsigned int		min_percent = 100;

			ogs = NULL;
			BEGIN_LIST_TRAVERSE_DATA_CONST(d2gslist_head, gs, t_d2gs)
			{
				if (!gs->active) continue;
				if (!gs->connection) continue;
				if (gs->state != d2gs_state_authed) continue;
				if (!gs->maxgame) continue;
				if (gs->gamenum >= gs->maxgame) continue;
				percent = 100 * gs->gamenum / gs->maxgame;
				if (percent < min_percent) {
					min_percent = percent;
					ogs = gs;
				}
			}
			END_LIST_TRAVERSE_DATA_CONST()
				return ogs;
		}

		extern int d2gs_set_state(t_d2gs * gs, t_d2gs_state state)
		{
			ASSERT(gs, -1);
			gs->state = state;
			return 0;
		}

		extern t_d2gs_state d2gs_get_state(t_d2gs const * gs)
		{
			ASSERT(gs, d2gs_state_none);
			return gs->state;
		}

		extern int d2gs_add_gamenum(t_d2gs * gs, int number)
		{
			ASSERT(gs, -1);
			gs->gamenum += number;
			return 0;
		}

		extern unsigned int d2gs_get_gamenum(t_d2gs const * gs)
		{
			ASSERT(gs, 0);
			return gs->gamenum;
		}


		extern int d2gs_set_maxgame(t_d2gs * gs, unsigned int maxgame)
		{
			ASSERT(gs, -1);
			gs->maxgame = maxgame;
			return 0;
		}

		extern unsigned int d2gs_get_maxgame(t_d2gs const * gs)
		{
			ASSERT(gs, 0);
			return gs->maxgame;
		}

		extern unsigned int d2gs_get_id(t_d2gs const * gs)
		{
			ASSERT(gs, 0);
			return gs->id;
		}

		extern unsigned int d2gs_get_ip(t_d2gs const * gs)
		{
			ASSERT(gs, 0);
			return gs->ip;
		}

		extern unsigned int d2gs_get_token(t_d2gs const * gs)
		{
			return gs->token;
		}

		extern unsigned int d2gs_make_token(t_d2gs * gs)
		{
			return ((unsigned int)std::rand()) ^ ((++(gs->token)) + ((unsigned int)std::time(NULL)));
		}

		extern t_connection * d2gs_get_connection(t_d2gs const * gs)
		{
			ASSERT(gs, NULL);
			return gs->connection;
		}

		extern int d2gs_active(t_d2gs * gs, t_connection * c)
		{
			ASSERT(gs, -1);
			ASSERT(c, -1);

			if (gs->active && gs->connection) {
				eventlog(eventlog_level_warn, __FUNCTION__, "game server %d is already actived, deactive previous connection first", gs->id);
				d2gs_deactive(gs, gs->connection);
			}
			total_d2gs++;
			eventlog(eventlog_level_info, __FUNCTION__, "game server %s (id: %d) actived (%d total)", addr_num_to_addr_str(d2cs_conn_get_addr(c),
				d2cs_conn_get_port(c)), gs->id, total_d2gs);
			gs->state = d2gs_state_authed;
			gs->connection = c;
			gs->active = 1;
			gs->gamenum = 0;
			gs->maxgame = 0;
			return 0;
		}

		extern int d2gs_deactive(t_d2gs * gs, t_connection * c)
		{
			t_game * game;

			ASSERT(gs, -1);
			if (!gs->active || !gs->connection) {
				eventlog(eventlog_level_warn, __FUNCTION__, "game server %d is not actived yet", gs->id);
				return -1;
			}
			if (gs->connection != c) {
				eventlog(eventlog_level_debug, __FUNCTION__, "game server %d connection mismatch,ignore it", gs->id);
				return 0;
			}
			total_d2gs--;
			eventlog(eventlog_level_info, __FUNCTION__, "game server %s (id: %d) deactived (%d left)", addr_num_to_addr_str(d2cs_conn_get_addr(gs->connection), d2cs_conn_get_port(gs->connection)), gs->id, total_d2gs);
			gs->state = d2gs_state_none;
			gs->connection = NULL;
			gs->active = 0;
			gs->maxgame = 0;
			eventlog(eventlog_level_info, __FUNCTION__, "destroying all games on game server %d", gs->id);
			BEGIN_LIST_TRAVERSE_DATA(d2cs_gamelist(), game, t_game)
			{
				if (game_get_d2gs(game) == gs) game_destroy(game, &curr_elem_);
			}
			END_LIST_TRAVERSE_DATA()
			if (gs->gamenum != 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "game server %d deactived but still with games left", gs->id);
			}
			gs->gamenum = 0;
			return 0;
		}

		extern unsigned int d2gs_calc_checksum(t_connection * c)
		{
			unsigned int	sessionnum, checksum, port, addr;
			unsigned int	i, len, ch;
			char const	* realmname;
			char const	* password;

			ASSERT(c, 0);
			sessionnum = d2cs_conn_get_sessionnum(c);
			checksum = prefs_get_d2gs_checksum();
			port = d2cs_conn_get_port(c);
			addr = d2cs_conn_get_addr(c);
			realmname = prefs_get_realmname();
			password = prefs_get_d2gs_password();

			len = std::strlen(realmname);
			for (i = 0; i < len; i++) {
				ch = (unsigned int)(unsigned char)realmname[i];
				checksum ^= ROTL(sessionnum, i, (std::numeric_limits<unsigned int>::digits));
				checksum ^= ROTL(port, ch, (std::numeric_limits<unsigned int>::digits));
			}
			len = std::strlen(password);
			for (i = 0; i < len; i++) {
				ch = (unsigned int)(unsigned char)password[i];
				checksum ^= ROTL(sessionnum, i, (std::numeric_limits<unsigned int>::digits));
				checksum ^= ROTL(port, ch, (std::numeric_limits<unsigned int>::digits));
			}
			checksum ^= addr;
			return checksum;
		}

		extern int d2gs_keepalive(void)
		{
			t_packet	* packet;
			t_d2gs *	gs;

			if (!(packet = packet_create(packet_class_d2gs))) {
				eventlog(eventlog_level_error, __FUNCTION__, "error creating packet");
				return -1;
			}
			packet_set_size(packet, sizeof(t_d2cs_d2gs_echoreq));
			packet_set_type(packet, D2CS_D2GS_ECHOREQ);
			/* FIXME: sequence number not set */
			bn_int_set(&packet->u.d2cs_d2gs.h.seqno, 0);
			BEGIN_LIST_TRAVERSE_DATA(d2gslist_head, gs, t_d2gs)
			{
				if (gs->active && gs->connection) {
					conn_push_outqueue(gs->connection, packet);
				}
			}
			END_LIST_TRAVERSE_DATA()
				packet_del_ref(packet);
			return 0;
		}

		extern int d2gs_restart_all_gs(void)
		{
			t_packet        * packet;
			t_d2gs          * gs;

			if (!(packet = packet_create(packet_class_d2gs))) {
				eventlog(eventlog_level_error, __FUNCTION__, "error creating packet");
				return -1;
			}
			packet_set_size(packet, sizeof(t_d2cs_d2gs_control));
			packet_set_type(packet, D2CS_D2GS_CONTROL);
			/* FIXME: sequence number not set */
			bn_int_set(&packet->u.d2cs_d2gs.h.seqno, 0);
			bn_int_set(&packet->u.d2cs_d2gs_control.cmd, D2CS_D2GS_CONTROL_CMD_RESTART);
			bn_int_set(&packet->u.d2cs_d2gs_control.value, prefs_get_d2gs_restart_delay());

			BEGIN_LIST_TRAVERSE_DATA(d2gslist_head, gs, t_d2gs)
			{
				if (gs->connection) {
					conn_push_outqueue(gs->connection, packet);
				}
			}

			END_LIST_TRAVERSE_DATA()
				packet_del_ref(packet);
			return 0;
		}

	}

}
