/*
 * Copyright (C) 2000  Onlyer (onlyer@263.net)
 * Copyright (C) 2002  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_ALIAS_COMMAND_TYPES
#define INCLUDED_ALIAS_COMMAND_TYPES

#ifdef ALIAS_COMMAND_INTERNAL_ACCESS

#ifdef JUST_NEED_TYPES
# include "common/list.h"
#else
# define JUST_NEED_TYPES
# include "common/list.h"
# undef JUST_NEED_TYPES
#endif

namespace pvpgn
{

	namespace bnetd
	{

		typedef struct
		{
			char const * line;
			int min;
			int max;
		} t_output;

		typedef struct
		{
			char const *  alias;
			t_list *      output; /* of t_output * */
		} t_alias;

	}

}

#endif

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_ALIAS_COMMAND_PROTOS
#define INCLUDED_ALIAS_COMMAND_PROTOS

#define JUST_NEED_TYPES
#include "connection.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern int aliasfile_load(char const * filename);
		extern int aliasfile_unload(void);
		extern int handle_alias_command(t_connection * c, char const * text);

	}

}

#endif
#endif
