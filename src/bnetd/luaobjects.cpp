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
#ifdef WITH_LUA
#include "common/setup_before.h"
#define ACCOUNT_INTERNAL_ACCESS
#define GAME_INTERNAL_ACCESS
#define CHANNEL_INTERNAL_ACCESS
#define CLAN_INTERNAL_ACCESS
#define TEAM_INTERNAL_ACCESS

#include "team.h"

#include <cctype>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <sstream>

#include "compat/strcasecmp.h"
#include "compat/snprintf.h"
#include "common/tag.h"
#include "common/util.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "common/flags.h"
#include "common/proginfo.h"
#include "common/addr.h"
#include "common/xstring.h"

#include "connection.h"
#include "message.h"
#include "channel.h"
#include "game.h"
#include "account.h"
#include "account_wrap.h"
#include "timer.h"
#include "ipban.h"
#include "command_groups.h"
#include "friends.h"
#include "clan.h"

#include "luaobjects.h"

#include "common/setup_after.h"


namespace pvpgn
{
	namespace bnetd
	{
		template <class T, class A>
		T join(const A &begin, const A &end, const T &t);

		extern std::map<std::string, std::string> get_account_object(unsigned int userid)
		{
			std::map<std::string, std::string> o_account;

			if (userid == 0)
				return o_account;

			if (t_account * account = accountlist_find_account_by_uid(userid))
				o_account = get_account_object(account);
			return o_account;
		}
		extern std::map<std::string, std::string> get_account_object(const char * username)
		{
			std::map<std::string, std::string> o_account;

			if (username[0] == '\0')
				return o_account;

			if (t_account * account = accountlist_find_account(username))
				o_account = get_account_object(account);
			return o_account;
		}
		extern std::map<std::string, std::string> get_account_object(t_account * account)
		{
			std::map<std::string, std::string> o_account;

			if (!account)
				return o_account;

			o_account["id"] = std_to_string(account_get_uid(account));
			o_account["name"] = account_get_name(account);
			if (const char * email = account_get_email(account)) // email can be empty
				o_account["email"] = email;
			o_account["commandgroups"] = std_to_string(account_get_command_groups(account));
			o_account["locked"] = account_get_auth_lock(account) ? "true" : "false";
			o_account["muted"] = account_get_auth_mute(account) ? "true" : "false";

			o_account["online"] = "false"; // set init as offline

			// if user online
			if (t_connection * c = account_get_conn(account))
			{
				o_account["online"] = "true";

				o_account["clientver"] = conn_get_clientver(c);
				o_account["latency"] = std_to_string(conn_get_latency(c));

				// non-game clients doesn't provide a country
				if (const char * country = conn_get_country(c))
					o_account["country"] = country;
				if (t_clienttag clienttag = conn_get_clienttag(c))
					o_account["clienttag"] = clienttag_uint_to_str(clienttag);
				if (t_game *game = conn_get_game(c))
					o_account["game_id"] = std_to_string(game_get_id(game));
				if (t_channel *channel = conn_get_channel(c))
					o_account["channel_id"] = std_to_string(channel_get_channelid(channel));
			}


			if (account->clanmember)
				o_account["clan_id"] = account->clanmember->clan->clanid;

			// - friends can be get from lua_account_get_friends
			// - teams can be get from lua_account_get_teams

			return o_account;
		}

		extern std::map<std::string, std::string> get_game_object(unsigned int gameid)
		{
			std::map<std::string, std::string> o_game;

			if (t_game * game = gamelist_find_game_byid(gameid))
				o_game = get_game_object(game);
			return o_game;
		}
		extern std::map<std::string, std::string> get_game_object(const char * gamename, t_clienttag clienttag, t_game_type gametype)
		{
			std::map<std::string, std::string> o_game;

			if (t_game * game = gamelist_find_game(gamename, clienttag, gametype))
				o_game = get_game_object(game);
			return o_game;
		}
		extern std::map<std::string, std::string> get_game_object(t_game * game)
		{
			std::map<std::string, std::string> o_game;

			if (!game)
				return o_game;

			o_game["id"] = std_to_string(game->id);
			o_game["name"] = game->name;
			o_game["pass"] = game->pass;
			o_game["info"] = game->info;
			o_game["type"] = std_to_string(game->type);
			o_game["flag"] = std_to_string(game->flag);

			o_game["address"] = addr_num_to_ip_str(game->addr);
			o_game["port"] = std_to_string(game->port);
			o_game["status"] = std_to_string(game->status);
			o_game["currentplayers"] = std_to_string(game->ref);
			o_game["totalplayers"] = std_to_string(game->count);
			o_game["maxplayers"] = std_to_string(game->maxplayers);
			if (game->mapname)
				o_game["mapname"] = game->mapname;
			o_game["option"] = std_to_string(game->option);
			o_game["maptype"] = std_to_string(game->maptype);
			o_game["tileset"] = std_to_string(game->tileset);
			o_game["speed"] = std_to_string(game->speed);
			o_game["mapsize_x"] = std_to_string(game->mapsize_x);
			o_game["mapsize_y"] = std_to_string(game->mapsize_y);
			if (t_connection *c = game->owner)
			{
				if (t_account *account = conn_get_account(c))
					o_game["owner"] = account_get_name(account);
			}

			std::vector<std::string> players;
			for (int i = 0; i < game->ref; i++)
			{
				if (t_account *account = game->players[i])
					players.push_back(account_get_name(account));
			}
			o_game["players"] = join(players.begin(), players.end(), std::string(","));


			o_game["bad"] = std_to_string(game->bad); // if 1, then the results will be ignored 

			std::vector<std::string> results;
			if (game->results)
			{
				for (int i = 0; i < game->count; i++)
					results.push_back(std_to_string(game->results[i]));
			}
			o_game["results"] = join(results.begin(), results.end(), std::string(","));
			// UNDONE: add report_heads and report_bodies: they are XML strings

			o_game["create_time"] = std_to_string(game->create_time);
			o_game["start_time"] = std_to_string(game->start_time);
			o_game["lastaccess_time"] = std_to_string(game->lastaccess_time);

			o_game["difficulty"] = std_to_string(game->difficulty);
			o_game["version"] = vernum_to_verstr(game->version);
			o_game["startver"] = std_to_string(game->startver);

			if (t_clienttag clienttag = game->clienttag)
				o_game["clienttag"] = clienttag_uint_to_str(clienttag);


			if (game->description)
				o_game["description"] = game->description;
			if (game->realmname)
				o_game["realmname"] = game->realmname;

			return o_game;
		}

		extern std::map<std::string, std::string> get_channel_object(unsigned int channelid)
		{
			std::map<std::string, std::string> o_channel;

			if (t_channel * channel = channellist_find_channel_bychannelid(channelid))
				o_channel = get_channel_object(channel);
			return o_channel;
		}
		extern std::map<std::string, std::string> get_channel_object(t_channel * channel)
		{
			std::map<std::string, std::string> o_channel;

			if (!channel)
				return o_channel;

			o_channel["id"] = std_to_string(channel->id);
			o_channel["name"] = channel->name;
			if (channel->shortname)
				o_channel["shortname"] = channel->shortname;
			if (channel->country)
				o_channel["country"] = channel->country;
			o_channel["flags"] = std_to_string(channel->flags);
			o_channel["maxmembers"] = std_to_string(channel->maxmembers);
			o_channel["currmembers"] = std_to_string(channel->currmembers);

			o_channel["clienttag"] = clienttag_uint_to_str(channel->clienttag);
			if (channel->realmname)
				o_channel["realmname"] = channel->realmname;
			if (channel->logname)
				o_channel["logname"] = channel->logname;


			// Westwood Online Extensions
			o_channel["minmembers"] = std_to_string(channel->minmembers);
			o_channel["gameType"] = std_to_string(channel->gameType);
			if (channel->gameExtension)
				o_channel["gameExtension"] = channel->gameExtension;


			std::vector<std::string> members;
			if (channel->memberlist)
			{
				t_channelmember *m = channel->memberlist;
				while (m)
				{
					if (t_account *account = conn_get_account(m->connection))
						members.push_back(account_get_name(account));
					m = m->next;
				}
			}
			o_channel["memberlist"] = join(members.begin(), members.end(), std::string(","));

			std::vector<std::string> bans;
			if (channel->banlist)
			{
				t_elem const * curr;
				LIST_TRAVERSE_CONST(channel->banlist, curr)
				{
					char * b;
					if (!(b = (char*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL name in banlist");
						continue;
					}
					bans.push_back(b);
				}
			}
			o_channel["banlist"] = join(bans.begin(), bans.end(), std::string(","));

			return o_channel;
		}


		extern std::map<std::string, std::string> get_clan_object(t_clan * clan)
		{
			std::map<std::string, std::string> o_clan;

			if (!clan)
				return o_clan;

			o_clan["id"] = std_to_string(clan->clanid);
			o_clan["tag"] = clantag_to_str(clan->tag);
			o_clan["clanname"] = clan->clanname;
			o_clan["created"] = std_to_string(clan->created);
			o_clan["creation_time"] = std_to_string(clan->creation_time);
			o_clan["clan_motd"] = clan->clan_motd;
			o_clan["channel_type"] = std_to_string(clan->channel_type); // 0--public 1--private

			// clanmembers can be get from api.clan_get_members

			return o_clan;
		}

		extern std::map<std::string, std::string> get_clanmember_object(t_clanmember * member)
		{
			std::map<std::string, std::string> o_member;

			if (!member)
				return o_member;

			o_member["username"] = account_get_name(member->memberacc);
			o_member["status"] = member->status;
			o_member["clan_id"] = std_to_string(member->clan->clanid);
			o_member["join_time"] = std_to_string(member->join_time);
			o_member["fullmember"] = std_to_string(member->fullmember);// 0 -- clanmember is only invited, 1 -- clanmember is fullmember

			return o_member;
		}

		extern std::map<std::string, std::string> get_team_object(t_team * team)
		{
			std::map<std::string, std::string> o_team;

			if (!team)
				return o_team;

			o_team["id"] = std_to_string(team->teamid);
			o_team["size"] = std_to_string(team->size);

			o_team["clienttag"] = clienttag_uint_to_str(team->clienttag);
			o_team["lastgame"] = std_to_string(team->lastgame);
			o_team["wins"] = std_to_string(team->wins);
			o_team["losses"] = std_to_string(team->losses);
			o_team["xp"] = std_to_string(team->xp);
			o_team["level"] = std_to_string(team->level);
			o_team["rank"] = std_to_string(team->rank);

			// usernames
			std::vector<std::string> members;
			if (team->members)
			{
				for (int i = 0; i < MAX_TEAMSIZE; i++)
					members.push_back(account_get_name(team->members[i]));
			}
			o_team["members"] = join(members.begin(), members.end(), std::string(","));

			return o_team;
		}

		extern std::map<std::string, std::string> get_friend_object(t_friend * f)
		{
			std::map<std::string, std::string> o_friend;

			if (!f)
				return o_friend;

			o_friend = get_account_object(f->friendacc);
			return o_friend;
		}


		/* Join two vector objects to string by delimeter */
		template <class T, class A>
		T join(const A &begin, const A &end, const T &t)
		{
			T result;
			for (A it = begin;
				it != end;
				it++)
			{
				if (!result.empty())
					result.append(t);
				result.append(*it);
			}
			return result;
		}
	}
}
#endif