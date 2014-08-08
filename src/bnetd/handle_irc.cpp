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

#include "compat/strcasecmp.h"
#include "common/irc_protocol.h"
#include "common/eventlog.h"
#include "common/bnethash.h"
#include "common/tag.h"
#include "common/util.h"
#include "common/list.h"
#include "common/addr.h"

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
			std::time_t temptime;
			char const * tempname;
			char const * temptimestr;

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

			if ((14 + std::strlen(server_get_hostname()) + 10 + std::strlen(PVPGN_SOFTWARE" "PVPGN_VERSION) + 1) <= MAX_IRC_MESSAGE_LEN)
				std::sprintf(temp, ":Your host is %s, running "PVPGN_SOFTWARE" "PVPGN_VERSION, server_get_hostname());
			else
				std::sprintf(temp, ":Maximum length exceeded");
			irc_send(conn, RPL_YOURHOST, temp);

			temptime = server_get_starttime(); /* FIXME: This should be build time */
			temptimestr = std::ctime(&temptime);
			if ((25 + std::strlen(temptimestr) + 1) <= MAX_IRC_MESSAGE_LEN)
				std::sprintf(temp, ":This server was created %s", temptimestr); /* FIXME: is ctime() portable? */
			else
				std::sprintf(temp, ":Maximum length exceeded");
			irc_send(conn, RPL_CREATED, temp);

			/* we don't give mode information on MYINFO we give it on ISUPPORT */
			if ((std::strlen(server_get_hostname()) + 7 + std::strlen(PVPGN_SOFTWARE" "PVPGN_VERSION) + 9 + 1) <= MAX_IRC_MESSAGE_LEN)
				std::sprintf(temp, "%s "PVPGN_SOFTWARE" "PVPGN_VERSION" - -", server_get_hostname());
			else
				std::sprintf(temp, ":Maximum length exceeded");
			irc_send(conn, RPL_MYINFO, temp);

			std::sprintf(temp, "NICKLEN=%d TOPICLEN=%d CHANNELLEN=%d PREFIX=%s CHANTYPES="CHANNEL_TYPE" NETWORK=%s IRCD="PVPGN_SOFTWARE,
				MAX_CHARNAME_LEN, MAX_TOPIC_LEN, MAX_CHANNELNAME_LEN, CHANNEL_PREFIX, prefs_get_irc_network_name());

			if ((std::strlen(temp)) <= MAX_IRC_MESSAGE_LEN)
				irc_send(conn, RPL_ISUPPORT, temp);
			else {
				std::sprintf(temp, ":Maximum length exceeded");
				irc_send(conn, RPL_ISUPPORT, temp);
			}

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
					irc_send(conn, ERR_ALREADYREGISTRED, ":You are already registred");
				}
				else {
					eventlog(eventlog_level_debug, __FUNCTION__, "[%d] got USER: user=\"%s\" realname=\"%s\"", conn_get_socket(conn), user, realname);
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

				/* start amadeo: code was sent by some unkown fellow of pvpgn (maybe u wanna give us your name
				   for any credits), it adds nick-registration, i changed some things here and there... */
				for (i = 0; ((e) && (e[i])); i++) {
					if (strcasecmp(e[i], "NICKSERV") == 0) {
						char * pass;
						char * p;

						pass = std::strchr(text, ' ');
						if (pass)
							*pass++ = '\0';

						if (strcasecmp(text, "identify") == 0) {
							switch (conn_get_state(conn)) {
							case conn_state_bot_password:
							{
															if (pass) {
																t_hash h;

																for (p = pass; *p; p++)
																if (std::isupper((int)*p)) *p = std::tolower(*p);
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
								eventlog(eventlog_level_trace, __FUNCTION__, "got /msg in unexpected connection state (%s)", conn_state_get_str(conn_get_state(conn)));
							}
						}
						else if (strcasecmp(text, "register") == 0) {
							unsigned int j;
							t_hash       passhash;
							t_account  * temp;
							char         msgtemp[MAX_IRC_MESSAGE_LEN];
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

							for (j = 0; j < std::strlen(pass); j++)
							if (std::isupper((int)pass[j])) pass[j] = std::tolower((int)pass[j]);

							bnet_hash(&passhash, std::strlen(pass), pass);

							snprintf(msgtemp, sizeof(msgtemp), "Trying to create account \"%s\" with password \"%s\"", username, pass);
							message_send_text(conn, message_type_info, conn, msgtemp);

							temp = accountlist_create_account(username, hash_get_str(passhash));
							if (!temp) {
								message_send_text(conn, message_type_error, conn, "Failed to create account!");
								eventlog(eventlog_level_debug, __FUNCTION__, "[%d] account \"%s\" not created (failed)", conn_get_socket(conn), username);
								conn_unget_chatname(conn, username);
								break;
							}

							snprintf(msgtemp, sizeof(msgtemp), "Account "UID_FORMAT" created.", account_get_uid(temp));
							message_send_text(conn, message_type_info, conn, msgtemp);
							eventlog(eventlog_level_debug, __FUNCTION__, "[%d] account \"%s\" created", conn_get_socket(conn), username);
							conn_unget_chatname(conn, username);
						}
						else {
							char tmp[MAX_IRC_MESSAGE_LEN + 1];
							message_send_text(conn, message_type_notice, NULL, "Invalid arguments for NICKSERV");
							snprintf(tmp, sizeof(tmp), ":Unrecognized command \"%s\"", text);
							message_send_text(conn, message_type_notice, NULL, tmp);
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
			char temp[MAX_IRC_MESSAGE_LEN];

			irc_send(conn, RPL_LISTSTART, "Channel :Users Names"); /* backward compatibility */

			if (numparams == 0) {
				t_elem const * curr;

				LIST_TRAVERSE_CONST(channellist(), curr)
				{
					t_channel const * channel = (const t_channel*)elem_get_data(curr);
					char const * tempname;
					char * topic = channel_get_topic(channel_get_name(channel));

					tempname = irc_convert_channel(channel, conn);

					/* FIXME: AARON: only list channels like in /channels command */
					if (topic) {
						if (std::strlen(tempname) + 1 + 20 + 1 + 1 + std::strlen(topic) < MAX_IRC_MESSAGE_LEN)
							snprintf(temp, sizeof(temp), "%s %u :%s", tempname, channel_get_length(channel), topic);
						else
							eventlog(eventlog_level_warn, __FUNCTION__, "LISTREPLY length exceeded");
					}
					else {
						if (std::strlen(tempname) + 1 + 20 + 1 + 1 < MAX_IRC_MESSAGE_LEN)
							snprintf(temp, sizeof(temp), "%s %u :", tempname, channel_get_length(channel));
						else
							eventlog(eventlog_level_warn, __FUNCTION__, "LISTREPLY length exceeded");
					}
					irc_send(conn, RPL_LIST, temp);
				}
			}
			else if (numparams >= 1) {
				int i;
				char ** e;

				e = irc_get_listelems(params[0]);
				/* FIXME: support wildcards! */

				for (i = 0; ((e) && (e[i])); i++) {
					t_channel const * channel;
					char const * verytemp; /* another good example for creative naming conventions :) */
					char const * tempname;
					char * topic;

					verytemp = irc_convert_ircname(e[i]);
					if (!verytemp)
						continue; /* something is wrong with the name ... */
					channel = channellist_find_channel_by_name(verytemp, NULL, NULL);
					if (!channel)
						continue; /* channel doesn't exist */

					topic = channel_get_topic(channel_get_name(channel));
					tempname = irc_convert_channel(channel, conn);

					if (topic) {
						if (std::strlen(tempname) + 1 + 20 + 1 + 1 + std::strlen(topic) < MAX_IRC_MESSAGE_LEN)
							snprintf(temp, sizeof(temp), "%s %u :%s", tempname, channel_get_length(channel), topic);
						else
							eventlog(eventlog_level_warn, __FUNCTION__, "LISTREPLY length exceeded");
					}
					else {
						if (std::strlen(tempname) + 1 + 20 + 1 + 1 < MAX_IRC_MESSAGE_LEN)
							snprintf(temp, sizeof(temp), "%s %u :", tempname, channel_get_length(channel));
						else
							eventlog(eventlog_level_warn, __FUNCTION__, "LISTREPLY length exceeded");
					}
					irc_send(conn, RPL_LIST, temp);
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

			if (numparams >= 1) {
				int i;

				temp[0] = '\0';
				for (i = 0; (i < numparams && (params) && (params[i])); i++) {
					if (connlist_find_connection_by_accountname(params[i])) {
						if (first)
							std::strcat(temp, ":");
						else
							std::strcat(temp, " ");
						std::strcat(temp, params[i]);
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
			char temp[MAX_IRC_MESSAGE_LEN];
			char temp2[MAX_IRC_MESSAGE_LEN];
			if (numparams >= 1) {
				int i;
				char ** e;
				t_connection * c;
				t_channel * chan;

				temp[0] = '\0';
				temp2[0] = '\0';
				e = irc_get_listelems(params[0]);
				for (i = 0; ((e) && (e[i])); i++) {
					if ((c = connlist_find_connection_by_accountname(e[i]))) {
						if (prefs_get_hide_addr() && !(account_get_command_groups(conn_get_account(conn)) & command_get_group("/admin-addr")))
							snprintf(temp, sizeof(temp), "%s %s hidden * :%s", e[i], clienttag_uint_to_str(conn_get_clienttag(c)), "PvPGN user");
						else
							snprintf(temp, sizeof(temp), "%s %s %s * :%s", e[i], clienttag_uint_to_str(conn_get_clienttag(c)), addr_num_to_ip_str(conn_get_addr(c)), "PvPGN user");
						irc_send(conn, RPL_WHOISUSER, temp);

						if ((chan = conn_get_channel(conn))) {
							char flg;
							unsigned int flags;

							flags = conn_get_flags(c);

							if (flags & MF_BLIZZARD)
								flg = '@';
							else if ((flags & MF_BNET) || (flags & MF_GAVEL))
								flg = '%';
							else if (flags & MF_VOICE)
								flg = '+';
							else flg = ' ';
							snprintf(temp2, sizeof(temp2), "%s :%c%s", e[i], flg, irc_convert_channel(chan, conn));
							irc_send(conn, RPL_WHOISCHANNELS, temp2);
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
