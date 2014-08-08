/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2000  Marco Ziech (mmz@gmx.net)
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
#ifndef INCLUDED_CHANNEL_TYPES
#define INCLUDED_CHANNEL_TYPES

#ifdef JUST_NEED_TYPES
# include "common/bn_type.h"
#else
# define JUST_NEED_TYPES
# include "common/bn_type.h"
# undef JUST_NEED_TYPES
#endif

# include "account.h"

#ifdef CHANNEL_INTERNAL_ACCESS

#include <cstdio>

#ifdef JUST_NEED_TYPES
# include "connection.h"
# include "common/list.h"
#else
# define JUST_NEED_TYPES
# include "connection.h"
# include "common/list.h"
# undef JUST_NEED_TYPES
#endif

#endif

namespace pvpgn
{

	namespace bnetd
	{

#ifdef CHANNEL_INTERNAL_ACCESS
		typedef struct channelmember
		{
			/* standalone mode */
			t_connection *         connection;
			struct channelmember * next;
		} t_channelmember;
#endif

		typedef enum
		{
			channel_flags_none = 0x00,
			channel_flags_public = 0x01,
			channel_flags_moderated = 0x02,
			channel_flags_restricted = 0x04,
			channel_flags_thevoid = 0x08,
			channel_flags_system = 0x10,
			channel_flags_official = 0x20,
			channel_flags_permanent = 0x40,
			channel_flags_allowbots = 0x80,
			channel_flags_allowopers = 0x100,
			channel_flags_clan = 0x200,
			channel_flags_autoname = 0x400
		} t_channel_flags;

		typedef struct channel
#ifdef CHANNEL_INTERNAL_ACCESS
		{
			char const *      name;
			char const *      shortname;  /* short "alias" for permanent channels, NULL if none */
			char const *      country;
			char const *      realmname;
			unsigned int      flags;
			int		      maxmembers;
			int		      currmembers;
			t_clienttag       clienttag;
			unsigned int      id;
			t_channelmember * memberlist;
			t_list *          banlist;    /* of char * */
			char *            logname;    /* NULL if not logged */
			std::FILE *       log;        /* NULL if not logging */

			/**
			*  Westwood Online Extensions
			*/
			int               minmembers;

			int               gameType;
			char *            gameExtension;
		}
#endif
		t_channel;

	}

}

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_CHANNEL_PROTOS
#define INCLUDED_CHANNEL_PROTOS

#define JUST_NEED_TYPES
#include "connection.h"
#include "message.h"
#include "common/list.h"
#include "common/tag.h"
#undef JUST_NEED_TYPES

#define CHANNEL_NAME_BANNED "THE VOID"
#define CHANNEL_NAME_KICKED "THE VOID"
#define CHANNEL_NAME_CHAT   "Chat"

namespace pvpgn
{

	namespace bnetd
	{

		extern int channel_set_userflags(t_connection * c);
		extern t_channel * channel_create(char const * fullname, char const * shortname, t_clienttag clienttag, int permflag, int botflag, int operflag, int logflag, char const * country, char const * realmname, int maxmembers, int moderated, int clanflag, int autoname, t_list * channellist);
		extern t_channel * channel_create(char const * fullname, char const * shortname, t_clienttag clienttag, int permflag, int botflag, int operflag, int logflag, char const * country, char const * realmname, int maxmembers, int moderated, int clanflag, int autoname);
		extern int channel_destroy(t_channel * channel, t_elem ** elem);
		extern char const * channel_get_name(t_channel const * channel);
		extern char const * channel_get_shortname(t_channel const * channel);
		extern t_clienttag channel_get_clienttag(t_channel const * channel);
		extern unsigned channel_get_flags(t_channel const * channel);
		extern int channel_set_flags(t_channel * channel, unsigned flags);
		extern int channel_get_permanent(t_channel const * channel);
		extern unsigned int channel_get_channelid(t_channel const * channel);
		extern int channel_set_channelid(t_channel * channel, unsigned int channelid);
		extern int channel_add_connection(t_channel * channel, t_connection * connection);
		extern int channel_del_connection(t_channel * channel, t_connection * connection, t_message_type mess, char const * text);
		extern void channel_update_latency(t_connection * conn);
		extern void channel_update_userflags(t_connection * conn);
		extern void channel_message_log(t_channel const * channel, t_connection * me, int fromuser, char const * text);
		extern void channel_message_send(t_channel const * channel, t_message_type type, t_connection * conn, char const * text);
		extern int channel_ban_user(t_channel * channel, char const * user);
		extern int channel_unban_user(t_channel * channel, char const * user);
		extern int channel_check_banning(t_channel const * channel, t_connection const * user);
		extern int channel_rejoin(t_connection * conn);
		extern t_list * channel_get_banlist(t_channel const * channel);
		extern int channel_get_length(t_channel const * channel);
		extern int channel_get_max(t_channel const * channel);
		extern int channel_set_max(t_channel * channel, int maxmembers);
		extern int channel_get_curr(t_channel const * channel);
		extern int channel_conn_is_tmpOP(t_channel const * channel, t_connection * c);
		extern int channel_conn_has_tmpVOICE(t_channel const * channel, t_connection * c);
		extern t_connection * channel_get_first(t_channel const * channel);
		extern t_connection * channel_get_next(void);

		extern int channellist_create(void);
		extern int channellist_destroy(void);
		extern int channellist_reload(void);
		extern t_list * channellist(void);
		extern t_channel * channellist_find_channel_by_name(char const * name, char const * locale, char const * realmname);
		extern t_channel * channellist_find_channel_bychannelid(unsigned int channelid);
		extern int channellist_get_length(void);

		/**
		*  Westwood Online Extensions
		*/
		extern int channel_get_min(t_channel const * channel);
		extern int channel_set_min(t_channel * channel, int minmembers);

		extern int channel_wol_get_game_type(t_channel const * channel);
		extern int channel_wol_set_game_type(t_channel * channel, int gameType);

		extern char const * channel_wol_get_game_extension(t_channel const * channel);
		extern int channel_wol_set_game_extension(t_channel * channel, char const * gameExtension);

	}

}

#endif
#endif
