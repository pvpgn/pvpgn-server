/*
 * Copyright (C) 2001  Marco Ziech (mmz@gmx.net)
 * Copyright (C) 2007  Pelish (pelish@gmail.com)
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

/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_IRC_PROTOS
#define INCLUDED_IRC_PROTOS
#define JUST_NEED_TYPES
# include "common/packet.h"
# include "connection.h"
# include "channel.h"
# include "message.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern int irc_send_cmd(t_connection * conn, char const * command, char const * params);
		extern int irc_send(t_connection * conn, int code, char const * params);
		extern int irc_send_ping(t_connection * conn);
		extern int irc_send_pong(t_connection * conn, char const * params);
		extern int irc_authenticate(t_connection * conn, char const * passhash);
		extern char const * irc_convert_channel(t_channel const * channel, t_connection * c);
		extern char const * irc_convert_ircname(char const * pircname);
		extern char ** irc_get_listelems(char * list);
		extern int irc_unget_listelems(char ** elems);
		extern char ** irc_get_paramelems(char * list);
		extern char ** irc_get_ladderelems(char * list);
		extern int irc_unget_ladderelems(char ** elems);
		extern int irc_unget_paramelems(char ** elems);
		extern int irc_message_postformat(t_packet * packet, t_connection const * dest);
		extern int irc_message_format(t_packet * packet, t_message_type type, t_connection * me, t_connection * dst, char const * text, unsigned int dstflags);
		extern int irc_send_rpl_namreply(t_connection * c, t_channel const * channel);
		extern int irc_who(t_connection * c, char const * name);
		extern int irc_send_motd(t_connection * conn);
		extern int _handle_nick_command(t_connection * conn, int numparams, char ** params, char * text);
		extern int _handle_ping_command(t_connection * conn, int numparams, char ** params, char * text);
		extern int _handle_pong_command(t_connection * conn, int numparams, char ** params, char * text);
		extern int _handle_join_command(t_connection * conn, int numparams, char ** params, char * text);
		extern int irc_send_topic(t_connection * c, t_channel const * channel);
		extern int _handle_topic_command(t_connection * conn, int numparams, char ** params, char * text);
		extern int _handle_kick_command(t_connection * conn, int numparams, char ** params, char * text);
		extern int _handle_mode_command(t_connection * conn, int numparams, char ** params, char * text);
		extern int _handle_time_command(t_connection * conn, int numparams, char ** params, char * text);
	}

}

#endif
#endif
