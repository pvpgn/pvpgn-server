/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999,2000 Ross Combs (rocombs@cs.nmsu.edu)
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
#include "common/setup_before.h"
#include "game.h"

#include <cerrno>
#include <cstring>
#include <cassert>

#include "compat/rename.h"
#include "compat/strcasecmp.h"
#include "common/eventlog.h"
#include "common/addr.h"
#include "common/bnettime.h"

#include "connection.h"
#include "channel.h"
#include "server.h"
#include "account.h"
#include "account_wrap.h"
#include "prefs.h"
#include "watch.h"
#include "realm.h"
#include "ladder.h"
#include "game_conv.h"
#include "common/setup_after.h"

#ifdef WITH_LUA
#include "luainterface.h"
#endif
namespace pvpgn
{

	namespace bnetd
	{

		DECLARE_ELIST_INIT(gamelist_head);
		static int glist_length = 0;
		static int totalcount = 0;


		static void game_choose_host(t_game * game);
		static void game_destroy(t_game * game);
		static int game_report(t_game * game);


		static void game_choose_host(t_game * game)
		{
			unsigned int i;

			if (game->count < 1)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "game has had no connections?!");
				return;
			}
			if (!game->connections)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "game has NULL connections array");
				return;
			}

			for (i = 0; i < game->count; i++)
			if (game->connections[i])
			{
				game->owner = game->connections[i];
				game->addr = conn_get_game_addr(game->connections[i]);
				game->port = conn_get_game_port(game->connections[i]);
				return;
			}
			eventlog(eventlog_level_warn, __FUNCTION__, "no valid connections found");
		}


		extern char const * game_type_get_str(t_game_type type)
		{
			switch (type)
			{
			case game_type_none:
				return "NONE";

			case game_type_melee:
				return "melee";

			case game_type_topvbot:
				return "top vs bottom";

			case game_type_ffa:
				return "free for all";

			case game_type_oneonone:
				return "one on one";

			case game_type_ctf:
				return "capture the flag";

			case game_type_greed:
				return "greed";

			case game_type_slaughter:
				return "slaughter";

			case game_type_sdeath:
				return "sudden death";

			case game_type_ladder:
				return "ladder";

			case game_type_ironman:
				return "ironman";

			case game_type_mapset:
				return "mapset";

			case game_type_teammelee:
				return "team melee";

			case game_type_teamffa:
				return "team free for all";

			case game_type_teamctf:
				return "team capture the flag";

			case game_type_pgl:
				return "PGL";

			case game_type_diablo:
				return "Diablo";

			case game_type_diablo2open:
				return "Diablo II (open)";

			case game_type_diablo2closed:
				return "Diablo II (closed)";

			case game_type_all:
			default:
				return "UNKNOWN";
			}
		}


		extern char const * game_status_get_str(t_game_status status)
		{
			switch (status)
			{
			case game_status_started:
				return "started";

			case game_status_full:
				return "full";

			case game_status_open:
				return "open";

			case game_status_done:
				return "done";

			case game_status_loaded:
				return "loaded";

			default:
				return "UNKNOWN";
			}
		}


		extern char const * game_result_get_str(t_game_result result)
		{
			switch (result)
			{
			case game_result_none:
				return "NONE";

			case game_result_win:
				return "WIN";

			case game_result_loss:
				return "LOSS";

			case game_result_draw:
				return "DRAW";

			case game_result_disconnect:
				return "DISCONNECT";

			case game_result_observer:
				return "OBSERVER";

			default:
				return "UNKNOWN";
			}
		}


		extern char const * game_option_get_str(t_game_option option)
		{
			switch (option)
			{
			case game_option_melee_normal:
				return "normal";
			case game_option_ffa_normal:
				return "normal";
			case game_option_oneonone_normal:
				return "normal";
			case game_option_ctf_normal:
				return "normal";
			case game_option_greed_10000:
				return "10000 minerals";
			case game_option_greed_7500:
				return "7500 minerals";
			case game_option_greed_5000:
				return "5000 minerals";
			case game_option_greed_2500:
				return "2500 minerals";
			case game_option_slaughter_60:
				return "60 minutes";
			case game_option_slaughter_45:
				return "45 minutes";
			case game_option_slaughter_30:
				return "30 minutes";
			case game_option_slaughter_15:
				return "15 minutes";
			case game_option_sdeath_normal:
				return "normal";
			case game_option_ladder_countasloss:
				return "count as loss";
			case game_option_ladder_nopenalty:
				return "no penalty";
			case game_option_mapset_normal:
				return "normal";
			case game_option_teammelee_4:
				return "4 teams";
			case game_option_teammelee_3:
				return "3 teams";
			case game_option_teammelee_2:
				return "2 teams";
			case game_option_teamffa_4:
				return "4 teams";
			case game_option_teamffa_3:
				return "3 teams";
			case game_option_teamffa_2:
				return "2 teams";
			case game_option_teamctf_4:
				return "4 teams";
			case game_option_teamctf_3:
				return "3 teams";
			case game_option_teamctf_2:
				return "2 teams";
			case game_option_topvbot_7:
				return "7 vs all";
			case game_option_topvbot_6:
				return "6 vs all";
			case game_option_topvbot_5:
				return "5 vs all";
			case game_option_topvbot_4:
				return "4 vs all";
			case game_option_topvbot_3:
				return "3 vs all";
			case game_option_topvbot_2:
				return "2 vs all";
			case game_option_topvbot_1:
				return "1 vs all";

			case game_option_none:
				return "none";
			default:
				return "UNKNOWN";
			}
		}


		extern char const * game_maptype_get_str(t_game_maptype maptype)
		{
			switch (maptype)
			{
			case game_maptype_selfmade:
				return "Self-Made";
			case game_maptype_blizzard:
				return "Blizzard";
			case game_maptype_ladder:
				return "Ladder";
			case game_maptype_pgl:
				return "PGL";
			case game_maptype_kbk:
				return "KBK";
			case game_maptype_compusa:
				return "CompUSA";
			default:
				return "Unknown";
			}
		}


		extern char const * game_tileset_get_str(t_game_tileset tileset)
		{
			switch (tileset)
			{
			case game_tileset_badlands:
				return "Badlands";
			case game_tileset_space:
				return "Space";
			case game_tileset_installation:
				return "Installation";
			case game_tileset_ashworld:
				return "Ash World";
			case game_tileset_jungle:
				return "Jungle";
			case game_tileset_desert:
				return "Desert";
			case game_tileset_ice:
				return "Ice";
			case game_tileset_twilight:
				return "Twilight";
			default:
				return "Unknown";
			}
		}


		extern char const * game_speed_get_str(t_game_speed speed)
		{
			switch (speed)
			{
			case game_speed_slowest:
				return "slowest";
			case game_speed_slower:
				return "slower";
			case game_speed_slow:
				return "slow";
			case game_speed_normal:
				return "normal";
			case game_speed_fast:
				return "fast";
			case game_speed_faster:
				return "faster";
			case game_speed_fastest:
				return "fastest";
			default:
				return "unknown";
			}
		}


		extern char const * game_difficulty_get_str(unsigned difficulty)
		{
			switch (difficulty)
			{
			case game_difficulty_normal:
				return "normal";
			case game_difficulty_nightmare:
				return "nightmare";
			case game_difficulty_hell:
				return "hell";
			case game_difficulty_hardcore_normal:
				return "hardcore normal";
			case game_difficulty_hardcore_nightmare:
				return "hardcore nightmare";
			case game_difficulty_hardcore_hell:
				return "hardcore hell";
			default:
				return "unknown";
			}
		}


		extern t_game * game_create(char const * name, char const * pass, char const * info, t_game_type type, int startver, t_clienttag clienttag, unsigned long gameversion)
		{
			t_game * game;

			if (!name)
			{
				eventlog(eventlog_level_info, __FUNCTION__, "got NULL game name");
				return NULL;
			}
			if (!pass)
			{
				eventlog(eventlog_level_info, __FUNCTION__, "got NULL game pass");
				return NULL;
			}
			if (!info)
			{
				eventlog(eventlog_level_info, __FUNCTION__, "got NULL game info");
				return NULL;
			}

			if (gamelist_find_game_available(name, clienttag, game_type_all))
			{
				eventlog(eventlog_level_info, __FUNCTION__, "game \"%s\" not created because it already exists", name);
				return NULL; /* already have a game by that name */
			}

			game = (t_game*)xmalloc(sizeof(t_game));
			game->name = xstrdup(name);
			game->pass = xstrdup(pass);
			game->info = xstrdup(info);
			if (!(game->clienttag = clienttag))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got UNKNOWN clienttag");
				xfree((void *)game->info); /* avoid warning */
				xfree((void *)game->pass); /* avoid warning */
				xfree((void *)game->name); /* avoid warning */
				xfree(game);
				return NULL;
			}

			game->type = type;
			game->addr = 0; /* will be set by first player */
			game->port = 0; /* will be set by first player */
			game->version = gameversion;
			game->startver = startver; /* start packet version */
			game->status = game_status_open;
			game->realm = 0;
			game->realmname = NULL;
			game->id = ++totalcount;
			game->mapname = NULL;
			game->ref = 0;
			game->count = 0;
			game->owner = NULL;
			game->connections = NULL;
			game->players = NULL;
			game->results = NULL;
			game->reported_results = NULL;
			game->report_heads = NULL;
			game->report_bodies = NULL;
			game->create_time = now;
			game->start_time = (std::time_t)0;
			game->lastaccess_time = now;
			game->option = game_option_none;
			game->maptype = game_maptype_none;
			game->tileset = game_tileset_none;
			game->speed = game_speed_none;
			game->mapsize_x = 0;
			game->mapsize_y = 0;
			game->maxplayers = 0;
			game->bad = 0;
			game->description = NULL;
			game->flag = std::strcmp(pass, "") ? game_flag_private : game_flag_none;
			game->difficulty = game_difficulty_none;

			game_parse_info(game, info);

			elist_add(&gamelist_head, &game->glist_link);
			glist_length++;

			eventlog(eventlog_level_info, __FUNCTION__, "game \"%s\" (pass \"%s\") type %hu(%s) startver %d created", name, pass, (unsigned short)type, game_type_get_str(game->type), startver);

			return game;
		}


		static void game_destroy(t_game * game)
		{
			unsigned int i;

			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return;
			}
			
#ifdef WITH_LUA
			lua_handle_game(game, NULL, luaevent_game_destroy);
#endif

			elist_del(&game->glist_link);
			glist_length--;

			if (game->realmname)
			{
				realm_add_game_number(realmlist_find_realm(game->realmname), -1);
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "game \"%s\" (count=%u ref=%u) removed from list...", game_get_name(game), game->count, game->ref);

			for (i = 0; i < game->count; i++)
			{
				if (game->report_bodies && game->report_bodies[i])
					xfree((void *)game->report_bodies[i]); /* avoid warning */
				if (game->report_heads && game->report_heads[i])
					xfree((void *)game->report_heads[i]); /* avoid warning */
				if (game->reported_results && game->reported_results[i])
					xfree((void *)game->reported_results[i]);
			}
			if (game->realmname)
				xfree((void *)game->realmname); /* avoid warining */
			if (game->report_bodies)
				xfree((void *)game->report_bodies); /* avoid warning */
			if (game->report_heads)
				xfree((void *)game->report_heads); /* avoid warning */
			if (game->results)
				xfree((void *)game->results); /* avoid warning */
			if (game->reported_results)
				xfree((void *)game->reported_results);
			if (game->connections)
				xfree((void *)game->connections); /* avoid warning */
			if (game->players)
				xfree((void *)game->players); /* avoid warning */
			if (game->mapname)
				xfree((void *)game->mapname); /* avoid warning */
			if (game->description)
				xfree((void *)game->description); /* avoid warning */

			xfree((void *)game->info); /* avoid warning */
			xfree((void *)game->pass); /* avoid warning */
			if (game->name) xfree((void *)game->name); /* avoid warning */
			xfree((void *)game); /* avoid warning */

			eventlog(eventlog_level_info, __FUNCTION__, "game deleted");

			return;
		}

		static int game_evaluate_results(t_game * game)
		{
			unsigned int i, j;
			unsigned int wins, losses, draws, disconnects, reports;

			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			if (!game->results)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "results array is NULL");
				return -1;
			}

			if (!game->reported_results)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "reported_results array is NULL");
				return -1;
			}

			int discisloss = game_discisloss(game);

			for (i = 0; i < game->count; i++)
			{
				wins = losses = draws = disconnects = reports = 0;

				for (j = 0; j < game->count; j++)
				{
					if (game->reported_results[j])
					{
						switch (game->reported_results[j][i])
						{
						case game_result_win:
							wins++;
							reports++;
							break;
						case game_result_loss:
							losses++;
							reports++;
							break;
						case game_result_draw:
							draws++;
							reports++;
							break;
						case game_result_disconnect:
							disconnects++;
							reports++;
							break;
						default:
							break;
						}
					}
				}
				eventlog(eventlog_level_debug, __FUNCTION__, "wins: %u losses: %u draws: %u disconnects: %u", wins, losses, draws, disconnects);

				//now decide what result we give
				if (!(reports)) // no results at all - game canceled before starting
				{
					game->results[i] = game_result_none;
					eventlog(eventlog_level_debug, __FUNCTION__, "deciding to give \"none\" to player %d", i);
				}
				else if ((disconnects >= draws) && (disconnects >= losses) && (disconnects >= wins))
				{
					if (discisloss)
					{
						game->results[i] = game_result_loss;         //losses are also bad...
						eventlog(eventlog_level_debug, __FUNCTION__, "deciding to give \"loss\" to player %d (due to discisloss)", i);
					}
					else
					{
						game->results[i] = game_result_disconnect; //consider disconnects the worst case...
						eventlog(eventlog_level_debug, __FUNCTION__, "deciding to give \"disconnect\" to player %d", i);
					}
				}
				else if ((losses >= wins) && (losses >= draws))
				{
					game->results[i] = game_result_loss;         //losses are also bad...
					eventlog(eventlog_level_debug, __FUNCTION__, "deciding to give \"loss\" to player %d", i);
				}
				else if ((draws >= wins))
				{
					game->results[i] = game_result_draw;
					eventlog(eventlog_level_debug, __FUNCTION__, "deciding to give \"draw\" to player %d", i);
				}
				else if (wins)
				{
					game->results[i] = game_result_win;
					eventlog(eventlog_level_debug, __FUNCTION__, "deciding to give \"win\" to player %d", i);
				}
			}
			return 0;
		}

		static int game_match_type(t_game_type type, const char *gametypes)
		{
			char *p, *q;
			int res;

			if (!gametypes || !gametypes[0]) return 0;

			gametypes = p = xstrdup(gametypes);
			res = 0;
			do {
				q = std::strchr(p, ',');
				if (q) *q = '\0';
				if (!strcasecmp(p, "topvbot")) {
					if (type == game_type_topvbot) { res = 1; break; }
				}
				else if (!strcasecmp(p, "melee")) {
					if (type == game_type_melee) { res = 1; break; }
				}
				else if (!strcasecmp(p, "ffa")) {
					if (type == game_type_ffa) { res = 1; break; }
				}
				else if (!strcasecmp(p, "oneonone")) {
					if (type == game_type_oneonone) { res = 1; break; }
				}
				if (q) p = q + 1;
			} while (q);

			free((void*)gametypes);
			return res;
		}

		static int game_sanity_check(t_game_result * results, t_account * * players, unsigned int count, unsigned int discisloss)
		{
			unsigned int winners = 0;
			unsigned int losers = 0;
			unsigned int draws = 0;
			unsigned int discs = 0;

			for (unsigned int curr = 0; curr < count; curr++)
			{
				if (!players[curr])
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL player[%u] (of %u)", curr, count);
					return -1;
				}

				switch (results[curr])
				{
				case game_result_win:
					winners++;
					break;
				case game_result_loss:
					losers++;
					break;
				case game_result_draw:
					draws++;
					break;
				case game_result_disconnect:
					discs++;
					break;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "bad results[%u]=%u", curr, (unsigned int)results[curr]);
					return -1;
				}
			}

			if (draws > 0)
			{
				if (draws != count)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "some, but not all players had a draw count=%u (winners=%u losers=%u draws=%u)", count, winners, losers, draws);
					return -1;
				}
				return 0;
			}

			if ((discisloss) && ((losers < 1) || (winners<1) || (winners>1 && (winners != losers))))
			{
				eventlog(eventlog_level_info, __FUNCTION__, "missing winner or loser for count=%u (winners=%u losers=%u)", count, winners, losers);
				return -1;
			}

			return 0;
		}

		static int game_report(t_game * game)
		{
			std::FILE *          fp;
			char *          realname;
			char *          tempname;
			unsigned int    i;
			unsigned int    realcount;
			t_ladder_info * ladder_info = NULL;
			char            clienttag_str[5];

			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			if (!game->clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got UNKNOWN clienttag");
				return -1;
			}
			if (!game->players)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "player array is NULL");
				return -1;
			}
			if (!game->reported_results)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "reported_results array is NULL");
				return -1;
			}
			if (!game->results)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "results array is NULL");
				return -1;
			}

#ifdef WITH_LUA
			lua_handle_game(game, NULL, luaevent_game_end);
#endif

			if (game->clienttag == CLIENTTAG_WARCRAFT3_UINT || game->clienttag == CLIENTTAG_WAR3XP_UINT)
				// war3 game reporting is done elsewhere, so we can skip this function
				return 0;

			if ((game->clienttag == CLIENTTAG_WCHAT_UINT) || (game->clienttag == CLIENTTAG_REDALERT_UINT)
				|| (game->clienttag == CLIENTTAG_DUNE2000_UINT) || (game->clienttag == CLIENTTAG_NOX_UINT)
				|| (game->clienttag == CLIENTTAG_NOXQUEST_UINT) || (game->clienttag == CLIENTTAG_RENEGADE_UINT)
				|| (game->clienttag == CLIENTTAG_RENGDFDS_UINT) || (game->clienttag == CLIENTTAG_EMPERORBD_UINT)
				|| (game->clienttag == CLIENTTAG_LOFLORE3_UINT) || (game->clienttag == CLIENTTAG_WWOL_UINT))
				// PELISH: We are not supporting ladders for all WOL clients yet
				return 0;

			if (game->clienttag == CLIENTTAG_DIABLOSHR_UINT ||
				game->clienttag == CLIENTTAG_DIABLORTL_UINT ||
				game->clienttag == CLIENTTAG_DIABLO2ST_UINT ||
				game->clienttag == CLIENTTAG_DIABLO2DV_UINT ||
				game->clienttag == CLIENTTAG_DIABLO2XP_UINT)
			{
				if (prefs_get_report_diablo_games() == 1)
					/* diablo games have transient players and no reported winners/losers */
					realcount = 0;
				else
				{
					eventlog(eventlog_level_info, __FUNCTION__, "diablo gamereport disabled: ignoring game");
					return 0;
				}
			}
			else
			{
				game_evaluate_results(game); // evaluate results from the reported results
				/* "compact" the game; move all the real players to the top... */
				realcount = 0;
				for (i = 0; i < game->count; i++)
				{
					if (!game->players[i])
					{
						eventlog(eventlog_level_error, __FUNCTION__, "player slot %u has NULL account", i);
						continue;
					}

					if (game->results[i] != game_result_none)
					{
						game->players[realcount] = game->players[i];
						game->results[realcount] = game->results[i];
						game->report_heads[realcount] = game->report_heads[i];
						game->report_bodies[realcount] = game->report_bodies[i];
						realcount++;
					}
				}

				/* then nuke duplicate players after the real players */
				for (i = realcount; i < game->count; i++)
				{
					game->players[i] = NULL;
					game->results[i] = game_result_none;
					game->report_heads[i] = NULL;
					game->report_bodies[i] = NULL;
				}

				if (realcount < 1)
				{
					eventlog(eventlog_level_info, __FUNCTION__, "ignoring game");
					return -1;
				}

				if (game_sanity_check(game->results, game->players, realcount, game_is_ladder(game) && game_discisloss(game)) < 0)
				{
					eventlog(eventlog_level_info, __FUNCTION__, "game results ignored due to inconsistencies");
					game->bad = 1;
				}

				if (((tag_check_wolv1(game->clienttag)) || (tag_check_wolv2(game->clienttag))) && (realcount == 2)
					&& (game->results[0] == game_result_disconnect) && (game->results[0] == game->results[1])) {
					//FIXME: Find more general solution for that (we are not supporting more than 2 players now)
					DEBUG0("WOL Both players got game_result_disconnect - ignoring game");
					game->bad = 1;
				}
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "realcount=%d count=%u", realcount, game->count);

			if (realcount >= 1 && !game->bad)
			{
				if (game_is_ladder(game))
				{
					t_ladder_id id;

					if (game_get_type(game) == game_type_ironman)
						id = ladder_id_ironman;
					else
						id = ladder_id_normal;

					for (i = 0; i < realcount; i++)
					{
						eventlog(eventlog_level_debug, __FUNCTION__, "realplayer %u result=%u", i + 1, (unsigned int)game->results[i]);

						if ((tag_check_wolv1(game->clienttag)) || (tag_check_wolv2(game->clienttag))) {
							id = ladder_id_solo;
							ladder_init_account_wol(game->players[i], game->clienttag, id);
							switch (game->results[i])
							{
							case game_result_win:
								account_inc_ladder_wins(game->players[i], game->clienttag, id);
								break;
							case game_result_loss:
								account_inc_ladder_losses(game->players[i], game->clienttag, id);
								break;
							case game_result_disconnect:
								account_inc_ladder_disconnects(game->players[i], game->clienttag, id);
								break;
							default:
								eventlog(eventlog_level_error, __FUNCTION__, "bad ladder game realplayer results[%u] = %u", i, game->results[i]);
								account_inc_ladder_disconnects(game->players[i], game->clienttag, id);
							}
						}
						else {
							ladder_init_account(game->players[i], game->clienttag, id);
							switch (game->results[i])
							{
							case game_result_win:
								account_inc_ladder_wins(game->players[i], game->clienttag, id);
								account_set_ladder_last_result(game->players[i], game->clienttag, id, game_result_get_str(game_result_win));
								break;
							case game_result_loss:
								account_inc_ladder_losses(game->players[i], game->clienttag, id);
								account_set_ladder_last_result(game->players[i], game->clienttag, id, game_result_get_str(game_result_loss));
								break;
							case game_result_draw:
								account_inc_ladder_draws(game->players[i], game->clienttag, id);
								account_set_ladder_last_result(game->players[i], game->clienttag, id, game_result_get_str(game_result_draw));
								break;
							case game_result_disconnect:
								account_inc_ladder_disconnects(game->players[i], game->clienttag, id);
								account_set_ladder_last_result(game->players[i], game->clienttag, id, game_result_get_str(game_result_disconnect));
								break;
							default:
								eventlog(eventlog_level_error, __FUNCTION__, "bad ladder game realplayer results[%u] = %u", i, game->results[i]);
								account_inc_ladder_disconnects(game->players[i], game->clienttag, id);
								account_set_ladder_last_result(game->players[i], game->clienttag, id, game_result_get_str(game_result_disconnect));
							}
							account_set_ladder_last_time(game->players[i], game->clienttag, id, bnettime());
						}
					}

					if ((tag_check_wolv1(game->clienttag)) || (tag_check_wolv2(game->clienttag))) {
						id = ladder_id_solo;
						ladder_update_wol(game->clienttag, id, game->players, game->results);
					}
					else {
						ladder_info = (t_ladder_info*)xmalloc(sizeof(t_ladder_info)*realcount);
						if (ladder_update(game->clienttag, id,
							realcount, game->players, game->results, ladder_info) < 0)
						{
							eventlog(eventlog_level_info, __FUNCTION__, "unable to update ladder stats");
							xfree(ladder_info);
							ladder_info = NULL;
						}
					}
				}
				else
				{
					if (!(tag_check_wolv1(game->clienttag)) || !(tag_check_wolv2(game->clienttag))) {
						for (i = 0; i < realcount; i++)
						{
							switch (game->results[i])
							{
							case game_result_win:
								account_inc_normal_wins(game->players[i], game->clienttag);
								account_set_normal_last_result(game->players[i], game->clienttag, game_result_get_str(game_result_win));
								break;
							case game_result_loss:
								account_inc_normal_losses(game->players[i], game->clienttag);
								account_set_normal_last_result(game->players[i], game->clienttag, game_result_get_str(game_result_loss));
								break;
							case game_result_draw:
								account_inc_normal_draws(game->players[i], game->clienttag);
								account_set_normal_last_result(game->players[i], game->clienttag, game_result_get_str(game_result_draw));
								break;
							case game_result_disconnect:
								account_inc_normal_disconnects(game->players[i], game->clienttag);
								account_set_normal_last_result(game->players[i], game->clienttag, game_result_get_str(game_result_disconnect));
								break;
							default:
								eventlog(eventlog_level_error, __FUNCTION__, "bad normal game realplayer results[%u] = %u", i, game->results[i]);
								account_inc_normal_disconnects(game->players[i], game->clienttag);
								account_set_normal_last_result(game->players[i], game->clienttag, game_result_get_str(game_result_disconnect));
							}
							account_set_normal_last_time(game->players[i], game->clienttag, bnettime());
						}
					}
				}
			}

			if (game_get_type(game) != game_type_ladder && prefs_get_report_all_games() != 1)
			{
				eventlog(eventlog_level_debug, __FUNCTION__, "not reporting normal games");
				return 0;
			}

			{
				struct std::tm * tmval;
				char        dstr[64];

				if (!(tmval = std::localtime(&now)))
					dstr[0] = '\0';
				else
					std::sprintf(dstr, "%04d%02d%02d%02d%02d%02d",
					1900 + tmval->tm_year,
					tmval->tm_mon + 1,
					tmval->tm_mday,
					tmval->tm_hour,
					tmval->tm_min,
					tmval->tm_sec);

				tempname = (char*)xmalloc(std::strlen(prefs_get_reportdir()) + 1 + 1 + 5 + 1 + 2 + 1 + std::strlen(dstr) + 1 + 6 + 1);
				std::sprintf(tempname, "%s/_bnetd-gr_%s_%06u", prefs_get_reportdir(), dstr, game->id);
				realname = (char*)xmalloc(std::strlen(prefs_get_reportdir()) + 1 + 2 + 1 + std::strlen(dstr) + 1 + 6 + 1);
				std::sprintf(realname, "%s/gr_%s_%06u", prefs_get_reportdir(), dstr, game->id);
			}

			if (!(fp = std::fopen(tempname, "w")))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not open report file \"%s\" for writing (std::fopen: %s)", tempname, std::strerror(errno));
				if (ladder_info)
					xfree(ladder_info);
				xfree(realname);
				xfree(tempname);
				return -1;
			}

			if (game->bad)
				std::fprintf(fp, "[ game results ignored due to inconsistencies ]\n\n");
			std::fprintf(fp, "name=\"%s\" id="GAMEID_FORMAT"\n",
				game_get_name(game),
				game->id);
			std::fprintf(fp, "clienttag=%4s type=\"%s\" option=\"%s\"\n",
				tag_uint_to_str(clienttag_str, game->clienttag),
				game_type_get_str(game->type),
				game_option_get_str(game->option));
			{
				struct std::tm * gametime;
				char        timetemp[GAME_TIME_MAXLEN];

				if (!(gametime = std::localtime(&game->create_time)))
					std::strcpy(timetemp, "?");
				else
					std::strftime(timetemp, sizeof(timetemp), GAME_TIME_FORMAT, gametime);
				std::fprintf(fp, "created=\"%s\" ", timetemp);

				if (!(gametime = std::localtime(&game->start_time)))
					std::strcpy(timetemp, "?");
				else
					std::strftime(timetemp, sizeof(timetemp), GAME_TIME_FORMAT, gametime);
				std::fprintf(fp, "started=\"%s\" ", timetemp);

				if (!(gametime = std::localtime(&now)))
					std::strcpy(timetemp, "?");
				else
					std::strftime(timetemp, sizeof(timetemp), GAME_TIME_FORMAT, gametime);
				std::fprintf(fp, "ended=\"%s\"\n", timetemp);
			}
			{
				char const * mapname;

				if (!(mapname = game_get_mapname(game)))
					mapname = "?";

				std::fprintf(fp, "mapfile=\"%s\" mapauth=\"%s\" mapsize=%ux%u tileset=\"%s\"\n",
					mapname,
					game_maptype_get_str(game_get_maptype(game)),
					game_get_mapsize_x(game), game_get_mapsize_y(game),
					game_tileset_get_str(game_get_tileset(game)));
			}
			std::fprintf(fp, "joins=%u maxplayers=%u\n",
				game_get_count(game),
				game_get_maxplayers(game));

			if (!prefs_get_hide_addr())
				std::fprintf(fp, "host=%s\n", addr_num_to_addr_str(game_get_addr(game), game_get_port(game)));

			std::fprintf(fp, "\n\n");

			if (game->clienttag == CLIENTTAG_DIABLORTL_UINT)
			for (i = 0; i < game->count; i++)
				std::fprintf(fp, "%-16s JOINED\n", account_get_name(game->players[i]));
			else
			if (ladder_info)
			for (i = 0; i < realcount; i++)
				std::fprintf(fp, "%-16s %-8s rating=%u [#%05u]  prob=%4.1f%%  K=%2u  adj=%+d\n",
				account_get_name(game->players[i]),
				game_result_get_str(game->results[i]),
				ladder_info[i].oldrating,
				ladder_info[i].oldrank,
				ladder_info[i].prob*100.0,
				ladder_info[i].k,
				ladder_info[i].adj);
			else
			for (i = 0; i < realcount; i++)
				std::fprintf(fp, "%-16s %-8s\n",
				account_get_name(game->players[i]),
				game_result_get_str(game->results[i]));
			std::fprintf(fp, "\n\n");

			if (ladder_info)
				xfree(ladder_info);

			for (i = 0; i < realcount; i++)
			{
				if (game->report_heads[i])
					std::fprintf(fp, "%s\n", game->report_heads[i]);
				else
					std::fprintf(fp, "[ game report header not available for player %u (\"%s\") ]\n", i + 1, account_get_name(game->players[i]));
				if (game->report_bodies[i])
					std::fprintf(fp, "%s\n", game->report_bodies[i]);
				else
					std::fprintf(fp, "[ game report body not available for player %u (\"%s\") ]\n\n", i + 1, account_get_name(game->players[i]));
			}
			std::fprintf(fp, "\n\n");

			if (game->clienttag == CLIENTTAG_STARCRAFT_UINT ||
				game->clienttag == CLIENTTAG_SHAREWARE_UINT ||
				game->clienttag == CLIENTTAG_BROODWARS_UINT ||
				game->clienttag == CLIENTTAG_WARCIIBNE_UINT)
			{
				for (i = 0; i < realcount; i++)
					std::fprintf(fp, "%s's normal record is now %u/%u/%u (%u draws)\n",
					account_get_name(game->players[i]),
					account_get_normal_wins(game->players[i], game->clienttag),
					account_get_normal_losses(game->players[i], game->clienttag),
					account_get_normal_disconnects(game->players[i], game->clienttag),
					account_get_normal_draws(game->players[i], game->clienttag));
			}
			if (game->clienttag == CLIENTTAG_STARCRAFT_UINT ||
				game->clienttag == CLIENTTAG_BROODWARS_UINT ||
				game->clienttag == CLIENTTAG_WARCIIBNE_UINT)
			{
				std::fprintf(fp, "\n");
				for (i = 0; i < realcount; i++)
					std::fprintf(fp, "%s's standard ladder record is now %u/%u/%u (rating %u [#%05u]) (%u draws)\n",
					account_get_name(game->players[i]),
					account_get_ladder_wins(game->players[i], game->clienttag, ladder_id_normal),
					account_get_ladder_losses(game->players[i], game->clienttag, ladder_id_normal),
					account_get_ladder_disconnects(game->players[i], game->clienttag, ladder_id_normal),
					account_get_ladder_rating(game->players[i], game->clienttag, ladder_id_normal),
					account_get_ladder_rank(game->players[i], game->clienttag, ladder_id_normal),
					account_get_ladder_draws(game->players[i], game->clienttag, ladder_id_normal));
			}
			if (game->clienttag == CLIENTTAG_WARCIIBNE_UINT)
			{
				std::fprintf(fp, "\n");
				for (i = 0; i < realcount; i++)
					std::fprintf(fp, "%s's ironman ladder record is now %u/%u/%u (rating %u [#%05u]) (%u draws)\n",
					account_get_name(game->players[i]),
					account_get_ladder_wins(game->players[i], game->clienttag, ladder_id_ironman),
					account_get_ladder_losses(game->players[i], game->clienttag, ladder_id_ironman),
					account_get_ladder_disconnects(game->players[i], game->clienttag, ladder_id_ironman),
					account_get_ladder_rating(game->players[i], game->clienttag, ladder_id_ironman),
					account_get_ladder_rank(game->players[i], game->clienttag, ladder_id_ironman),
					account_get_ladder_draws(game->players[i], game->clienttag, ladder_id_ironman));
			}

			std::fprintf(fp, "\nThis game lasted %lu minutes (elapsed).\n", ((unsigned long int)std::difftime(now, game->start_time)) / 60);
			
#ifdef WITH_LUA
			lua_handle_game(game, NULL, luaevent_game_report);
#endif

			if (std::fclose(fp) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not close report file \"%s\" after writing (std::fclose: %s)", tempname, std::strerror(errno));
				xfree(realname);
				xfree(tempname);
				return -1;
			}

			if (p_rename(tempname, realname) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not std::rename report file to \"%s\" (std::rename: %s)", realname, std::strerror(errno));
				xfree(realname);
				xfree(tempname);
				return -1;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "game report saved as \"%s\"", realname);
			xfree(realname);
			xfree(tempname);
			return 0;
		}


		extern unsigned int game_get_id(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return 0;
			}
			return game->id;
		}


		extern char const * game_get_name(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return NULL;
			}
			return game->name ? game->name : "BNet";
		}


		extern t_game_type game_get_type(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return game_type_none;
			}
			return game->type;
		}


		extern t_game_maptype game_get_maptype(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return game_maptype_none;
			}
			return game->maptype;
		}


		extern int game_set_maptype(t_game * game, t_game_maptype maptype)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			game->maptype = maptype;
			return 0;
		}


		extern t_game_tileset game_get_tileset(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return game_tileset_none;
			}
			return game->tileset;
		}


		extern int game_set_tileset(t_game * game, t_game_tileset tileset)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			game->tileset = tileset;
			return 0;
		}


		extern t_game_speed game_get_speed(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return game_speed_none;
			}
			return game->speed;
		}


		extern int game_set_speed(t_game * game, t_game_speed speed)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			game->speed = speed;
			return 0;
		}


		extern unsigned int game_get_mapsize_x(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return 0;
			}
			return game->mapsize_x;
		}


		extern int game_set_mapsize_x(t_game * game, unsigned int x)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			game->mapsize_x = x;
			return 0;
		}


		extern unsigned int game_get_mapsize_y(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return 0;
			}
			return game->mapsize_y;
		}


		extern int game_set_mapsize_y(t_game * game, unsigned int y)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			game->mapsize_y = y;
			return 0;
		}


		extern unsigned int game_get_maxplayers(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return 0;
			}
			return game->maxplayers;
		}


		extern int game_set_maxplayers(t_game * game, unsigned int maxplayers)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			game->maxplayers = maxplayers;
			return 0;
		}


		extern unsigned int game_get_difficulty(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return 0;
			}
			return game->difficulty;
		}


		extern int game_set_difficulty(t_game * game, unsigned int difficulty)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			game->difficulty = difficulty;
			return 0;
		}


		extern char const * game_get_description(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return NULL;
			}
			return game->description;
		}


		extern int game_set_description(t_game * game, char const * description)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			if (!description)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL description");
				return -1;
			}

			if (game->description != NULL) xfree((void *)game->description);
			game->description = xstrdup(description);

			return 0;
		}


		extern char const * game_get_pass(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return NULL;
			}
			return game->pass;
		}


		extern char const * game_get_info(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return NULL;
			}
			return game->info;
		}


		extern int game_get_startver(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return 0;
			}
			return game->startver;
		}


		extern unsigned long game_get_version(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return 0;
			}
			return game->version;
		}


		extern unsigned int game_get_ref(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return 0;
			}
			return game->ref;
		}


		extern unsigned int game_get_count(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return 0;
			}
			return game->count;
		}


		extern void game_set_status(t_game * game, t_game_status status)
		{
			if (!game) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return;
			}
			// [quetzal] 20020829 - this should prevent invalid status changes
			// its like started game cant become open and so on
			if (game->status == game_status_started &&
				(status == game_status_open || status == game_status_full || status == game_status_loaded)) {
				eventlog(eventlog_level_error, "game_set_status",
					"attempting to set status '%s' (%d) to started game", game_status_get_str(status), status);
				return;
			}

			if (game->status == game_status_done && status != game_status_done) {
				eventlog(eventlog_level_error, "game_set_status",
					"attempting to set status '%s' (%d) to done game", game_status_get_str(status), status);
				return;
			}

			if (status == game_status_started && game->start_time == (std::time_t)0)
				game->start_time = now;
			game->status = status;

#ifdef WITH_LUA
			lua_handle_game(game, NULL, luaevent_game_changestatus);
#endif
		}


		extern t_game_status game_get_status(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return game_status_started;
			}
			return game->status;
		}


		extern unsigned int game_get_addr(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return 0;
			}

			return game->addr; /* host byte order */
		}


		extern unsigned short game_get_port(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return 0;
			}

			return game->port; /* host byte order */
		}


		extern unsigned int game_get_latency(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return 0;
			}
			if (game->ref < 1)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "game \"%s\" has no players", game_get_name(game));
				return 0;
			}
			if (!game->players)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "game \"%s\" has NULL players array (ref=%u)", game_get_name(game), game->ref);
				return 0;
			}
			if (!game->players[0])
			{
				eventlog(eventlog_level_error, __FUNCTION__, "game \"%s\" has NULL players[0] entry (ref=%u)", game_get_name(game), game->ref);
				return 0;
			}

			return 0; /* conn_get_latency(game->players[0]); */
		}

		extern t_connection * game_get_player_conn(t_game const * game, unsigned int i)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return NULL;
			}
			if (game->ref < 1)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "game \"%s\" has no players", game_get_name(game));
				return NULL;
			}
			if (!game->players)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "game \"%s\" has NULL player array (ref=%u)", game_get_name(game), game->ref);
				return NULL;
			}
			if (!game->players[i])
			{
				eventlog(eventlog_level_error, __FUNCTION__, "game \"%s\" has NULL players[i] entry (ref=%u)", game_get_name(game), game->ref);
				return NULL;
			}
			return game->connections[i];
		}

		extern t_clienttag game_get_clienttag(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return 0;
			}
			return game->clienttag;
		}


		extern int game_add_player(t_game * game, char const * pass, int startver, t_connection * c)
		{
			t_connection * * tempc;
			t_account * *    tempp;
			t_game_result *  tempr;
			t_game_result ** temprr;
			char const * *   temprh;
			char const * *   temprb;
			unsigned int i = 0;

			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			if (!pass)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL password");
				return -1;
			}
			if (startver != STARTVER_UNKNOWN && startver != STARTVER_GW1 && startver != STARTVER_GW3 && startver != STARTVER_GW4 && startver != STARTVER_REALM1)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad game startver %d", startver);
				return -1;
			}
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (game->type == game_type_ladder && (account_get_normal_wins(conn_get_account(c), conn_get_clienttag(c)) < 10 && conn_get_wol(c) == 0))
				/* if () ... */
			{
				eventlog(eventlog_level_error, __FUNCTION__, "can not join ladder game without 10 normal wins");
				return -1;
			}

			{
				t_clienttag gt;

				if (!(gt = game_get_clienttag(game)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not get clienttag for game");
					return -1;
				}
			}

			if (game->pass[0] != '\0' && strcasecmp(game->pass, pass) != 0)
			{
				eventlog(eventlog_level_debug, __FUNCTION__, "game \"%s\" password mismatch \"%s\"!=\"%s\"", game_get_name(game), game->pass, pass);
				return -1;
			}

			if (game->connections && (game->count > 0))
			{
				for (i = 0; i < game->count; i++)
				{
					if (game->connections[i] == NULL)
					{
						game->connections[i] = c;
						game->players[i] = conn_get_account(c);
						game->results[i] = game_result_none;
						game->reported_results[i] = NULL;
						game->report_heads[i] = NULL;
						game->report_bodies[i] = NULL;

						game->ref++;
						game->lastaccess_time = now;
						break;
					}
				}

			}

			if ((i == game->count) || (game->count == 0))
			{

				if (!game->connections) /* some std::realloc()s are broken */
					tempc = (t_connection**)xmalloc((game->count + 1)*sizeof(t_connection *));
				else
					tempc = (t_connection**)xrealloc(game->connections, (game->count + 1)*sizeof(t_connection *));
				game->connections = tempc;
				if (!game->players) /* some std::realloc()s are broken */
					tempp = (t_account**)xmalloc((game->count + 1)*sizeof(t_account *));
				else
					tempp = (t_account**)xrealloc(game->players, (game->count + 1)*sizeof(t_account *));
				game->players = tempp;

				if (!game->results) /* some std::realloc()s are broken */
					tempr = (t_game_result*)xmalloc((game->count + 1)*sizeof(t_game_result));
				else
					tempr = (t_game_result*)xrealloc(game->results, (game->count + 1)*sizeof(t_game_result));
				game->results = tempr;

				if (!game->reported_results)
					temprr = (t_game_result**)xmalloc((game->count + 1)*sizeof(t_game_result *));
				else
					temprr = (t_game_result**)xrealloc(game->reported_results, (game->count + 1)*sizeof(t_game_result *));
				game->reported_results = temprr;

				if (!game->report_heads) /* some xrealloc()s are broken */
					temprh = (const char**)xmalloc((game->count + 1)*sizeof(char const *));
				else
					temprh = (const char**)xrealloc((void *)game->report_heads, (game->count + 1)*sizeof(char const *)); /* avoid compiler warning */
				game->report_heads = temprh;

				if (!game->report_bodies) /* some xrealloc()s are broken */
					temprb = (const char**)xmalloc((game->count + 1)*sizeof(char const *));
				else
					temprb = (const char**)xrealloc((void *)game->report_bodies, (game->count + 1)*sizeof(char const *)); /* avoid compiler warning */
				game->report_bodies = temprb;

				game->connections[game->count] = c;
				game->players[game->count] = conn_get_account(c);
				game->results[game->count] = game_result_none;
				game->reported_results[game->count] = NULL;
				game->report_heads[game->count] = NULL;
				game->report_bodies[game->count] = NULL;

				game->count++;
				game->ref++;
				game->lastaccess_time = now;

			} // end of "if ((i == game->count) || (game->count == 0))"

			if (game->startver != startver && startver != STARTVER_UNKNOWN) /* with join startver ALWAYS unknown [KWS] */
				eventlog(eventlog_level_error, __FUNCTION__, "player \"%s\" client \"%s\" startver %u joining game startver %u (count=%u ref=%u)", account_get_name(conn_get_account(c)), clienttag_uint_to_str(conn_get_clienttag(c)), startver, game->startver, game->count, game->ref);

			game_choose_host(game);

			return 0;
		}

		extern int game_del_player(t_game * game, t_connection * c)
		{
			char const * tname;
			unsigned int i;
			t_account *  account;

			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (!game->players)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "player array is NULL");
				return -1;
			}
			if (!game->reported_results)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "reported results array is NULL");
				return -1;
			}
			account = conn_get_account(c);

			if (conn_get_leavegamewhisper_ack(c) == 0)
			{
				watchlist->dispatch(conn_get_account(c), NULL, conn_get_clienttag(c), Watch::ET_leavegame);
				conn_set_leavegamewhisper_ack(c, 1); //1 = already whispered. We reset this each std::time user joins a channel
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "game \"%s\" has ref=%u, count=%u; trying to remove player \"%s\"", game_get_name(game), game->ref, game->count, account_get_name(account));

			for (i = 0; i < game->count; i++)
			if (game->players[i] == account && game->connections[i])
			{
				eventlog(eventlog_level_debug, __FUNCTION__, "removing player #%u \"%s\" from \"%s\", %u players left", i, (tname = account_get_name(account)), game_get_name(game), game->ref - 1);
				game->connections[i] = NULL;
				if (!(game->reported_results[i]))
					eventlog(eventlog_level_debug, __FUNCTION__, "player \"%s\" left without reporting (valid) results", tname);

				eventlog(eventlog_level_debug, __FUNCTION__, "player deleted... (ref=%u)", game->ref);

				if (game->ref < 2)
				{
					eventlog(eventlog_level_debug, __FUNCTION__, "no more players, reporting game");
					game_report(game);
					eventlog(eventlog_level_debug, __FUNCTION__, "no more players, destroying game");
					game_destroy(game);
					return 0;
				}

				game->ref--;
				game->lastaccess_time = now;

				game_choose_host(game);

#ifdef WITH_LUA
				lua_handle_game(game, c, luaevent_game_userleft);
#endif
				return 0;
			}

			eventlog(eventlog_level_error, __FUNCTION__, "player \"%s\" was not in the game", account_get_name(account));
			return -1;
		}

		extern t_account * game_get_player(t_game * game, unsigned int i)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return NULL;
			}

			if (!(i < game->count))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "requested illegal player id %u", i);
				return NULL;
			}

			return game->players[i];
		}

		extern int game_set_report(t_game * game, t_account * account, char const * rephead, char const * repbody)
		{
			unsigned int pos;

			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}
			if (!game->players)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "player array is NULL");
				return -1;
			}
			if (!game->report_heads)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "report_heads array is NULL");
				return -1;
			}
			if (!game->report_bodies)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "report_bodies array is NULL");
				return -1;
			}
			if (!rephead)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "report head is NULL");
				return -1;
			}
			if (!repbody)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "report body is NULL");
				return -1;
			}

			{
				unsigned int i;

				pos = game->count;
				for (i = 0; i < game->count; i++)
				if (game->players[i] == account)
					pos = i;
			}
			if (pos == game->count)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not find player \"%s\" to set result", account_get_name(account));
				return -1;
			}

			game->report_heads[pos] = xstrdup(rephead);
			game->report_bodies[pos] = xstrdup(repbody);

			return 0;
		}

		extern int game_set_reported_results(t_game * game, t_account * account, t_game_result * results)
		{
			unsigned int i, j;
			t_game_result result;

			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			if (!results)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL results");
				return -1;
			}

			if (!game->players)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "player array is NULL");
				return -1;
			}

			if (!game->reported_results)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "reported_results array is NULL");
				return -1;
			}

			for (i = 0; i < game->count; i++)
			{
				if ((game->players[i] == account)) break;
			}

			if (i == game->count)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not find player \"%s\" to set reported results", account_get_name(account));
				return -1;
			}

			if (game->reported_results[i])
			{
				eventlog(eventlog_level_error, __FUNCTION__, "player \"%s\" allready reported results - skipping this report", account_get_name(account));
				return -1;
			}

			for (j = 0; j < game->count; j++)
			{
				result = results[j];
				switch (result)
				{
				case game_result_win:
				case game_result_loss:
				case game_result_draw:
				case game_result_observer:
				case game_result_disconnect:
					break;
				case game_result_none:
				case game_result_playing:
					if (i != j) break; /* accept none/playing only from "others" */
				default: /* result is invalid */
					if (i != j)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "ignoring bad reported result %u for player \"%s\"", (unsigned int)result, account_get_name(game->players[j]));
						results[i] = game_result_none;
					}
					else {
						eventlog(eventlog_level_error, __FUNCTION__, "got bad reported result %u for self - skipping results", (unsigned int)result);
						return -1;
					}
				}
			}

			game->reported_results[i] = results;

			return 0;
		}


		extern int game_set_self_report(t_game * game, t_account * account, t_game_result result)
		{
			unsigned int i;
			t_game_result * results;

			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			if (!game->players)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "player array is NULL");
				return -1;
			}

			if (!game->reported_results)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "reported_results array is NULL");
				return -1;
			}

			results = (t_game_result*)xmalloc(sizeof(t_game_result)*game->count);

			for (i = 0; i < game->count; i++)
			{
				if ((game->players[i] == account))
					results[i] = result;
				else
					results[i] = game_result_none;
			}

			game_set_reported_results(game, account, results);

			return 0;
		}

		extern t_game_result * game_get_reported_results(t_game * game, t_account * account)
		{
			unsigned int i;

			if (!(game))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return NULL;
			}

			if (!(account))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return NULL;
			}

			if (!(game->players))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "player array is NULL");
				return NULL;
			}

			if (!(game->reported_results))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "reported_results array is NULL");
				return NULL;
			}

			for (i = 0; i < game->count; i++)
			{
				if ((game->players[i] == account)) break;
			}

			if (i == game->count)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not find player \"%s\" to set reported results", account_get_name(account));
				return NULL;
			}

			if (!(game->reported_results[i]))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "player \"%s\" has not reported any results", account_get_name(account));
				return NULL;
			}

			return game->reported_results[i];
		}


		extern char const * game_get_mapname(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return NULL;
			}

			return game->mapname;
		}


		extern int game_set_mapname(t_game * game, char const * mapname)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			if (!mapname)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL mapname");
				return -1;
			}

			if (game->mapname != NULL) xfree((void *)game->mapname);

			game->mapname = xstrdup(mapname);

			return 0;
		}


		extern t_connection * game_get_owner(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return NULL;
			}
			return game->owner;
		}


		extern std::time_t game_get_create_time(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return (std::time_t)0;
			}

			return game->create_time;
		}


		extern std::time_t game_get_start_time(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return (std::time_t)0;
			}

			return game->start_time;
		}


		extern int game_set_option(t_game * game, t_game_option option)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}

			game->option = option;
			return 0;
		}


		extern t_game_option game_get_option(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return game_option_none;
			}

			return game->option;
		}


		extern int gamelist_create(void)
		{
			elist_init(&gamelist_head);
			glist_length = 0;
			return 0;
		}


		extern int gamelist_destroy(void)
		{
			/* FIXME: if called with active games, games are not freed */
			elist_init(&gamelist_head);
			glist_length = 0;

			return 0;
		}

		extern t_elist * gamelist(void)
		{
			return &gamelist_head;
		}

		extern int gamelist_get_length(void)
		{
			return glist_length;
		}


		extern t_game * gamelist_find_game(char const * name, t_clienttag ctag, t_game_type type)
		{
			t_elist *curr;
			t_game *game;

			elist_for_each(curr, &gamelist_head)
			{
				game = elist_entry(curr, t_game, glist_link);
				if ((type == game_type_all || game->type == type)
					&& ctag == game->clienttag
					&& game->name
					&& !strcasecmp(name, game->name)) return game;
			}

			return NULL;
		}


		extern t_game * gamelist_find_game_available(char const * name, t_clienttag ctag, t_game_type type)
		{
			t_elist *curr;
			t_game *game;
			t_game_status status;

			elist_for_each(curr, &gamelist_head)
			{
				game = elist_entry(curr, t_game, glist_link);
				status = game->status;

				if ((type == game_type_all || game->type == type) && (ctag == game->clienttag) && (game->name)
					&& (!strcasecmp(name, game->name)) && (game->status != game_status_started) &&
					(game->status != game_status_done))
					return game;
			}

			return NULL;
		}

		extern t_game * gamelist_find_game_byid(unsigned int id)
		{
			t_elist *curr;
			t_game *game;

			elist_for_each(curr, &gamelist_head)
			{
				game = elist_entry(curr, t_game, glist_link);
				if (game->id == id)
					return game;
			}

			return NULL;
		}


		extern void gamelist_traverse(t_glist_func cb, void *data)
		{
			t_elist *curr;
			t_game *game;

#ifdef WITH_LUA
			struct glist_cbdata *cbdata = (struct glist_cbdata*)data;
			// check that data is has needed struct
			if (cbdata->identifier && strcmp(cbdata->identifier, "gamelist_join") == 0)
			{
				// get gamelist from Lua script: pair(gameid=gamename)
				std::vector<t_game*> gamelist = lua_handle_game_list(cbdata->c);
				if (gamelist.size() > 0)
				{
					// display games to a user according to the given order
					for (std::vector<t_game*>::size_type i = 0; i != gamelist.size(); i++)
					{
						if (game = gamelist_find_game_byid(gamelist[i]->id))
						{
							if (gamelist[i]->name)
								game->name = xstrdup(gamelist[i]->name); // override game name
							cb(game, data); // display game item
						}
					}
					// break next execution (override of display below gamelist)
					return;
				}
			}
#endif
			elist_for_each(curr, &gamelist_head)
			{
				if (cb(elist_entry(curr, t_game, glist_link), data) < 0) return;
			}
		}


		extern int gamelist_total_games(void)
		{
			return totalcount;
		}

		extern int game_set_realm(t_game * game, unsigned int realm)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}
			game->realm = realm;
			return 0;
		}

		extern unsigned int game_get_realm(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return 0;
			}
			return game->realm;
		}

		extern int game_set_realmname(t_game * game, char const * realmname)
		{
			char const * temp;

			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}

			if (realmname)
				temp = xstrdup(realmname);
			else
				temp = NULL;

			if (game->realmname)
				xfree((void *)game->realmname); /* avoid warning */
			game->realmname = temp;
			return 0;
		}

		extern  char const * game_get_realmname(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return NULL;
			}
			return game->realmname;
		}


		extern void gamelist_check_voidgame(void)
		{
			t_elist *curr, *save;
			t_game *game;

			elist_for_each_safe(curr, &gamelist_head, save)
			{
				game = elist_entry(curr, t_game, glist_link);
				if (!game->realm)
					continue;
				if (game->ref >= 1)
					continue;
				if ((now - game->lastaccess_time) > MAX_GAME_EMPTY_TIME)
					game_destroy(game);
			}
		}

		extern void game_set_flag(t_game * game, t_game_flag flag)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return;
			}
			game->flag = flag;
		}


		extern t_game_flag game_get_flag(t_game const * game)
		{
			if (!game)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return game_flag_none;
			}
			return game->flag;
		}

		extern int game_get_count_by_clienttag(t_clienttag ct)
		{
			t_game *game;
			t_elist *curr;
			int clienttaggames = 0;

			if (!ct) {
				eventlog(eventlog_level_error, __FUNCTION__, "got UNKNOWN clienttag");
				return 0;
			}

			/* Get number of games for client tag specific */
			elist_for_each(curr, &gamelist_head)
			{
				game = elist_entry(curr, t_game, glist_link);
				if (game_get_clienttag(game) == ct)
					clienttaggames++;
			}

			return clienttaggames;
		}

		static int game_match_name(const char *name, const char *prefix)
		{
			/* the easy cases */
			if (!name || !*name) return 1;
			if (!prefix || !*prefix) return 1;

			if (!std::strncmp(name, prefix, std::strlen(prefix))) return 1;

			return 0;
		}

		extern int game_is_ladder(t_game *game)
		{
			assert(game);

			/* all normal ladder games are still counted as ladder games */
			if (game->type == game_type_ladder ||
				game->type == game_type_ironman) return 1;

			/* addition game types are also checked against gamename prefix if set */
			if (game_match_type(game_get_type(game), prefs_get_ladder_games()) &&
				game_match_name(game_get_name(game), prefs_get_ladder_prefix())) return 1;

			return 0;
		}

		extern int game_discisloss(t_game *game)
		{
			assert(game);

			if (prefs_get_discisloss())
				return 1;

			/* all normal ladder games provide discisloss option themselves */
			if (game->type == game_type_ladder ||
				game->type == game_type_ironman) return game->option == game_option_ladder_countasloss;

			/* additional game types that are consideres ladder are always considered discasloss */
			if (game_match_type(game_get_type(game), prefs_get_ladder_games()) &&
				game_match_name(game_get_name(game), prefs_get_ladder_prefix())) return 1;

			/* all other games are handled as usual */
			return  game->option == game_option_ladder_countasloss;
		}

		extern int game_set_channel(t_game * game, t_channel * channel)
		{
			if (!game) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return -1;
			}

			if (!channel) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return -1;
			}

			game->channel = channel;
			return 0;
		}

		extern t_channel * game_get_channel(t_game * game)
		{
			if (!game) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL game");
				return NULL;
			}

			return game->channel;

		}

	}

}
