/*
 * Copyright (C) 2004 CreepLord (creeplord@pvpgn.org)
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
#include "anongame.h"

#include <cstring>
#include <cstdlib>

#include "compat/strdup.h"
#include "common/packet.h"
#include "common/eventlog.h"
#include "common/tag.h"
#include "common/queue.h"
#include "common/bn_type.h"
#include "common/list.h"
#include "common/addr.h"
#include "common/xalloc.h"
#include "common/trans.h"

#include "team.h"
#include "account.h"
#include "account_wrap.h"
#include "connection.h"
#include "prefs.h"
#include "versioncheck.h"
#include "tournament.h"
#include "timer.h"
#include "ladder.h"
#include "server.h"
#include "anongame_maplists.h"
#include "anongame_gameresult.h"
#include "common/setup_after.h"

#define MAX_LEVEL 100

namespace pvpgn
{

	namespace bnetd
	{

		/* [quetzal] 20020827 - this one get modified by anongame_queue player when there're enough
		 * players and map has been chosen based on their preferences. otherwise its NULL
		 */
		static const char *mapname = NULL;

		static int players[ANONGAME_TYPES] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		static t_connection *player[ANONGAME_TYPES][ANONGAME_MAX_GAMECOUNT];

		/* [quetzal] 20020815 - queue to hold matching players */
		static t_list *matchlists[ANONGAME_TYPES][MAX_LEVEL];

		long average_anongame_search_time = 30;
		unsigned int anongame_search_count = 0;

		/**********************************************************************************/
		static t_connection *_connlist_find_connection_by_uid(int uid);
		static char const *_conn_get_versiontag(t_connection * c);

		static int _anongame_gametype_to_queue(int type, int gametype);
		static int _anongame_level_by_queue(t_connection * c, int queue);
		static const char * _get_map_from_prefs(int queue, t_uint32 cur_prefs, t_clienttag clienttag);
		static unsigned int _anongame_get_gametype_tab(int queue);

		static int _anongame_totalplayers(int queue);
		static int _anongame_totalteams(int queue);

		static int _handle_anongame_search(t_connection * c, t_packet const *packet);
		static int _anongame_queue(t_connection * c, int queue, t_uint32 map_prefs);
		static int _anongame_compare_level(void const *a, void const *b);
		static int _anongame_order_queue(int queue);
		static int _anongame_match(t_connection * c, int queue);
		static int _anongame_search_found(int queue);
		/**********************************************************************************/

		static t_connection *_connlist_find_connection_by_uid(int uid)
		{
			return connlist_find_connection_by_account(accountlist_find_account_by_uid(uid));
		}

		static char const *_conn_get_versiontag(t_connection * c)
		{
			return versioncheck_get_versiontag(conn_get_versioncheck(c));
		}

		/**********/

		static char const *_anongame_queue_to_string(int queue)
		{
			switch (queue) {
			case ANONGAME_TYPE_1V1:
				return "PG 1v1";
			case ANONGAME_TYPE_2V2:
				return "PG 2v2";
			case ANONGAME_TYPE_3V3:
				return "PG 3v3";
			case ANONGAME_TYPE_4V4:
				return "PG 4v4";
			case ANONGAME_TYPE_SMALL_FFA:
				return "PG SFFA";
			case ANONGAME_TYPE_AT_2V2:
				return "AT 2v2";
			case ANONGAME_TYPE_TEAM_FFA:
				return "AT TFFA";
			case ANONGAME_TYPE_AT_3V3:
				return "AT 3v3";
			case ANONGAME_TYPE_AT_4V4:
				return "AT 4v4";
			case ANONGAME_TYPE_TY:
				return "TOURNEY";
			case ANONGAME_TYPE_5V5:
				return "PG 5v5";
			case ANONGAME_TYPE_6V6:
				return "PG 6v6";
			case ANONGAME_TYPE_2V2V2:
				return "PG 2v2v2";
			case ANONGAME_TYPE_3V3V3:
				return "PG 3v3v3";
			case ANONGAME_TYPE_4V4V4:
				return "PG 4v4v4";
			case ANONGAME_TYPE_2V2V2V2:
				return "PG 2v2v2v2";
			case ANONGAME_TYPE_3V3V3V3:
				return "PG 3v3v3v3";
			case ANONGAME_TYPE_AT_2V2V2:
				return "AT 2v2v2";
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "invalid queue number %d", queue);
				return "error";
			}
		}

		static int _anongame_gametype_to_queue(int type, int gametype)
		{
			switch (type) {
			case 0:		/* PG */
				switch (gametype) {
				case 0:
					return ANONGAME_TYPE_1V1;
				case 1:
					return ANONGAME_TYPE_2V2;
				case 2:
					return ANONGAME_TYPE_3V3;
				case 3:
					return ANONGAME_TYPE_4V4;
				case 4:
					return ANONGAME_TYPE_SMALL_FFA;
				case 5:
					return ANONGAME_TYPE_5V5;
				case 6:
					return ANONGAME_TYPE_6V6;
				case 7:
					return ANONGAME_TYPE_2V2V2;
				case 8:
					return ANONGAME_TYPE_3V3V3;
				case 9:
					return ANONGAME_TYPE_4V4V4;
				case 10:
					return ANONGAME_TYPE_2V2V2V2;
				case 11:
					return ANONGAME_TYPE_3V3V3V3;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "invalid PG game type: %d", gametype);
					return -1;
				}
			case 1:		/* AT */
				switch (gametype) {
				case 0:
					return ANONGAME_TYPE_AT_2V2;
				case 2:
					return ANONGAME_TYPE_AT_3V3;
				case 3:
					return ANONGAME_TYPE_AT_4V4;
				case 4:
					return ANONGAME_TYPE_AT_2V2V2;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "invalid AT game type: %d", gametype);
					return -1;
				}
			case 2:		/* TY */
				return ANONGAME_TYPE_TY;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "invalid type: %d", type);
				return -1;
			}
		}

		static int _anongame_level_by_queue(t_connection * c, int queue)
		{
			t_clienttag ct = conn_get_clienttag(c);

			switch (queue) {
			case ANONGAME_TYPE_1V1:
				return account_get_ladder_level(conn_get_account(c), ct, ladder_id_solo);
			case ANONGAME_TYPE_2V2:
			case ANONGAME_TYPE_3V3:
			case ANONGAME_TYPE_4V4:
			case ANONGAME_TYPE_5V5:
			case ANONGAME_TYPE_6V6:
			case ANONGAME_TYPE_2V2V2:
			case ANONGAME_TYPE_3V3V3:
			case ANONGAME_TYPE_4V4V4:
			case ANONGAME_TYPE_2V2V2V2:
			case ANONGAME_TYPE_3V3V3V3:
				return account_get_ladder_level(conn_get_account(c), ct, ladder_id_ffa);
			case ANONGAME_TYPE_SMALL_FFA:
			case ANONGAME_TYPE_TEAM_FFA:
				return account_get_ladder_level(conn_get_account(c), ct, ladder_id_ffa);
			case ANONGAME_TYPE_AT_2V2:
			case ANONGAME_TYPE_AT_3V3:
			case ANONGAME_TYPE_AT_4V4:
			case ANONGAME_TYPE_AT_2V2V2:
				return 0;
			case ANONGAME_TYPE_TY:	/* set to ((wins * 3) + ties - losses) ie. prelim score */
				return tournament_get_player_score(conn_get_account(c));
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "unknown queue: %d", queue);
				return -1;
			}
		}

		static const char * _get_map_from_prefs(int queue, t_uint32 cur_prefs, t_clienttag clienttag)
		{
			int i, j = 0;
			const char *default_map, *selected;
			char *res_maps[32];
			char clienttag_str[5];

			switch (clienttag) {
			case CLIENTTAG_WARCRAFT3_UINT:
				default_map = "Maps\\(8)PlainsOfSnow.w3m";
				break;
			case CLIENTTAG_WAR3XP_UINT:
				default_map = "Maps\\(8)PlainsOfSnow.w3m";
				break;
			default:
				ERROR1("invalid clienttag: %s", tag_uint_to_str(clienttag_str, clienttag));
				return "Maps\\(8)PlainsOfSnow.w3m";
			}

			for (i = 0; i < 32; i++)
				res_maps[i] = NULL;

			for (i = 0; i < 32; i++) {
				if (cur_prefs & 1)
					res_maps[j++] = maplists_get_map(queue, clienttag, i + 1);
				cur_prefs >>= 1;
			}

			i = std::rand() % j;
			if (res_maps[i])
				selected = res_maps[i];
			else
				selected = default_map;

			eventlog(eventlog_level_trace, __FUNCTION__, "got map %s from prefs", selected);
			return selected;
		}

		static unsigned int _anongame_get_gametype_tab(int queue)
		{
			/* dizzy: this changed in 1.05 */
			switch (queue) {
			case ANONGAME_TYPE_1V1:
				return SERVER_ANONGAME_SOLO_STR;
			case ANONGAME_TYPE_2V2:
			case ANONGAME_TYPE_3V3:
			case ANONGAME_TYPE_4V4:
			case ANONGAME_TYPE_5V5:
			case ANONGAME_TYPE_6V6:
			case ANONGAME_TYPE_2V2V2:
			case ANONGAME_TYPE_3V3V3:
			case ANONGAME_TYPE_4V4V4:
			case ANONGAME_TYPE_2V2V2V2:
			case ANONGAME_TYPE_3V3V3V3:
				return SERVER_ANONGAME_TEAM_STR;
			case ANONGAME_TYPE_SMALL_FFA:
				return SERVER_ANONGAME_SFFA_STR;
			case ANONGAME_TYPE_TEAM_FFA:
				return 0;		/* Team FFA is no longer supported */
			case ANONGAME_TYPE_AT_2V2:
				return SERVER_ANONGAME_AT2v2_STR;
			case ANONGAME_TYPE_AT_3V3:
				return SERVER_ANONGAME_AT3v3_STR;
			case ANONGAME_TYPE_AT_4V4:
				return SERVER_ANONGAME_AT4v4_STR;
			case ANONGAME_TYPE_AT_2V2V2:
				return SERVER_ANONGAME_AT2v2_STR;	/* fixme */
			case ANONGAME_TYPE_TY:
				return SERVER_ANONGAME_TY_STR;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "invalid queue (%d)", queue);
				return 0;
			}
		}

		static int _anongame_totalplayers(int queue)
		{
			switch (queue) {
			case ANONGAME_TYPE_1V1:
				return 2;
			case ANONGAME_TYPE_2V2:
			case ANONGAME_TYPE_AT_2V2:
			case ANONGAME_TYPE_SMALL_FFA:	/* fixme: total players not always 4 */
				return 4;
			case ANONGAME_TYPE_3V3:
			case ANONGAME_TYPE_AT_3V3:
			case ANONGAME_TYPE_2V2V2:
			case ANONGAME_TYPE_AT_2V2V2:
				return 6;
			case ANONGAME_TYPE_4V4:
			case ANONGAME_TYPE_AT_4V4:
			case ANONGAME_TYPE_TEAM_FFA:
			case ANONGAME_TYPE_2V2V2V2:
				return 8;
			case ANONGAME_TYPE_3V3V3:
				return 9;
			case ANONGAME_TYPE_5V5:
				return 10;
			case ANONGAME_TYPE_6V6:
			case ANONGAME_TYPE_4V4V4:
			case ANONGAME_TYPE_3V3V3V3:
				return 12;
			case ANONGAME_TYPE_TY:
				return tournament_get_totalplayers();
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "unknown queue: %d", queue);
				return 0;
			}
		}

		static int _anongame_totalteams(int queue)
		{
			/* dont forget to change this if you make some game type with more teams */
#define ANONGAME_MAX_TEAMS	4
			switch (queue) {
			case ANONGAME_TYPE_1V1:
			case ANONGAME_TYPE_SMALL_FFA:
				return 0;
			case ANONGAME_TYPE_2V2:
			case ANONGAME_TYPE_3V3:
			case ANONGAME_TYPE_4V4:
			case ANONGAME_TYPE_5V5:
			case ANONGAME_TYPE_6V6:
			case ANONGAME_TYPE_AT_2V2:
			case ANONGAME_TYPE_AT_3V3:
			case ANONGAME_TYPE_AT_4V4:
				return 2;
			case ANONGAME_TYPE_2V2V2:
			case ANONGAME_TYPE_3V3V3:
			case ANONGAME_TYPE_4V4V4:
			case ANONGAME_TYPE_AT_2V2V2:
				return 3;
			case ANONGAME_TYPE_TEAM_FFA:	/* not even used */
			case ANONGAME_TYPE_2V2V2V2:
			case ANONGAME_TYPE_3V3V3V3:
				return 4;
			case ANONGAME_TYPE_TY:
				return 2;		/* fixme: does not support 2v2v2 - tournament_get_totalteams() */
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "unknown queue: %d", queue);
				return 0;
			}
		}

		/**********/
		static int _handle_anongame_search(t_connection * c, t_packet const *packet)
		{
			int i, j, temp, set = 1;
			t_packet *rpacket;
			t_connection *tc[6];
			t_anongame *a, *ta;
			t_uint8 teamsize = 0;
			t_uint8 option = bn_byte_get(packet->u.client_findanongame.option);

			if (!(a = conn_get_anongame(c))) {
				if (!(a = conn_create_anongame(c))) {
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] conn_create_anongame failed", conn_get_socket(c));
					return -1;
				}
			}

			conn_set_anongame_search_starttime(c, now);

			switch (option) {
			case CLIENT_FINDANONGAME_AT_INVITER_SEARCH:
				a->count = bn_int_get(packet->u.client_findanongame_at_inv.count);
				a->id = bn_int_get(packet->u.client_findanongame_at_inv.id);
				a->tid = bn_int_get(packet->u.client_findanongame_at_inv.tid);
				a->race = bn_int_get(packet->u.client_findanongame_at_inv.race);
				a->map_prefs = bn_int_get(packet->u.client_findanongame_at_inv.map_prefs);
				a->type = bn_byte_get(packet->u.client_findanongame_at_inv.type);
				a->gametype = bn_byte_get(packet->u.client_findanongame_at_inv.gametype);
				teamsize = bn_byte_get(packet->u.client_findanongame_at_inv.teamsize);
				break;
			case CLIENT_FINDANONGAME_AT_SEARCH:
				a->count = bn_int_get(packet->u.client_findanongame_at.count);
				a->id = bn_int_get(packet->u.client_findanongame_at.id);
				a->tid = bn_int_get(packet->u.client_findanongame_at.tid);
				a->race = bn_int_get(packet->u.client_findanongame_at.race);
				teamsize = bn_byte_get(packet->u.client_findanongame_at.teamsize);
				break;
			case CLIENT_FINDANONGAME_SEARCH:
				a->count = bn_int_get(packet->u.client_findanongame.count);
				a->id = bn_int_get(packet->u.client_findanongame.id);
				a->race = bn_int_get(packet->u.client_findanongame.race);
				a->map_prefs = bn_int_get(packet->u.client_findanongame.map_prefs);
				a->type = bn_byte_get(packet->u.client_findanongame.type);
				a->gametype = bn_byte_get(packet->u.client_findanongame.gametype);
				break;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "invalid search option (%d)", option);
				return -1;
			}

			if (option != CLIENT_FINDANONGAME_AT_SEARCH)
			if ((a->queue = _anongame_gametype_to_queue(a->type, a->gametype)) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "invalid queue: %d", a->queue);
				return -1;
			}

			account_set_w3pgrace(conn_get_account(c), conn_get_clienttag(c), a->race);

			/* send search reply to client */
			if (!(rpacket = packet_create(packet_class_bnet)))
				return -1;
			packet_set_size(rpacket, sizeof(t_server_anongame_search_reply));
			packet_set_type(rpacket, SERVER_ANONGAME_SEARCH_REPLY);
			bn_byte_set(&rpacket->u.server_anongame_search_reply.option, SERVER_FINDANONGAME_SEARCH);
			bn_int_set(&rpacket->u.server_anongame_search_reply.count, a->count);
			bn_int_set(&rpacket->u.server_anongame_search_reply.reply, 0);
			temp = (int)average_anongame_search_time;
			packet_append_data(rpacket, &temp, 2);
			conn_push_outqueue(c, rpacket);
			packet_del_ref(rpacket);
			/* end search reply */

			switch (option) {
			case CLIENT_FINDANONGAME_AT_INVITER_SEARCH:
				for (i = 0; i < teamsize; i++) {	/* assign player conns to tc[] array */
					if (!(tc[i] = _connlist_find_connection_by_uid(bn_int_get(packet->u.client_findanongame_at_inv.info[i])))) {
						eventlog(eventlog_level_error, __FUNCTION__, "[%d] got NULL connection", conn_get_socket(tc[i]));
						return -1;
					}
				}
				for (i = 0; i < teamsize; i++) {	/* assign info from inviter to other team players */
					if (!(ta = conn_get_anongame(tc[i]))) {
						if (!(ta = conn_create_anongame(tc[i]))) {
							eventlog(eventlog_level_error, __FUNCTION__, "[%d] conn_create_anongame failed", conn_get_socket(tc[i]));
							return -1;
						}
					}
					for (j = 0; j < teamsize; j++)	/* add each players conn to each anongame struct */
						ta->tc[j] = tc[j];

					ta->type = a->type;
					ta->gametype = a->gametype;
					ta->queue = a->queue;
					ta->map_prefs = a->map_prefs;

					if (ta->tid != a->tid)
						set = 0;
				}
				if (!set)		/* check if search packet has been recieved from each team member */
					return 0;
				break;
			case CLIENT_FINDANONGAME_AT_SEARCH:
				for (i = 0; i < teamsize; i++) {	/* assign player conns to tc[] array */
					if (!(tc[i] = _connlist_find_connection_by_uid(bn_int_get(packet->u.client_findanongame_at.info[i])))) {
						eventlog(eventlog_level_error, __FUNCTION__, "[%d] got NULL connection", conn_get_socket(tc[i]));
						return -1;
					}
				}
				for (i = 0; i < teamsize; i++) {	/* check if search packet has been recieved from each team member */
					if (!(ta = conn_get_anongame(tc[i])))
						return 0;
					if (ta->tid != a->tid)
						return 0;
				}
				break;
			case CLIENT_FINDANONGAME_SEARCH:
				tc[0] = c;
				a->tc[0] = c;
				break;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "invalid search option (%d)", option);
				return -1;
			}

			if (_anongame_queue(tc[0], a->queue, a->map_prefs) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "queue failed");
				return -1;
			}

			_anongame_match(c, a->queue);

			/* if enough players are queued send found packet */
			if (players[a->queue] == _anongame_totalplayers(a->queue))
			if (_anongame_search_found(a->queue) < 0)
				return -1;

			return 0;
		}

		static int _anongame_queue(t_connection * c, int queue, t_uint32 map_prefs)
		{
			int level;
			t_matchdata *md;

			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
			}

			if (queue >= ANONGAME_TYPES) {
				eventlog(eventlog_level_error, __FUNCTION__, "unknown queue: %d", queue);
				return -1;
			}

			level = _anongame_level_by_queue(c, queue);

			if (!matchlists[queue][level])
				matchlists[queue][level] = list_create();

			md = (t_matchdata*)xmalloc(sizeof(t_matchdata));
			md->c = c;
			md->map_prefs = map_prefs;
			md->versiontag = _conn_get_versiontag(c);

			list_append_data(matchlists[queue][level], md);

			return 0;
		}

		static int _anongame_compare_level(void const *a, void const *b)
		{
			t_connection *ca = *(t_connection * const *)a;
			t_connection *cb = *(t_connection * const *)b;

			int level_a = _anongame_level_by_queue(ca, anongame_get_queue(conn_get_anongame(ca)));
			int level_b = _anongame_level_by_queue(cb, anongame_get_queue(conn_get_anongame(cb)));

			return (level_a > level_b) ? -1 : ((level_a < level_b) ? 1 : 0);
		}

		static int _anongame_order_queue(int queue)
		{
			if (_anongame_totalteams(queue) != 0 && !anongame_arranged(queue)) {	/* no need to reorder 1v1, sffa, or AT queues */
				int i, j;
				t_connection *temp;
				int level[ANONGAME_MAX_TEAMS];
				int teams = _anongame_totalteams(queue);	/* number of teams */
				int ppt = players[queue] / teams;	/* players per team */

				for (i = 0; i < ANONGAME_MAX_TEAMS; i++)
					level[i] = 0;

				for (i = 0; i < ppt - 1; i++) {	/* loop through the number of players per team */
					for (j = 0; j < teams; j++) {
						level[j] = level[j] + _anongame_level_by_queue(player[queue][i * ppt + j], queue);
					}

					if (teams == 2) {
						/* 1 >= 2 */
						if (level[i * teams] >= level[i * teams + 1]) {
							temp = player[queue][(i + 1) * teams];
							player[queue][(i + 1) * teams] = player[queue][(i + 1) * teams + 1];
							player[queue][(i + 1) * teams + 1] = temp;
						}
						/* 2 >= 1 */
						else if (level[i * teams + 1] >= level[i * teams]) {
							;		/* nothing to do */
						}
					}
					/* end 2 teams */
					else if (teams == 3) {
						/* 1 >= 2 >= 3 */
						if (level[i * 3] >= level[i * 3 + 1] && level[i * 3 + 1] >= level[i * 3 + 2]) {
							temp = player[queue][(i + 1) * 3];
							player[queue][(i + 1) * 3] = player[queue][(i + 1) * 3 + 2];
							player[queue][(i + 1) * 3 + 2] = temp;
						}
						/* 1 >= 3 >= 2 */
						else if (level[i * 3] >= level[i * 3 + 2] && level[i * 3 + 2] >= level[i * 3 + 1]) {
							temp = player[queue][(i + 1) * 3];
							player[queue][(i + 1) * 3] = player[queue][(i + 1) * 3 + 2];
							player[queue][(i + 1) * 3 + 2] = player[queue][(i + 1) * 3 + 1];
							player[queue][(i + 1) * 3 + 1] = temp;
						}
						/* 2 >= 1 >= 3 */
						else if (level[i * 3 + 1] >= level[i * 3] && level[i * 3] >= level[i * 3 + 2]) {
							temp = player[queue][(i + 1) * 3];
							player[queue][(i + 1) * 3] = player[queue][(i + 1) * 3 + 1];
							player[queue][(i + 1) * 3 + 1] = player[queue][(i + 1) * 3 + 2];
							player[queue][(i + 1) * 3 + 2] = temp;
						}
						/* 2 >= 3 >= 1 */
						else if (level[i * 3 + 1] >= level[i * 3 + 2] && level[i * 3 + 2] >= level[i * 3]) {
							temp = player[queue][(i + 1) * 3 + 1];
							player[queue][(i + 1) * 3 + 1] = player[queue][(i + 1) * 3 + 2];
							player[queue][(i + 1) * 3 + 2] = temp;
						}
						/* 3 >= 1 >= 2 */
						else if (level[i * 3 + 2] >= level[i * 3] && level[i * 3] >= level[i * 3 + 1]) {
							temp = player[queue][(i + 1) * 3];
							player[queue][(i + 1) * 3] = player[queue][(i + 1) * 3 + 1];
							player[queue][(i + 1) * 3 + 1] = temp;
						}
						/* 3 >= 2 >= 1 */
						else if (level[i * 3 + 2] >= level[i * 3 + 1] && level[i * 3 + 1] >= level[i * 3]) {
							;		/* nothing to do */
						}
					}
					/* end 3 teams */
					else if (teams == 4) {
						/* 1234 */
						if (level[i * 4] >= level[i * 4 + 1] && level[i * 4 + 1] >= level[i * 4 + 2] && level[i * 4 + 2] >= level[i * 4 + 3]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = temp;
							temp = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = temp;
						}
						/* 1243 */
						else if (level[i * 4] >= level[i * 4 + 1] && level[i * 4 + 1] >= level[i * 4 + 2] && level[i * 4 + 3] >= level[i * 4 + 2]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = temp;
						}
						/* 1324 */
						else if (level[i * 4] >= level[i * 4 + 2] && level[i * 4 + 2] >= level[i * 4 + 1] && level[i * 4 + 1] >= level[i * 4 + 3]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = temp;
						}
						/* 1342 */
						else if (level[i * 4] >= level[i * 4 + 2] && level[i * 4 + 2] >= level[i * 4 + 3] && level[i * 4 + 3] >= level[i * 4 + 1]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = temp;
						}
						/* 1423 */
						else if (level[i * 4] >= level[i * 4 + 3] && level[i * 4 + 3] >= level[i * 4 + 1] && level[i * 4 + 1] >= level[i * 4 + 2]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = temp;
						}
						/* 1432 */
						else if (level[i * 4] >= level[i * 4 + 3] && level[i * 4 + 3] >= level[i * 4 + 2] && level[i * 4 + 2] >= level[i * 4 + 1]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = temp;
						}
						/* 2134 */
						else if (level[i * 4 + 1] >= level[i * 4] && level[i * 4] >= level[i * 4 + 2] && level[i * 4 + 2] >= level[i * 4 + 3]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = temp;
						}
						/* 2143 */
						else if (level[i * 4 + 1] >= level[i * 4] && level[i * 4] >= level[i * 4 + 3] && level[i * 4 + 3] >= level[i * 4 + 2]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = temp;
							temp = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = temp;
						}
						/* 2314 */
						else if (level[i * 4 + 1] >= level[i * 4 + 2] && level[i * 4 + 2] >= level[i * 4] && level[i * 4] >= level[i * 4 + 3]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = temp;
						}
						/* 2341 */
						else if (level[i * 4 + 1] >= level[i * 4 + 2] && level[i * 4 + 2] >= level[i * 4 + 3] && level[i * 4 + 3] >= level[i * 4]) {
							temp = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = temp;
						}
						/* 2413 */
						else if (level[i * 4 + 1] >= level[i * 4 + 3] && level[i * 4 + 3] >= level[i * 4] && level[i * 4] >= level[i * 4 + 2]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = temp;
						}
						/* 2431 */
						else if (level[i * 4 + 1] >= level[i * 4 + 3] && level[i * 4 + 3] >= level[i * 4 + 2] && level[i * 4 + 2] >= level[i * 4]) {
							temp = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = temp;
						}
						/* 3124 */
						else if (level[i * 4 + 2] >= level[i * 4] && level[i * 4] >= level[i * 4 + 1] && level[i * 4 + 1] >= level[i * 4 + 3]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = temp;
						}
						/* 3142 */
						else if (level[i * 4 + 2] >= level[i * 4] && level[i * 4] >= level[i * 4 + 3] && level[i * 4 + 3] >= level[i * 4 + 1]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = temp;
						}
						/* 3214 */
						else if (level[i * 4 + 2] >= level[i * 4 + 1] && level[i * 4 + 1] >= level[i * 4] && level[i * 4] >= level[i * 4 + 3]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = temp;
						}
						/* 3241 */
						else if (level[i * 4 + 2] >= level[i * 4 + 1] && level[i * 4 + 1] >= level[i * 4 + 3] && level[i * 4 + 3] >= level[i * 4]) {
							temp = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = temp;
						}
						/* 3412 */
						else if (level[i * 4 + 2] >= level[i * 4 + 3] && level[i * 4 + 3] >= level[i * 4] && level[i * 4] >= level[i * 4 + 1]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = temp;
							temp = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = temp;
						}
						/* 3421 */
						else if (level[i * 4 + 2] >= level[i * 4 + 3] && level[i * 4 + 3] >= level[i * 4 + 1] && level[i * 4 + 1] >= level[i * 4]) {
							temp = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = player[queue][(i + 1) * 4 + 3];
							player[queue][(i + 1) * 4 + 3] = temp;
						}
						/* 4123 */
						else if (level[i * 4 + 3] >= level[i * 4] && level[i * 4] >= level[i * 4 + 1] && level[i * 4 + 1] >= level[i * 4 + 2]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = temp;
						}
						/* 4132 */
						else if (level[i * 4 + 3] >= level[i * 4] && level[i * 4] >= level[i * 4 + 2] && level[i * 4 + 2] >= level[i * 4 + 1]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = temp;
						}
						/* 4213 */
						else if (level[i * 4 + 3] >= level[i * 4 + 1] && level[i * 4 + 1] >= level[i * 4] && level[i * 4] >= level[i * 4 + 2]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = temp;
						}
						/* 4231 */
						else if (level[i * 4 + 3] >= level[i * 4 + 1] && level[i * 4 + 1] >= level[i * 4 + 2] && level[i * 4 + 2] >= level[i * 4]) {
							temp = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = player[queue][(i + 1) * 4 + 2];
							player[queue][(i + 1) * 4 + 2] = temp;
						}
						/* 4312 */
						else if (level[i * 4 + 3] >= level[i * 4 + 2] && level[i * 4 + 2] >= level[i * 4] && level[i * 4] >= level[i * 4 + 1]) {
							temp = player[queue][(i + 1) * 4];
							player[queue][(i + 1) * 4] = player[queue][(i + 1) * 4 + 1];
							player[queue][(i + 1) * 4 + 1] = temp;
						}
						/* 4321 */
						else if (level[i * 4 + 3] >= level[i * 4 + 2] && level[i * 4 + 2] >= level[i * 4 + 1] && level[i * 4 + 1] >= level[i * 4]) {
							;		/* nothing to do */
						}
					}			/* end 4 teams */
				}			/* end ppt loop */
			}				/* end "if" statement */
			return 0;
		}

		static int _anongame_match(t_connection * c, int queue)
		{
			int level = _anongame_level_by_queue(c, queue);
			int delta = 0;
			int i;
			t_matchdata *md;
			t_elem *curr;
			int diff;
			t_anongame *a = conn_get_anongame(c);
			t_uint32 cur_prefs = a->map_prefs;
			t_connection *inv_c[ANONGAME_MAX_TEAMS];
			int maxlevel, minlevel;
			int teams = 0;
			players[queue] = 0;

			eventlog(eventlog_level_trace, __FUNCTION__, "[%d] matching started for level %d player in queue %d", conn_get_socket(c), level, queue);

			diff = war3_get_maxleveldiff();
			maxlevel = level + diff;
			minlevel = (level - diff < 0) ? 0 : level - diff;

			while (abs(delta) < (diff + 1)) {
				if ((level + delta <= maxlevel) && (level + delta >= minlevel)) {
					eventlog(eventlog_level_trace, __FUNCTION__, "Traversing level %d players", level + delta);

					LIST_TRAVERSE(matchlists[queue][level + delta], curr) {
						md = (t_matchdata*)elem_get_data(curr);
						if (md->versiontag && _conn_get_versiontag(c) && !std::strcmp(md->versiontag, _conn_get_versiontag(c)) && (cur_prefs & md->map_prefs)) {
							/* set maxlevel and minlevel to keep all players within 6 levels */
							maxlevel = (level + delta + diff < maxlevel) ? level + delta + diff : maxlevel;
							minlevel = (level + delta - diff > minlevel) ? level + delta - diff : minlevel;
							cur_prefs &= md->map_prefs;

							/* AT match */
							if (anongame_arranged(queue)) {

								/* set the inv_c for unqueueing later */
								inv_c[teams] = md->c;

								a = conn_get_anongame(md->c);

								/* add all the players on the team to player[][] */
								for (i = 0; i < _anongame_totalplayers(queue) / _anongame_totalteams(queue); i++) {
									player[queue][teams + i * _anongame_totalteams(queue)] = a->tc[i];
									players[queue]++;
								}
								teams++;

								/* check for enough players */
								if (players[queue] == _anongame_totalplayers(queue)) {

									/* unqueue just the single team entry */
									for (i = 0; i < teams; i++)
										anongame_unqueue(inv_c[i], queue);

									mapname = _get_map_from_prefs(queue, cur_prefs, conn_get_clienttag(c));
									return 0;
								}

								/* PG match */
							}
							else {
								player[queue][players[queue]++] = md->c;

								if (players[queue] == _anongame_totalplayers(queue)) {
									/* first sort queue by level */
									std::qsort(player[queue], players[queue], sizeof(t_connection *), _anongame_compare_level);
									/* next call reodering function */
									_anongame_order_queue(queue);
									/* unqueue players */
									for (i = 0; i < players[queue]; i++)
										anongame_unqueue(player[queue][i], queue);

									mapname = _get_map_from_prefs(queue, cur_prefs, conn_get_clienttag(c));
									return 0;
								}
							}
						}
					}
				}

				if (delta <= 0 || level - delta < 0)
					delta = abs(delta) + 1;
				else
					delta = -delta;

				if (level + delta > MAX_LEVEL)
					delta = -delta;

				if (level + delta < 0)
					break;		/* cant really happen */

			}
			eventlog(eventlog_level_trace, __FUNCTION__, "[%d] Matching finished, not enough players (found %d)", conn_get_socket(c), players[queue]);
			mapname = NULL;
			return 0;
		}

		static int w3routeip = -1;	/* changed by dizzy to show the w3routeshow addr if available */
		static unsigned short w3routeport = BNETD_W3ROUTE_PORT;

		static int _anongame_search_found(int queue)
		{
			t_packet *rpacket;
			t_anongameinfo *info;
			t_anongame *a;
			int i, j;
			t_saf_pt2 *pt2;

			/* FIXME: maybe periodically lookup w3routeaddr to support dynamic ips?
			 * (or should dns lookup be even quick enough to do it everytime?)
			 */

			if (w3routeip == -1) {
				t_addr *routeraddr;

				routeraddr = addr_create_str(prefs_get_w3route_addr(), 0, BNETD_W3ROUTE_PORT);

				if (!routeraddr) {
					eventlog(eventlog_level_error, __FUNCTION__, "error getting w3route_addr");
					return -1;
				}

				w3routeip = addr_get_ip(routeraddr);
				w3routeport = addr_get_port(routeraddr);
				addr_destroy(routeraddr);
			}

			info = anongameinfo_create(_anongame_totalplayers(queue));

			if (!info) {
				eventlog(eventlog_level_error, __FUNCTION__, "anongameinfo_create failed");
				return -1;
			}

			/* create data to be appended to end of packet */
			pt2 = (t_saf_pt2*)xmalloc(sizeof(t_saf_pt2));
			bn_int_set(&pt2->unknown1, 0xFFFFFFFF);
			bn_int_set(&pt2->anongame_string, _anongame_get_gametype_tab(queue));
			bn_byte_set(&pt2->totalplayers, _anongame_totalplayers(queue));
			bn_byte_set(&pt2->totalteams, _anongame_totalteams(queue));	/* 1v1 & sffa are set to zero in _anongame_totalteams() */
			bn_short_set(&pt2->unknown2, 0);
			bn_byte_set(&pt2->visibility, 2);	/* visibility. 0x01 - dark 0x02 - default */
			bn_byte_set(&pt2->unknown3, 2);

			/* send found packet to each of the players */
			for (i = 0; i < players[queue]; i++) {
				if (!(a = conn_get_anongame(player[queue][i]))) {
					eventlog(eventlog_level_error, __FUNCTION__, "no anongame struct for queued player");
					xfree(pt2);
					return -1;
				}

				a->info = info;
				a->playernum = i + 1;

				for (j = 0; j < players[queue]; j++) {
					a->info->player[j] = player[queue][j];
					a->info->account[j] = conn_get_account(player[queue][j]);
				}

				if (!(rpacket = packet_create(packet_class_bnet))) {
					xfree(pt2);
					return -1;
				}

				packet_set_size(rpacket, sizeof(t_server_anongame_found));
				packet_set_type(rpacket, SERVER_ANONGAME_FOUND);
				bn_byte_set(&rpacket->u.server_anongame_found.option, 1);
				bn_int_set(&rpacket->u.server_anongame_found.count, a->count);
				bn_int_set(&rpacket->u.server_anongame_found.unknown1, 0);
				{			/* trans support */
					unsigned int w3ip = w3routeip;
					unsigned short w3port = w3routeport;

					trans_net(conn_get_addr(player[queue][i]), &w3ip, &w3port);

					/* if ip to send is 0.0.0.0 (which will not work anyway) try
					 * to guess the reachable IP of pvpgn by using the local
					 * endpoing address of the bnet class connection */
					if (!w3ip)
						w3ip = conn_get_real_local_addr(player[queue][i]);

					bn_int_nset(&rpacket->u.server_anongame_found.ip, w3ip);
					bn_short_set(&rpacket->u.server_anongame_found.port, w3port);
				}
				bn_byte_set(&rpacket->u.server_anongame_found.unknown2, i + 1);
				bn_byte_set(&rpacket->u.server_anongame_found.unknown3, queue);
				bn_short_set(&rpacket->u.server_anongame_found.unknown4, 0);
				bn_int_set(&rpacket->u.server_anongame_found.id, 0xdeadbeef);
				bn_byte_set(&rpacket->u.server_anongame_found.unknown5, 6);
				bn_byte_set(&rpacket->u.server_anongame_found.type, a->type);
				bn_byte_set(&rpacket->u.server_anongame_found.gametype, a->gametype);
				packet_append_string(rpacket, mapname);
				packet_append_data(rpacket, pt2, sizeof(t_saf_pt2));
				conn_push_outqueue(player[queue][i], rpacket);
				packet_del_ref(rpacket);
			}

			/* clear queue */
			players[queue] = 0;
			xfree(pt2);
			return 0;
		}

		/**********************************************************************************/
		/* external functions */
		/**********************************************************************************/
		extern int anongame_matchlists_create()
		{
			int i, j;

			for (i = 0; i < ANONGAME_TYPES; i++) {
				for (j = 0; j < MAX_LEVEL; j++) {
					matchlists[i][j] = NULL;
				}
			}
			return 0;
		}

		extern int anongame_matchlists_destroy()
		{
			int i, j;
			for (i = 0; i < ANONGAME_TYPES; i++) {
				for (j = 0; j < MAX_LEVEL; j++) {
					if (matchlists[i][j]) {
						list_destroy(matchlists[i][j]);
					}
				}
			}
			return 0;
		}

		/**********/
		extern int handle_anongame_search(t_connection * c, t_packet const *packet)
		{
			return _handle_anongame_search(c, packet);
		}

		extern int anongame_unqueue(t_connection * c, int queue)
		{
			int i;
			t_elem *curr;
			t_matchdata *md;

			if (queue < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got negative queue id (%d)", queue);
				return -1;
			}

			if (queue >= ANONGAME_TYPES) {
				eventlog(eventlog_level_error, __FUNCTION__, "unknown queue: %d", queue);
				return -1;
			}

			if (conn_get_anongame_search_starttime(c) != ((std::time_t) 0)) {
				average_anongame_search_time *= anongame_search_count;
				average_anongame_search_time += (long)std::difftime(std::time(NULL), conn_get_anongame_search_starttime(c));
				anongame_search_count++;
				average_anongame_search_time /= anongame_search_count;
				if (anongame_search_count > 20000)
					anongame_search_count = anongame_search_count / 2;	/* to prevent an overflow of the average time */
				conn_set_anongame_search_starttime(c, ((std::time_t) 0));
			}

			for (i = 0; i < MAX_LEVEL; i++) {
				if (matchlists[queue][i] == NULL)
					continue;

				LIST_TRAVERSE(matchlists[queue][i], curr) {
					md = (t_matchdata*)elem_get_data(curr);
					if (md->c == c) {
						eventlog(eventlog_level_trace, __FUNCTION__, "unqueued player [%d] level %d", conn_get_socket(c), i);
						list_remove_elem(matchlists[queue][i], &curr);
						xfree(md);
						return 0;
					}
				}
			}

			/* Output error to std::log for PG queues, AT players are queued with single
			 * entry. Because anongame_unqueue() is called for each player, only the first
			 * time called will the team be removed, the rest are therefore not an error.
			 * [Omega]
			 */
			if (anongame_arranged(queue) == 0) {
				eventlog(eventlog_level_trace, __FUNCTION__, "[%d] player not found in \"%s\" queue", conn_get_socket(c), _anongame_queue_to_string(queue));
				return -1;
			}

			return 0;
		}

		/**********/
		extern char anongame_arranged(int queue)
		{
			switch (queue) {
			case ANONGAME_TYPE_AT_2V2:
			case ANONGAME_TYPE_AT_3V3:
			case ANONGAME_TYPE_AT_4V4:
			case ANONGAME_TYPE_AT_2V2V2:
				return 1;
			case ANONGAME_TYPE_TY:
				return tournament_is_arranged();
			default:
				return 0;
			}
		}

		extern int anongame_evaluate_results(t_anongame * anongame)
		{
			int i, j, number;
			int wins[ANONGAME_MAX_GAMECOUNT];
			int losses[ANONGAME_MAX_GAMECOUNT];
			int result;
			t_anongame_gameresult *results;
			t_anongameinfo *anoninfo = anongame->info;

			for (i = 0; i < ANONGAME_MAX_GAMECOUNT; i++) {
				wins[i] = 0;
				losses[i] = 0;
			}

			for (i = 0; i < anongame_get_totalplayers(anongame); i++) {
				if ((results = anoninfo->results[i])) {
					for (j = 0; j < gameresult_get_number_of_results(results); j++) {
						number = gameresult_get_player_number(results, j) - 1;
						result = gameresult_get_player_result(results, j);

						if ((result == W3_GAMERESULT_WIN))
							wins[number]++;
						if ((result == W3_GAMERESULT_LOSS))
							losses[number]++;
					}
				}
			}

			for (i = 0; i < anongame_get_totalplayers(anongame); i++) {
				if ((wins[i] > losses[i])) {
					if ((anoninfo->result[i] != W3_GAMERESULT_WIN)) {
						eventlog(eventlog_level_trace, __FUNCTION__, "player %d reported DISC/LOSS for self, but others agree on WIN", i + 1);
						anoninfo->result[i] = W3_GAMERESULT_WIN;
					}
				}
				else {
					if ((anoninfo->result[i] != W3_GAMERESULT_LOSS)) {
						eventlog(eventlog_level_trace, __FUNCTION__, "player %d reported DISC/WIN for self, but others agree on LOSS", i + 1);
						anoninfo->result[i] = W3_GAMERESULT_LOSS;
					}
				}
			}

			return 0;

		}

		extern int anongame_stats(t_connection * c)
		{
			int i;
			int wins = 0, losses = 0, discs = 0;
			t_connection *gamec = conn_get_routeconn(c);
			t_anongame *a = conn_get_anongame(gamec);
			int tp = anongame_get_totalplayers(a);
			int oppon_level[ANONGAME_MAX_GAMECOUNT];
			t_uint8 gametype = a->queue;
			t_uint8 plnum = a->playernum;
			t_clienttag ct = conn_get_clienttag(c);
			int tt = _anongame_totalteams(gametype);

			/* do nothing till all other players have w3route conn closed */
			for (i = 0; i < tp; i++)
			if (i + 1 != plnum && a->info->player[i])
			if (conn_get_routeconn(a->info->player[i]))
				return 0;

			anongame_evaluate_results(a);

			/* count wins, losses, discs */
			for (i = 0; i < tp; i++) {
				if (a->info->result[i] == W3_GAMERESULT_WIN)
					wins++;
				else if (a->info->result[i] == W3_GAMERESULT_LOSS)
					losses++;
				else
					discs++;
			}

			/* do some sanity checking (hack prevention) */
			switch (gametype) {
			case ANONGAME_TYPE_SMALL_FFA:
				if (wins != 1) {
					eventlog(eventlog_level_info, __FUNCTION__, "bogus game result: wins != 1 in small ffa game");
					return -1;
				}
				break;
			case ANONGAME_TYPE_TEAM_FFA:
				if (!discs && wins != 2) {
					eventlog(eventlog_level_info, __FUNCTION__, "bogus game result: wins != 2 in team ffa game");
					return -1;
				}
				break;
			default:
				if (!discs && wins > losses) {
					eventlog(eventlog_level_info, __FUNCTION__, "bogus game result: wins > losses");
					return -1;
				}
				break;
			}

			/* prevent users from getting loss if server is shutdown (does not prevent errors from crash) - [Omega] */
			/* also discard games with no winners at all (i.e. games where game host disc'ed and so all players do) */
			if (!wins)
				return -1;

			/* according to zap, order of players in anongame is:
			 * for PG: t1_p1, t2_p1, t1_p2, t2_p2, ...
			 * for AT: t1_p1, t1_p2, ..., t2_p1, t2_p2, ...
			 *
			 * (Not True.. follows same order as PG)
			 *  4v4     = t1_p1, t2_p1, t1_p2, t2_p2, t1_p3, t2_p3, t1_p4, t2_p4
			 *  3v3v3   = t1_p1, t2_p1, t3_p1, t1_p2, t2_p2, t3_p2, t1_p3, t2_p3, t3_p3
			 *  2v2v2v2 = t1_p1, t2_p1, t3_p1, t4_p1, t1_p2, t2_p2, t3_p2, t4_p2
			 */

			/* opponent level calculation has to be done here, because later on, the level of other players
			 * may allready be modified
			 */
			for (i = 0; i < tp; i++) {
				int j, k, l;
				t_account *oacc;
				oppon_level[i] = 0;
				switch (gametype) {
				case ANONGAME_TYPE_TY:
					/* FIXME-TY: ADD TOURNAMENT STATS RECORDING (this part not required?) */
					break;
				case ANONGAME_TYPE_1V1:
					oppon_level[i] = account_get_ladder_level(a->info->account[(i + 1) % tp], ct, ladder_id_solo);
					break;
				case ANONGAME_TYPE_SMALL_FFA:
					/* oppon_level = average level of all other players */
					for (j = 0; j < tp; j++)
					if (i != j)
						oppon_level[i] += account_get_ladder_level(a->info->account[j], ct, ladder_id_ffa);
					oppon_level[i] /= (tp - 1);
					break;
				case ANONGAME_TYPE_AT_2V2:
				case ANONGAME_TYPE_AT_3V3:
				case ANONGAME_TYPE_AT_4V4:
					oacc = a->info->account[(i + 1) % tp];
					oppon_level[i] = team_get_level(account_find_team_by_teamid(oacc, account_get_currentatteam(oacc)));
					break;
				case ANONGAME_TYPE_AT_2V2V2:
					oacc = a->info->account[(i + 1) % tp];
					oppon_level[i] = team_get_level(account_find_team_by_teamid(oacc, account_get_currentatteam(oacc)));
					oacc = a->info->account[(i + 2) % tp];
					oppon_level[i] = team_get_level(account_find_team_by_teamid(oacc, account_get_currentatteam(oacc)));
					oppon_level[i] /= 2;
					break;
				default:
					/* oppon_level = average level of all opponents
					 * this should work for all PG team games
					 * [Omega] */
					k = i + 1;
					for (j = 0; j < (tp / tt); j++) {
						for (l = 0; l < (tt - 1); l++) {
							oppon_level[i] += account_get_ladder_level(a->info->account[k % tp], ct, ladder_id_team);
							k++;
						}
						k++;
					}
					oppon_level[i] /= (tp / tt * (tt - 1));
				}
			}

			for (i = 0; i < tp; i++) {
				t_account *acc;
				t_team *team;
				unsigned int currteam;
				int result = a->info->result[i];

				if (result == -1)
					result = W3_GAMERESULT_LOSS;

				acc = a->info->account[i];

				switch (gametype) {
				case ANONGAME_TYPE_TY:
					if (result == W3_GAMERESULT_WIN)
						tournament_add_stat(acc, 1);
					if (result == W3_GAMERESULT_LOSS)
						tournament_add_stat(acc, 2);
					/* FIXME-TY: how to do ties? */
					break;
				case ANONGAME_TYPE_AT_2V2:
				case ANONGAME_TYPE_AT_3V3:
				case ANONGAME_TYPE_AT_4V4:
				case ANONGAME_TYPE_AT_2V2V2:

					if ((currteam = account_get_currentatteam(acc))) {
						team = account_find_team_by_teamid(acc, currteam);
						team_set_saveladderstats(team, gametype, result, oppon_level[i], ct);
					}

					break;
				case ANONGAME_TYPE_1V1:
				case ANONGAME_TYPE_2V2:
				case ANONGAME_TYPE_3V3:
				case ANONGAME_TYPE_4V4:
				case ANONGAME_TYPE_SMALL_FFA:
				case ANONGAME_TYPE_5V5:
				case ANONGAME_TYPE_6V6:
				case ANONGAME_TYPE_2V2V2:
				case ANONGAME_TYPE_3V3V3:
				case ANONGAME_TYPE_4V4V4:
				case ANONGAME_TYPE_2V2V2V2:
				case ANONGAME_TYPE_3V3V3V3:
					if (result == W3_GAMERESULT_WIN)
						account_set_saveladderstats(acc, gametype, game_result_win, oppon_level[i], ct);
					if (result == W3_GAMERESULT_LOSS)
						account_set_saveladderstats(acc, gametype, game_result_loss, oppon_level[i], ct);
					break;
				default:
					break;
				}
			}
			/* aaron: now update war3 ladders */
			ladders.update();
			return 1;
		}

		/**********/
		extern t_anongameinfo *anongameinfo_create(int totalplayers)
		{
			t_anongameinfo *temp;
			int i;

			temp = (t_anongameinfo*)xmalloc(sizeof(t_anongameinfo));

			temp->totalplayers = temp->currentplayers = totalplayers;
			for (i = 0; i < ANONGAME_MAX_GAMECOUNT; i++) {
				temp->player[i] = NULL;
				temp->account[i] = NULL;
				temp->result[i] = -1;	/* consider DISC default */
				temp->results[i] = NULL;
			}

			return temp;
		}

		extern void anongameinfo_destroy(t_anongameinfo * i)
		{
			int j;

			if (!i) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongameinfo");
				return;
			}
			for (j = 0; j < ANONGAME_MAX_GAMECOUNT; j++)
			if (i->results[j])
				gameresult_destroy(i->results[j]);
			xfree(i);
		}

		/**********/
		extern t_anongameinfo *anongame_get_info(t_anongame * a)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return NULL;
			}

			return a->info;
		}

		extern int anongame_get_currentplayers(t_anongame * a)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return 0;
			}
			if (!a->info) {
				eventlog(eventlog_level_error, __FUNCTION__, "NULL anongameinfo");
				return 0;
			}

			return a->info->currentplayers;
		}

		extern int anongame_get_totalplayers(t_anongame * a)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return 0;
			}
			if (!a->info) {
				eventlog(eventlog_level_error, __FUNCTION__, "NULL anongameinfo");
				return 0;
			}

			return a->info->totalplayers;
		}

		extern t_connection *anongame_get_player(t_anongame * a, int plnum)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return NULL;
			}
			if (!a->info) {
				eventlog(eventlog_level_error, __FUNCTION__, "NULL anongameinfo");
				return NULL;
			}

			if (plnum < 0 || plnum > 7 || plnum >= a->info->totalplayers) {
				eventlog(eventlog_level_error, __FUNCTION__, "invalid plnum: %d", plnum);
				return NULL;
			}

			return a->info->player[plnum];
		}

		extern int anongame_get_count(t_anongame * a)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return 0;
			}
			return a->count;
		}

		extern t_uint32 anongame_get_id(t_anongame * a)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return 0;
			}
			return a->id;
		}

		extern t_connection *anongame_get_tc(t_anongame * a, int tpnumber)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return 0;
			}
			return a->tc[tpnumber];
		}

		extern t_uint32 anongame_get_race(t_anongame * a)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return 0;
			}
			return a->race;
		}

		extern t_uint32 anongame_get_handle(t_anongame * a)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return 0;
			}
			return a->handle;
		}

		extern unsigned int anongame_get_addr(t_anongame * a)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return 0;
			}
			return a->addr;
		}

		extern char anongame_get_loaded(t_anongame * a)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return 0;
			}
			return a->loaded;
		}

		extern char anongame_get_joined(t_anongame * a)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return 0;
			}
			return a->joined;
		}

		extern t_uint8 anongame_get_playernum(t_anongame * a)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return 0;
			}
			return a->playernum;
		}

		extern t_uint8 anongame_get_queue(t_anongame * a)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return 0;
			}
			return a->queue;
		}

		/**********/
		extern void anongame_set_result(t_anongame * a, int result)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return;
			}
			if (!a->info) {
				eventlog(eventlog_level_error, __FUNCTION__, "NULL anongameinfo");
				return;
			}

			if (a->playernum < 1 || a->playernum > ANONGAME_MAX_GAMECOUNT) {
				eventlog(eventlog_level_error, __FUNCTION__, "invalid playernum: %d", a->playernum);
				return;
			}

			a->info->result[a->playernum - 1] = result;
		}

		extern void anongame_set_gameresults(t_anongame * a, t_anongame_gameresult * results)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return;
			}
			if (!a->info) {
				eventlog(eventlog_level_error, __FUNCTION__, "NULL anongameinfo");
				return;
			}

			if (a->playernum < 1 || a->playernum > ANONGAME_MAX_GAMECOUNT) {
				eventlog(eventlog_level_error, __FUNCTION__, "invalid playernum: %d", a->playernum);
				return;
			}

			a->info->results[a->playernum - 1] = results;
		}

		extern void anongame_set_handle(t_anongame * a, t_uint32 h)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return;
			}

			a->handle = h;
		}

		extern void anongame_set_addr(t_anongame * a, unsigned int addr)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return;
			}

			a->addr = addr;
		}

		extern void anongame_set_loaded(t_anongame * a, char loaded)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return;
			}

			a->loaded = loaded;
		}

		extern void anongame_set_joined(t_anongame * a, char joined)
		{
			if (!a) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame");
				return;
			}

			a->joined = joined;
		}

		/**********/
		/* move to own .c/.h file for handling w3route connections */
		extern int handle_w3route_packet(t_connection * c, t_packet const *const packet)
		{
			/* [smith] 20030427 fixed Big-Endian/Little-Endian conversion (Solaris bug) then
			 * use  packet_append_data for append platform dependent data types - like
			 * "int", cos this code was broken for BE platforms. it's rewriten in platform
			 * independent style whis usege bn_int and other bn_* like datatypes and
			 * fuctions for wor with datatypes - bn_int_set(), what provide right
			 * byteorder, not depended on LE/BE
			 * fixed broken htonl() conversion for BE platforms - change it to
			 * bn_int_nset(). i hope it's worked on intel too %) */

			t_packet *rpacket;
			t_connection *gamec;
			char const *username;
			t_anongame *a = NULL;
			t_uint8 gametype, plnum;
			int tp, i;

			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] got NULL connection", conn_get_socket(c));
				return -1;
			}
			if (!packet) {
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] got NULL packet", conn_get_socket(c));
				return -1;
			}
			if (packet_get_class(packet) != packet_class_w3route) {
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] got bad packet (class %d)", conn_get_socket(c), packet_get_class(packet));
				return -1;
			}
			if (conn_get_state(c) != conn_state_connected) {
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] not connected", conn_get_socket(c));
				return -1;
			}

			/* init route connection */
			if (packet_get_type(packet) == CLIENT_W3ROUTE_REQ) {
				t_connection *oldc;

				eventlog(eventlog_level_trace, __FUNCTION__, "[%d] sizeof t_client_w3route_req %lu", conn_get_socket(c), sizeof(t_client_w3route_req));
				username = packet_get_str_const(packet, sizeof(t_client_w3route_req), MAX_USERNAME_LEN);
				eventlog(eventlog_level_info, __FUNCTION__, "[%d] got username '%s'", conn_get_socket(c), username);
				gamec = connlist_find_connection_by_accountname(username);

				if (!gamec) {
					eventlog(eventlog_level_info, __FUNCTION__, "[%d] no game connection found for this w3route connection; closing", conn_get_socket(c));
					conn_set_state(c, conn_state_destroy);
					return 0;
				}

				if (!(a = conn_get_anongame(gamec))) {
					eventlog(eventlog_level_info, __FUNCTION__, "[%d] no anongame struct for game connection", conn_get_socket(c));
					conn_set_state(c, conn_state_destroy);
					return 0;
				}

				if (bn_int_get((unsigned char const *)packet->u.data + sizeof(t_client_w3route_req)+std::strlen(username) + 2) != anongame_get_id(a)) {
					eventlog(eventlog_level_info, __FUNCTION__, "[%d] client sent wrong id for user '%s', closing connection", conn_get_socket(c), username);
					conn_set_state(c, conn_state_destroy);
					return 0;
				}

				oldc = conn_get_routeconn(gamec);
				if (oldc) {
					conn_set_routeconn(oldc, NULL);
					conn_set_state(oldc, conn_state_destroy);
				}

				if (conn_set_routeconn(c, gamec) < 0 || conn_set_routeconn(gamec, c) < 0) {
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] conn_set_routeconn failed", conn_get_socket(c));
					return -1;
				}

				/* set clienttag for w3route connections; we can do conn_get_clienttag() on them */
				conn_set_clienttag(c, conn_get_clienttag(gamec));

				anongame_set_addr(a, bn_int_get((unsigned char const *)packet->u.data + sizeof(t_client_w3route_req)+std::strlen(username) + 2 + 12));
				anongame_set_joined(a, 0);
				anongame_set_loaded(a, 0);
				anongame_set_result(a, -1);
				anongame_set_gameresults(a, NULL);

				anongame_set_handle(a, bn_int_get(packet->u.client_w3route_req.handle));

				if (!(rpacket = packet_create(packet_class_w3route))) {
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] packet_create failed", conn_get_socket(c));
					return -1;
				}

				packet_set_size(rpacket, sizeof(t_server_w3route_ack));
				packet_set_type(rpacket, SERVER_W3ROUTE_ACK);
				bn_byte_set(&rpacket->u.server_w3route_ack.unknown1, 7);
				bn_short_set(&rpacket->u.server_w3route_ack.unknown2, 0);
				bn_int_set(&rpacket->u.server_w3route_ack.unknown3, SERVER_W3ROUTE_ACK_UNKNOWN3);

				bn_short_set(&rpacket->u.server_w3route_ack.unknown4, 0xcccc);
				bn_byte_set(&rpacket->u.server_w3route_ack.playernum, anongame_get_playernum(a));
				bn_short_set(&rpacket->u.server_w3route_ack.unknown5, 0x0002);
				bn_short_set(&rpacket->u.server_w3route_ack.port, conn_get_port(c));
				bn_int_nset(&rpacket->u.server_w3route_ack.ip, conn_get_addr(c));
				bn_int_set(&rpacket->u.server_w3route_ack.unknown7, 0);
				bn_int_set(&rpacket->u.server_w3route_ack.unknown8, 0);
				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);

				return 0;
			}
			else {
				gamec = conn_get_routeconn(c);
				if (gamec)
					a = conn_get_anongame(gamec);
			}

			if (!gamec) {
				eventlog(eventlog_level_info, __FUNCTION__, "[%d] no game connection found for this w3route connection", conn_get_socket(c));
				return 0;
			}

			if (!a) {
				eventlog(eventlog_level_info, __FUNCTION__, "[%d] no anongame struct found for this w3route connection", conn_get_socket(c));
				return 0;
			}

			gametype = anongame_get_queue(a);
			plnum = anongame_get_playernum(a);
			tp = anongame_get_totalplayers(a);

			/* handle these packets _before_ checking for routeconn of other players */
			switch (packet_get_type(packet)) {
			case CLIENT_W3ROUTE_ECHOREPLY:
				return 0;
			case CLIENT_W3ROUTE_CONNECTED:
				return 0;
			case CLIENT_W3ROUTE_GAMERESULT:
			case CLIENT_W3ROUTE_GAMERESULT_W3XP:
			{

												   /* insert reading of whole packet into t_gameresult */

												   t_anongame_gameresult *gameresult;
												   int result;

												   t_timer_data data;
												   t_anongameinfo *inf = anongame_get_info(a);
												   t_connection *ac;

												   data.p = NULL;

												   if (!(gameresult = anongame_gameresult_parse(packet)))
													   result = -1;
												   else		/* own result is always stored as first result */
													   result = gameresult_get_player_result(gameresult, 0);

												   eventlog(eventlog_level_trace, __FUNCTION__, "[%d] got W3ROUTE_GAMERESULT: %08x", conn_get_socket(c), result);

												   if (!inf) {
													   eventlog(eventlog_level_error, __FUNCTION__, "[%d] NULL anongameinfo", conn_get_socket(c));
													   return -1;
												   }

												   anongame_set_gameresults(a, gameresult);
												   anongame_set_result(a, result);

												   conn_set_state(c, conn_state_destroy);

												   /* activate timers on open w3route connectons */
												   if (result == W3_GAMERESULT_WIN) {
													   for (i = 0; i < tp; i++) {
														   if (anongame_get_player(a, i)) {
															   ac = conn_get_routeconn(anongame_get_player(a, i));
															   if (ac) {
																   /* 300 seconds or 5 minute timer */
																   timerlist_add_timer(ac, now + (std::time_t) 300, conn_shutdown, data);
																   eventlog(eventlog_level_trace, __FUNCTION__, "[%d] started timer to close w3route", conn_get_socket(ac));
															   }
														   }
													   }
												   }

												   return 0;
			}
			}

			for (i = 0; i < tp; i++)
			if (i + 1 != plnum && anongame_get_player(a, i))
			if (!conn_get_routeconn(anongame_get_player(a, i)) || !conn_get_anongame(anongame_get_player(a, i))) {
				eventlog(eventlog_level_info, __FUNCTION__, "[%d] not all players have w3route connections up yet", conn_get_socket(c));
				return 0;
			}

			/* handle these packets _after_ checking for routeconns of other players */
			switch (packet_get_type(packet)) {
			case CLIENT_W3ROUTE_LOADINGDONE:
				eventlog(eventlog_level_trace, __FUNCTION__, "[%d] got LOADINGDONE, playernum: %d", conn_get_socket(c), plnum);

				anongame_set_loaded(a, 1);

				for (i = 0; i < tp; i++) {
					if (!anongame_get_player(a, i))	/* ignore disconnected players */
						continue;
					if (!(rpacket = packet_create(packet_class_w3route))) {
						eventlog(eventlog_level_error, __FUNCTION__, "[%d] packet_create failed", conn_get_socket(c));
						return -1;
					}
					packet_set_size(rpacket, sizeof(t_server_w3route_loadingack));
					packet_set_type(rpacket, SERVER_W3ROUTE_LOADINGACK);
					bn_byte_set(&rpacket->u.server_w3route_loadingack.playernum, plnum);
					conn_push_outqueue(conn_get_routeconn(anongame_get_player(a, i)), rpacket);
					packet_del_ref(rpacket);
				}

				/* have all players loaded? */
				for (i = 0; i < tp; i++)
				if (i + 1 != plnum && anongame_get_player(a, i) && !anongame_get_loaded(conn_get_anongame(anongame_get_player(a, i))))
					return 0;

				for (i = 0; i < tp; i++) {
					if (!anongame_get_player(a, i))
						continue;

					if (!(rpacket = packet_create(packet_class_w3route))) {
						eventlog(eventlog_level_error, __FUNCTION__, "[%d] packet_create failed", conn_get_socket(c));
						return -1;
					}

					packet_set_size(rpacket, sizeof(t_server_w3route_ready));
					packet_set_type(rpacket, SERVER_W3ROUTE_READY);
					bn_byte_set(&rpacket->u.server_w3route_host.unknown1, 0);
					conn_push_outqueue(conn_get_routeconn(anongame_get_player(a, i)), rpacket);
					packet_del_ref(rpacket);
				}

				break;

			case CLIENT_W3ROUTE_ABORT:
				eventlog(eventlog_level_debug, __FUNCTION__, "[%d] got W3ROUTE_ABORT", conn_get_socket(c));
				break;

			default:
				eventlog(eventlog_level_trace, __FUNCTION__, "[%d] default: got packet type: %04x", conn_get_socket(c), packet_get_type(packet));
			}

			return 0;
		}

		extern int handle_anongame_join(t_connection * c)
		{
			t_anongame *a, *ja, *oa;
			t_connection *jc, *o;
			t_packet *rpacket;
			int tp, level;
			char gametype;
			t_account *acct;
			t_clienttag ct = conn_get_clienttag(c);

			static t_server_w3route_playerinfo2 pl2;
			static t_server_w3route_levelinfo2 li2;
			static t_server_w3route_playerinfo_addr pl_addr;

			int i, j;

			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] got NULL connection", conn_get_socket(c));
				return -1;
			}
			if (!(conn_get_routeconn(c))) {
				eventlog(eventlog_level_info, __FUNCTION__, "[%d] no route connection", conn_get_socket(c));
				return -1;
			}
			if (!(a = conn_get_anongame(c))) {
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] no anongame struct", conn_get_socket(c));
				return -1;
			}

			anongame_set_joined(a, 1);
			gametype = anongame_get_queue(a);
			tp = anongame_get_totalplayers(a);

			/* wait till all players have w3route conns */
			for (i = 0; i < tp; i++)
			if (anongame_get_player(a, i) && (!conn_get_routeconn(anongame_get_player(a, i)) || !conn_get_anongame(anongame_get_player(a, i)) || !anongame_get_joined(conn_get_anongame(anongame_get_player(a, i))))) {
				eventlog(eventlog_level_info, __FUNCTION__, "[%d] not all players have joined game BNet yet", conn_get_socket(c));
				return 0;
			}

			/* then send each player info about all others */
			for (j = 0; j < tp; j++) {
				jc = anongame_get_player(a, j);
				if (!jc)		/* ignore disconnected players */
					continue;
				ja = conn_get_anongame(jc);

				/* send a playerinfo packet for this player to each other player */
				for (i = 0; i < tp; i++) {
					if (i + 1 != anongame_get_playernum(ja)) {
						eventlog(eventlog_level_trace, __FUNCTION__, "i = %d", i);

						if (!(o = anongame_get_player(ja, i))) {
							eventlog(eventlog_level_warn, __FUNCTION__, "[%d] player %d disconnected, ignoring", conn_get_socket(c), i);
							continue;
						}

						if (!(rpacket = packet_create(packet_class_w3route))) {
							eventlog(eventlog_level_error, __FUNCTION__, "[%d] packet_create failed", conn_get_socket(c));
							return -1;
						}

						packet_set_size(rpacket, sizeof(t_server_w3route_playerinfo));
						packet_set_type(rpacket, SERVER_W3ROUTE_PLAYERINFO);


						if (!(oa = conn_get_anongame(o))) {
							eventlog(eventlog_level_error, __FUNCTION__, "[%d] no anongame struct of player %d", conn_get_socket(c), i);
							return -1;
						}

						bn_int_set(&rpacket->u.server_w3route_playerinfo.handle, anongame_get_handle(oa));
						bn_byte_set(&rpacket->u.server_w3route_playerinfo.playernum, anongame_get_playernum(oa));

						packet_append_string(rpacket, conn_get_username(o));

						/* playerinfo2 */
						bn_byte_set(&pl2.unknown1, 8);
						bn_int_set(&pl2.id, anongame_get_id(oa));
						bn_int_set(&pl2.race, anongame_get_race(oa));
						packet_append_data(rpacket, &pl2, sizeof(pl2));

						/* external addr */
						bn_short_set(&pl_addr.unknown1, 2);
						{		/* trans support */
							unsigned short port = conn_get_game_port(o);
							unsigned int addr = conn_get_game_addr(o);

							trans_net(conn_get_game_addr(jc), &addr, &port);

							bn_short_nset(&pl_addr.port, port);
							bn_int_nset(&pl_addr.ip, addr);
						}
						bn_int_set(&pl_addr.unknown2, 0);
						bn_int_set(&pl_addr.unknown3, 0);
						packet_append_data(rpacket, &pl_addr, sizeof(pl_addr));

						/* local addr */
						bn_short_set(&pl_addr.unknown1, 2);
						bn_short_nset(&pl_addr.port, conn_get_game_port(o));
						bn_int_set(&pl_addr.ip, anongame_get_addr(oa));
						bn_int_set(&pl_addr.unknown2, 0);
						bn_int_set(&pl_addr.unknown3, 0);
						packet_append_data(rpacket, &pl_addr, sizeof(pl_addr));

						conn_push_outqueue(conn_get_routeconn(jc), rpacket);
						packet_del_ref(rpacket);
					}
				}

				/* levelinfo */
				if (!(rpacket = packet_create(packet_class_w3route))) {
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] packet_create failed", conn_get_socket(c));
					return -1;
				}

				packet_set_size(rpacket, sizeof(t_server_w3route_levelinfo));
				packet_set_type(rpacket, SERVER_W3ROUTE_LEVELINFO);
				bn_byte_set(&rpacket->u.server_w3route_levelinfo.numplayers, anongame_get_currentplayers(a));

				for (i = 0; i < tp; i++) {
					if (!anongame_get_player(ja, i))
						continue;

					bn_byte_set(&li2.plnum, i + 1);
					bn_byte_set(&li2.unknown1, 3);

					switch (gametype) {
					case ANONGAME_TYPE_1V1:
						level = account_get_ladder_level(conn_get_account(anongame_get_player(ja, i)), ct, ladder_id_solo);
						break;
					case ANONGAME_TYPE_SMALL_FFA:
					case ANONGAME_TYPE_TEAM_FFA:
						level = account_get_ladder_level(conn_get_account(anongame_get_player(ja, i)), ct, ladder_id_ffa);
						break;
					case ANONGAME_TYPE_AT_2V2:
					case ANONGAME_TYPE_AT_3V3:
					case ANONGAME_TYPE_AT_4V4:
					case ANONGAME_TYPE_AT_2V2V2:
						acct = conn_get_account(anongame_get_player(ja, i));
						level = team_get_level(account_find_team_by_teamid(acct, account_get_currentatteam(acct)));
						break;
					case ANONGAME_TYPE_TY:
						level = 0;	/* FIXME-TY: WHAT TO DO HERE */
						break;
					default:
						level = account_get_ladder_level(conn_get_account(anongame_get_player(ja, i)), ct, ladder_id_team);
						break;
					}

					/* first anongame shows level 0 as level 1 */
					bn_byte_set(&li2.level, level ? level : 1);

					bn_short_set(&li2.unknown2, 0);
					packet_append_data(rpacket, &li2, sizeof(li2));
				}

				conn_push_outqueue(conn_get_routeconn(jc), rpacket);
				packet_del_ref(rpacket);

				/* startgame1 */
				if (!(rpacket = packet_create(packet_class_w3route))) {
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] packet_create failed", conn_get_socket(c));
					return -1;
				}
				packet_set_size(rpacket, sizeof(t_server_w3route_startgame1));
				packet_set_type(rpacket, SERVER_W3ROUTE_STARTGAME1);
				conn_push_outqueue(conn_get_routeconn(jc), rpacket);
				packet_del_ref(rpacket);

				/* startgame2 */
				if (!(rpacket = packet_create(packet_class_w3route))) {
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] packet_create failed", conn_get_socket(c));
					return -1;
				}
				packet_set_size(rpacket, sizeof(t_server_w3route_startgame2));
				packet_set_type(rpacket, SERVER_W3ROUTE_STARTGAME2);
				conn_push_outqueue(conn_get_routeconn(jc), rpacket);
				packet_del_ref(rpacket);
			}
			return 0;
		}

		extern const char * anongame_get_map_from_prefs(int queue, t_clienttag clienttag)
		{
			int i, j = 0;
			const char *default_map, *selected;
			char *res_maps[32];
			char clienttag_str[5];

			switch (clienttag) {
			case CLIENTTAG_REDALERT2_UINT:
				default_map = "eb2.map";
				break;
			case CLIENTTAG_YURISREV_UINT:
				default_map = "xgrinder.map";
				break;
			default:
				ERROR1("invalid clienttag: %s", tag_uint_to_str(clienttag_str, clienttag));
				return "eb2.map";
			}

			for (i = 0; i < 32; i++)
				res_maps[i] = NULL;

			for (i = 0; i < maplists_get_totalmaps_by_queue(clienttag, queue); i++) {
				res_maps[j++] = maplists_get_map(queue, clienttag, i + 1);
			}

			i = std::rand() % j;
			if (res_maps[i])
				selected = res_maps[i];
			else
				selected = default_map;

			eventlog(eventlog_level_trace, __FUNCTION__, "got map %s from prefs", selected);
			return selected;

		}

	}

}
