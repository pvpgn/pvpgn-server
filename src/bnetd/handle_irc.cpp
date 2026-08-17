/*
 * Copyright (C) 2001  Marco Ziech (mmz@gmx.net)
 * Copyright (C) 2005  Bryan Biedenkapp (gatekeep@gmail.com)
 * Copyright (C) 2006,2007  Pelish (pelish@gmail.com)
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
#include "handle_irc.h"

#include <cstring>
#include <cctype>
#include <cstdlib>
#include <string>

#include <nonstd/optional.hpp>

#include "compat/localtime_s.h"
#include "compat/strcasecmp.h"
#include "common/irc_protocol.h"
#include "common/eventlog.h"
#include "common/bnethash.h"
#include "common/tag.h"
#include "common/util.h"
#include "common/list.h"
#include "common/addr.h"
#include "common/xstring.h"

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
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		typedef int(*t_irc_command)(t_connection * conn, int numparams, char ** params, char * text);

		typedef struct {
			const char     * irc_command_string;
			t_irc_command    irc_command_handler;
		} t_irc_command_table_row;

		static int _handle_user_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_pass_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_privmsg_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_notice_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_quit_command(t_connection * conn, int numparams, char ** params, char * text);

		static int _handle_who_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_list_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_names_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_userhost_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_ison_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_whois_command(t_connection * conn, int numparams, char ** params, char * text);
		static int _handle_part_command(t_connection * conn, int numparams, char ** params, char * text);


		/* state "connected" handlers */
		static const t_irc_command_table_row irc_con_command_table[] =
		{
			{ "NICK", _handle_nick_command },
			{ "USER", _handle_user_command },
			{ "PING", _handle_ping_command },
			{ "PONG", _handle_pong_command },
			{ "PASS", _handle_pass_command },
			{ "PRIVMSG", _handle_privmsg_command },
			{ "NOTICE", _handle_notice_command },
			{ "QUIT", _handle_quit_command },

			{ NULL, NULL }
		};

		/* state "logged in" handlers */
		static const t_irc_command_table_row irc_log_command_table[] =
		{
			{ "WHO", _handle_who_command },
			{ "LIST", _handle_list_command },
			{ "TOPIC", _handle_topic_command },
			{ "JOIN", _handle_join_command },
			{ "NAMES", _handle_names_command },
			{ "MODE", _handle_mode_command },
			{ "USERHOST", _handle_userhost_command },
			{ "ISON", _handle_ison_command },
			{ "WHOIS", _handle_whois_command },
			{ "PART", _handle_part_command },
			{ "KICK", _handle_kick_command },
			{ "TIME", _handle_time_command },

			{ NULL, NULL }
		};


		extern int handle_irc_con_command(t_connection * conn, char const * command, int numparams, char ** params, char * text)
		{
			t_irc_command_table_row const *p;

			for (p = irc_con_command_table; p->irc_command_string != NULL; p++) {
				if (strcasecmp(command, p->irc_command_string) == 0) {
					if (p->irc_command_handler != NULL)
						return ((p->irc_command_handler)(conn, numparams, params, text));
				}
			}
			return -1;
		}

		extern int handle_irc_log_command(t_connection * conn, char const * command, int numparams, char ** params, char * text)
		{
			t_irc_command_table_row const *p;

			for (p = irc_log_command_table; p->irc_command_string != NULL; p++) {
				if (strcasecmp(command, p->irc_command_string) == 0) {
					if (p->irc_command_handler != NULL)
						return ((p->irc_command_handler)(conn, numparams, params, text));
				}
			}
			return -1;
		}

		extern int handle_irc_welcome(t_connection * conn)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			
			char const * tempname;

			if (!conn) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			tempname = conn_get_loggeduser(conn);

			if ((34 + std::strlen(tempname) + 1) <= MAX_IRC_MESSAGE_LEN)
				std::sprintf(temp, ":Welcome to the %s IRC Network %s", prefs_get_irc_network_name(), tempname);
			else
				std::sprintf(temp, ":Maximum length exceeded");
			irc_send(conn, RPL_WELCOME, temp);

			if ((14 + std::strlen(server_get_hostname()) + 10 + std::strlen(PVPGN_SOFTWARE " " PVPGN_VERSION) + 1) <= MAX_IRC_MESSAGE_LEN)
				std::sprintf(temp, ":Your host is %s, running " PVPGN_SOFTWARE " " PVPGN_VERSION, server_get_hostname());
			else
				std::sprintf(temp, ":Maximum length exceeded");
			irc_send(conn, RPL_YOURHOST, temp);

			char temptimestr[256] = {};
			{
				std::time_t temptime = server_get_starttime(); /* FIXME: This should be build time */;
				struct std::tm calendartime = {};
				if (pvpgn::localtime_s(&temptime, &calendartime) == nullptr || std::strftime(temptimestr, sizeof(temptimestr), "%c", &calendartime) == 0)
				{
					std::strcpy(temptimestr, "?");
				}
			}
			if ((25 + std::strlen(temptimestr) + 1) <= MAX_IRC_MESSAGE_LEN)
				std::sprintf(temp, ":This server was created %s", temptimestr);
			else
				std::sprintf(temp, ":Maximum length exceeded");
			irc_send(conn, RPL_CREATED, temp);

			/* we don't give mode information on MYINFO we give it on ISUPPORT */
			if ((std::strlen(server_get_hostname()) + 7 + std::strlen(PVPGN_SOFTWARE " " PVPGN_VERSION) + 9 + 1) <= MAX_IRC_MESSAGE_LEN)
				std::sprintf(temp, "%s " PVPGN_SOFTWARE " " PVPGN_VERSION " - -", server_get_hostname());
			else
				std::sprintf(temp, ":Maximum length exceeded");
			irc_send(conn, RPL_MYINFO, temp);

			std::sprintf(temp, "NICKLEN=%d TOPICLEN=%d CHANNELLEN=%d PREFIX=%s CHANTYPES=" CHANNEL_TYPE " NETWORK=%s IRCD=" PVPGN_SOFTWARE,
				MAX_CHARNAME_LEN, MAX_TOPIC_LEN, MAX_CHANNELNAME_LEN, CHANNEL_PREFIX, prefs_get_irc_network_name());

			irc_send(conn, RPL_ISUPPORT, temp);

			irc_send_motd(conn);

			message_send_text(conn, message_type_notice, NULL, "This is an experimental service");

			conn_set_state(conn, conn_state_bot_password);

			if (conn_get_ircpass(conn)) {
				message_send_text(conn, message_type_notice, NULL, "Trying to authenticate with PASS ...");
				irc_authenticate(conn, conn_get_ircpass(conn));
			}
			else {
				message_send_text(conn, message_type_notice, NULL, "No PASS command received. Please identify yourself by /msg NICKSERV identify <password>.");
			}
			return 0;
		}

		static int _handle_user_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			/* RFC 2812 says: */
			/* <user> <mode> <unused> :<realname>*/
			/* ircII and X-Chat say: */
			/* mz SHODAN localhost :Marco Ziech */
			/* BitchX says: */
			/* mz +iws mz :Marco Ziech */
			/* Don't bother with, params 1 and 2 anymore they don't contain what they should. */
			char * user = NULL;
			char * realname = NULL;

			if ((numparams >= 3) && (params[0]) && (text)) {
				user = params[0];
				realname = text;

				if (conn_get_user(conn)) {
					irc_send(conn, ERR_ALREADYREGISTRED, ":You are already registered");
				}
				else {
					eventlog(eventlog_level_debug, __FUNCTION__, "[{}] got USER: user=\"{}\" realname=\"{}\"", conn_get_socket(conn), user, realname);
					conn_set_user(conn, user);
					conn_set_owner(conn, realname);
					if (conn_get_loggeduser(conn))
						handle_irc_welcome(conn); /* only send the welcome if we have USER and NICK */
				}
			}
			else {
				irc_send(conn, ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
			}
			return 0;
		}

		static int _handle_pass_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			if ((!conn_get_ircpass(conn)) && (conn_get_state(conn) == conn_state_bot_username)) {
				t_hash h;

				if (numparams >= 1) {
					bnet_hash(&h, std::strlen(params[0]), params[0]);
					conn_set_ircpass(conn, hash_get_str(h));
				}
				else
					irc_send(conn, ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
			}
			else {
				irc_send(conn, ERR_ALREADYREGISTRED, ":Unauthorized command (already registered)");
			}
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

				/* start amadeo: code was sent by some unknown fellow of pvpgn (maybe u wanna give us your name
				   for any credits), it adds nick-registration, i changed some things here and there... */
				for (i = 0; ((e) && (e[i])); i++) {
					if (strcasecmp(e[i], "NICKSERV") == 0) {
						char * pass;

						pass = std::strchr(text, ' ');
						if (pass)
							*pass++ = '\0';

						if (strcasecmp(text, "identify") == 0) {
							switch (conn_get_state(conn)) {
							case conn_state_bot_password:
							{
															if (pass) {
																t_hash h;

																strtolower(pass);
																bnet_hash(&h, std::strlen(pass), pass);
																irc_authenticate(conn, hash_get_str(h));
															}
															else {
																message_send_text(conn, message_type_notice, NULL, "Syntax: IDENTIFY <password> (max 16 characters)");
															}
															break;
							}
							case conn_state_loggedin:
							{
														message_send_text(conn, message_type_notice, NULL, "You don't need to IDENTIFY");
														break;
							}
							default:;
								eventlog(eventlog_level_trace, __FUNCTION__, "got /msg in unexpected connection state ({})", conn_state_get_str(conn_get_state(conn)));
							}
						}
						else if (strcasecmp(text, "register") == 0) {
							t_hash       passhash;
							t_account  * temp;
							char       * username = (char *)conn_get_loggeduser(conn);

							if (account_check_name(username)<0) {
								message_send_text(conn, message_type_error, conn, "Account name contains invalid symbol!");
								break;
							}

							if (!prefs_get_allow_new_accounts()){
								message_send_text(conn, message_type_error, conn, "Account creation is not allowed");
								break;
							}

							if (!pass || pass[0] == '\0' || (std::strlen(pass)>16)) {
								message_send_text(conn, message_type_error, conn, "Syntax: REGISTER <password> (max 16 characters)");
								break;
							}

							strtolower(pass);

							bnet_hash(&passhash, std::strlen(pass), pass);

							message_send_text(conn, message_type_info, conn, std::string("Trying to create account \"" + std::string(username) + "\" with password \"" + std::string(pass) + "\"").c_str());

							temp = accountlist_create_account(username, hash_get_str(passhash));
							if (!temp) {
								message_send_text(conn, message_type_error, conn, "Failed to create account!");
								eventlog(eventlog_level_debug, __FUNCTION__, "[{}] account \"{}\" not created (failed)", conn_get_socket(conn), username);
								conn_unget_chatname(conn, username);
								break;
							}

							message_send_text(conn, message_type_info, conn, std::string("Account #" + std::to_string(account_get_uid(temp)) + " created."));
							eventlog(eventlog_level_debug, __FUNCTION__, "[{}] account \"{}\" created", conn_get_socket(conn), username);
							conn_unget_chatname(conn, username);
						}
						else {
							message_send_text(conn, message_type_notice, nullptr, "Invalid arguments for NICKSERV");
							message_send_text(conn, message_type_notice, nullptr, std::string(":Unrecognized command \"" + std::string(text) + "\"").c_str());
						}
					}
					else if (conn_get_state(conn) == conn_state_loggedin) {
						if (e[i][0] == '#') {
							/* channel message */
							t_channel * channel;

							if ((channel = channellist_find_channel_by_name(irc_convert_ircname(e[i]), NULL, NULL))) {
								if ((std::strlen(text) >= 9) && (std::strncmp(text, "\001ACTION ", 8) == 0) && (text[std::strlen(text) - 1] == '\001')) {
									/* at least "\001ACTION \001" */
									/* it's a CTCP ACTION message */
									text = text + 8;
									text[std::strlen(text) - 1] = '\0';
									channel_message_send(channel, message_type_emote, conn, text);
								}
								else {
									channel_message_log(channel, conn, 1, text);
									channel_message_send(channel, message_type_talk, conn, text);
								}
							}
							else {
								irc_send(conn, ERR_NOSUCHCHANNEL, ":No such channel");
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

		static int _handle_notice_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			if ((numparams >= 1) && (text)) {
				int i;
				char ** e;

				e = irc_get_listelems(params[0]);
				/* FIXME: support wildcards! */

				for (i = 0; ((e) && (e[i])); i++) {
					if (conn_get_state(conn) == conn_state_loggedin) {
						t_connection * user;

						if ((user = connlist_find_connection_by_accountname(e[i]))) {
							message_send_text(user, message_type_notice, conn, text);
						}
						else {
							irc_send(conn, ERR_NOSUCHNICK, ":No such user");
						}
					}
				}
				if (e)
					irc_unget_listelems(e);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "NOTICE :Not enough parameters");
			return 0;
		}

		static int _handle_who_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			if (numparams >= 1) {
				int i;
				char ** e;

				e = irc_get_listelems(params[0]);
				for (i = 0; ((e) && (e[i])); i++) {
					irc_who(conn, e[i]);
				}
				irc_send(conn, RPL_ENDOFWHO, ":End of WHO list"); /* RFC2812 only requires this to be sent if a list of names was given. Undernet seems to always send it, so do we :) */
				if (e)
					irc_unget_listelems(e);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "WHO :Not enough parameters");
			return 0;
		}

		static int _handle_list_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			irc_send(conn, RPL_LISTSTART, "Channel :Users Names"); /* backward compatibility */

			if (numparams == 0)
			{
				t_elem const * curr;
				LIST_TRAVERSE_CONST(channellist(), curr)
				{
					t_channel const * channel = (const t_channel*)elem_get_data(curr);
					char const * tempname = irc_convert_channel(channel, conn);
					nonstd::optional<std::string> topic = channel_get_topic(channel);

					/* FIXME: AARON: only list channels like in /channels command */
					std::string tmp = fmt::format("{} {} :{}", tempname, channel_get_length(channel), topic.value_or(""));

					if (tmp.length() > MAX_IRC_MESSAGE_LEN)
						eventlog(eventlog_level_warn, __FUNCTION__, "LISTREPLY length exceeded");

					irc_send(conn, RPL_LIST, tmp.c_str());
				}
			}
			else if (numparams >= 1)
			{
				int i;
				char ** e;

				e = irc_get_listelems(params[0]);
				/* FIXME: support wildcards! */

				for (i = 0; ((e) && (e[i])); i++)
				{
					char const * verytemp = irc_convert_ircname(e[i]);
					if (!verytemp)
						continue; /* something is wrong with the name ... */

					t_channel const * channel = channellist_find_channel_by_name(verytemp, NULL, NULL);
					if (!channel)
						continue; /* channel doesn't exist */

					nonstd::optional<std::string> topic = channel_get_topic(channel);
					char const * tempname = irc_convert_channel(channel, conn);

					std::string tmp = fmt::format("{} {} :{}", tempname, channel_get_length(channel), topic.value_or(""));

					if (tmp.length() > MAX_IRC_MESSAGE_LEN)
						eventlog(eventlog_level_warn, __FUNCTION__, "LISTREPLY length exceeded");

					irc_send(conn, RPL_LIST, tmp.c_str());
				}

				if (e)
					irc_unget_listelems(e);
			}

			irc_send(conn, RPL_LISTEND, ":End of LIST command");

			return 0;
		}

		static int _handle_names_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			t_channel * channel;

			if (numparams >= 1) {
				char ** e;
				char const * verytemp;
				int i;

				e = irc_get_listelems(params[0]);
				for (i = 0; ((e) && (e[i])); i++) {
					verytemp = irc_convert_ircname(e[i]);

					if (!verytemp)
						continue; /* something is wrong with the name ... */
					channel = channellist_find_channel_by_name(verytemp, NULL, NULL);
					if (!channel)
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

		static int _handle_userhost_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			/* FIXME: Send RPL_USERHOST */
			return 0;
		}

		static int _handle_quit_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			conn_quit_channel(conn, text);
			conn_set_state(conn, conn_state_destroy);
			return 0;
		}

		static int _handle_ison_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			char first = 1;

			if (numparams >= 1)
			{
				int i;

				temp[0] = '\0';
				for (i = 0; (i < numparams && (params) && (params[i])); i++)
				{
					if (connlist_find_connection_by_accountname(params[i]))
					{
						std::snprintf(temp, sizeof temp, "%s%s", first ? ":" : " ", params[i]);
						first = 0;
					}
				}
				irc_send(conn, RPL_ISON, temp);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "ISON :Not enough parameters");
			return 0;
		}

		static int _handle_whois_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			std::string tmp;

			if (numparams >= 1)
			{
				int i;
				char ** e = irc_get_listelems(params[0]);
				t_connection * c;
				t_channel * chan;

				for (i = 0; ((e) && (e[i])); i++)
				{
					if ((c = connlist_find_connection_by_accountname(e[i])))
					{
						if (prefs_get_hide_addr() && !(account_get_command_groups(conn_get_account(conn)) & command_get_group("/admin-addr")))
							tmp = std::string(e[i]) + " " + std::string(clienttag_uint_to_str(conn_get_clienttag(c))) + " hidden * :PvPGN user";
						else
							tmp = std::string(e[i]) + " " + std::string(clienttag_uint_to_str(conn_get_clienttag(c))) + " " + std::string(addr_num_to_ip_str(conn_get_addr(c))) + " * :PvPGN user";
						irc_send(conn, RPL_WHOISUSER, tmp.c_str());

						if ((chan = conn_get_channel(conn)))
						{
							std::string flg;
							auto flags = conn_get_flags(c);

							if (flags & MF_BLIZZARD)
								flg = '@';
							else if ((flags & MF_BNET) || (flags & MF_GAVEL))
								flg = '%';
							else if (flags & MF_VOICE)
								flg = '+';
							else
								flg = ' ';

							tmp = std::string(e[i]) + " :" + flg + std::string(irc_convert_channel(chan, conn));
							irc_send(conn, RPL_WHOISCHANNELS, tmp.c_str());
						}

					}
					else
						irc_send(conn, ERR_NOSUCHNICK, ":No such nick/channel");

				}
				irc_send(conn, RPL_ENDOFWHOIS, ":End of /WHOIS list");
				if (e)
					irc_unget_listelems(e);
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "WHOIS :Not enough parameters");
			return 0;
		}

		static int _handle_part_command(t_connection * conn, int numparams, char ** params, char * text)
		{
			conn_part_channel(conn);
			return 0;
		}

	}

}
