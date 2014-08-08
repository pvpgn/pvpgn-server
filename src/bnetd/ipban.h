/*
 * Copyright (C) 2000  Gediminas (gugini@fortas.ktu.lt)
 * Copyright (C) 2002  Bartomiej Butyn (bartek@milc.com.pl)
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
#ifndef INCLUDED_IPBAN_TYPES
#define INCLUDED_IPBAN_TYPES

#ifdef IPBAN_INTERNAL_ACCESS

#include <ctime>

#define MAX_FUNC_LEN 10
#define MAX_IP_STR   32
#define MAX_TIME_STR 9

#define IPBAN_FUNC_ADD     1
#define IPBAN_FUNC_DEL     2
#define IPBAN_FUNC_LIST    3
#define IPBAN_FUNC_CHECK   4
#define IPBAN_FUNC_UNKNOWN 5


namespace pvpgn
{

	namespace bnetd
	{

		typedef enum
		{
			ipban_type_exact,     /* 192.168.0.10              */
			ipban_type_wildcard,  /* 192.168.*.*               */
			ipban_type_range,     /* 192.168.0.10-192.168.0.25 */
			ipban_type_netmask,   /* 192.168.0.0/255.255.0.0   */
			ipban_type_prefix     /* 192.168.0.0/16            */
		} t_ipban_type;

		typedef struct ipban_entry_struct
		{
			char *                      info1; /* third octet */
			char *                      info2; /* third octet */
			char *                      info3; /* third octet */
			char *                      info4; /* fourth octet */
			int                         type;
			std::time_t			endtime;
		} t_ipban_entry;

	}

}

#endif

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_IPBAN_PROTOS
#define INCLUDED_IPBAN_PROTOS

#include <ctime>

#define JUST_NEED_TYPES
#include "connection.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern int ipbanlist_create(void);
		extern int ipbanlist_destroy(void);
		extern int ipbanlist_load(char const * filename);
		extern int ipbanlist_save(char const * filename);
		extern int ipbanlist_check(char const * addr);
		extern int ipbanlist_add(t_connection * c, char const * cp, std::time_t endtime);
		extern int ipbanlist_unload_expired(void);
		extern std::time_t ipbanlist_str_to_time_t(t_connection * c, char const * timestr);
		extern int handle_ipban_command(t_connection * c, char const * text);

	}

}

#endif
#endif
