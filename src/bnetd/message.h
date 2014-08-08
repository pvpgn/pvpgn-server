/*
 * Copyright (C) 1998,1999,2001  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_MESSAGE_TYPES
#define INCLUDED_MESSAGE_TYPES

#ifdef MESSAGE_INTERNAL_ACCESS

#ifdef JUST_NEED_TYPES
# include "common/packet.h"
# include "connection.h"
#else
# define JUST_NEED_TYPES
# include "common/packet.h"
# include "connection.h"
# undef JUST_NEED_TYPES
#endif

#endif

namespace pvpgn
{

	namespace bnetd
	{

		typedef enum
		{
			message_type_adduser,
			message_type_join,
			message_type_part,
			message_type_whisper,
			message_type_talk,
			message_type_broadcast,
			message_type_channel,
			message_type_userflags,
			message_type_whisperack,
			message_type_friendwhisperack,
			message_type_channelfull,
			message_type_channeldoesnotexist,
			message_type_channelrestricted,
			message_type_info,
			message_type_error,
			message_type_emote,
			message_type_uniqueid,
			message_type_mode,
			message_type_kick,
			message_type_quit,

			/**
			*  IRC specific messages
			*/
			message_type_nick,
			message_type_notice,
			message_type_namreply,
			message_type_topic,

			/**
			*  Westwood Online Extensions
			*/
			message_type_host,
			message_type_invmsg,
			message_type_page,
			message_wol_joingame,
			message_type_gameopt_talk,
			message_type_gameopt_whisper,
			message_wol_start_game,
			message_wol_advertr,
			message_wol_chanchk,
			message_wol_userip,

			message_type_null
		} t_message_type;

		typedef enum {
			message_class_normal,
			message_class_charjoin	/* use char*account (if account isnt d2 char is "") */
		} t_message_class;

		typedef struct message
#ifdef MESSAGE_INTERNAL_ACCESS
		{
			unsigned int   num_cached;
			t_packet * *   packets;    /* cached messages */
			t_conn_class * classes;    /* classes of cached message connections */
			unsigned int * dstflags;   /* overlaid flags of cached messages */
			t_message_class * mclasses; /* classes of cached messages */
			/* ---- */
			t_message_type type;       /* format of message */
			t_connection * src;        /* originator message */
			char const *   text;       /* text of message */
		}
#endif
		t_message;

	}

}

#endif

/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_MESSAGE_PROTOS
#define INCLUDED_MESSAGE_PROTOS

#include <cstdio>
#define JUST_NEED_TYPES
#include <string>
#include "connection.h"
#include "common/bnet_protocol.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern char * message_format_line(t_connection const * c, char const * in);
		extern t_message * message_create(t_message_type type, t_connection * src, char const * text);
		extern int message_destroy(t_message * message);
		extern int message_send(t_message * message, t_connection * dst);
		extern int message_send_all(t_message * message);
		extern int message_send_admins(t_connection * src, t_message_type type, char const * text);

		/* the following are "shortcuts" to avoid calling message_create(), message_send(), message_destroy() */
		extern int message_send_text(t_connection * dst, t_message_type type, t_connection * src, std::string text);
		extern int message_send_text(t_connection * dst, t_message_type type, t_connection * src, char const * text);
		extern int message_send_formatted(t_connection * dst, char const * text);
		extern int message_send_file(t_connection * dst, std::FILE * fd);
		extern int messagebox_show(t_connection * dst, char const * text, char const * caption = "", int type = SERVER_MESSAGEBOX_OK);

	}

}

#endif
#endif
