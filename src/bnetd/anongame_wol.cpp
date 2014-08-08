/*
 * Copyright (C) 2008  Pelish (pelish@gmail.com)
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

#define ANONGAME_WOL_INTERNAL_ACCESS
#include "common/setup_before.h"
#include "anongame_wol.h"

#include <cstring>
#include <cctype>
#include <cstdlib>

#include "compat/snprintf.h"
#include "compat/strcasecmp.h"

#include "common/irc_protocol.h"
#include "common/packet.h"
#include "common/eventlog.h"
#include "common/tag.h"
#include "common/list.h"
#include "common/anongame_protocol.h"

#include "irc.h"
#include "handle_wol.h"
#include "connection.h"
#include "channel.h"
#include "anongame.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		typedef int(*t_anong_tag)(t_anongame_wol_player * player, char * param);

		typedef struct {
			const char     * wol_anong_tag_string;
			t_anong_tag      wol_anong_tag_handler;
		} t_wol_anongame_tag_table_row;

		static t_list * anongame_wol_matchlist_head = NULL;

		static int _handle_address_tag(t_anongame_wol_player * player, char * param);
		static int _handle_port_tag(t_anongame_wol_player * player, char * param);
		static int _handle_country_tag(t_anongame_wol_player * player, char * param);
		static int _handle_colour_tag(t_anongame_wol_player * player, char * param);

		static const t_wol_anongame_tag_table_row t_wol_anongame_tag_table[] =
		{
			{ MATCHTAG_ADDRESS, _handle_address_tag },
			{ MATCHTAG_PORT, _handle_port_tag },
			{ MATCHTAG_COUNTRY, _handle_country_tag },
			{ MATCHTAG_COLOUR, _handle_colour_tag },

			{ NULL, NULL }
		};

		static int anongame_wol_set_playersetting(t_anongame_wol_player * player, char const * tag, char * param)
		{
			t_wol_anongame_tag_table_row const *p;

			for (p = t_wol_anongame_tag_table; p->wol_anong_tag_string != NULL; p++) {
				if (strcasecmp(tag, p->wol_anong_tag_string) == 0) {
					if (p->wol_anong_tag_handler != NULL)
						return ((p->wol_anong_tag_handler)(player, param));
				}
			}
			return -1;
		}

		/* anongame_wol player functions: */

		static t_anongame_wol_player * anongame_wol_player_create(t_connection * conn)
		{
			t_anongame_wol_player * player;

			player = (t_anongame_wol_player*)xmalloc(sizeof(t_anongame_wol_player));

			player->conn = conn;

			/* Used only in Red Alert 2 and Yuri's Revenge */
			player->address = 0;
			player->port = 0;
			player->country = -2; /* Default values are form packet dumps - not prefered country */
			player->colour = -2; /* Default values are form packet dumps - not prefered colour */

			conn_wol_set_anongame_player(conn, player);

			list_append_data(anongame_wol_matchlist_head, player);

			DEBUG1("[** WOL **] annongame player created: %s", conn_get_chatname(conn));

			return player;
		}

		static int anongame_wol_player_destroy(t_anongame_wol_player * player, t_elem ** curr)
		{
			if (list_remove_data(anongame_wol_matchlist_head, player, curr) < 0){
				ERROR0("could not remove item from list");
				return -1;
			}

			DEBUG0("[** WOL **] destroying annongame player");

			xfree(player);

			return 0;
		}

		static t_connection * anongame_wol_player_get_conn(t_anongame_wol_player const * player)
		{
			if (!player) {
				ERROR0("got NULL player");
				return NULL;
			}

			return player->conn;
		}


		static int anongame_wol_player_get_address(t_anongame_wol_player const * player)
		{
			if (!player) {
				ERROR0("got NULL player");
				return -1;
			}

			return player->address;
		}

		static int _handle_address_tag(t_anongame_wol_player * player, char * param)
		{
			if (!player) {
				ERROR0("got NULL player");
				return -1;
			}

			if (param)
				player->address = std::atoi(param);

			return 0;
		}

		static int anongame_wol_player_get_port(t_anongame_wol_player const * player)
		{
			if (!player) {
				ERROR0("got NULL player");
				return -1;
			}

			return player->port;
		}

		static int _handle_port_tag(t_anongame_wol_player * player, char * param)
		{
			if (!player) {
				ERROR0("got NULL player");
				return -1;
			}

			if (param)
				player->port = std::atoi(param);

			return 0;
		}

		static int anongame_wol_player_get_country(t_anongame_wol_player const * player)
		{
			if (!player) {
				ERROR0("got NULL player");
				return -1;
			}

			return player->country;
		}

		static int _handle_country_tag(t_anongame_wol_player * player, char * param)
		{
			if (!player) {
				ERROR0("got NULL player");
				return -1;
			}

			if (param)
				player->country = std::atoi(param);

			return 0;
		}

		static int anongame_wol_player_get_colour(t_anongame_wol_player const * player)
		{
			if (!player) {
				ERROR0("got NULL player");
				return -1;
			}

			return player->colour;
		}

		static int _handle_colour_tag(t_anongame_wol_player * player, char * param)
		{
			if (!player) {
				ERROR0("got NULL player");
				return -1;
			}

			if (param)
				player->colour = std::atoi(param);

			return 0;
		}

		/* Matchlist functions:*/

		extern int anongame_wol_matchlist_create(void)
		{
			anongame_wol_matchlist_head = list_create();

			return 0;
		}

		extern int anongame_wol_matchlist_destroy(void)
		{
			t_anongame_wol_player * player;
			t_elem * curr;

			if (anongame_wol_matchlist_head) {
				LIST_TRAVERSE(anongame_wol_matchlist_head, curr) {
					if (!(player = (t_anongame_wol_player*)elem_get_data(curr))) { /* should not happen */
						ERROR0("wol_matchlist contains NULL item");
						continue;
					}
					anongame_wol_player_destroy(player, &curr);
				}

				if (list_destroy(anongame_wol_matchlist_head) < 0)
					return -1;
				anongame_wol_matchlist_head = NULL;
			}

			return 0;
		}

		extern t_list * anongame_wol_matchlist(void)
		{
			return anongame_wol_matchlist_head;
		}

		static t_anongame_wol_player * anongame_wol_matchlist_find_player_by_conn(t_connection * conn)
		{
			t_anongame_wol_player * player;
			t_elem * curr;

			if (!conn) {
				ERROR0("got NULL conn");
				return NULL;
			}

			LIST_TRAVERSE(anongame_wol_matchlist(), curr) {
				t_anongame_wol_player * player = (t_anongame_wol_player *)elem_get_data(curr);
				if (conn == anongame_wol_player_get_conn(player)) {
					return player;
				}
			}

			return NULL;
		}

		static int anongame_wol_matchlist_get_length(void)
		{
			return list_get_length(anongame_wol_matchlist_head);
		}

		/* support functions */

		static int _send_msg(t_connection * conn, char const * command, char const * text)
		{
			t_packet * p;
			char data[MAX_IRC_MESSAGE_LEN + 1];
			unsigned len;
			char const * nick;

			if (!conn) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (!command) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL command");
				return -1;
			}
			if (!text) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL text");
				return -1;
			}
			if (!(p = packet_create(packet_class_raw))) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not create packet");
				return -1;
			}

			nick = conn_get_loggeduser(conn);
			if (!nick)
				nick = "UserName";

			/* snprintf isn't portable -> check message length first */
			len = 1 + 12 + 1 + std::strlen(command) + 1 + std::strlen(nick) + 1 + std::strlen(text) + 2;
			if (len > MAX_IRC_MESSAGE_LEN) {
				eventlog(eventlog_level_error, __FUNCTION__, "message to send is too large (%u bytes)", len);
				return -1;
			}
			else
				std::sprintf(data, ":matchbot!u@h %s %s %s", command, nick, text);

			DEBUG2("[%d] sent \"%s\"", conn_get_socket(conn), data);
			std::strcat(data, "\r\n");
			packet_set_size(p, 0);
			packet_append_data(p, data, len);
			conn_push_outqueue(conn, p);
			packet_del_ref(p);
			return 0;
		}

		static int _get_pair(int * i, int * j, int max, bool different)
		{
			max++; /* We want to use just real maximum number - no metter what function do */

			if (*i == -2)
				*i = (max * rand() / (RAND_MAX + 1));

			if (*j == -2)
				*j = (max * rand() / (RAND_MAX + 1));

			if ((different) && (*i == *j)) {
				do {
					*j = (max * rand() / (RAND_MAX + 1));
				} while (*i == *j);
			}
			return 0;
		}


		static int anongame_wol_trystart(t_anongame_wol_player const * player1)
		{
			t_elem * curr;
			char temp[MAX_IRC_MESSAGE_LEN];
			char _temp[MAX_IRC_MESSAGE_LEN];
			int random;

			t_anongame_wol_player * player2;
			t_connection * conn_pl1;
			t_connection * conn_pl2;
			char const * channelname;
			const char * mapname;
			t_clienttag ctag;

			std::memset(temp, 0, sizeof(temp));
			std::memset(_temp, 0, sizeof(_temp));

			/**
			 * Expected start message is
			 *
			 * Red Alert 2:
			 * :Start [rand_num],0,0,[credits],0,1,[super_weapon],1,1,0,1,x,2,1,[mapsize_kilobytes],[mapname],1:
			 *     [nick1_name],[nick1_country],[nick1_colour],[nick1_addr_hex],[nick1_nat],[nick1_prt_hex],
			 *     [nick2_name],[nick2_country],[nick2_colour],[nick2_addr_hex],[nick2_nat],[nick2_prt_hex]
			 *
			 * Yuris Revenge Quick game:
			 *
			 * Yuris Revenge Quick Coop:
			 *
			 */

			if (!player1) {
				ERROR0("got NULL player");
				return -1;
			}

			conn_pl1 = anongame_wol_player_get_conn(player1);
			ctag = conn_get_clienttag(conn_pl1);
			channelname = channel_get_name(conn_get_channel(conn_pl1));

			LIST_TRAVERSE(anongame_wol_matchlist(), curr) {
				player2 = (t_anongame_wol_player *)elem_get_data(curr);
				conn_pl2 = anongame_wol_player_get_conn(player2);

				if ((player1 != player2) && ((conn_get_channel(conn_pl1)) == (conn_get_channel(conn_pl2)))) {
					switch (ctag) {
					case CLIENTTAG_REDALERT2_UINT: {
													   random = rand();

													   if (std::strcmp(channelname, RAL2_CHANNEL_FFA) == 0) {
														   int pl1_colour = anongame_wol_player_get_colour(player1);
														   int pl1_country = anongame_wol_player_get_country(player1);
														   int pl2_colour = anongame_wol_player_get_colour(player2);
														   int pl2_country = anongame_wol_player_get_country(player2);

														   DEBUG0("Generating SOLO game for Red Alert 2");

														   _get_pair(&pl1_colour, &pl2_colour, 7, true);
														   _get_pair(&pl1_country, &pl2_country, 8, false);
														   mapname = anongame_get_map_from_prefs(ANONGAME_TYPE_1V1, ctag);

														   /* We have madatory of game */
														   snprintf(_temp, sizeof(_temp), ":Start %u,0,0,10000,0,1,0,1,1,0,1,x,2,1,165368,%s,1:", random, mapname);
														   std::strcat(temp, _temp);

														   /* GameHost informations */
														   snprintf(_temp, sizeof(_temp), "%s,%d,%d,%x,1,%x,", conn_get_chatname(conn_pl1), pl1_country, pl1_colour, anongame_wol_player_get_address(player1), anongame_wol_player_get_port(player1));
														   std::strcat(temp, _temp);

														   /* GameJoinie informations */
														   snprintf(_temp, sizeof(_temp), "%s,%d,%d,%x,1,%x", conn_get_chatname(conn_pl2), pl2_country, pl2_colour, anongame_wol_player_get_address(player2), anongame_wol_player_get_port(player2));
														   std::strcat(temp, _temp);

														   _send_msg(conn_pl1, "PRIVMSG", temp);
														   _send_msg(conn_pl2, "PRIVMSG", temp);
													   }
													   else
														   ERROR1("undefined channel type for %s channel", channelname);
					}
						return 0;
					case CLIENTTAG_YURISREV_UINT: {
													  random = rand();

													  if (std::strcmp(channelname, YURI_CHANNEL_FFA) == 0) {
														  int pl1_colour = anongame_wol_player_get_colour(player1);
														  int pl1_country = anongame_wol_player_get_country(player1);
														  int pl2_colour = anongame_wol_player_get_colour(player2);
														  int pl2_country = anongame_wol_player_get_country(player2);

														  DEBUG0("Generating SOLO game for Yuri's Revenge");

														  _get_pair(&pl1_colour, &pl2_colour, 7, true);
														  _get_pair(&pl1_country, &pl2_country, 9, false);
														  mapname = anongame_get_map_from_prefs(ANONGAME_TYPE_1V1, ctag);

														  /* We have madatory of game */
														  snprintf(_temp, sizeof(_temp), ":Start %u,0,0,10000,0,0,1,1,1,0,3,0,x,2,1,163770,%s,1:", random, mapname);
														  std::strcat(temp, _temp);

														  /* GameHost informations */
														  snprintf(_temp, sizeof(_temp), "%s,%d,%d,-2,-2,", conn_get_chatname(conn_pl1), pl1_country, pl1_colour);
														  std::strcat(temp, _temp);
														  snprintf(_temp, sizeof(_temp), "%x,1,%x,", anongame_wol_player_get_address(player1), anongame_wol_player_get_port(player1));
														  std::strcat(temp, _temp);

														  /* GameJoinie informations */
														  snprintf(_temp, sizeof(_temp), "%s,%d,%d,-2,-2,", conn_get_chatname(conn_pl2), pl2_country, pl2_colour);
														  std::strcat(temp, _temp);
														  snprintf(_temp, sizeof(_temp), "%x,1,%x", anongame_wol_player_get_address(player2), anongame_wol_player_get_port(player2));
														  std::strcat(temp, _temp);

														  _send_msg(conn_pl1, "PRIVMSG", temp);
														  _send_msg(conn_pl2, "PRIVMSG", temp);
													  }
													  else if (std::strcmp(channelname, YURI_CHANNEL_COOP) == 0) {
														  DEBUG0("Generating COOP game for Yuri's Revenge");

														  /* We have madatory of game */
														  snprintf(_temp, sizeof(_temp), ":Start %u,0,0,10000,10,0,1,1,0,1,3,0,x,2,1,163770,C1A01MD.MAP,1:", random);
														  std::strcat(temp, _temp);

														  /* GameHost informations */
														  snprintf(_temp, sizeof(_temp), "%s,0,4,0,-2,", conn_get_chatname(conn_pl1));
														  std::strcat(temp, _temp);
														  snprintf(_temp, sizeof(_temp), "%x,1,%x,", anongame_wol_player_get_address(player1), anongame_wol_player_get_port(player1));
														  std::strcat(temp, _temp);

														  /* GameJoinie informations */
														  snprintf(_temp, sizeof(_temp), "%s,0,5,1,-2,", conn_get_chatname(anongame_wol_player_get_conn(player2)));
														  std::strcat(temp, _temp);
														  snprintf(_temp, sizeof(_temp), "%x,1,%x", anongame_wol_player_get_address(player2), anongame_wol_player_get_port(player2));
														  std::strcat(temp, _temp);

														  /* Some computers for coop games */
														  snprintf(_temp, sizeof(_temp), ":@:0,-1,-1,-2,-2,0,-1,-1,-2,-2,1,8,1,-2,-2,1,8,2,-2,-2,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,");
														  std::strcat(temp, _temp);

														  _send_msg(conn_pl1, "PRIVMSG", temp);
														  _send_msg(conn_pl2, "PRIVMSG", temp);
													  }
													  else
														  ERROR1("undefined channel type for %s channel", channelname);
					}
						return 0;
					default:
						DEBUG0("unsupported client for WOL Matchgame");
						return 0;
					}
				}
			}

			return 0;
		}

		static int anongame_wol_tokenize_line(t_connection * conn, char const * text)
		{
			t_anongame_wol_player * player;
			t_tag ctag;
			char const * channelname;
			char * command;     /* Match, CINFO, SINFO, Pings */
			char * tag;         /* ADR, COL, COU... */
			char * param;       /* value of paramtag */
			char * line;        /* Copy of text */
			char * temp;
			char * p;

			if (!conn) {
				ERROR0("got NULL conn");
				return -1;
			}

			if (!text) {
				ERROR0("got NULL text");
				return -1;
			}

			if (!(player = anongame_wol_player_create(conn))) {
				ERROR0("player was not created");
				return -1;
			}

			/**
			 * Here are expected privmsgs:
			 * :user!EMPR@host PRIVMSG matchbot :Match COU=-1,COL=-1,SHA=-1,SHB=-1,LOC=2,RAT=0
			 * :user!RAL2@host PRIVMSG matchbot :Match COU=-2, COL=-2, LOC=1, RAT=3, RES=640, ADR=202680512, NAT=1, PRT=1295
			 * :user!RAL2@host PRIVMSG matchbot :Match COU=-2, COL=-2, LOC=1, RAT=3, RES=640, ADR=202680512, NAT=1, PRT=1340
			 * :user!RAL2@host PRIVMSG matchbot :Match COU=-2, COL=-2, LOC=25, RAT=1, RES=640, ADR=-501962560, NAT=1, PRT=1320
			 * :user!YURI@host PRIVMSG matchbot :Match COU=-2, COL=-2, MBR=1, LOC=5, RAT=2, RES=800, ADR=202680512, NAT=1, PRT=1390
			 * :user!RNGD@host PRIVMSG matchbot :CINFO VER=1329937315 CPU=2981 MEM=1023 TPOINTS=0 PLAYED=0 PINGS=00FFFFFFFFFFFFFF
			 * :user!RNGD@host PRIVMSG matchbot :SINFO 4F453BA3DBAE41CB00000000000000002d# 9Dedicated Renegade Server-C&C_Field.mix07FF0656FFFF1C2D|090000000000
			 * :user!YURI@host PRIVMSG matchbot :Pings nickname,2;
			 */

			line = (char *)xmalloc(std::strlen(text) + 2);
			strcpy(line, text);

			command = line;
			if (!(temp = strchr(command, ' '))) {
				WARN0("got malformed line (missing command)");
				xfree(line);
				return -1;
			}
			*temp++ = '\0';

			if ((std::strcmp(command, "Match") == 0)) {
				strcat(temp, ","); /* FIXME: This is DUMB - without that we lost the last tag/param */

				for (p = temp; *p && (*p != '\0'); p++) {
					if ((*p == ',') || (*p == ' ')) {
						*p++ = '\0';
						if (temp[0] == ' ') /* Pelish: Emperor sends line without spaces but RA2/Yuri do */
							*temp++;
						tag = temp;
						param = strchr(tag, '=');
						if (param)
							*param++ = '\0';
						if (anongame_wol_set_playersetting(player, tag, param) == -1)
							WARN2("[** WOL **] got unknown tag %s param %s", tag, param);
						temp = p;
					}
				}
				_send_msg(conn, "PRIVMSG", ":Working");
				anongame_wol_trystart(player);
			}
			else {
				DEBUG1("[** WOL **] got line /%s/", text);
			}

			if (line)
				xfree(line);

			return 0;
		}

		/* Functions for getting/sending player informations */

		extern int anongame_wol_destroy(t_connection * conn)
		{
			t_elem *curr;

			/* Player destroying */

			LIST_TRAVERSE(anongame_wol_matchlist_head, curr) {
				t_anongame_wol_player * player = (t_anongame_wol_player*)elem_get_data(curr);

				if (conn == anongame_wol_player_get_conn(player))
					anongame_wol_player_destroy(player, &curr);
			}

			return 0;
		}

		extern int anongame_wol_privmsg(t_connection * conn, int numparams, char ** params, char * text)
		{

			if (!conn) {
				ERROR0("got NULL conn");
				return -1;
			}

			anongame_wol_tokenize_line(conn, text);

			return 0;
		}

	}

}
