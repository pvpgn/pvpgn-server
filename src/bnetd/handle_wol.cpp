/*
 * Copyright (C) 2001  Marco Ziech (mmz@gmx.net)
 * Copyright (C) 2005  Bryan Biedenkapp (gatekeep@gmail.com)
 * Copyright (C) 2006,2007,2008  Pelish (pelish@gmail.com)
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
#include "handle_wol.h"

#include <cstring>
#include <cctype>
#include <cstdlib>

#include "compat/strcasecmp.h"
#include "common/irc_protocol.h"
#include "common/eventlog.h"
#include "common/bnethash.h"
#include "common/tag.h"
#include "common/util.h"
#include "common/list.h"
#include "common/addr.h"
#include "common/trans.h"

#include "common/packet.h"

#include "compat/snprintf.h"

#include "prefs.h"
#include "command.h"
#include "irc.h"
#include "account.h"
#include "account_wrap.h"
#include "command_groups.h"
#include "channel.h"
#include "message.h"
#include "tick.h"
#include "topic.h"
#include "server.h"
#include "friends.h"
#include "clan.h"
#include "game.h"
#include "anongame_wol.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		typedef int(*t_wol_command)(t_connection * conn, int numparams, char ** params, char * text);

		typedef struct {
			const char     * wol_command_string;
			t_wol_command    wol_command_handler;
		} t_wol_command_table_row;

		static int _handle_user_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_pass_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_privmsg_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_quit_command(t_connection * conn, int numparams, char ** params, char * text);

		static int _handle_list_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_names_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_part_command(t_connection * conn, int numparams, char ** params, char * text);

		static int _handle_cvers_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_verchk_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_apgar_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_setopt_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_serial_command(t_connection * conn, int numparams, char ** params, char * text);

		static int _handle_squadinfo_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_clanbyname_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_setcodepage_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_getcodepage_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_setlocale_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_getlocale_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_getinsider_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_joingame_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_gameopt_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_finduser_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_finduserex_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_page_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_startg_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_advertr_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_advertc_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_chanchk_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_getbuddy_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_addbuddy_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_delbuddy_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_host_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_invmsg_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_invdel_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_userip_command(t_connection * conn, int numparams, char ** params, char * text);

		/* Ladder server commands (we will probalby move this commands to any another handle file */
		static int _handle_listsearch_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_rungsearch_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_highscore_command(t_connection * conn, int numparams, char ** params, char * text);

		/* state "connected" handlers */
		static const t_wol_command_table_row wol_con_command_table[] =
		{
			{ "NICK", _handle_nick_command },
			{ "USER", _handle_user_command },
			{ "PING", _handle_ping_command },
			{ "PONG", _handle_pong_command },
			{ "PASS", _handle_pass_command },
			{ "PRIVMSG", _handle_privmsg_command },
			{ "QUIT", _handle_quit_command },

			{ "CVERS", _handle_cvers_command },
			{ "VERCHK", _handle_verchk_command },
			{ "APGAR", _handle_apgar_command },
			{ "SETOPT", _handle_setopt_command },
			{ "SERIAL", _handle_serial_command },

			/* Ladder server commands */
			{ "LISTSEARCH", _handle_listsearch_command },
			{ "RUNGSEARCH", _handle_rungsearch_command },
			{ "HIGHSCORE", _handle_highscore_command },

			{ NULL, NULL }
		};

		/* state "logged in" handlers */
		static const t_wol_command_table_row wol_log_command_table[] =
		{
			{ "LIST", _handle_list_command },
			{ "TOPIC", _handle_topic_command },
			{ "JOIN", _handle_join_command },
			{ "NAMES", _handle_names_command },
			{ "PART", _handle_part_command },

			{ "SQUADINFO", _handle_squadinfo_command },
			{ "CLANBYNAME", _handle_clanbyname_command },
			{ "SETCODEPAGE", _handle_setcodepage_command },
			{ "SETLOCALE", _handle_setlocale_command },
			{ "GETCODEPAGE", _handle_getcodepage_command },
			{ "GETLOCALE", _handle_getlocale_command },
			{ "GETINSIDER", _handle_getinsider_command },
			{ "JOINGAME", _handle_joingame_command },
			{ "GAMEOPT", _handle_gameopt_command },
			{ "FINDUSER", _handle_finduser_command },
			{ "FINDUSEREX", _handle_finduserex_command },
			{ "PAGE", _handle_page_command },
			{ "STARTG", _handle_startg_command },
			{ "ADVERTR", _handle_advertr_command },
			{ "ADVERTC", _handle_advertc_command },
			{ "CHANCHK", _handle_chanchk_command },
			{ "GETBUDDY", _handle_getbuddy_command },
			{ "ADDBUDDY", _handle_addbuddy_command },
			{ "DELBUDDY", _handle_delbuddy_command },
			{ "TIME", _handle_time_command },
			{ "KICK", _handle_kick_command },
			{ "MODE", _handle_mode_command },
			{ "HOST", _handle_host_command },
			{ "INVMSG", _handle_invmsg_command },
			{ "INVDEL", _handle_invdel_command },
			{ "USERIP", _handle_userip_command },

			{ NULL, NULL }
		};

		extern int handle_wol_con_command(t_connection * conn, char const * command, int numparams, char ** params, char * text)
		{
			t_wol_command_table_row const *p;

			for (p = wol_con_command_table; p->wol_command_string != NULL; p++) {
				if (strcasecmp(command, p->wol_command_string) == 0) {
					if (p->wol_command_handler != NULL)
						return ((p->wol_command_handler)(conn, numparams, params, text));
				}
			}
			return -1;
		}

		extern int handle_wol_log_command(t_connection * conn, char const * command, int numparams, char ** params, char * text)
		{
			t_wol_command_table_row const *p;

			for (p = wol_log_command_table; p->wol_command_string != NULL; p++) {
				if (strcasecmp(command, p->wol_command_string) == 0) {
					if (p->wol_command_handler != NULL)
						return ((p->wol_command_handler)(conn, numparams, params, text));
				}
			}
			return -1;
		}

		static int handle_wol_authenticate(t_connection * conn, char const * passhash)
		{
			t_account * a;
			char const * tempapgar;
			char const * temphash;
			char const * username;
			char temp[MAX_IRC_MESSAGE_LEN];

			if (!conn) {
				ERROR0("got NULL connection");
				return 0;
			}
			if (!passhash) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL passhash");
				return 0;
			}
			username = conn_get_loggeduser(conn);
			if (!username) {
				/* redundant sanity check */
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn->protocol.loggeduser");
				return 0;
			}
			a = accountlist_find_account(username);
			if (!a) {
				/* FIXME: Send real error code */
				message_send_text(conn, message_type_notice, NULL, "Authentication failed.");
				return 0;
			}
			tempapgar = conn_wol_get_apgar(conn);
			temphash = account_get_wol_apgar(a);

			if (connlist_find_connection_by_account(a) && prefs_get_kick_old_login() == 0) {
				snprintf(temp, sizeof(temp), "%s :Account is already in use!", conn_get_loggeduser(conn));
				irc_send(conn, ERR_NICKNAMEINUSE, temp);
			}
			else if (account_get_auth_lock(a) == 1) {
				/* FIXME: Send real error code */
				message_send_text(conn, message_type_notice, NULL, "Authentication rejected (account is locked) ");
			}
			else {
				if (!temphash) {
					/* Account auto creating */
					account_set_wol_apgar(a, tempapgar);
					temphash = account_get_wol_apgar(a);
				}
				if ((tempapgar) && (temphash) && (std::strcmp(temphash, tempapgar) == 0)) {
					/* LOGIN is OK. We sends motd */
					conn_login(conn, a, username);
					conn_set_state(conn, conn_state_loggedin);
					irc_send_motd(conn);
				}
				else {
					irc_send(conn, RPL_BAD_LOGIN, ":You have specified an invalid password for that nickname."); /* bad APGAR */
					conn_increment_passfail_count(conn);
					//std::sprintf(temp,":Closing Link %s[Some.host]:(Password needed for that nickname.)",conn_get_loggeduser(conn));
					//message_send_text(conn,message_type_error,conn,temp);
				}
			}
			return 0;
		}

		extern int handle_wol_welcome(t_connection * conn)
		{
			char _temp[MAX_IRC_MESSAGE_LEN];

			/* This function need rewrite */
			conn_set_state(conn, conn_state_bot_password);

			if (conn_wol_get_apgar(conn)) {
				handle_wol_authenticate(conn, conn_wol_get_apgar(conn));
			}
			else {
				message_send_text(conn, message_type_notice, NULL, "No APGAR command received!");
			}

			return 0;
		}

		static int handle_wol_send_claninfo(t_connection * conn, t_clan * clan)
		{
			char _temp[MAX_IRC_MESSAGE_LEN];
			unsigned int clanid;
			const char * clantag;
			const char * clanname;

			std::memset(_temp, 0, sizeof(_temp));

			if (!conn) {
				ERROR0("got NULL connection");
				return -1;
			}

			if (clan) {
				clanid = clan_get_clanid(clan);
				clantag = clantag_to_str(clan_get_clantag(clan));
				clanname = clan_get_name(clan);
				snprintf(_temp, sizeof(_temp), "%u`%s`%s`0`0`1`0`0`0`0`0`0`0`x`x`x", clanid, clanname, clantag);
				irc_send(conn, RPL_BATTLECLAN, _temp);
			}
			else {
				snprintf(_temp, sizeof(_temp), ":ID does not exist");
				irc_send(conn, ERR_IDNOEXIST, _temp);
			}
			return 0;
		}

		/* Commands: */

		static int _handle_user_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			/**
			 *  In WOL isnt used USER command (only for backward compatibility)
			 *  RFC 2812 says:
			 *  USER <user> <mode> <unused> :<realname>
			 *
			 *  There is WOL imput expected:
			 *  USER UserName HostName irc.westwood.com :RealName
			 */

			char * user = NULL;
			t_account * a;

			user = (char *)conn_get_loggeduser(conn);

			if (conn_get_user(conn)) {
				/* FIXME: Send real ERROR code/message */
				irc_send(conn, ERR_ALREADYREGISTRED, ":You are already registred");
			}
			else {
				eventlog(eventlog_level_debug, __FUNCTION__, "[%d][** WOL **] got USER: user=\"%s\"", conn_get_socket(conn), user);

				a = accountlist_find_account(user);
				if (!a) {
					/* Auto-create account */
					t_account * tempacct;
					t_hash pass_hash;
					char * pass = xstrdup(conn_wol_get_apgar(conn)); /* FIXME: Do not use bnet passhash when we have wol passhash */

					for (unsigned j = 0; j < std::strlen(pass); j++)
					if (std::isupper((int)pass[j])) pass[j] = std::tolower((int)pass[j]);

					bnet_hash(&pass_hash, std::strlen(pass), pass);

					tempacct = accountlist_create_account(user, hash_get_str(pass_hash));
					if (!tempacct) {
						/* FIXME: Send real ERROR code/message */
						irc_send(conn, RPL_BAD_LOGIN, ":Account creating failed");
						return 0;
					}
					if (pass)
						xfree((void *)pass);

					conn_set_user(conn, user);
					conn_set_owner(conn, user);
					if (conn_get_loggeduser(conn))
						handle_wol_welcome(conn); /* only send the welcome if we have USER and NICK */
				}
				else {
					conn_set_user(conn, user);
					conn_set_owner(conn, user);
					if (conn_get_loggeduser(conn))
						handle_wol_welcome(conn); /* only send the welcome if we have USER and NICK */
				}
			}
			return 0;
		}

		static int _handle_pass_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			/**
			 * PASS is not used in WOL
			 * only for backward compatibility sent client PASS supersecret
			 * real password sent client by apgar command
			 */

			return 0;
		}

		static int _handle_privmsg_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			if ((numparams >= 1) && (text))
			{
				int i;
				char ** e;

				e = irc_get_listelems(params[0]);
				/* FIXME: support wildcards! */

				/* start amadeo: code was sent by some unkown fellow of pvpgn (maybe u wanna give us your name
				   for any credits), it adds nick-registration, i changed some things here and there... */
				for (i = 0; ((e) && (e[i])); i++) {
					if (strcasecmp(e[i], "matchbot") == 0) {
						/* Anongames WOL support */
						anongame_wol_privmsg(conn, numparams, params, text);
					}
					else if (conn_get_state(conn) == conn_state_loggedin) {
						if (e[i][0] == '#') {
							/* channel message */
							t_channel * channel;
							char msgtemp[MAX_IRC_MESSAGE_LEN];

							//PELISH: We does not support talk for not inside-channel clients now but in WOL is that feature not needed
							if (channel = conn_get_channel(conn)) {
								if ((std::strlen(text) >= 9) && (std::strncmp(text, "\001ACTION ", 8) == 0) && (text[std::strlen(text) - 1] == '\001')) {
									/* at least "\001ACTION \001" */
									/* it's a CTCP ACTION message */
									text = text + 8;
									text[std::strlen(text) - 1] = '\0';
									channel_message_send(channel, message_type_emote, conn, text);
								}
								else {
									if (text[0] == '/') {
										/* "/" commands (like "/help..." */
										handle_command(conn, text);
									}
									else {
										channel_message_log(channel, conn, 1, text);
										channel_message_send(channel, message_type_talk, conn, text);
									}
								}
							}
							else {
								snprintf(msgtemp, sizeof(msgtemp), "%s :No such channel", e[0]);
								irc_send(conn, ERR_NOSUCHCHANNEL, msgtemp);
							}
						}
						else {
							/* whisper */
							t_connection * user;

							if ((user = connlist_find_connection_by_accountname(e[i])))
							{
								message_send_text(user, message_type_whisper, conn, text);
							}
							else
							{
								irc_send(conn, ERR_NOSUCHNICK, ":No such user");
							}
						}
					}
				}
				if (e)
					irc_unget_listelems(e);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters");
			return 0;
		}

		struct gamelist_data {
			unsigned tcount, counter;
			t_connection *conn;
		};

		static int append_game_info(t_game* game, void* vdata)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			char temp_a[MAX_IRC_MESSAGE_LEN];
			gamelist_data* data = static_cast<gamelist_data*>(vdata);
			t_channel *  gamechannel;
			const char * gamename;
			char * topic = NULL;

			std::memset(temp, 0, sizeof(temp));
			std::memset(temp_a, 0, sizeof(temp_a));

			data->tcount++;

			if (game_get_status(game) != game_status_open) {
				eventlog(eventlog_level_debug, __FUNCTION__, "[%d] not listing because game is not open", conn_get_socket(data->conn));
				return 0;
			}
			if (game_get_clienttag(game) != conn_get_clienttag(data->conn)) {
				eventlog(eventlog_level_debug, __FUNCTION__, "[%d] not listing because game is for a different client", conn_get_socket(data->conn));
				return 0;
			}

			if (!(gamechannel = game_get_channel(game))) {
				ERROR0("game have no channel");
				return 0;
			}
			if (!(gamename = irc_convert_channel(gamechannel, data->conn))) {
				ERROR0("game have no name");
				return 0;
			}

			topic = channel_get_topic(channel_get_name(gamechannel));

			if (topic) {
				if (std::strlen(gamename) + 1 + 20 + 1 + 1 + strlen(topic) > MAX_IRC_MESSAGE_LEN) {
					WARN0("LISTREPLY length exceeded");
					return 0;
				}
			}
			else {
				if (std::strlen(gamename) + 1 + 20 + 1 + 1 > MAX_IRC_MESSAGE_LEN) {
					WARN0("LISTREPLY length exceeded");
					return 0;
				}
			}

			/***
			 * WOLv1:
			 * : 326 u #nick's_game 1 0 2 0 0 1122334455 128::
			 * WOLv2:
			 * : 326 u #nick's_game 1 0 21 0 16777216 1122334455 128::g040
			 * : 326 u #nick's_game 1 0 41 1 2048 1122334455 128::g12P25,2097731398,0,0,0,WATERF~3.YRM // anon_game
			 */
			/**
			 *  The layout of the game list entry is something like this:
			 *  #game_channel_name currentusers maxplayers gameType gameIsTournment gameExtension longIP LOCK::topic
			 */

			std::strcat(temp, gamename);
			std::strcat(temp, " ");

			snprintf(temp_a, sizeof(temp_a), "%u ", game_get_ref(game)); /* curent players */
			std::strcat(temp, temp_a);

			snprintf(temp_a, sizeof(temp_a), "%u ", game_get_maxplayers(game)); /* max players */
			std::strcat(temp, temp_a);

			snprintf(temp_a, sizeof(temp_a), "%u ", channel_wol_get_game_type(gamechannel)); /* game type */
			std::strcat(temp, temp_a);

			snprintf(temp_a, sizeof(temp_a), "%u ", (game_get_type(game) == game_type_ladder) ? 1 : 0); /* tournament */
			std::strcat(temp, temp_a);

			snprintf(temp_a, sizeof(temp_a), "%s ", channel_wol_get_game_extension(gamechannel));  /* game extension */
			std::strcat(temp, temp_a);

			snprintf(temp_a, sizeof(temp_a), "%u ", conn_get_addr(game_get_owner(game))); /* owner IP - FIXME: address translation here!! */
			std::strcat(temp, temp_a);

			if (std::strcmp(game_get_pass(game), "") == 0)
				std::strcat(temp, "128"); /* game is unloocked 128 == no_pass */
			else
				std::strcat(temp, "384"); /* game is loocked 384 == pass */

			std::strcat(temp, "::");

			if (topic) {
				snprintf(temp_a, sizeof(temp_a), "%s", topic);  /* topic */
				std::strcat(temp, temp_a);
			}

			data->counter++;
			irc_send(data->conn, RPL_GAME_CHANNEL, temp);

			return 0;
		}

		static int _handle_list_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			t_elem const * curr;

			irc_send(conn, RPL_LISTSTART, "Channel :Users Names"); /* backward compatibility */

			if ((numparams == 0) || ((numparams == 2) && (params[0]) && (params[1]) && (std::strcmp(params[0], params[1]) != 0))) {
				/**
				 * LIST all chat channels
				 * Emperor sends as params[0] == -1 if want QuickMatch channels too, 0 if not.
				 * This sends also NOX but we dunno why.
				 * DUNE 2000 use params[0] to determine channels by channeltype
				 */

				LIST_TRAVERSE_CONST(channellist(), curr) {
					t_channel const * channel = (const t_channel*)elem_get_data(curr);
					char const * tempname;

					tempname = irc_convert_channel(channel, conn);

					if ((tag_check_wolv1(conn_get_clienttag(conn))) && (std::strlen(tempname) > MAX_WOLV1_CHANNELNAME_LEN))
						continue;

					/* FIXME: Delete this if games are not in channels */
					if ((channel_wol_get_game_type(channel) != 0))
						continue;

					sprintf(temp, "%s %u ", tempname, channel_get_length(channel));

					if (channel_get_flags(channel) & channel_flags_permanent)
						std::strcat(temp, "1");  /* Official channel */
					else
						std::strcat(temp, "0");  /* User channel */

					if (tag_check_wolv1(conn_get_clienttag(conn)))
						std::strcat(temp, ":");     /* WOLv1 ends by ":" FIXME: Should be an TOPIC after ":"*/
					else
						std::strcat(temp, " 388");  /* WOLv2 ends by "388" */

					if (std::strlen(temp) > MAX_IRC_MESSAGE_LEN)
						WARN0("LISTREPLY length exceeded");

					irc_send(conn, RPL_CHANNEL, temp);
				}
			}
			/**
			*  Known channel game types:
			*  0 = Westwood Chat channels, 1 = Command & Conquer Win95 channels, 2 = Red Alert Win95 channels,
			*  3 = Red Alert Counterstrike channels, 4 = Red Alert Aftermath channels, 5 = CnC Sole Survivor channels,
			*  12 = C&C Renegade channels, 14 = Dune 2000 channels, 16 = Nox channels, 18 = Tiberian Sun channels,
			*  21 = Red Alert 1 v 3.03 channels, 31 = Emperor: Battle for Dune, 33 = Red Alert 2,
			*  37 = Nox Quest channels, 38,39,40 = Quickgame channels, 41 = Yuri's Revenge
			*/
			if ((numparams == 0) || ((numparams == 2) && (params[0]) && (params[1]) && (std::strcmp(params[0], params[1]) == 0))) {
				eventlog(eventlog_level_debug, __FUNCTION__, "[** WOL **] LIST [Game]");
				/* list games */
				struct gamelist_data data;
				data.tcount = 0;
				data.counter = 0;
				data.conn = conn;
				gamelist_traverse(&append_game_info, &data);
				DEBUG3("[%d] LIST sent %u of %u games", conn_get_socket(conn), data.counter, data.tcount);
			}
			irc_send(conn, RPL_LISTEND, ":End of LIST command");
			return 0;
		}

		static int _handle_quit_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			if (conn_get_channel(conn))
				conn_quit_channel(conn, text);

			irc_send(conn, RPL_QUIT, ":goodbye");

			conn_set_state(conn, conn_state_destroy);

			return 0;
		}

		static int _handle_names_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			t_channel * channel;

			if (numparams >= 1) {
				char ** e;
				char const * ircname;
				char const * verytemp;
				char temp[MAX_IRC_MESSAGE_LEN];
				int i;

				e = irc_get_listelems(params[0]);
				for (i = 0; ((e) && (e[i])); i++) {
					verytemp = irc_convert_ircname(e[i]);

					if (!verytemp)
						continue; /* something is wrong with the name ... */
					channel = channellist_find_channel_by_name(verytemp, NULL, NULL);
					if ((!channel) && (!(channel = conn_get_channel(conn))))
						continue; /* channel doesn't exist */
					irc_send_rpl_namreply(conn, channel);
				}
				if (e)
					irc_unget_listelems(e);
			}
			else if (numparams == 0) {
				irc_send_rpl_namreply(conn, NULL);
			}
			return 0;
		}

		static int _handle_part_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			t_game * game;

			conn_part_channel(conn);

			if ((game = conn_get_game(conn)) && (game_get_status(game) == game_status_open))
				conn_set_game(conn, NULL, NULL, NULL, game_type_none, 0);

			return 0;
		}

		/**
		*  Fallowing commands are only in Westwood Online protocol
		*/
		static int _handle_cvers_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			t_clienttag clienttag;

			/* Ignore command but set clienttag */

			/**
			*  Heres the imput expected:
			*  CVERS [oldvernum] [SKU]
			*
			*  SKU is specific number for any WOL client (Tiberian sun, RedAlert 2 etc.)
			*  This is the best way to set clienttag, because CVERS is the first command which
			*  client send to server.
			*/

			if (numparams == 2) {
				clienttag = tag_sku_to_uint(std::atoi(params[1]));
				if (clienttag != CLIENTTAG_WWOL_UINT)
					conn_set_clienttag(conn, clienttag);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "CVERS :Not enough parameters");
			return 0;
		}

		static int _handle_verchk_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			t_clienttag clienttag;

			/**
			*  Heres the imput expected:
			*  vercheck [SKU] [version]
			*
			*  Heres the output expected:
			*
			*  1) Update non-existant:
			*  :[servername] 379 [username] :none none none 1 [SKU] NONREQ
			*  2) Update existant:
			*  :[servername] 379 [username] :none none none [oldversnum] [SKU] REQ
			*/

			if (numparams == 2) {
				clienttag = tag_sku_to_uint(std::atoi(params[0]));
				if (clienttag != CLIENTTAG_WWOL_UINT)
					conn_set_clienttag(conn, clienttag);

				snprintf(temp, sizeof(temp), ":none none none 1 %s NONREQ", params[0]);
				eventlog(eventlog_level_debug, __FUNCTION__, "[** WOL **] VERCHK %s", temp);
				irc_send(conn, RPL_VERCHK_NONREQ, temp);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "VERCHK :Not enough parameters");
			return 0;
		}

		static int _handle_apgar_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char * apgar = NULL;

			if ((numparams >= 1) && (params[0])) {
				apgar = params[0];
				conn_wol_set_apgar(conn, apgar);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "APGAR :Not enough parameters");
			return 0;
		}

		static int _handle_serial_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			// Ignore command
			return 0;
		}

		static int _handle_squadinfo_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			t_clan * clan;

			if ((numparams >= 1) && (params[0])) {
				if (std::strcmp(params[0], "0") == 0) {
					/* 0 == claninfo for itself */
					clan = account_get_clan(conn_get_account(conn));
					handle_wol_send_claninfo(conn, clan);
				}
				else {
					/* claninfo for clanid (params[0]) */
					clan = clanlist_find_clan_by_clanid(std::atoi(params[0]));
					handle_wol_send_claninfo(conn, clan);
				}
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "SQUADINFO :Not enough parameters");
			return 0;
		}

		static int _handle_clanbyname_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			t_clan * clan;

			if ((numparams >= 1) && (params[0])) {
				clan = account_get_clan(accountlist_find_account(params[0]));
				handle_wol_send_claninfo(conn, clan);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "CLANBYNAME :Not enough parameters");
			return 0;
		}

		static int _handle_setopt_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char ** elems;

			/**
			*   This is option for enabling/disabling Page and Find user.
			*
			*   Heres the input expected:
			*   SETOPT 17,32
			*
			*   First parameter: 16 == FindDisabled 17 == FindEnabled
			*   Second parameter: 32 == PageDisabled 33 == PageEnabled
			*/

			if ((numparams >= 1) && (params[0])) {
				elems = irc_get_listelems(params[0]);

				if ((elems) && (elems[0]) && (elems[1])) {
					conn_wol_set_findme(conn, ((std::strcmp(elems[0], "17") == 0) ? true : false));
					conn_wol_set_pageme(conn, ((std::strcmp(elems[1], "33") == 0) ? true : false));
				}
				if (elems)
					irc_unget_listelems(elems);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "SETOPT :Not enough parameters");
			return 0;
		}

		static int _handle_setcodepage_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char * codepage = NULL;

			if ((numparams >= 1) && (params[0])) {
				codepage = params[0];
				conn_wol_set_codepage(conn, std::atoi(codepage));
				irc_send(conn, RPL_SET_CODEPAGE, codepage);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "SETCODEPAGE :Not enough parameters");
			return 0;
		}

		static int _handle_getcodepage_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			char _temp[MAX_IRC_MESSAGE_LEN];

			std::memset(temp, 0, sizeof(temp));
			std::memset(_temp, 0, sizeof(_temp));

			if ((numparams >= 1) && (params[0])) {
				int i;
				for (i = 0; i < numparams; i++) {
					t_connection * user;
					int codepage = 0;

					if (user = connlist_find_connection_by_accountname(params[i]))
						codepage = conn_wol_get_codepage(user);

					snprintf(_temp, sizeof(_temp), "%s`%u", params[i], codepage);
					std::strcat(temp, _temp);

					if (i < numparams - 1)
						std::strcat(temp, "`");
				}
				irc_send(conn, RPL_GET_CODEPAGE, temp);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "GETCODEPAGE :Not enough parameters");
			return 0;
		}

		static int _handle_setlocale_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			t_account * account = conn_get_account(conn);
			int locale;

			if ((numparams >= 1) && (params[0])) {
				locale = std::atoi(params[0]);
				account_set_locale(account, locale);
				irc_send(conn, RPL_SET_LOCALE, params[0]);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "SETLOCALE :Not enough parameters");
			return 0;
		}

		static int _handle_getlocale_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			char _temp[MAX_IRC_MESSAGE_LEN];

			std::memset(temp, 0, sizeof(temp));
			std::memset(_temp, 0, sizeof(_temp));

			if ((numparams >= 1) && (params[0])) {
				int i;
				for (i = 0; i < numparams; i++) {
					t_account * account;
					int locale = 0;

					if (account = accountlist_find_account(params[i]))
						locale = account_get_locale(account);

					snprintf(_temp, sizeof(_temp), "%s`%u", params[i], locale);
					std::strcat(temp, _temp);
					if (i < numparams - 1)
						std::strcat(temp, "`");
				}
				irc_send(conn, RPL_GET_LOCALE, temp);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "GETLOCALE :Not enough parameters");
			return 0;
		}

		static int _handle_getinsider_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char _temp[MAX_IRC_MESSAGE_LEN];

			/**
			 * Here is imput expected:
			 *   GETINSIDER [nickname]
			 * Here is output expected:
			 *   :[servername] 399 [nick] [nickname]`0
			 */

			std::memset(_temp, 0, sizeof(_temp));

			if ((numparams >= 1) && (params[0])) {
				snprintf(_temp, sizeof(_temp), "%s`%u", params[0], 0);
				irc_send(conn, RPL_GET_INSIDER, _temp);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "GETINSIDER :Not enough parameters");
			return 0;
		}

		static int _handle_joingame_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char _temp[MAX_IRC_MESSAGE_LEN];

			std::memset(_temp, 0, sizeof(_temp));

			/**
			*  Basically this has 2 modes, Join Game and Create Game output is pretty much
			*  the same...input and output of JOINGAME is listed below. By the way, there is a
			*  hack in here, for Red Alert 1, it use's JOINGAME for some reason to join a lobby channel.
			*
			*   Here is WOLv1 input expected:
			*   JOINGAME [#Game_channel_name] [MinPlayers] [MaxPlayers] [channelType] 1 1 [gameIsTournament]
			*   Knowed channelTypes (0-chat, 1-cnc, 2-ra1, 3-racs, 4-raam, 5-solsurv... listed in tag.cpp)
			*
			*   Here is WOLv2 input expected:
			*   JOINGAME [#Game_channel_name] [MinPlayers] [MaxPlayers] [channelType] unknown unknown [gameIsTournament] [gameExtension] [password_optional]
			*
			*   Heres the output expected:
			*   user!WWOL@hostname JOINGAME [MinPlayers] [MaxPlayers] [channelType] unknown clanID [longIP] [gameIsTournament] :[#Game_channel_name]
			*/
			if ((numparams == 2) || (numparams == 3)) {
				char ** e;

				e = irc_get_listelems(params[0]);
				if ((e) && (e[0])) {
					char const * gamename = irc_convert_ircname(e[0]);
					//    		char * old_channel_name = NULL;
					t_game * game;
					t_game_type gametype;
					t_channel * channel;
					t_channel * old_channel = conn_get_channel(conn);
					char gamepass[MAX_GAMEPASS_LEN];

					std::memset(gamepass, 0, sizeof(gamepass));

					if ((conn_get_clienttag(conn) == CLIENTTAG_REDALERT_UINT)
						&& (channellist_find_channel_by_name(gamename, NULL, NULL) != NULL)
						&& (gamelist_find_game_available(gamename, conn_get_clienttag(conn), game_type_all) == NULL)) {
						/* BUG in Red Alert 1 v3.03e - forwarding to _handle_join_command */
						DEBUG0("BUG in RA1 v3.03e - forwarding to _handle_join_command");
						_handle_join_command(conn, numparams, params, text);
						if (e)
							irc_unget_listelems(e);
						return 0;
					}

					if (!(gamename) || !(game = gamelist_find_game_available(gamename, conn_get_clienttag(conn), game_type_all))) {
						snprintf(_temp, sizeof(_temp), "%s :Game channel has closed", e[0]);
						irc_send(conn, ERR_GAMEHASCLOSED, _temp);
						if (e)
							irc_unget_listelems(e);
						return 0;
					}

					channel = game_get_channel(game);

					if (game_get_ref(game) == game_get_maxplayers(game)) {
						snprintf(_temp, sizeof(_temp), "%s :Channel is full", e[0]);
						irc_send(conn, ERR_CHANNELISFULL, _temp);
						if (e)
							irc_unget_listelems(e);
						return 0;
					}

					if (channel_check_banning(channel, conn)) {
						snprintf(_temp, sizeof(_temp), "%s :You are banned from that channel.", e[0]);
						irc_send(conn, ERR_BANNEDFROMCHAN, _temp);
						if (e)
							irc_unget_listelems(e);
						return 0;
					}

					if (std::strcmp(game_get_pass(game), "") != 0) {
						if ((numparams == 3) && (params[2]) && (std::strcmp(params[2], game_get_pass(game)) == 0)) {
							strcpy(gamepass, params[2]);
						}
						else {
							snprintf(_temp, sizeof(_temp), "%s :Bad password", e[0]);
							irc_send(conn, ERR_BADCHANNELKEY, _temp);
							if (e)
								irc_unget_listelems(e);
							return 0;
						}
					}

					gametype = game_get_type(game);

					if ((conn_set_game(conn, gamename, gamepass, "", gametype, 0)) < 0) {
						snprintf(_temp, sizeof(_temp), "%s :JOINGAME failed", e[0]);
						irc_send(conn, ERR_GAMEHASCLOSED, _temp);
					}
					else {
						/*conn_set_channel()*/
						channel = game_get_channel(game);
						conn_set_channel_var(conn, channel);
						channel_add_connection(channel, conn);
						channel = conn_get_channel(conn);

						if (channel != old_channel) {
							if (tag_check_wolv1(conn_get_clienttag(conn))) {
								/* WOLv1 JOINGAME message */
								std::sprintf(_temp, "%u %u %u 1 1 %u :%s", channel_get_min(channel), game_get_maxplayers(game), channel_wol_get_game_type(channel),
									((game_get_type(game) == game_type_ladder) ? 1 : 0), irc_convert_channel(channel, conn));
							}
							else {
								/* WOLv2 JOINGAME message with BATTLECLAN support */
								t_clan * clan = account_get_clan(conn_get_account(conn));
								unsigned int clanid = 0;

								if (clan)
									clanid = clan_get_clanid(clan);

								std::sprintf(_temp, "%u %u %u 1 %u %u %u :%s", channel_get_min(channel), game_get_maxplayers(game), channel_wol_get_game_type(channel),
									clanid, conn_get_addr(conn), ((game_get_type(game) == game_type_ladder) ? 1 : 0), irc_convert_channel(channel, conn));
							}

							channel_set_userflags(conn);
							/* we have to send the JOINGAME acknowledgement */
							channel_message_send(channel, message_wol_joingame, conn, _temp);

							irc_send_topic(conn, channel);

							irc_send_rpl_namreply(conn, channel);
						}
						else {
							snprintf(_temp, sizeof(_temp), "%s :JOINGAME failed", e[0]);
							irc_send(conn, ERR_GAMEHASCLOSED, _temp);
						}
					}
				}
				if (e)
					irc_unget_listelems(e);
				return 0;
			}
			else if ((numparams >= 7)) {
				char ** e;

				eventlog(eventlog_level_debug, __FUNCTION__, "[** WOL **] JOINGAME: * Create * (%s, %s)",
					params[0], params[1]);

				if ((numparams == 7)) {
					/* WOLv1 JOINGAME Create */
					snprintf(_temp, sizeof(_temp), "%s %s %s %s 0 %s :%s", params[1], params[2], params[3], params[4], params[6], params[0]);
				}
				/* WOLv2 JOINGAME Create */
				else if ((numparams >= 8)) {
					t_clan * clan = account_get_clan(conn_get_account(conn));
					unsigned int clanid = 0;

					if (clan)
						clanid = clan_get_clanid(clan);
					snprintf(_temp, sizeof(_temp), "%s %s %s %s %u %u %s :%s", params[1], params[2], params[3], params[4], clanid, conn_get_addr(conn), params[6], params[0]);
				}
				eventlog(eventlog_level_debug, __FUNCTION__, "[** WOL **] JOINGAME [Game Options] (%s)", _temp);

				e = irc_get_listelems(params[0]);
				if ((e) && (e[0])) {
					char const * gamename = irc_convert_ircname(e[0]);
					t_game_type gametype;
					char gamepass[MAX_GAMEPASS_LEN];

					std::memset(gamepass, 0, sizeof(gamepass));

					if (std::strcmp(params[6], "1") == 0)
						gametype = game_type_ladder;
					else
						gametype = game_type_ffa;
					//    		    gametype = game_type_none;

					if ((numparams >= 8) && (params[8])) {
						strcpy(gamepass, params[8]);
					}

					if ((!(gamename)) || ((conn_set_game(conn, gamename, gamepass, "", gametype, 0))<0)) {
						irc_send(conn, ERR_NOSUCHCHANNEL, ":JOINGAME failed"); /* FIXME: be more precise; what is the real error code for that? */
					}
					else {
						t_game * game = conn_get_game(conn);
						t_channel * channel = channel_create(gamename, gamename, NULL, 0, 1, 1, prefs_get_chanlog(), NULL, NULL, (prefs_get_maxusers_per_channel() > 0) ? prefs_get_maxusers_per_channel() : -1, 0, 0, 0, NULL);
						game_set_channel(game, channel);
						conn_set_channel_var(conn, channel);
						channel_add_connection(channel, conn);
						channel_set_userflags(conn);

						game_set_maxplayers(game, std::atoi(params[2]));

						channel_set_min(channel, std::atoi(params[1]));
						// HACK: Currently, this is the best way to set the channel game type...
						channel_wol_set_game_type(channel, std::atoi(params[3]));

						if (params[7])
							channel_wol_set_game_extension(channel, params[7]);
						else
							channel_wol_set_game_extension(channel, "0");

						// we have to send the JOINGAME acknowledgement
						message_send_text(conn, message_wol_joingame, conn, _temp);
						irc_send_topic(conn, channel);
						irc_send_rpl_namreply(conn, channel);
					}
				}
				if (e)
					irc_unget_listelems(e);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "JOINGAME :Not enough parameters");
			return 0;
		}

		static int _handle_gameopt_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];

			/**
			*  Basically this has 2 modes as like in PRIVMSG - whisper and talk. What is in
			*  text is pretty much unknown, we just dump this to the client to deal with...
			*
			*	Heres the output expected (when gameopt is channel talk):
			*	user!WWOL@hostname GAMEOPT #game_channel_name :gameOptions
			*
			*	Heres the output expected (when gameopt is whispered):
			*	user!WWOL@hostname GAMEOPT sender_nick_name :gameOptions
			*/

			if ((numparams >= 1) && (params[0]) && (text)) {
				int i;
				char ** e;
				t_connection * user;
				t_channel * channel;

				e = irc_get_listelems(params[0]);
				/* FIXME: support wildcards! */

				for (i = 0; ((e) && (e[i])); i++) {
					if (e[i][0] == '#') {
						/* channel gameopt */
						if (channel = conn_get_channel(conn)) {
							channel_message_send(channel, message_type_gameopt_talk, conn, text);
						}
						else {
							snprintf(temp, sizeof(temp), "%s :No such channel", params[0]);
							irc_send(conn, ERR_NOSUCHCHANNEL, temp);
						}
					}
					else {
						/* user gameopt */
						if ((user = connlist_find_connection_by_accountname(e[i]))) {
							message_send_text(user, message_type_gameopt_whisper, conn, text);
						}
						else {
							snprintf(temp, sizeof(temp), "%s :No such nick", e[i]);
							irc_send(conn, ERR_NOSUCHNICK, temp);
						}
					}
				}
				if (e)
					irc_unget_listelems(e);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "GAMEOPT :Not enough parameters");
			return 0;
		}

		static int _handle_finduser_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char _temp[MAX_IRC_MESSAGE_LEN];
			char const * wolname = NULL;

			std::memset(_temp, 0, sizeof(_temp));

			if ((numparams >= 1) && (params[0])) {
				t_connection * user;

				if ((user = connlist_find_connection_by_accountname(params[0])) && (conn_wol_get_findme(user))) {
					wolname = irc_convert_channel(conn_get_channel(user), conn);
					snprintf(_temp, sizeof(_temp), "0 :%s", wolname); /* User found in channel wolname */
				}
				else
					snprintf(_temp, sizeof(_temp), "1 :"); /* user not loged or have not allowed find */

				irc_send(conn, RPL_FIND_USER, _temp);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "FINDUSER :Not enough parameters");
			return 0;
		}

		static int _handle_finduserex_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char _temp[MAX_IRC_MESSAGE_LEN];
			char const * wolname = NULL;

			std::memset(_temp, 0, sizeof(_temp));

			if ((numparams >= 1) && (params[0])) {
				t_connection * user;

				if ((user = connlist_find_connection_by_accountname(params[0])) && (conn_wol_get_findme(user))) {
					wolname = irc_convert_channel(conn_get_channel(user), conn);
					snprintf(_temp, sizeof(_temp), "0 :%s,0", wolname); /* User found in channel wolname */
				}
				else
					snprintf(_temp, sizeof(_temp), "1 :"); /* user not loged or have not allowed find */

				irc_send(conn, RPL_FIND_USER_EX, _temp);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "FINDUSEREX :Not enough parameters");
			return 0;;
		}

		static int _handle_page_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char _temp[MAX_IRC_MESSAGE_LEN];
			bool paged = false;

			if ((numparams >= 1) && (params[0]) && (text)) {
				t_connection * user;

				if (std::strcmp(params[0], "0") == 0) {
					/* PAGE for MY BATTLECLAN */
					t_clan * clan = account_get_clan(conn_get_account(conn));

					if ((clan) && (clan_send_message_to_online_members(clan, message_type_page, conn, text) >= 1))
						paged = true;
				}
				else if ((user = connlist_find_connection_by_accountname(params[0])) && (conn_wol_get_pageme(user))) {
					message_send_text(user, message_type_page, conn, text);
					paged = true;
				}

				if (paged)
					snprintf(_temp, sizeof(_temp), "0 :"); /* Page was succesfull */
				else
					snprintf(_temp, sizeof(_temp), "1 :"); /* User not loged in or have not allowed page */

				irc_send(conn, RPL_PAGE, _temp);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "PAGE :Not enough parameters");
			return 0;
		}

		static int _handle_startg_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			char _temp_a[MAX_IRC_MESSAGE_LEN];
			t_channel * channel;
			t_game * game;

			/**
			*  Imput expected:
			*   STARTG [channel_name] [nick1](,nick2_optional)
			*
			*  Heres the output expected (this can have up-to 8 entries (ie 8 players):
			*  (we are assuming for this example that user1 is the game owner)
			*
			*   WOLv1:
			*   :user1!WWOL@hostname STARTG u :owner_ip gameNumber time_t
			*
			*   WOLv2:
			*   :user1!WWOL@hostname STARTG u :user1 xxx.xxx.xxx.xxx user2 xxx.xxx.xxx.xxx :gameNumber time_t
			*/

			if ((numparams >= 2) && (params[1])) {
				int i;
				char ** e;

				std::memset(temp, 0, sizeof(temp));

				e = irc_get_listelems(params[1]);
				/* FIXME: support wildcards! */

				if (!(game = conn_get_game(conn))) {
					ERROR0("conn has not game");
					return 0;
				}

				std::strcat(temp, ":");

				if (tag_check_wolv1(conn_get_clienttag(conn))) {
					t_connection * user;
					channel = conn_get_channel(conn);

					for (user = channel_get_first(channel); user; user = channel_get_next()) {
						char const * name = conn_get_chatname(user);
						if (std::strcmp(conn_get_chatname(conn), name) != 0) {
							snprintf(temp, sizeof(temp), "%s ", addr_num_to_ip_str(conn_get_addr(game_get_owner(game))));
						}
					}
				}
				else {
					for (i = 0; ((e) && (e[i])); i++) {
						t_connection * user;
						const char * addr = NULL;

						if ((user = connlist_find_connection_by_accountname(e[i]))) {
							addr = addr_num_to_ip_str(conn_get_addr(user));
						}
						snprintf(_temp_a, sizeof(_temp_a), "%s %s ", e[i], addr);
						std::strcat(temp, _temp_a);
					}
					std::strcat(temp, ":");
				}

				game_set_status(game, game_status_started);

				snprintf(_temp_a, sizeof(_temp_a), "%u %u", game_get_id(game), game_get_start_time(game));
				std::strcat(temp, _temp_a);

				for (i = 0; ((e) && (e[i])); i++) {
					t_connection * user;
					if ((user = connlist_find_connection_by_accountname(e[i]))) {
						message_send_text(user, message_wol_start_game, conn, temp);
					}
				}

				if (e)
					irc_unget_listelems(e);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "STARTG :Not enough parameters");
			return 0;
		}

		static int _handle_advertr_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];

			std::memset(temp, 0, sizeof(temp));

			/**
			*  Heres the imput expected
			*  ADVERTR [channel]
			*
			*  Heres the output expected
			*  :[servername] ADVERTR 5 [channel]
			*/

			if ((numparams >= 1) && (params[0])) {
				snprintf(temp, sizeof(temp), "5 %s", params[0]);
				message_send_text(conn, message_wol_advertr, conn, temp);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "ADVERTR :Not enough parameters");
			return 0;
		}

		static int _handle_advertc_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			/* FIXME: Not implemented yet */
			return 0;
		}

		static int _handle_chanchk_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			t_channel * channel;

			std::memset(temp, 0, sizeof(temp));

			/**
			*  Heres the imput expected
			*  chanchk [channel]
			*
			*  Heres the output expected
			*  :[servername] CHANCHK [channel]
			*/

			if ((numparams >= 1) && (params[0])) {
				if ((channel = channellist_find_channel_by_name(irc_convert_ircname(params[0]), NULL, NULL))) {
					std::strcat(temp, params[0]);
					message_send_text(conn, message_wol_chanchk, conn, temp);
				}
				else {
					/* FIXME: This is not dumped from original servers... this is probably wrong */
					snprintf(temp, sizeof(temp), "%s :No such channel", params[0]);
					irc_send(conn, ERR_NOSUCHCHANNEL, temp);
				}
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "CHANCHK :Not enough parameters");
			return 0;
		}

		static int _handle_getbuddy_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			char _temp[MAX_IRC_MESSAGE_LEN];
			char const * friend_name;
			t_account * my_acc;
			t_account * friend_acc;
			t_list * flist;
			t_friend * fr;
			int num;
			unsigned int uid;
			int i;

			std::memset(temp, 0, sizeof(temp));
			std::memset(_temp, 0, sizeof(_temp));

			/**
			*  Heres the output expected
			*  :[servername] 333 [user] [buddy_name1]`[buddy_name2]`
			*
			*  Without names:
			*  :[servername] 333 [user]
			*/

			my_acc = conn_get_account(conn);
			num = account_get_friendcount(my_acc);

			flist = account_get_friends(my_acc);

			if (flist != NULL) {
				for (i = 0; i < num; i++) {
					if ((!(uid = account_get_friend(my_acc, i))) || (!(fr = friendlist_find_uid(flist, uid)))) {
						eventlog(eventlog_level_error, __FUNCTION__, "friend uid in list");
						continue;
					}
					friend_acc = friend_get_account(fr);
					friend_name = account_get_name(friend_acc);
					snprintf(_temp, sizeof(_temp), "%s`", friend_name);
					std::strcat(temp, _temp);
				}
			}
			irc_send(conn, RPL_GET_BUDDY, temp);
			return 0;
		}

		static int _handle_addbuddy_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			t_account * my_acc;
			t_account * friend_acc;

			std::memset(temp, 0, sizeof(temp));

			/**
			*  Heres the imput expected
			*  ADDBUDDY [buddy_name]
			*
			*  Heres the output expected
			*  :[servername] 334 [user] [buddy_name]
			*/

			if ((numparams >= 1) && (params[0])) {
				my_acc = conn_get_account(conn);
				if (friend_acc = accountlist_find_account(params[0])) {
					account_add_friend(my_acc, friend_acc);
					/* FIXME: Check if add friend is done if not then send right message */

					snprintf(temp, sizeof(temp), "%s", params[0]);
					irc_send(conn, RPL_ADD_BUDDY, temp);
				}
				else {
					snprintf(temp, sizeof(temp), "%s :No such nick", params[0]);
					irc_send(conn, ERR_NOSUCHNICK, temp);
					/* NOTE: this is not dumped from WOL, this not shows message
					  but in Emperor doesnt gives name to list, in RA2 have no efect */
				}
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "ADDBUDDY :Not enough parameters");
			return 0;
		}

		static int _handle_delbuddy_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			char const * friend_name;
			t_account * my_acc;
			int num;

			std::memset(temp, 0, sizeof(temp));

			/**
			*  Heres the imput expected
			*  DELBUDDY [buddy_name]
			*
			*  Heres the output expected
			*  :[servername] 335 [user] [buddy_name]
			*/

			if ((numparams >= 1) && (params[0])) {
				friend_name = params[0];
				my_acc = conn_get_account(conn);

				num = account_remove_friend2(my_acc, friend_name);

				/**
				*  FIXME: Check if remove friend is done if not then send right message
				*  Btw I dont know another then RPL_DEL_BUDDY message yet.
				*/

				snprintf(temp, sizeof(temp), "%s", friend_name);
				irc_send(conn, RPL_DEL_BUDDY, temp);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "DELBUDDY :Not enough parameters");
			return 0;
		}

		static int _handle_host_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			t_connection * user;

			std::memset(temp, 0, sizeof(temp));

			if ((numparams >= 1) && (params[0])) {
				if ((user = connlist_find_connection_by_accountname(params[0]))) {
					snprintf(temp, sizeof(temp), ": %s", text);
					message_send_text(user, message_type_host, conn, temp);
				}
				else {
					snprintf(temp, sizeof(temp), "%s :No such nick", params[0]);
					irc_send(conn, ERR_NOSUCHNICK, temp);
				}
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "HOST :Not enough parameters");
			return 0;
		}

		static int _handle_invmsg_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			char ** e;
			t_connection * user;
			int i;

			/**
			 *  Here is the imput expected:
			 *  INVMSG [channel] [unknown] [invited],[invited2_optional]
			 *  [unknown] can be 1 or 2
			 *
			 *  Here is the output expected:
			 *  :user!WWOL@hostname INVMSG [invited] [channel] [unknown]
			 */

			if ((numparams >= 3) && (params[0]) && (params[1]) && (params[2])) {
				std::memset(temp, 0, sizeof(temp));
				e = irc_get_listelems(params[2]);

				for (i = 0; ((e) && (e[i])); i++) {
					if ((user = connlist_find_connection_by_accountname(e[i]))) {
						snprintf(temp, sizeof(temp), "%s %s", params[0], params[1]);
						/* FIXME: set user to linvitelist! */
						message_send_text(user, message_type_invmsg, conn, temp);
					}
				}
			}
			else {
				irc_send(conn, ERR_NEEDMOREPARAMS, "INVMSG :Not enough parameters");
			}
			return 0;
		}

		static int _handle_invdel_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			/* FIXME: Not implemented yet */
			return 0;
		}

		static int _handle_userip_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			t_connection * user;
			const char * addr = NULL;

			std::memset(temp, 0, sizeof(temp));

			if ((numparams >= 1) && (params[0])) {
				if ((user = connlist_find_connection_by_accountname(params[0]))) {
					addr = addr_num_to_ip_str(conn_get_addr(user));
					//FIXME: We are not sure of first parameter. It can be also nickname of command sender
					snprintf(temp, sizeof(temp), "%s %s", params[0], addr);
					message_send_text(conn, message_wol_userip, conn, temp);
				}
				else {
					snprintf(temp, sizeof(temp), "%s :No such nick", params[0]);
					irc_send(conn, ERR_NOSUCHNICK, temp);
				}
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "USERIP :Not enough parameters");
			return 0;
		}

		/**
		 * LADDER Server commands:
		 */
		static int _ladder_send(t_connection * conn, char const * command)
		{
			t_packet * p;
			char data[MAX_IRC_MESSAGE_LEN + 1];
			unsigned len;

			p = packet_create(packet_class_raw);

			if (command)
				len = (std::strlen(command) + 6);

			if (len > MAX_IRC_MESSAGE_LEN) {
				eventlog(eventlog_level_error, __FUNCTION__, "message to send is too large (%u bytes)", len);
				return -1;
			}
			else {
				std::sprintf(data, "\r\n\r\n\r\n%s", command);
				//std::sprintf(data,"%s",command);
			}

			packet_set_size(p, 0);
			packet_append_data(p, data, len);
			eventlog(eventlog_level_debug, __FUNCTION__, "[%d] sent \"%s\"", conn_get_socket(conn), data);
			conn_push_outqueue(conn, p);
			packet_del_ref(p);

			/* In ladder server we must destroy connection after send packet */
			conn_set_state(conn, conn_state_destroy);

			return 0;
		}

		static int _ladder_is_integer(char * test)
		{
			for (char const* ptr = test; *ptr; ++ptr) {
				if (!std::isdigit(*ptr)) {
					return 0; /* Is not integer */
				}
				else
					return 1; /* Is integer */
			}
		}

		static int _handle_listsearch_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char ** e;
			int i = 0;
			unsigned rank = 0;
			unsigned points = 0;
			unsigned wins = 0;
			unsigned losses = 0;
			unsigned disconnects = 0;
			char temp[MAX_IRC_MESSAGE_LEN];
			char data[MAX_IRC_MESSAGE_LEN];
			t_account * cl_account;
			t_clienttag cl_tag;
			t_ladder_id id = ladder_id_solo;

			std::memset(data, 0, sizeof(data));

			if ((numparams >= 1) && (params[0]) && (text)) {
				cl_tag = tag_sku_to_uint(std::atoi(params[0]));

				e = irc_get_ladderelems(text);

				//TIMESTAMP 1147130452
				//TOTAL 12033
				//NOTFOUND
				// TIMESTAMP 1188740860
				// 'TOTAL 27466
				/*    std::sprintf(temp,"TIMESTAMP %lu\n", std::time(NULL));
					std::strcat(data,temp);
					std::sprintf(temp,"TOTAL 88\n");
					std::strcat(data,temp);*/

				for (i = 0; e[i]; i++) {
					/* Now we have in e[i] names */
					if (e[i] && (std::strcmp(e[i], ":") != 0)) {
						cl_account = accountlist_find_account(e[i]);
						if (cl_account && cl_tag && (rank = account_get_ladder_rank(cl_account, cl_tag, id))) {
							points = account_get_ladder_points(cl_account, cl_tag, id);
							wins = account_get_ladder_wins(cl_account, cl_tag, id);
							losses = account_get_ladder_losses(cl_account, cl_tag, id);
							disconnects = account_get_ladder_disconnects(cl_account, cl_tag, id);
							std::sprintf(temp, "%u  %s  %u  %u  %u  0  %u\r\n", rank, e[i], points, wins, losses, disconnects);
							std::strcat(data, temp);
						}
						else
							std::strcat(data, "NOTFOUND\r\n");
					}
				}

				if (e)
					irc_unget_ladderelems(e);

				_ladder_send(conn, data);
			}
			else {
				WARN0("Not enough parameters");
				conn_set_state(conn, conn_state_destroy);
				return 0;
			}
			return 0;
		}

		static int _handle_rungsearch_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			char data[MAX_IRC_MESSAGE_LEN];
			unsigned rank = 0;
			unsigned points = 0;
			unsigned wins = 0;
			unsigned losses = 0;
			unsigned disconnects = 0;
			t_account * cl_account;
			t_clienttag cl_tag;
			t_ladder_id id = ladder_id_solo;

			std::memset(data, 0, sizeof(data));

			if ((numparams >= 4) && (params[0]) && (params[1]) && (params[3])) {
				cl_tag = tag_sku_to_uint(std::atoi(params[3]));

				if ((cl_tag != CLIENTTAG_TIBERNSUN_UINT) && (cl_tag != CLIENTTAG_TIBSUNXP_UINT)
					&& (cl_tag != CLIENTTAG_REDALERT2_UINT) && (cl_tag != CLIENTTAG_YURISREV_UINT)) {
					// PELISH: We are not supporting ladders for all WOL clients yet
					std::strcat(data, "\r\n");
					_ladder_send(conn, data);
					DEBUG1("Wants rung search for SKU %s", params[3]);
					return 0;
				}

				if (_ladder_is_integer(params[0]) == 0) {
					/* rungsearch want to line for one player (nick is in params[0]) */
					cl_account = accountlist_find_account(params[0]);
					if (cl_account && cl_tag && (rank = account_get_ladder_rank(cl_account, cl_tag, id))) {
						points = account_get_ladder_points(cl_account, cl_tag, id);
						wins = account_get_ladder_wins(cl_account, cl_tag, id);
						losses = account_get_ladder_losses(cl_account, cl_tag, id);
						disconnects = account_get_ladder_disconnects(cl_account, cl_tag, id);
						std::sprintf(temp, "%u  %s  %u  %u  %u  0  %u\r\n", rank, params[0], points, wins, losses, disconnects);
					}
					else
						std::sprintf(temp, "\r\n");
					_ladder_send(conn, temp);
				}
				else {
					/* Standard RUNG search */
					int i;
					unsigned start = std::atoi(params[0]);
					unsigned count = std::atoi(params[1]);

					eventlog(eventlog_level_debug, __FUNCTION__, "Start(%u) Count(%u)", start, count);

					LadderList* ladderList = NULL;

					ladderList = ladders.getLadderList(LadderKey(id, cl_tag, ladder_sort_default, ladder_time_default));
					for (i = start; i < start + count; i++) {
						const LadderReferencedObject* referencedObject = NULL;
						cl_account = NULL;
						if (((referencedObject = ladderList->getReferencedObject(i))) && (cl_account = referencedObject->getAccount())) {
							rank = account_get_ladder_rank(cl_account, cl_tag, id);
							points = account_get_ladder_points(cl_account, cl_tag, id);
							wins = account_get_ladder_wins(cl_account, cl_tag, id);
							losses = account_get_ladder_losses(cl_account, cl_tag, id);
							disconnects = account_get_ladder_disconnects(cl_account, cl_tag, id);
							std::sprintf(temp, "%u  %s  %u  %u  %u  0  %u\r\n", rank, account_get_name(cl_account), points, wins, losses, disconnects);
							std::strcat(data, temp);
						}
						else {
							std::strcat(data, "\r\n");
							_ladder_send(conn, data);
							return 0;
						}
					}
					_ladder_send(conn, data);
				}
			}
			else {
				WARN0("Not enough parameters");
				conn_set_state(conn, conn_state_destroy);
				return 0;
			}
			return 0;
		}

		static int _handle_highscore_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			/*    char ** e;
				int i = 0;
				unsigned rank = 2;
				unsigned points = 258;
				unsigned wins = 0;
				unsigned losses = 0;
				unsigned unknown = 0;  // Here is nuber before Nick and Honor Badges in Yuri (1-999)
				unsigned disconnects = 0;
				char temp[MAX_IRC_MESSAGE_LEN];
				char data[MAX_IRC_MESSAGE_LEN];
				t_account * cl_account;
				t_clienttag cltag;

				std::memset(temp,0,sizeof(temp));
				std::memset(data,0,sizeof(data));

				if (text)
				e = irc_get_ladderelems(text);

				if (params[0])
				cltag = tag_sku_to_uint(std::atoi(params[0]));

				for (i=0;e[i];i++) {
				if (e[i] && (std::strcmp(e[i], ":") != 0)) {
				cl_account = accountlist_find_account(e[i]);
				if (cl_account) {
				wins = account_get_normal_wins(cl_account, cltag);
				losses = account_get_normal_losses(cl_account, cltag);
				disconnects = account_get_normal_disconnects(cl_account, cltag);
				std::sprintf(temp,"%u  %s  %u  %u  %u  %u  %u\r\n",rank+i,e[i],points,wins,losses,unknown,disconnects);
				std::strcat(data,temp);
				}
				else
				std::strcat(data,"NOTFOUND\r\n");
				}
				}

				if (e)
				irc_unget_ladderelems(e);

				_ladder_send(conn,data);
				*/
			conn_set_state(conn, conn_state_destroy);
			return 0;
		}

	}

}
