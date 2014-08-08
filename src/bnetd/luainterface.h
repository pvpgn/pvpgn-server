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
#ifndef INCLUDED_LUAINTERFACE_PROTOS
#define INCLUDED_LUAINTERFACE_PROTOS

#ifdef WIN32
#pragma comment(lib, "lua5.1.lib")
#endif

#define JUST_NEED_TYPES
#include "connection.h"
#include "luawrapper.h"
#include <stdio.h>
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{
		typedef enum {
			luaevent_command,
			luaevent_command_before,

			luaevent_game_create,
			luaevent_game_report,
			luaevent_game_end,
			luaevent_game_destroy,
			luaevent_game_changestatus,
			luaevent_game_userjoin,
			luaevent_game_userleft,
			
			luaevent_channel_message, // user-to-channel
			luaevent_channel_userjoin,
			luaevent_channel_userleft,

			luaevent_user_whisper, // user-to-user
			luaevent_user_login,
			luaevent_user_disconnect,

			luaevent_server_start,
			luaevent_server_rehash,
			luaevent_server_mainloop,

			luaevent_client_readmemory

		} t_luaevent_type;


		extern void lua_load(char const * scriptdir);
		extern void lua_unload();

		extern int lua_handle_command(t_connection * c, char const * text, t_luaevent_type luaevent);
		extern void lua_handle_game(t_game * game, t_connection * c, t_luaevent_type luaevent);
		extern std::vector<t_game*> lua_handle_game_list(t_connection * c);

		extern int lua_handle_channel(t_channel * channel, t_connection * c, char const * message_text, t_message_type message_type, t_luaevent_type luaevent);
		extern int lua_handle_user(t_connection * c, t_connection * c_dst, char const * message_text, t_luaevent_type luaevent);
		extern const char * lua_handle_user_icon(t_connection * c, const char * iconinfo);
		extern void lua_handle_server(t_luaevent_type luaevent);
		extern void lua_handle_client(t_connection * c, int request_id, std::vector<int> data, t_luaevent_type luaevent);

	}

}

#endif
#endif
#endif