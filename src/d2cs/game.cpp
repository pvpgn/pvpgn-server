/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
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
#include "common/setup_before.h"
#include "setup.h"
#include "game.h"

#include <ctime>

#include "compat/strcasecmp.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "prefs.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace d2cs
	{

		static t_list		* gamelist_head = NULL;
		static t_elem const	* gamelist_curr_elem = NULL;
		static unsigned int	total_game = 0;
		static unsigned int	game_id = 0;
		static t_game_charinfo * game_find_character(t_game * game, char const * charname);

		extern t_list * d2cs_gamelist(void)
		{
			return gamelist_head;
		}

		extern t_elem const * gamelist_get_curr_elem(void)
		{
			return gamelist_curr_elem;
		}

		extern void gamelist_set_curr_elem(t_elem const * elem)
		{
			gamelist_curr_elem = elem;
			return;
		}

		extern int d2cs_gamelist_create(void)
		{
			gamelist_head = list_create();
			return 0;
		}

		extern int d2cs_gamelist_destroy(void)
		{
			t_game * game;

			BEGIN_LIST_TRAVERSE_DATA(gamelist_head, game, t_game)
			{
				game_destroy(game, &curr_elem_);
			}
			END_LIST_TRAVERSE_DATA();

			if (list_destroy(gamelist_head) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error destroy connection list");
				return -1;
			}
			gamelist_head = NULL;
			return 0;
		}

		extern t_game * d2cs_gamelist_find_game(char const * gamename)
		{
			t_game * game;

			ASSERT(gamename, NULL);
			BEGIN_LIST_TRAVERSE_DATA(gamelist_head, game, t_game)
			{
				if (!strcasecmp(game->name, gamename)) return game;
			}
			END_LIST_TRAVERSE_DATA()
				return NULL;
		}

		extern t_game * gamelist_find_game_by_id(unsigned int id)
		{
			t_game * game;

			BEGIN_LIST_TRAVERSE_DATA(gamelist_head, game, t_game)
			{
				if (game->id == id) return game;
			}
			END_LIST_TRAVERSE_DATA()
				return NULL;
		}

		extern t_game * gamelist_find_game_by_d2gs_and_id(unsigned int d2gs_id, unsigned int d2gs_gameid)
		{
			t_game * game;

			BEGIN_LIST_TRAVERSE_DATA(gamelist_head, game, t_game)
			{
				if (!game->created) continue;
				if (game->d2gs_gameid != d2gs_gameid) continue;
				if (d2gs_get_id(game->d2gs) != d2gs_id) continue;
				return game;
			}
			END_LIST_TRAVERSE_DATA()
				return NULL;
		}

		extern t_game * gamelist_find_character(char const * charname)
		{
			t_game	* game;

			ASSERT(charname, NULL);
			BEGIN_LIST_TRAVERSE_DATA(gamelist_head, game, t_game)
			{
				if (game_find_character(game, charname)) return game;
			}
			END_LIST_TRAVERSE_DATA();
			return NULL;
		}

		extern void d2cs_gamelist_check_voidgame(void)
		{
			t_game	* game;
			std::time_t	now;
			int timeout;

			timeout = prefs_get_max_game_idletime();
			if (!timeout) return;
			now = std::time(NULL);
			BEGIN_LIST_TRAVERSE_DATA(gamelist_head, game, t_game)
			{
				if (!game->currchar) {
					if ((now - game->lastaccess_time) > timeout) {
						eventlog(eventlog_level_info, __FUNCTION__, "game %s is empty too long std::time,destroying it", game->name);
						game_destroy(game, &curr_elem_);
					}
				}
			}
			END_LIST_TRAVERSE_DATA()
		}

		extern t_game * d2cs_game_create(char const * gamename, char const * gamepass, char const * gamedesc,
			unsigned int gameflag)
		{
			t_game	* game;
			std::time_t	now;

			ASSERT(gamename, NULL);
			ASSERT(gamepass, NULL);
			ASSERT(gamedesc, NULL);
			if (d2cs_gamelist_find_game(gamename)) {
				eventlog(eventlog_level_error, __FUNCTION__, "game %s already exist", gamename);
				return NULL;
			}
			game = (t_game*)xmalloc(sizeof(t_game));
			game->name = xstrdup(gamename);
			game->pass = xstrdup(gamepass);
			game->desc = xstrdup(gamedesc);
			game->charlist = list_create();
			now = std::time(NULL);
			game_id++;
			if (game_id == 0) game_id = 1;
			game->id = game_id;
			game->created = 0;
			game->create_time = now;
			game->lastaccess_time = now;
			game->gameflag = gameflag;
			game->charlevel = 0;
			game->leveldiff = 0;
			game->d2gs_gameid = 0;
			game->d2gs = NULL;
			game->maxchar = MAX_CHAR_PER_GAME;
			game->currchar = 0;
			list_prepend_data(gamelist_head, game);
			total_game++;
			eventlog(eventlog_level_info, __FUNCTION__, "game %s pass=%s desc=%s gameflag=0x%08X created (%d total)", gamename, gamepass,
				gamedesc, gameflag, total_game);
			return game;
		}

		extern int game_destroy(t_game * game, t_elem ** elem)
		{
			t_elem		* curr;
			t_game_charinfo	* charinfo;

			ASSERT(game, -1);
			if (gamelist_curr_elem && (game == elem_get_data(gamelist_curr_elem))) {
				gamelist_curr_elem = elem_get_next_const(gamelist_head, gamelist_curr_elem);
			}
			if (list_remove_data(gamelist_head, game, elem) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error remove game %s on game list", game->name);
				return -1;
			}
			total_game--;
			eventlog(eventlog_level_info, __FUNCTION__, "game %s removed from game list (%d left)", game->name, total_game);
			LIST_TRAVERSE(game->charlist, curr)
			{
				if ((charinfo = (t_game_charinfo*)elem_get_data(curr))) {
					if (charinfo->charname) xfree((void *)charinfo->charname);
					xfree(charinfo);
				}
				list_remove_elem(game->charlist, &curr);
			}
			list_destroy(game->charlist);

			if (game->d2gs) {
				d2gs_add_gamenum(game->d2gs, -1);
				gqlist_check_creategame(d2gs_get_maxgame(game->d2gs) - d2gs_get_gamenum(game->d2gs));
			}
			if (game->desc) xfree((void *)game->desc);
			if (game->pass) xfree((void *)game->pass);
			if (game->name) xfree((void *)game->name);
			xfree(game);
			return 0;
		}

		static t_game_charinfo * game_find_character(t_game * game, char const * charname)
		{
			t_game_charinfo * charinfo;

			ASSERT(game, NULL);
			ASSERT(charname, NULL);
			if (!game->charlist) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL character list in game %s", game->name);
				return NULL;
			}
			BEGIN_LIST_TRAVERSE_DATA(game->charlist, charinfo, t_game_charinfo)
			{
				if (!charinfo->charname) continue;
				if (!strcmp_charname(charinfo->charname, charname)) return charinfo;
			}
			END_LIST_TRAVERSE_DATA()
				return NULL;
		}

		extern int game_add_character(t_game * game, char const * charname, unsigned char chclass,
			unsigned char level)
		{
			t_game_charinfo	* charinfo;

			ASSERT(game, -1);
			ASSERT(charname, -1);
			charinfo = game_find_character(game, charname);
			if (charinfo) {
				eventlog(eventlog_level_info, __FUNCTION__, "updating character %s (game %s) status", charname, game->name);
				charinfo->chclass = chclass;
				charinfo->level = level;
				return 0;
			}
			charinfo = (t_game_charinfo*)xmalloc(sizeof(t_game_charinfo));
			charinfo->charname = xstrdup(charname);
			charinfo->chclass = chclass;
			charinfo->level = level;
			list_append_data(game->charlist, charinfo);
			game->currchar++;
			game->lastaccess_time = std::time(NULL);
			eventlog(eventlog_level_info, __FUNCTION__, "added character %s to game %s (%d total)", charname, game->name, game->currchar);
			return 0;
		}

		extern int game_del_character(t_game * game, char const * charname)
		{
			t_game_charinfo * charinfo;
			t_elem * elem;

			ASSERT(game, -1);
			ASSERT(charname, -1);
			if (!(charinfo = game_find_character(game, charname))) {
				eventlog(eventlog_level_error, __FUNCTION__, "character %s not found in game %s", charname, game->name);
				return -1;
			}
			if (list_remove_data(game->charlist, charinfo, &elem)) {
				eventlog(eventlog_level_error, __FUNCTION__, "error remove character %s from game %s", charname, game->name);
				return -1;
			}
			if (charinfo->charname) xfree((void *)charinfo->charname);
			xfree(charinfo);
			game->currchar--;
			game->lastaccess_time = std::time(NULL);
			eventlog(eventlog_level_info, __FUNCTION__, "removed character %s from game %s (%d left)", charname, game->name, game->currchar);
			return 0;
		}

		extern int game_set_d2gs_gameid(t_game * game, unsigned int d2gs_gameid)
		{
			ASSERT(game, -1);
			game->d2gs_gameid = d2gs_gameid;
			return 0;
		}

		extern unsigned int game_get_d2gs_gameid(t_game const * game)
		{
			ASSERT(game, 0);
			return game->d2gs_gameid;
		}

		extern unsigned int d2cs_game_get_id(t_game const * game)
		{
			ASSERT(game, 0);
			return game->id;
		}

		extern unsigned int game_get_gameflag_ladder(t_game const * game)
		{
			ASSERT(game, 0);
			return gameflag_get_ladder(game->gameflag);
		}


		extern int game_set_d2gs(t_game * game, t_d2gs * gs)
		{
			ASSERT(game, -1);
			game->d2gs = gs;
			return 0;
		}

		extern t_d2gs * game_get_d2gs(t_game const * game)
		{
			ASSERT(game, NULL);
			return game->d2gs;
		}

		extern int game_set_leveldiff(t_game * game, unsigned int leveldiff)
		{
			ASSERT(game, -1);
			game->leveldiff = leveldiff;
			return 0;
		}

		extern int game_set_charlevel(t_game * game, unsigned int charlevel)
		{
			ASSERT(game, -1);
			game->charlevel = charlevel;
			return 0;
		}

		extern unsigned int game_get_charlevel(t_game const * game)
		{
			ASSERT(game, 0);
			return game->charlevel;
		}

		extern unsigned int game_get_leveldiff(t_game const * game)
		{
			ASSERT(game, 0);
			return game->leveldiff;
		}

		extern unsigned int game_get_maxlevel(t_game const * game)
		{
			int	maxlevel;

			ASSERT(game, 0);
			maxlevel = game->charlevel + game->leveldiff;
			if (maxlevel > prefs_get_game_maxlevel())
				maxlevel = prefs_get_game_maxlevel();
			return maxlevel;
		}

		extern unsigned int game_get_minlevel(t_game const * game)
		{
			int	minlevel;

			ASSERT(game, 0);
			minlevel = game->charlevel - game->leveldiff;
			if (minlevel < 0) minlevel = 0;
			return minlevel;
		}

		extern unsigned int game_get_gameflag_expansion(t_game const * game)
		{
			ASSERT(game, 0);
			return gameflag_get_expansion(game->gameflag);
		}

		extern unsigned int game_get_gameflag_hardcore(t_game const * game)
		{
			ASSERT(game, 0);
			return gameflag_get_hardcore(game->gameflag);
		}

		extern unsigned int game_get_gameflag_difficulty(t_game const * game)
		{
			ASSERT(game, 0);
			return gameflag_get_difficulty(game->gameflag);
		}

		extern int game_set_gameflag_ladder(t_game * game, unsigned int ladder)
		{
			ASSERT(game, -1);
			gameflag_set_ladder(game->gameflag, ladder);
			return 0;
		}

		extern int game_set_gameflag_expansion(t_game * game, unsigned int expansion)
		{
			ASSERT(game, -1);
			gameflag_set_expansion(game->gameflag, expansion);
			return 0;
		}

		extern int game_set_gameflag_hardcore(t_game * game, unsigned int hardcore)
		{
			ASSERT(game, -1);
			gameflag_set_hardcore(game->gameflag, hardcore);
			return 0;
		}

		extern int game_set_gameflag_difficulty(t_game * game, unsigned int difficulty)
		{
			ASSERT(game, -1);
			gameflag_set_difficulty(game->gameflag, difficulty);
			return 0;
		}

		extern unsigned int game_get_created(t_game const * game)
		{
			ASSERT(game, 0);
			return game->created;
		}

		extern int game_set_created(t_game * game, unsigned int created)
		{
			ASSERT(game, -1);
			game->created = created;
			return 0;
		}

		extern unsigned int game_get_maxchar(t_game const * game)
		{
			ASSERT(game, 0);
			return game->maxchar;
		}

		extern int game_set_maxchar(t_game * game, unsigned int maxchar)
		{
			ASSERT(game, -1);
			game->maxchar = maxchar;
			return 0;
		}

		extern unsigned int game_get_currchar(t_game const * game)
		{
			ASSERT(game, 0);
			return game->currchar;
		}

		extern char const * d2cs_game_get_name(t_game const * game)
		{
			ASSERT(game, NULL);
			return game->name;
		}

		extern char const * game_get_desc(t_game const * game)
		{
			ASSERT(game, NULL);
			return game->desc;
		}

		extern char const * d2cs_game_get_pass(t_game const * game)
		{
			ASSERT(game, NULL);
			return game->pass;
		}

		extern unsigned int game_get_gameflag(t_game const * game)
		{
			ASSERT(game, 0);
			return game->gameflag;
		}

		extern int d2cs_game_get_create_time(t_game const * game)
		{
			ASSERT(game, -1);
			return game->create_time;
		}

		extern int game_set_create_time(t_game * game, int create_time)
		{
			ASSERT(game, -1);
			game->create_time = create_time;
			return 0;
		}

		extern t_list * game_get_charlist(t_game const * game)
		{
			ASSERT(game, NULL);
			return game->charlist;
		}

		extern unsigned int gamelist_get_totalgame(void)
		{
			return total_game;
		}

	}

}
