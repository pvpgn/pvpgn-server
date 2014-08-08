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
/*****/
#ifdef WITH_LUA
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_LUAFUNCTION_PROTOS
#define INCLUDED_LUAFUNCTION_PROTOS

#define JUST_NEED_TYPES
#include "luawrapper.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern int __message_send_text(lua_State* L);
		extern int __eventlog(lua_State* L);

		extern int __account_get_by_name(lua_State* L);
		extern int __account_get_by_id(lua_State* L);
		extern int __account_get_attr(lua_State* L);
		extern int __account_set_attr(lua_State* L);
		extern int __account_get_friends(lua_State* L);
		extern int __account_get_teams(lua_State* L);

		extern int __clan_get_members(lua_State* L);

		extern int __game_get_by_id(lua_State* L);
		extern int __game_get_by_name(lua_State* L);

		extern int __channel_get_by_id(lua_State* L);

		extern int __server_get_users(lua_State* L);
		extern int __server_get_games(lua_State* L);
		extern int __server_get_channels(lua_State* L);

		extern int __client_kill(lua_State* L);
		extern int __client_readmemory(lua_State* L);

		extern int __command_get_group(lua_State* L);
		extern int __icon_get_rank(lua_State* L);
		extern int __describe_command(lua_State* L);
		extern int __messagebox_show(lua_State* L);

		extern int __localize(lua_State* L);
	}

}

#endif
#endif
#endif