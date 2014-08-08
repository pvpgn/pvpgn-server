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
#define GAME_INTERNAL_ACCESS
#ifdef WITH_LUA
#include "common/setup_before.h"

#include "team.h"

#include <cctype>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <cmath>

#include "compat/strcasecmp.h"
#include "compat/snprintf.h"
#include "compat/pdir.h"
#include "common/tag.h"
#include "common/util.h"
#include "common/eventlog.h"

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
#include "prefs.h"


#include "luawrapper.h"
#include "luainterface.h"
#include "luafunctions.h"
#include "luaobjects.h"


#include "common/setup_after.h"


namespace pvpgn
{

	namespace bnetd
	{
		lua::vm vm;

		char _msgtemp[MAX_MESSAGE_LEN];
		char _msgtemp2[MAX_MESSAGE_LEN];


		void _register_functions();


		/* Unload all the lua scripts */
		extern void lua_unload()
		{
			// nothing to do, "vm.initialize()" already destroys lua vm before initialize
		}

		/* Initialize lua, register functions and load scripts */
		extern void lua_load(char const * scriptdir)
		{
			eventlog(eventlog_level_info, __FUNCTION__, "Loading Lua interface...");

			try
			{
				// init lua virtual machine
				vm.initialize();

				std::vector<std::string> files = dir_getfiles(scriptdir, ".lua", true);

				// load all files from the script directory
				for (int i = 0; i < files.size(); ++i)
				{
					vm.load_file(files[i].c_str());

					snprintf(_msgtemp, sizeof(_msgtemp), "%s", files[i].c_str());
					eventlog(eventlog_level_trace, __FUNCTION__, _msgtemp);
				}

				_register_functions();

				snprintf(_msgtemp, sizeof(_msgtemp), "Lua sripts were successfully loaded (%u files)", files.size());
				eventlog(eventlog_level_info, __FUNCTION__, _msgtemp);
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, e.what());
			}
			catch (...)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "lua exception\n");
			}

			// handle start event
			lua_handle_server(luaevent_server_start);
		}


		/* Register C++ functions to be able use them from lua scripts */
		void _register_functions()
		{
			// register package 'api' with functions
			static const luaL_Reg api[] =
			{
				{ "message_send_text", __message_send_text },
				{ "eventlog", __eventlog },
				{ "account_get_by_id", __account_get_by_id },
				{ "account_get_by_name", __account_get_by_name },
				{ "account_get_attr", __account_get_attr },
				{ "account_set_attr", __account_set_attr },
				{ "account_get_friends", __account_get_friends },
				{ "account_get_teams", __account_get_teams },
				{ "clan_get_members", __clan_get_members },

				{ "game_get_by_id", __game_get_by_id },
				{ "game_get_by_name", __game_get_by_name },

				{ "channel_get_by_id", __channel_get_by_id },

				{ "server_get_users", __server_get_users },
				{ "server_get_games", __server_get_games },
				{ "server_get_channels", __server_get_channels },

				{ "client_kill", __client_kill },
				{ "client_readmemory", __client_readmemory },

				{ "command_get_group", __command_get_group },
				{ "icon_get_rank", __icon_get_rank },
				{ "describe_command", __describe_command },
				{ "messagebox_show", __messagebox_show },

				{ "localize", __localize },

				{ 0, 0 }
			};
			vm.reg("api", api);

			// register standalone functions
			//vm.reg("sum", _sum); // (test function)



			// global variables
			lua::table g(vm);
			g.update("PVPGN_SOFTWARE", PVPGN_SOFTWARE);
			g.update("PVPGN_VERSION", PVPGN_VERSION);

			// config variables from bnetd.conf
			lua::transaction bind(vm);
			bind.lookup("config");
			{
				lua::table config = bind.table();
				config.update("filedir", prefs_get_filedir());
				config.update("i18ndir", prefs_get_i18ndir());
				config.update("scriptdir", prefs_get_scriptdir());
				config.update("reportdir", prefs_get_reportdir());
				config.update("chanlogdir", prefs_get_chanlogdir());
				config.update("localizefile", prefs_get_localizefile());
				config.update("motdfile", prefs_get_motdfile());
				config.update("motdw3file", prefs_get_motdw3file());
				config.update("issuefile", prefs_get_issuefile());
				config.update("channelfile", prefs_get_channelfile());
				config.update("newsfile", prefs_get_newsfile());
				config.update("adfile", prefs_get_adfile());
				config.update("topicfile", prefs_get_topicfile());
				config.update("ipbanfile", prefs_get_ipbanfile());
				config.update("helpfile", prefs_get_helpfile());
				config.update("mpqfile", prefs_get_mpqfile());
				config.update("logfile", prefs_get_logfile());
				config.update("realmfile", prefs_get_realmfile());
				config.update("maildir", prefs_get_maildir());
				config.update("versioncheck_file", prefs_get_versioncheck_file());
				config.update("mapsfile", prefs_get_mapsfile());
				config.update("xplevelfile", prefs_get_xplevel_file());
				config.update("xpcalcfile", prefs_get_xpcalc_file());
				config.update("ladderdir", prefs_get_ladderdir());
				config.update("command_groups_file", prefs_get_command_groups_file());
				config.update("tournament_file", prefs_get_tournament_file());
				config.update("statusdir", prefs_get_outputdir());
				config.update("aliasfile", prefs_get_aliasfile());
				config.update("anongame_infos_file", prefs_get_anongame_infos_file());
				config.update("DBlayoutfile", prefs_get_DBlayoutfile());
				config.update("supportfile", prefs_get_supportfile());
				config.update("transfile", prefs_get_transfile());
				config.update("customicons_file", prefs_get_customicons_file());
				config.update("loglevels", prefs_get_loglevels());
				config.update("d2cs_version", prefs_get_d2cs_version());
				config.update("allow_d2cs_setname", prefs_allow_d2cs_setname());
				config.update("iconfile", prefs_get_iconfile());
				config.update("war3_iconfile", prefs_get_war3_iconfile());
				config.update("star_iconfile", prefs_get_star_iconfile());
				config.update("tosfile", prefs_get_tosfile());
				config.update("allowed_clients", prefs_get_allowed_clients());
				config.update("skip_versioncheck", prefs_get_skip_versioncheck());
				config.update("allow_bad_version", prefs_get_allow_bad_version());
				config.update("allow_unknown_version", prefs_get_allow_unknown_version());
				config.update("version_exeinfo_match", prefs_get_version_exeinfo_match());
				config.update("version_exeinfo_maxdiff", prefs_get_version_exeinfo_maxdiff());
				config.update("usersync", prefs_get_user_sync_timer());
				config.update("userflush", prefs_get_user_flush_timer());
				config.update("userstep", prefs_get_user_step());
				config.update("latency", prefs_get_latency());
				config.update("nullmsg", prefs_get_nullmsg());
				config.update("shutdown_delay", prefs_get_shutdown_delay());
				config.update("shutdown_decr", prefs_get_shutdown_decr());
				config.update("ipban_check_int", prefs_get_ipban_check_int());
				config.update("new_accounts", prefs_get_allow_new_accounts());
				config.update("max_accounts", prefs_get_max_accounts());
				config.update("kick_old_login", prefs_get_kick_old_login());
				config.update("ask_new_channel", prefs_get_ask_new_channel());
				config.update("report_all_games", prefs_get_report_all_games());
				config.update("report_diablo_games", prefs_get_report_diablo_games());
				config.update("hide_pass_games", prefs_get_hide_pass_games());
				config.update("hide_started_games", prefs_get_hide_started_games());
				config.update("hide_temp_channels", prefs_get_hide_temp_channels());
				config.update("disc_is_loss", prefs_get_discisloss());
				config.update("ladder_games", prefs_get_ladder_games());
				config.update("ladder_prefix", prefs_get_ladder_prefix());
				config.update("enable_conn_all", prefs_get_enable_conn_all());
				config.update("hide_addr", prefs_get_hide_addr());
				config.update("chanlog", prefs_get_chanlog());
				config.update("quota", prefs_get_quota());
				config.update("quota_lines", prefs_get_quota_lines());
				config.update("quota_time", prefs_get_quota_time());
				config.update("quota_wrapline", prefs_get_quota_wrapline());
				config.update("quota_maxline", prefs_get_quota_maxline());
				config.update("quota_dobae", prefs_get_quota_dobae());
				config.update("mail_support", prefs_get_mail_support());
				config.update("mail_quota", prefs_get_mail_quota());
				config.update("log_notice", prefs_get_log_notice());
				config.update("passfail_count", prefs_get_passfail_count());
				config.update("passfail_bantime", prefs_get_passfail_bantime());
				config.update("maxusers_per_channel", prefs_get_maxusers_per_channel());
				config.update("savebyname", prefs_get_savebyname());
				config.update("sync_on_logoff", prefs_get_sync_on_logoff());
				config.update("hashtable_size", prefs_get_hashtable_size());
				config.update("account_allowed_symbols", prefs_get_account_allowed_symbols());
				config.update("account_force_username", prefs_get_account_force_username());
				config.update("max_friends", prefs_get_max_friends());
				config.update("track", prefs_get_track());
				config.update("trackaddrs", prefs_get_trackserv_addrs());
				config.update("location", prefs_get_location());
				config.update("description", prefs_get_description());
				config.update("url", prefs_get_url());
				config.update("contact_name", prefs_get_contact_name());
				config.update("contact_email", prefs_get_contact_email());
				config.update("servername", prefs_get_servername());
				config.update("max_connections", prefs_get_max_connections());
				config.update("max_concurrent_logins", prefs_get_max_concurrent_logins());
				config.update("use_keepalive", prefs_get_use_keepalive());
				config.update("max_conns_per_IP", prefs_get_max_conns_per_IP());
				config.update("servaddrs", prefs_get_bnetdserv_addrs());
				config.update("udptest_port", prefs_get_udptest_port());
				config.update("w3routeaddr", prefs_get_w3route_addr());
				config.update("initkill_timer", prefs_get_initkill_timer());
				config.update("wolv1addrs", prefs_get_wolv1_addrs());
				config.update("wolv2addrs", prefs_get_wolv2_addrs());
				config.update("wgameresaddrs", prefs_get_wgameres_addrs());
				config.update("apiregaddrs", prefs_get_apireg_addrs());
				config.update("woltimezone", prefs_get_wol_timezone());
				config.update("wollongitude", prefs_get_wol_longitude());
				config.update("wollatitude", prefs_get_wol_latitude());
				config.update("wol_autoupdate_serverhost", prefs_get_wol_autoupdate_serverhost());
				config.update("wol_autoupdate_username", prefs_get_wol_autoupdate_username());
				config.update("wol_autoupdate_password", prefs_get_wol_autoupdate_password());
				config.update("ircaddrs", prefs_get_irc_addrs());
				config.update("irc_network_name", prefs_get_irc_network_name());
				config.update("hostname", prefs_get_hostname());
				config.update("irc_latency", prefs_get_irc_latency());
				config.update("telnetaddrs", prefs_get_telnet_addrs());
				config.update("war3_ladder_update_secs", prefs_get_war3_ladder_update_secs());
				config.update("XML_output_ladder", prefs_get_XML_output_ladder());
				config.update("output_update_secs", prefs_get_output_update_secs());
				config.update("XML_status_output", prefs_get_XML_status_output());
				config.update("clan_newer_time", prefs_get_clan_newer_time());
				config.update("clan_max_members", prefs_get_clan_max_members());
				config.update("clan_channel_default_private", prefs_get_clan_channel_default_private());
				config.update("clan_min_invites", prefs_get_clan_min_invites());
			}

		}





		/* Lua Events (called from scripts) */
#ifndef _LUA_EVENTS_

		extern int lua_handle_command(t_connection * c, char const * text, t_luaevent_type luaevent)
		{
			t_account * account;
			const char * func_name;
			int result = 0;
			switch (luaevent)
			{
			case luaevent_command:
				func_name = "handle_command";
				break;
			case luaevent_command_before:
				func_name = "handle_command_before";
				break;
			default:
				return result;
			}
			try
			{
				if (!(account = conn_get_account(c)))
					return 0;

				std::map<std::string, std::string> o_account = get_account_object(account);
				lua::transaction(vm) << lua::lookup(func_name) << o_account << text << lua::invoke >> result << lua::end; // invoke lua function

			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, e.what());
			}
			catch (...)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "lua exception\n");
			}
			return result;
		}

		extern void lua_handle_game(t_game * game, t_connection * c, t_luaevent_type luaevent)
		{
			t_account * account;
			const char * func_name;
			switch (luaevent)
			{
			case luaevent_game_create:
				func_name = "handle_game_create";
				break;
			case luaevent_game_report:
				func_name = "handle_game_report";
				break;
			case luaevent_game_end:
				func_name = "handle_game_end";
				break;
			case luaevent_game_destroy:
				func_name = "handle_game_destroy";
				break;
			case luaevent_game_changestatus:
				func_name = "handle_game_changestatus";
				break;
			case luaevent_game_userjoin:
				func_name = "handle_game_userjoin";
				break;
			case luaevent_game_userleft:
				func_name = "handle_game_userleft";
				break;
			default:
				return;
			}
			try
			{
				std::map<std::string, std::string> o_game = get_game_object(game);

				// handle_game_userjoin & handle_game_userleft
				if (c)
				{
					if (!(account = conn_get_account(c)))
						return;

					std::map<std::string, std::string> o_account = get_account_object(account);
					lua::transaction(vm) << lua::lookup(func_name) << o_game << o_account << lua::invoke << lua::end; // invoke lua function
				}
				// other functions
				else
				{
					lua::transaction(vm) << lua::lookup(func_name) << o_game << lua::invoke << lua::end; // invoke lua function
				}

			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, e.what());
			}
			catch (...)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "lua exception\n");
			}
		}

		std::vector<t_game*> lua_handle_game_list(t_connection * c)
		{
			t_account * account;
			std::vector<std::string> columns, data;
			std::vector<t_game*> result;
			try
			{
				if (!(account = conn_get_account(c)))
					return result;

				std::map<std::string, std::string> o_account = get_account_object(account);
				lua::transaction(vm) << lua::lookup("handle_game_list") << o_account << lua::invoke >> columns >> data << lua::end; // invoke lua function
			
				// check consistency of data and columns
				if (columns.size() != data.size() || std::floor(data.size() / (columns.size()) != data.size() / columns.size()))
					return result;

				// fill map result
				for (std::vector<std::string>::size_type i = 1; i < data.size(); i += columns.size())
				{
					// init empty game struct
					t_game * game = (t_game*)xmalloc(sizeof(t_game));
					game->id = 0;
					game->name = NULL;

					// next columns 
					for (int j = 1; j < columns.size(); j++)
					{
						if (columns[j] == "id")
							game->id = atoi(data[i+j-1].c_str());
						else if (columns[j] == "name")
							game->name = xstrdup(data[i + j - 1].c_str());
					}
					result.push_back(game);
				}
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, e.what());
			}
			catch (...)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "lua exception\n");
			}
			return result;
		}


		extern int lua_handle_channel(t_channel * channel, t_connection * c, char const * message_text, t_message_type message_type, t_luaevent_type luaevent)
		{
			int result = 0;
			t_account * account;
			const char * func_name;
			switch (luaevent)
			{
			case luaevent_channel_message:
				func_name = "handle_channel_message";
				break;
			case luaevent_channel_userjoin:
				func_name = "handle_channel_userjoin";
				break;
			case luaevent_channel_userleft:
				func_name = "handle_channel_userleft";
				break;
			default:
				return 0;
			}
			try
			{
				if (!(account = conn_get_account(c)))
					return 0;

				std::map<std::string, std::string> o_account = get_account_object(account);
				std::map<std::string, std::string> o_channel = get_channel_object(channel);

				// handle_channel_userleft & handle_channel_message
				if (message_text)
					lua::transaction(vm) << lua::lookup(func_name) << o_channel << o_account << message_text << message_type << lua::invoke >> result << lua::end; // invoke lua function
				// other functions
				else
					lua::transaction(vm) << lua::lookup(func_name) << o_channel << o_account << lua::invoke << lua::end; // invoke lua function
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, e.what());
			}
			catch (...)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "lua exception\n");
			}
			return result;
		}


		extern int lua_handle_user(t_connection * c, t_connection * c_dst, char const * message_text, t_luaevent_type luaevent)
		{
			t_account * account, *account_dst;
			const char * func_name;
			int result = 0;
			switch (luaevent)
			{
			case luaevent_user_whisper:
				func_name = "handle_user_whisper";
				break;
			case luaevent_user_login:
				func_name = "handle_user_login";
				break;
			case luaevent_user_disconnect:
				func_name = "handle_user_disconnect";
				break;
			default:
				return 0;
			}
			try
			{
				if (!(account = conn_get_account(c)))
					return 0;

				std::map<std::string, std::string> o_account = get_account_object(account);

				// handle_server_whisper
				if (c_dst && message_text)
				{
					if (!(account_dst = conn_get_account(c_dst)))
						return 0;
					std::map<std::string, std::string> o_account_dst = get_account_object(account_dst);

					lua::transaction(vm) << lua::lookup(func_name) << o_account << o_account_dst << message_text << lua::invoke >> result << lua::end; // invoke lua function
				}
				// other functions
				else
					lua::transaction(vm) << lua::lookup(func_name) << o_account << lua::invoke << lua::end; // invoke lua function
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, e.what());
			}
			catch (...)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "lua exception\n");
			}
			return result;
		}

		extern const char * lua_handle_user_icon(t_connection * c, const char * iconinfo)
		{
			t_account * account;
			const char * result;
			try
			{
				if (!(account = conn_get_account(c)))
					return 0;
				std::map<std::string, std::string> o_account = get_account_object(account);

				lua::transaction(vm) << lua::lookup("handle_user_icon") << o_account << iconinfo << lua::invoke >> result << lua::end; // invoke lua function
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, e.what());
			}
			catch (...)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "lua exception\n");
			}
			return result;
		}



		extern void lua_handle_server(t_luaevent_type luaevent)
		{
			const char * func_name;
			switch (luaevent)
			{
			case luaevent_server_start:
				func_name = "main"; // when all lua scripts are loaded
				break;
			case luaevent_server_mainloop:
				func_name = "handle_server_mainloop"; // one time per second
				break;
			case luaevent_server_rehash:
				func_name = "handle_server_rehash"; // when restart Lua VM
				break;
			default:
				return;
			}
			try
			{
				lua::transaction(vm) << lua::lookup(func_name) << lua::invoke << lua::end; // invoke lua function
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, e.what());
			}
			catch (...)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "lua exception\n");
			}
		}

		extern void lua_handle_client(t_connection * c, int request_id, std::vector<int> data, t_luaevent_type luaevent)
		{
			t_account * account;
			const char * func_name;
			switch (luaevent)
			{
			case luaevent_client_readmemory:
				func_name = "handle_client_readmemory";
				break;
			default:
				return;
			}
			try
			{
				if (!(account = conn_get_account(c)))
					return;

				std::map<std::string, std::string> o_account = get_account_object(account);

				lua::transaction(vm) << lua::lookup(func_name) << o_account << request_id << data << lua::invoke << lua::end; // invoke lua function
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, e.what());
			}
			catch (...)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "lua exception\n");
			}
		}
#endif


	}
}
#endif