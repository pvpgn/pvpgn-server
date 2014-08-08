/*
 * Copyright (C) 2000            Dizzy
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
#ifndef INCLUDED_HELPFILE_H
#define INCLUDED_HELPFILE_H

#ifndef JUST_NEED_TYPES

#define JUST_NEED_TYPES
#include "connection.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern int helpfile_init(char const * filename);
		extern int helpfile_unload(void);
		extern int handle_help_command(t_connection *, char const *);
		extern int describe_command(t_connection * c, char const * cmd);
	}

}

#endif
#endif
