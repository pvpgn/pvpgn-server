/*
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
#ifndef INCLUDED_COMMAND_GROUPS_TYPES
#define INCLUDED_COMMAND_GROUPS_TYPES

namespace pvpgn
{

	namespace bnetd
	{

#ifdef COMMAND_GROUPS_INTERNAL_ACCESS

		typedef struct
		{
			char *	 command;
			unsigned int group;
		} t_command_groups;

#endif

	}

}

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_COMMAND_GROUPS_PROTOS
#define INCLUDED_COMMAND_GROUPS_PROTOS

namespace pvpgn
{

	namespace bnetd
	{

		extern int command_groups_load(char const * filename);
		extern int command_groups_unload(void);
		extern int command_groups_reload(char const * filename);
		extern unsigned int command_get_group(char const * command);

	}

}

#endif
#endif
