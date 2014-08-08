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
#ifndef INCLUDED_LUAOBJECTS_PROTOS
#define INCLUDED_LUAOBJECTS_PROTOS



#define JUST_NEED_TYPES
#include "connection.h"
#include <cctype>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern std::map<std::string, std::string> get_account_object(unsigned int userid);
		extern std::map<std::string, std::string> get_account_object(const char *username);
		extern std::map<std::string, std::string> get_account_object(t_account *account);
		extern std::map<std::string, std::string> get_game_object(unsigned int gameid);
		extern std::map<std::string, std::string> get_game_object(const char * gamename, t_clienttag clienttag, t_game_type gametype);
		extern std::map<std::string, std::string> get_game_object(t_game * game);
		extern std::map<std::string, std::string> get_channel_object(unsigned int channelid);
		extern std::map<std::string, std::string> get_channel_object(t_channel * channel);
		extern std::map<std::string, std::string> get_clan_object(t_clan * clan);
		extern std::map<std::string, std::string> get_clanmember_object(t_clanmember * member);
		extern std::map<std::string, std::string> get_team_object(t_team * team);
		extern std::map<std::string, std::string> get_friend_object(t_friend * f);
	}

}

#endif
#endif
#endif