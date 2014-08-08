/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2000,2001  Marco Ziech (mmz@gmx.net)
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
#define CHANNEL_INTERNAL_ACCESS
#include "common/setup_before.h"
#include "channel.h"

#include <cstring>
#include <cerrno>
#include <cstdlib>

#include "compat/strdup.h"
#include "compat/strcasecmp.h"
#include "compat/snprintf.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "common/util.h"
#include "common/token.h"
#include "common/tag.h"
#include "common/xalloc.h"

#include "connection.h"
#include "message.h"
#include "account.h"
#include "account_wrap.h"
#include "prefs.h"
#include "irc.h"
#include "i18n.h"
#include "common/setup_after.h"

#ifdef WITH_LUA
#include "luainterface.h"
#endif

namespace pvpgn
{

	namespace bnetd
	{

		static t_list * channellist_head = NULL;

		static t_channelmember * memberlist_curr = NULL;
		static int totalcount = 0;


		static int channellist_load_permanent(char const * filename);
		static t_channel * channellist_find_channel_by_fullname(char const * name);
		static char * channel_format_name(char const * sname, char const * country, char const * realmname, unsigned int id);

		extern int channel_set_userflags(t_connection * c);

		extern t_channel * channel_create(char const * fullname, char const * shortname, t_clienttag clienttag, int permflag, int botflag, int operflag, int logflag, char const * country, char const * realmname, int maxmembers, int moderated, int clanflag, int autoname, t_list * channellist)
		{
			t_channel * channel;

			if (!fullname)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL fullname");
				return NULL;
			}
			if (fullname[0] == '\0')
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got empty fullname");
				return NULL;
			}
			if (shortname && shortname[0] == '\0')
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got empty shortname");
				return NULL;
			}

			/* non-permanent already checks for this in conn_set_channel */
			if (permflag)
			{
				if ((channel = channellist_find_channel_by_fullname(fullname)))
				{
					if ((channel_get_clienttag(channel)) && (clienttag) && (channel_get_clienttag(channel) == clienttag))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not create duplicate permanent channel (fullname \"%s\")", fullname);
						return NULL;
					}
					else if (((channel->flags & channel_flags_allowbots) != (botflag ? channel_flags_allowbots : 0)) ||
						((channel->flags & channel_flags_allowopers) != (operflag ? channel_flags_allowopers : 0)) ||
						(channel->maxmembers != maxmembers) ||
						((channel->flags & channel_flags_moderated) != (moderated ? channel_flags_moderated : 0)) ||
						(channel->logname && logflag == 0) || (!(channel->logname) && logflag == 1))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "channel parameters do not match for \"%s\" and \"%s\"", fullname, channel->name);
						return NULL;
					}
				}
			}

			channel = (t_channel*)xmalloc(sizeof(t_channel));

			if (permflag)
			{
				channel->flags = channel_flags_public;
				if (clienttag && maxmembers != -1) /* approximation.. we want things like "Starcraft USA-1" */
					channel->flags |= channel_flags_system;
			}
			else
				channel->flags = channel_flags_none;

			if (moderated)
				channel->flags |= channel_flags_moderated;

			if (shortname && (!strcasecmp(shortname, CHANNEL_NAME_KICKED)
				|| !strcasecmp(shortname, CHANNEL_NAME_BANNED)))
				channel->flags |= channel_flags_thevoid;

			eventlog(eventlog_level_debug, __FUNCTION__, "creating new channel \"%s\" shortname=%s%s%s clienttag=%s%s%s country=%s%s%s realm=%s%s%s", fullname,
				shortname ? "\"" : "(", /* all this is doing is printing the name in quotes else "none" in parens */
				shortname ? shortname : "none",
				shortname ? "\"" : ")",
				clienttag ? "\"" : "(",
				clienttag ? clienttag_uint_to_str(clienttag) : "none",
				clienttag ? "\"" : ")",
				country ? "\"" : "(",
				country ? country : "none",
				country ? "\"" : ")",
				realmname ? "\"" : "(",
				realmname ? realmname : "none",
				realmname ? "\"" : ")");


			channel->name = xstrdup(fullname);

			if (!shortname)
				channel->shortname = NULL;
			else
				channel->shortname = xstrdup(shortname);

			channel->clienttag = clienttag;

			if (country)
				channel->country = xstrdup(country);
			else
				channel->country = NULL;

			if (realmname)
				channel->realmname = xstrdup(realmname);
			else
				channel->realmname = NULL;

			channel->banlist = list_create();

			totalcount++;
			if (totalcount == 0) /* if we wrap (yeah right), don't use id 0 */
				totalcount = 1;
			channel->id = totalcount;
			channel->maxmembers = maxmembers;
			channel->currmembers = 0;
			channel->memberlist = NULL;

			if (permflag) channel->flags |= channel_flags_permanent;
			if (botflag)  channel->flags |= channel_flags_allowbots;
			if (operflag) channel->flags |= channel_flags_allowopers;
			if (clanflag) channel->flags |= channel_flags_clan;
			if (autoname) channel->flags |= channel_flags_autoname;

			if (logflag)
			{
				std::time_t      now;
				struct std::tm * tmnow;
				char        dstr[64];
				char        timetemp[CHANLOG_TIME_MAXLEN];

				now = std::time(NULL);

				if (!(tmnow = std::localtime(&now)))
					dstr[0] = '\0';
				else
					std::sprintf(dstr, "%04d%02d%02d%02d%02d%02d",
					1900 + tmnow->tm_year,
					tmnow->tm_mon + 1,
					tmnow->tm_mday,
					tmnow->tm_hour,
					tmnow->tm_min,
					tmnow->tm_sec);

				channel->logname = (char*)xmalloc(std::strlen(prefs_get_chanlogdir()) + 9 + std::strlen(dstr) + 1 + 6 + 1); /* dir + "/chanlog-" + dstr + "-" + id + NUL */
				std::sprintf(channel->logname, "%s/chanlog-%s-%06u", prefs_get_chanlogdir(), dstr, channel->id);

				if (!(channel->log = std::fopen(channel->logname, "w")))
					eventlog(eventlog_level_error, __FUNCTION__, "could not open channel log \"%s\" for writing (std::fopen: %s)", channel->logname, std::strerror(errno));
				else
				{
					std::fprintf(channel->log, "name=\"%s\"\n", channel->name);
					if (channel->shortname)
						std::fprintf(channel->log, "shortname=\"%s\"\n", channel->shortname);
					else
						std::fprintf(channel->log, "shortname=none\n");
					std::fprintf(channel->log, "permanent=\"%s\"\n", (channel->flags & channel_flags_permanent) ? "true" : "false");
					std::fprintf(channel->log, "allowbotse=\"%s\"\n", (channel->flags & channel_flags_allowbots) ? "true" : "false");
					std::fprintf(channel->log, "allowopers=\"%s\"\n", (channel->flags & channel_flags_allowopers) ? "true" : "false");
					if (channel->clienttag)
						std::fprintf(channel->log, "clienttag=\"%s\"\n", clienttag_uint_to_str(channel->clienttag));
					else
						std::fprintf(channel->log, "clienttag=none\n");

					if (tmnow)
						std::strftime(timetemp, sizeof(timetemp), CHANLOG_TIME_FORMAT, tmnow);
					else
						std::strcpy(timetemp, "?");
					std::fprintf(channel->log, "created=\"%s\"\n\n", timetemp);
					std::fflush(channel->log);
				}
			}
			else
			{
				channel->logname = NULL;
				channel->log = NULL;
			}

			channel->gameType = 0;
			channel->gameExtension = NULL;

			if (channellist)
				list_append_data(channellist, channel);
			else
				DEBUG0("channel was not added into any channellist");

			eventlog(eventlog_level_debug, __FUNCTION__, "channel created successfully");
			return channel;
		}

		extern t_channel * channel_create(char const * fullname, char const * shortname, t_clienttag clienttag, int permflag, int botflag, int operflag, int logflag, char const * country, char const * realmname, int maxmembers, int moderated, int clanflag, int autoname)
		{
			return channel_create(fullname, shortname, clienttag, permflag, botflag, operflag, logflag, country, realmname, maxmembers, moderated, clanflag, autoname, channellist_head);
		}

		extern int channel_destroy(t_channel * channel, t_elem ** curr)
		{
			t_elem * ban;

			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return -1;
			}

			if (channel->memberlist)
			{
				eventlog(eventlog_level_debug, __FUNCTION__, "channel is not empty, deferring");
				channel->flags &= ~channel_flags_permanent; /* make it go away when the last person leaves */
				return -1;
			}

			if (list_remove_data(channellist_head, channel, curr) < 0)
			{
				//        eventlog(eventlog_level_error,__FUNCTION__,"could not remove item from list");
				eventlog(eventlog_level_info, __FUNCTION__, "channel was not removed from any list");
				//        return -1;
			}

			eventlog(eventlog_level_info, __FUNCTION__, "destroying channel \"%s\"", channel->name);

			if (channel->gameExtension)
				xfree(channel->gameExtension);

			LIST_TRAVERSE(channel->banlist, ban)
			{
				char const * banned;

				if (!(banned = (char*)elem_get_data(ban)))
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL name in banlist");
				else
					xfree((void *)banned); /* avoid warning */
				if (list_remove_elem(channel->banlist, &ban) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "unable to remove item from list");
			}
			list_destroy(channel->banlist);

			if (channel->log)
			{
				std::time_t      now;
				struct std::tm * tmnow;
				char        timetemp[CHANLOG_TIME_MAXLEN];

				now = std::time(NULL);
				if ((!(tmnow = std::localtime(&now))))
					std::strcpy(timetemp, "?");
				else
					std::strftime(timetemp, sizeof(timetemp), CHANLOG_TIME_FORMAT, tmnow);
				std::fprintf(channel->log, "\ndestroyed=\"%s\"\n", timetemp);

				if (std::fclose(channel->log) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not close channel log \"%s\" after writing (std::fclose: %s)", channel->logname, std::strerror(errno));
			}

			if (channel->logname)
				xfree((void *)channel->logname); /* avoid warning */

			if (channel->country)
				xfree((void *)channel->country); /* avoid warning */

			if (channel->realmname)
				xfree((void *)channel->realmname); /* avoid warning */

			if (channel->shortname)
				xfree((void *)channel->shortname); /* avoid warning */

			xfree((void *)channel->name); /* avoid warning */

			xfree(channel);

			return 0;
		}


		extern char const * channel_get_name(t_channel const * channel)
		{
			if (!channel)
			{
				eventlog(eventlog_level_warn, __FUNCTION__, "got NULL channel");
				return "";
			}

			return channel->name;
		}


		extern t_clienttag channel_get_clienttag(t_channel const * channel)
		{
			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return 0;
			}

			return channel->clienttag;
		}


		extern unsigned channel_get_flags(t_channel const * channel)
		{
			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return channel_flags_none;
			}

			return channel->flags;
		}

		extern int channel_set_flags(t_channel * channel, unsigned flags)
		{
			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return -1;
			}
			channel->flags = flags;

			return 0;
		}

		extern int channel_get_permanent(t_channel const * channel)
		{
			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return 0;
			}

			return (channel->flags & channel_flags_permanent);
		}


		extern unsigned int channel_get_channelid(t_channel const * channel)
		{
			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return 0;
			}
			return channel->id;
		}


		extern int channel_set_channelid(t_channel * channel, unsigned int channelid)
		{
			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return -1;
			}
			channel->id = channelid;
			return 0;
		}

		extern int channel_rejoin(t_connection * conn)
		{
			t_channel const * channel;
			char const * temp;
			char const * chname;

			if (!(channel = conn_get_channel(conn)))
				return -1;

			if (!(temp = channel_get_name(channel)))
				return -1;

			chname = xstrdup(temp);
			conn_part_channel(conn);
			if (conn_set_channel(conn, chname) < 0)
				conn_set_channel(conn, CHANNEL_NAME_BANNED);
			xfree((void *)chname);
			return 0;
		}


		extern int channel_add_connection(t_channel * channel, t_connection * connection)
		{
			t_channelmember * member;
			t_connection *    user;

			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return -1;
			}
			if (!connection)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (channel_check_banning(channel, connection))
			{
				channel_message_log(channel, connection, 0, "JOIN FAILED (banned)");
				return -1;
			}

			member = (t_channelmember*)xmalloc(sizeof(t_channelmember));
			member->connection = connection;
			member->next = channel->memberlist;
			channel->memberlist = member;
			channel->currmembers++;

			channel_message_log(channel, connection, 0, "JOINED");

			message_send_text(connection, message_type_channel, connection, channel_get_name(channel));

			if ((!(channel->flags & channel_flags_permanent))
				&& (!(channel->flags & channel_flags_thevoid))
				&& (!(channel->flags & channel_flags_clan))
				&& (channel->currmembers == 1)
				&& (account_is_operator_or_admin(conn_get_account(connection), channel_get_name(channel)) == 0))
			{
				message_send_text(connection, message_type_info, connection, localize(connection, "you are now tempOP for this channel"));
				conn_set_tmpOP_channel(connection, (char *)channel_get_name(channel));
				channel_update_userflags(connection);
			}
			if (!(channel_get_flags(channel) & channel_flags_thevoid))
			for (user = channel_get_first(channel); user; user = channel_get_next())
			{
				message_send_text(connection, message_type_adduser, user, NULL);
				/* In WOL gamechannels we send JOINGAME ack explicitely to self */
				if (!conn_get_game(connection))
					message_send_text(user, message_type_join, connection, NULL);
			}
			else {
				if (!conn_get_game(connection))
					message_send_text(connection, message_type_join, connection, NULL);
			}

			if (conn_is_irc_variant(connection) && (!conn_get_game(connection))) {
				channel_set_userflags(connection);
				message_send_text(connection, message_type_topic, connection, NULL);
				message_send_text(connection, message_type_namreply, connection, NULL);
			}

			/* please don't remove this notice */
			if (channel->log)
				message_send_text(connection, message_type_info, connection, prefs_get_log_notice());

#ifdef WITH_LUA
			lua_handle_channel(channel, connection, NULL, message_type_null, luaevent_channel_userjoin);
#endif

			return 0;
		}


		extern int channel_del_connection(t_channel * channel, t_connection * connection, t_message_type type, char const * text)
		{
			t_channelmember * curr;
			t_channelmember * temp;
			t_elem * curr2;

			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return -1;
			}
			if (!connection)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			channel_message_send(channel, type, connection, text);
			channel_message_log(channel, connection, 0, "PARTED");

			curr = channel->memberlist;
			if (curr->connection == connection)
			{
				channel->memberlist = channel->memberlist->next;
				xfree(curr);
			}
			else
			{
				while (curr->next && curr->next->connection != connection)
					curr = curr->next;

				if (curr->next)
				{
					temp = curr->next;
					curr->next = curr->next->next;
					xfree(temp);
				}
				else
				{
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] connection not in channel member list", conn_get_socket(connection));
					return -1;
				}
			}
			channel->currmembers--;

			if (conn_get_tmpOP_channel(connection) &&
				std::strcmp(conn_get_tmpOP_channel(connection), channel_get_name(channel)) == 0)
			{
				conn_set_tmpOP_channel(connection, NULL);
			}

#ifdef WITH_LUA
			lua_handle_channel(channel, connection, text, type, luaevent_channel_userleft);
#endif

			if (!channel->memberlist && !(channel->flags & channel_flags_permanent)) /* if channel is empty, delete it unless it's a permanent channel */
			{
				channel_destroy(channel, &curr2);
			}

			return 0;
		}


		extern void channel_update_latency(t_connection * me)
		{
			t_channel *    channel;
			t_message *    message;
			t_connection * c;

			if (!me)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (!(channel = conn_get_channel(me)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "connection has no channel");
				return;
			}

			if (!(message = message_create(message_type_userflags, me, NULL))) /* handles NULL text */
				return;

			for (c = channel_get_first(channel); c; c = channel_get_next())
			if (conn_get_class(c) == conn_class_bnet)
				message_send(message, c);
			message_destroy(message);
		}


		extern void channel_update_userflags(t_connection * me)
		{
			t_channel *    channel;
			t_message *    message;
			t_connection * c;

			if (!me)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}
			if (!(channel = conn_get_channel(me)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "connection has no channel");
				return;
			}

			if (!(message = message_create(message_type_userflags, me, NULL))) /* handles NULL text */
				return;

			for (c = channel_get_first(channel); c; c = channel_get_next())
				message_send(message, c);

			message_destroy(message);
		}


		extern void channel_message_log(t_channel const * channel, t_connection * me, int fromuser, char const * text)
		{
			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return;
			}
			if (!me)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			if (channel->log)
			{
				std::time_t       now;
				struct std::tm *  tmnow;
				char         timetemp[CHANLOG_TIME_MAXLEN];

				now = std::time(NULL);
				if ((!(tmnow = std::localtime(&now))))
					std::strcpy(timetemp, "?");
				else
					std::strftime(timetemp, sizeof(timetemp), CHANLOGLINE_TIME_FORMAT, tmnow);

				if (fromuser)
					std::fprintf(channel->log, "%s: \"%s\" \"%s\"\n", timetemp, conn_get_username(me), text);
				else
					std::fprintf(channel->log, "%s: \"%s\" %s\n", timetemp, conn_get_username(me), text);
				std::fflush(channel->log);
			}
		}


		extern void channel_message_send(t_channel const * channel, t_message_type type, t_connection * me, char const * text)
		{
			t_connection * c;
			unsigned int   heard;
			char const *   tname;
			t_message *    message1; //send to people with clienttag matching channel clienttag
			// or everyone when channel has no clienttag set
			t_message *    message2; //send to people with clienttag not matching channel clienttag
			t_message *    message_to_send;
			t_account *    acc;


			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return;
			}
			if (!me)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return;
			}

			acc = conn_get_account(me);

			if (channel_get_flags(channel) & channel_flags_thevoid) // no talking in the void
			if (type != message_type_join && type != message_type_part)
				return;

			// if user is muted
			if (account_get_auth_mute(acc) == 1)
			{
				std::string msgtemp;
				msgtemp = localize(me, "You can't talk on the channel. Your account has been muted");
				msgtemp += account_get_mutetext(acc, false);
				message_send_text(me, message_type_error, me, msgtemp);
				return;
			}

			if (channel_get_flags(channel) & channel_flags_moderated) // moderated channel - only admins,OPs and voices may talk
			{
				if (type == message_type_talk || type == message_type_emote)
				{
					if (!((account_is_operator_or_admin(acc, channel_get_name(channel))) ||
						(channel_conn_has_tmpVOICE(channel, me)) || (account_get_auth_voice(acc, channel_get_name(channel)) == 1)))
					{
						message_send_text(me, message_type_error, me, localize(me, "This channel is moderated"));
						return;
					}
				}
			}

			if (!channel->clienttag){
				if (!(message1 = message_create(type, me, text)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not create message1");
					return;
				}
				message2 = NULL;
			}
			else {
				if (!(message1 = message_create(type, me, text)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not create message1");
					return;
				}
				if (!(message2 = message_create(type, me, text)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not create message2");
					message_destroy(message1);
					return;
				}
			}


			heard = 0;
			tname = conn_get_chatname(me);
			for (c = channel_get_first(channel); c; c = channel_get_next())
			{
				if (c == me && (type == message_type_talk || type == message_type_gameopt_talk))
					continue; /* ignore ourself */
				if (c == me && (!conn_is_irc_variant(c)) && type == message_type_part)
					continue; /* only on irc we need to inform ourself about leaving the channel */
				if (c != me && (!conn_is_irc_variant(c)) && (channel_get_flags(channel) & channel_flags_thevoid) && (type == message_type_join || type == message_type_part))
					continue; /* make sure we even get join part information about self in The Void */
				if ((type == message_type_talk || type == message_type_whisper || type == message_type_emote || type == message_type_broadcast) &&
					conn_check_ignoring(c, tname) == 1)
					continue; /* ignore squelched players */

				if (!channel->clienttag || channel->clienttag == conn_get_clienttag(c)) {
					message_to_send = message1;
				}
				else {
					message_to_send = message2;
				}

				if (message_send(message_to_send, c) == 0 && c != me)
					heard = 1;
			}

			conn_unget_chatname(me, tname);

			message_destroy(message1);
			if (message2)
				message_destroy(message2);

			if ((conn_get_wol(me) == 0))
			{
				if (!heard && (type == message_type_talk || type == message_type_emote))
					message_send_text(me, message_type_info, me, localize(me, "No one hears you."));
			}

#ifdef WITH_LUA
			if (type != message_type_part)
				lua_handle_channel((t_channel*)channel, me, text, type, luaevent_channel_message);
#endif
		}

		extern int channel_ban_user(t_channel * channel, char const * user)
		{
			t_elem const * curr;
			char *         temp;

			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return -1;
			}
			if (!user)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL user");
				return -1;
			}
			if (!channel->name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got channel with NULL name");
				return -1;
			}

			if (strcasecmp(channel->name, CHANNEL_NAME_BANNED) == 0 ||
				strcasecmp(channel->name, CHANNEL_NAME_KICKED) == 0)
				return -1;

			LIST_TRAVERSE_CONST(channel->banlist, curr)
			if (strcasecmp((char*)elem_get_data(curr), user) == 0)
				return 0;

			temp = xstrdup(user);
			list_append_data(channel->banlist, temp);

			return 0;
		}


		extern int channel_unban_user(t_channel * channel, char const * user)
		{
			t_elem * curr;

			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return -1;
			}
			if (!user)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL user");
				return -1;
			}

			LIST_TRAVERSE(channel->banlist, curr)
			{
				char const * banned;

				if (!(banned = (char*)elem_get_data(curr)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL name in banlist");
					continue;
				}
				if (strcasecmp(banned, user) == 0)
				{
					if (list_remove_elem(channel->banlist, &curr) < 0)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "unable to remove item from list");
						return -1;
					}
					xfree((void *)banned); /* avoid warning */
					return 0;
				}
			}

			return -1;
		}


		extern int channel_check_banning(t_channel const * channel, t_connection const * user)
		{
			t_elem const * curr;

			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return -1;
			}
			if (!user)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL user");
				return -1;
			}

			if (!(channel->flags & channel_flags_allowbots) && conn_get_class(user) == conn_class_bot)
				return 1;

			LIST_TRAVERSE_CONST(channel->banlist, curr)
			if (conn_match(user, (char*)elem_get_data(curr)) == 1)
				return 1;

			return 0;
		}


		extern int channel_get_length(t_channel const * channel)
		{
			t_channelmember const * curr;
			int                     count;

			for (curr = channel->memberlist, count = 0; curr; curr = curr->next, count++);

			return count;
		}


		extern t_connection * channel_get_first(t_channel const * channel)
		{
			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return NULL;
			}

			memberlist_curr = channel->memberlist;

			return channel_get_next();
		}

		extern t_connection * channel_get_next(void)
		{

			t_channelmember * member;

			if (memberlist_curr)
			{
				member = memberlist_curr;
				memberlist_curr = memberlist_curr->next;

				return member->connection;
			}
			return NULL;
		}


		extern t_list * channel_get_banlist(t_channel const * channel)
		{
			if (!channel)
			{
				eventlog(eventlog_level_warn, __FUNCTION__, "got NULL channel");
				return NULL;
			}

			return channel->banlist;
		}


		extern char const * channel_get_shortname(t_channel const * channel)
		{
			if (!channel)
			{
				eventlog(eventlog_level_warn, __FUNCTION__, "got NULL channel");
				return NULL;
			}

			return channel->shortname;
		}


		static int channellist_load_permanent(char const * filename)
		{
			std::FILE *       fp;
			unsigned int line;
			unsigned int pos;
			int          botflag;
			int          operflag;
			int          logflag;
			unsigned int modflag;
			char *       buff;
			char *       name;
			char *       sname;
			char *       tag;
			t_clienttag  clienttag;
			char *       bot;
			char *       oper;
			char *       log;
			char *       country;
			char *       max;
			char *       moderated;
			char *       newname;
			char *       realmname;

			if (!filename)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
				return -1;
			}

			if (!(fp = std::fopen(filename, "r")))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not open channel file \"%s\" for reading (std::fopen: %s)", filename, std::strerror(errno));
				return -1;
			}

			for (line = 1; (buff = file_get_line(fp)); line++)
			{
				if (buff[0] == '#' || buff[0] == '\0')
				{
					continue;
				}
				pos = 0;
				if (!(name = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing name in line %u in file \"%s\"", line, filename);
					continue;
				}
				if (!(sname = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing sname in line %u in file \"%s\"", line, filename);
					continue;
				}
				if (!(tag = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing tag in line %u in file \"%s\"", line, filename);
					continue;
				}
				if (!(bot = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing bot in line %u in file \"%s\"", line, filename);
					continue;
				}
				if (!(oper = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing oper in line %u in file \"%s\"", line, filename);
					continue;
				}
				if (!(log = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing log in line %u in file \"%s\"", line, filename);
					continue;
				}
				if (!(country = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing country in line %u in file \"%s\"", line, filename);
					continue;
				}
				if (!(realmname = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing realmname in line %u in file \"%s\"", line, filename);
					continue;
				}
				if (!(max = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing max in line %u in file \"%s\"", line, filename);
					continue;
				}
				if (!(moderated = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing mod in line %u in file \"%s\"", line, filename);
					continue;
				}

				switch (str_get_bool(bot))
				{
				case 1:
					botflag = 1;
					break;
				case 0:
					botflag = 0;
					break;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "invalid boolean value \"%s\" for field 4 on line %u in file \"%s\"", bot, line, filename);
					continue;
				}

				switch (str_get_bool(oper))
				{
				case 1:
					operflag = 1;
					break;
				case 0:
					operflag = 0;
					break;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "invalid boolean value \"%s\" for field 5 on line %u in file \"%s\"", oper, line, filename);
					continue;
				}

				switch (str_get_bool(log))
				{
				case 1:
					logflag = 1;
					break;
				case 0:
					logflag = 0;
					break;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "invalid boolean value \"%s\" for field 5 on line %u in file \"%s\"", log, line, filename);
					continue;
				}

				switch (str_get_bool(moderated))
				{
				case 1:
					modflag = 1;
					break;
				case 0:
					modflag = 0;
					break;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "invalid boolean value \"%s\" for field 10 on line %u in file \"%s\"", moderated, line, filename);
					continue;
				}

				if (std::strcmp(sname, "NULL") == 0)
					sname = NULL;
				if (std::strcmp(tag, "NULL") == 0)
					clienttag = 0;
				else
					clienttag = clienttag_str_to_uint(tag);
				if (std::strcmp(name, "NONE") == 0)
					name = NULL;
				if (std::strcmp(country, "NULL") == 0)
					country = NULL;
				if (std::strcmp(realmname, "NULL") == 0)
					realmname = NULL;

				if (name)
				{
					channel_create(name, sname, clienttag, 1, botflag, operflag, logflag, country, realmname, std::atoi(max), modflag, 0, 0);
				}
				else
				{
					newname = channel_format_name(sname, country, realmname, 1);
					if (newname)
					{
						channel_create(newname, sname, clienttag, 1, botflag, operflag, logflag, country, realmname, std::atoi(max), modflag, 0, 1);
						xfree(newname);
					}
					else
					{
						eventlog(eventlog_level_error, __FUNCTION__, "cannot format channel name");
					}
				}

				/* FIXME: call channel_delete() on current perm channels and do a
				   channellist_find_channel() and set the long name, perm flag, etc,
				   otherwise call channel_create(). This will make HUPing the server
				   handle re-reading this file correctly. */
			}

			file_get_line(NULL); // clear file_get_line buffer
			if (std::fclose(fp) < 0)
				eventlog(eventlog_level_error, __FUNCTION__, "could not close channel file \"%s\" after reading (std::fclose: %s)", filename, std::strerror(errno));
			return 0;
		}

		static char * channel_format_name(char const * sname, char const * country, char const * realmname, unsigned int id)
		{
			char * fullname;
			unsigned int len;

			if (!sname)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL sname");
				return NULL;
			}
			len = std::strlen(sname) + 1; /* FIXME: check lengths and format */
			if (country)
				len = len + std::strlen(country) + 1;
			if (realmname)
				len = len + std::strlen(realmname) + 1;
			len = len + 32 + 1;

			fullname = (char*)xmalloc(len);
			std::sprintf(fullname, "%s%s%s%s%s-%d",
				realmname ? realmname : "",
				realmname ? " " : "",
				sname,
				country ? " " : "",
				country ? country : "",
				id);
			return fullname;
		}

		extern int channellist_reload(void)
		{
			t_elem * curr;
			t_channel * channel, *old_channel;
			t_channelmember * memberlist, *member, *old_member;
			t_list * channellist_old;

			if (channellist_head)
			{

				channellist_old = list_create();

				/* First pass - get members */
				LIST_TRAVERSE(channellist_head, curr)
				{
					if (!(channel = (t_channel*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "channel list contains NULL item");
						continue;
					}
					/* Trick to avoid automatic channel destruction */
					channel->flags |= channel_flags_permanent;
					if (channel->memberlist)
					{
						/* we need only channel name and memberlist */

						old_channel = (t_channel *)xmalloc(sizeof(t_channel));
						old_channel->shortname = xstrdup(channel->shortname);
						old_channel->memberlist = NULL;
						member = channel->memberlist;

						/* First pass */
						while (member)
						{
							old_member = (t_channelmember*)xmalloc(sizeof(t_channelmember));
							old_member->connection = member->connection;

							if (old_channel->memberlist)
								old_member->next = old_channel->memberlist;
							else
								old_member->next = NULL;

							old_channel->memberlist = old_member;
							member = member->next;
						}

						/* Second pass - remove connections from channel */
						member = old_channel->memberlist;
						while (member)
						{
							channel_del_connection(channel, member->connection, message_type_quit, NULL);
							conn_set_channel_var(member->connection, NULL);
							member = member->next;
						}

						list_prepend_data(channellist_old, old_channel);
					}

					/* Channel is empty - Destroying it */
					channel->flags &= ~channel_flags_permanent;
					if (channel_destroy(channel, &curr) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "could not destroy channel");

				}

				/* Cleanup and reload */

				if (list_destroy(channellist_head) < 0)
					return -1;

				channellist_head = NULL;
				channellist_create();

				/* Now put all users on their previous channel */

				LIST_TRAVERSE(channellist_old, curr)
				{
					if (!(channel = (t_channel*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "old channel list contains NULL item");
						continue;
					}

					memberlist = channel->memberlist;
					while (memberlist)
					{
						member = memberlist;
						memberlist = memberlist->next;
						conn_set_channel(member->connection, channel->shortname);
					}
				}


				/* Ross don't blame me for this but this way the code is cleaner */

				LIST_TRAVERSE(channellist_old, curr)
				{
					if (!(channel = (t_channel*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "old channel list contains NULL item");
						continue;
					}

					memberlist = channel->memberlist;
					while (memberlist)
					{
						member = memberlist;
						memberlist = memberlist->next;
						xfree((void*)member);
					}

					if (channel->shortname)
						xfree((void*)channel->shortname);

					if (list_remove_data(channellist_old, channel, &curr) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "could not remove item from list");
					xfree((void*)channel);

				}

				if (list_destroy(channellist_old) < 0)
					return -1;
			}
			return 0;

		}

		extern int channellist_create(void)
		{
			channellist_head = list_create();

			return channellist_load_permanent(prefs_get_channelfile());
		}


		extern int channellist_destroy(void)
		{
			t_channel *    channel;
			t_elem * curr;

			if (channellist_head)
			{
				LIST_TRAVERSE(channellist_head, curr)
				{
					if (!(channel = (t_channel*)elem_get_data(curr))) /* should not happen */
					{
						eventlog(eventlog_level_error, __FUNCTION__, "channel list contains NULL item");
						continue;
					}

					channel_destroy(channel, &curr);
				}

				if (list_destroy(channellist_head) < 0)
					return -1;
				channellist_head = NULL;
			}

			return 0;
		}


		extern t_list * channellist(void)
		{
			return channellist_head;
		}


		extern int channellist_get_length(void)
		{
			return list_get_length(channellist_head);
		}

		extern int channel_get_max(t_channel const * channel)
		{
			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return 0;
			}

			return channel->maxmembers;
		}

		extern int channel_set_max(t_channel * channel, int maxmembers)
		{
			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return 0;
			}

			if (maxmembers)
				channel->maxmembers = maxmembers;
			return 1;
		}

		extern int channel_get_curr(t_channel const * channel)
		{
			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return 0;
			}

			return channel->currmembers;

		}

		extern int channel_conn_is_tmpOP(t_channel const * channel, t_connection * c)
		{
			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return 0;
			}

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return 0;
			}

			if (!conn_get_tmpOP_channel(c)) return 0;

			if (std::strcmp(conn_get_tmpOP_channel(c), channel_get_name(channel)) == 0) return 1;

			return 0;
		}

		extern int channel_conn_has_tmpVOICE(t_channel const * channel, t_connection * c)
		{
			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return 0;
			}

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return 0;
			}

			if (!conn_get_tmpVOICE_channel(c)) return 0;

			if (std::strcmp(conn_get_tmpVOICE_channel(c), channel_get_name(channel)) == 0) return 1;

			return 0;
		}

		static t_channel * channellist_find_channel_by_fullname(char const * name)
		{
			t_channel *    channel;
			t_elem const * curr;

			if (channellist_head)
			{
				LIST_TRAVERSE(channellist_head, curr)
				{
					channel = (t_channel*)elem_get_data(curr);
					if (!channel->name)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found channel with NULL name");
						continue;
					}

					if (strcasecmp(channel->name, name) == 0)
						return channel;
				}
			}

			return NULL;
		}


		/* Find a channel based on the name.
		 * Create a new channel if it is a permanent-type channel and all others
		 * are full.
		 */
		extern t_channel * channellist_find_channel_by_name(char const * name, char const * country, char const * realmname)
		{
			t_channel *    channel;
			t_elem const * curr;
			int            foundperm;
			int            foundlang;
			int            maxchannel; /* the number of "rollover" channels that exist */
			char const *   saveshortname;
			char const *   savespecialname;
			t_clienttag    savetag;
			int            savebotflag;
			int            saveoperflag;
			int            savelogflag;
			unsigned int   savemoderated;
			char const *   savecountry;
			char const *   saverealmname;
			int            savemaxmembers;
			t_channel *    special_channel;

			// try to make gcc happy and initialize all variables
			saveshortname = savespecialname = savecountry = saverealmname = NULL;
			savetag = savebotflag = saveoperflag = savelogflag = savemaxmembers = savemoderated = 0;

			maxchannel = 0;
			foundperm = 0;
			foundlang = 0;
			if (channellist_head)
			{
				LIST_TRAVERSE(channellist_head, curr)
				{
					channel = (t_channel*)elem_get_data(curr);
					if (!channel->name)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found channel with NULL name");
						continue;
					}

					if (strcasecmp(channel->name, name) == 0)
					{
						// eventlog(eventlog_level_debug,__FUNCTION__,"found exact match for \"%s\"",name);
						return channel;
					}

					if (channel->shortname && strcasecmp(channel->shortname, name) == 0)
					{
						special_channel = channellist_find_channel_by_name(channel->name, country, realmname);
						if (special_channel) channel = special_channel;

						/* FIXME: what should we do if the client doesn't have a country?  For now, just take the first
						 * channel that would otherwise match. */
						if (((!channel->country && !foundlang) || !country ||
							(channel->country && country && (std::strcmp(channel->country, country) == 0))) &&
							(!channel->realmname || !realmname || !std::strcmp(channel->realmname, realmname)))
						{
							if (channel->maxmembers == -1 || channel->currmembers < channel->maxmembers)
							{
								eventlog(eventlog_level_debug, __FUNCTION__, "found permanent channel \"%s\" for \"%s\"", channel->name, name);
								return channel;
							}

							if (!foundlang && (channel->country)) //remember we had found a language specific channel but it was full
							{
								foundlang = 1;
								if (!(channel->flags & channel_flags_autoname))
									savespecialname = channel->name;
								maxchannel = 0;
							}

							maxchannel++;
						}

						// eventlog(eventlog_level_debug,__FUNCTION__,"countries didn't match");

						foundperm = 1;

						/* save off some info in case we need to create a new copy */
						saveshortname = channel->shortname;
						savetag = channel->clienttag;
						savebotflag = channel->flags & channel_flags_allowbots;
						saveoperflag = channel->flags & channel_flags_allowopers;
						if (channel->logname)
							savelogflag = 1;
						else
							savelogflag = 0;
						if (country)
							savecountry = country;
						else
							savecountry = channel->country;
						if (realmname)
							saverealmname = realmname;
						else
							saverealmname = channel->realmname;
						savemaxmembers = channel->maxmembers;
						savemoderated = channel->flags & channel_flags_moderated;
					}
				}
			}

			/* we've gone thru the whole list and either there was no match or the
			 * channels are all full.
			 */

			if (foundperm) /* All the channels were full, create a new one */
			{
				char * channelname;

				if (!foundlang || !savespecialname)
				{
					if (!(channelname = channel_format_name(saveshortname, savecountry, saverealmname, maxchannel + 1)))
						return NULL;
				}
				else
				{
					if (!(channelname = channel_format_name(savespecialname, NULL, saverealmname, maxchannel + 1)))
						return NULL;
				}

				channel = channel_create(channelname, saveshortname, savetag, 1, savebotflag, saveoperflag, savelogflag, savecountry, saverealmname, savemaxmembers, savemoderated, 0, 1);
				xfree(channelname);

				eventlog(eventlog_level_debug, __FUNCTION__, "created copy \"%s\" of channel \"%s\"", (channel) ? (channel->name) : ("<failed>"), name);
				return channel;
			}

			/* no match */
			eventlog(eventlog_level_debug, __FUNCTION__, "could not find channel \"%s\"", name);
			return NULL;
		}


		extern t_channel * channellist_find_channel_bychannelid(unsigned int channelid)
		{
			t_channel *    channel;
			t_elem const * curr;

			if (channellist_head)
			{
				LIST_TRAVERSE(channellist_head, curr)
				{
					channel = (t_channel*)elem_get_data(curr);
					if (!channel->name)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found channel with NULL name");
						continue;
					}
					if (channel->id == channelid)
						return channel;
				}
			}

			return NULL;
		}

		extern int channel_set_userflags(t_connection * c)
		{
			unsigned int	newflags;
			char const *	channel;
			t_account  *	acc;

			if (!c) return -1; // user not connected, no need to update his flags

			acc = conn_get_account(c);

			/* well... unfortunatly channel_get_name never returns NULL but "" instead
			   so we first have to check if user is in a channel at all */
			if ((!conn_get_channel(c)) || (!(channel = channel_get_name(conn_get_channel(c)))))
				return -1;

			if (account_get_auth_admin(acc, channel) == 1 || account_get_auth_admin(acc, NULL) == 1)
				newflags = MF_BLIZZARD;
			else if (account_get_auth_operator(acc, channel) == 1 ||
				account_get_auth_operator(acc, NULL) == 1)
				newflags = MF_BNET;
			else if (channel_conn_is_tmpOP(conn_get_channel(c), c))
				newflags = MF_GAVEL;
			else if ((account_get_auth_voice(acc, channel) == 1) ||
				(channel_conn_has_tmpVOICE(conn_get_channel(c), c)))
				newflags = MF_VOICE;
			else
			if ((conn_get_clienttag(c) == CLIENTTAG_WARCRAFT3_UINT) ||
				(conn_get_clienttag(c) == CLIENTTAG_WAR3XP_UINT))
				newflags = W3_ICON_SET;
			else
				newflags = 0;

			if (conn_get_flags(c) != newflags) {
				conn_set_flags(c, newflags);
				channel_update_userflags(c);
			}

			return 0;
		}

		/**
		*  Westwood Online Extensions
		*/
		extern int channel_get_min(t_channel const * channel)
		{
			if (!channel) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return 0;
			}

			return channel->minmembers;
		}

		extern int channel_set_min(t_channel * channel, int minmembers)
		{
			if (!channel) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return 0;
			}

			if (minmembers)
				channel->minmembers = minmembers;
			return 1;
		}

		extern int channel_wol_get_game_type(t_channel const * channel)
		{
			if (!channel)
			{
				ERROR0("got NULL channel");
				return -1;
			}

			return channel->gameType;
		}

		extern int channel_wol_set_game_type(t_channel * channel, int gameType)
		{
			if (!channel)
			{
				ERROR0("got NULL channel");
				return -1;
			}

			if (gameType)
				channel->gameType = gameType;

			return 0;
		}

		extern char const * channel_wol_get_game_extension(t_channel const * channel)
		{
			if (!channel)
			{
				ERROR0("got NULL channel");
				return 0;
			}

			return channel->gameExtension;
		}

		extern int channel_wol_set_game_extension(t_channel * channel, char const * gameExtension)
		{
			if (!channel)
			{
				ERROR0("got NULL channel");
				return -1;
			}

			if (!gameExtension)
			{
				ERROR0("got NULL gameExtension");
				return -1;
			}

			if (channel->gameExtension)
				xfree(channel->gameExtension);

			channel->gameExtension = xstrdup(gameExtension);

			return 0;
		}


	}

}
