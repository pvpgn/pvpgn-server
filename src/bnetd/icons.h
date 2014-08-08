/*
* Copyright (C) 2014  HarpyWar (harpywar@gmail.com)
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
#ifndef INCLUDED_ICONS_TYPES
#define INCLUDED_ICONS_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		typedef struct
		{
			unsigned int rating;	// rating or something else
			char * rank;			// any string value
			char * icon_code;		// icon code
		} t_icon_info;

		typedef struct
		{
			char * clienttag;
			char * attr_key;
			t_list * icon_info;		// list of t_icon_info
			t_list * iconstash;		// list of t_icon_var_info

			t_list * vars;		// list of t_icon_var_info
			const char * stats;
		} t_iconset_info;

		typedef struct
		{
			char * key;	
			char * value;
		} t_icon_var_info;
	}

}

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_ICONS_PROTOS
#define INCLUDED_ICONS_PROTOS

#define JUST_NEED_TYPES
# include "account.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{
		extern int handle_icon_command(t_connection * c, char const *text);
		extern char const * customicons_stash_find(t_clienttag clienttag, char const * code, bool return_alias = false);
		extern std::string customicons_stash_get_list(t_clienttag clienttag, bool return_alias = false);

		extern int prefs_get_custom_icons();
		extern t_icon_info * customicons_get_icon_by_account(t_account * account, t_clienttag clienttag);
		extern const char * customicons_get_stats_text(t_account * account, t_clienttag clienttag);
		extern t_icon_info * customicons_get_icon_by_rating(int rating, char * clienttag);


		extern int customicons_load(char const * filename);
		extern int customicons_unload(void);

	}

}

/*****/
#endif
#endif
