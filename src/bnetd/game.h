/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_GAME_TYPES
#define INCLUDED_GAME_TYPES

#ifdef GAME_INTERNAL_ACCESS

#ifdef JUST_NEED_TYPES
# include "account.h"
# include "connection.h"
# include "channel.h"
# include "common/tag.h"
# include "common/elist.h"
#else
# define JUST_NEED_TYPES
# include "account.h"
# include "connection.h"
# include "channel.h"
# include "common/tag.h"
# include "common/elist.h"
# undef JUST_NEED_TYPES
#endif

#endif

namespace pvpgn
{

	namespace bnetd
	{

		typedef enum
		{
			game_type_none,
			game_type_all,
			game_type_topvbot,
			game_type_melee,
			game_type_ffa,
			game_type_oneonone,
			game_type_ctf,
			game_type_greed,
			game_type_slaughter,
			game_type_sdeath,
			game_type_ladder,
			game_type_ironman,
			game_type_mapset,
			game_type_teammelee,
			game_type_teamffa,
			game_type_teamctf,
			game_type_pgl,
			game_type_diablo,
			game_type_diablo2open,
			game_type_diablo2closed,
			game_type_anongame
		} t_game_type;

		typedef enum
		{
			game_status_started,
			game_status_full,
			game_status_open,
			game_status_loaded,
			game_status_done
		} t_game_status;

		typedef enum
		{
			game_result_none,
			game_result_win,
			game_result_loss,
			game_result_draw,
			game_result_disconnect,
			game_result_observer,
			game_result_playing
		} t_game_result;

		typedef enum
		{
			game_option_none,
			game_option_melee_normal,
			game_option_ffa_normal,
			game_option_oneonone_normal,
			game_option_ctf_normal,
			game_option_greed_10000,
			game_option_greed_7500,
			game_option_greed_5000,
			game_option_greed_2500,
			game_option_slaughter_60,
			game_option_slaughter_45,
			game_option_slaughter_30,
			game_option_slaughter_15,
			game_option_sdeath_normal,
			game_option_ladder_countasloss,
			game_option_ladder_nopenalty,
			game_option_mapset_normal,
			game_option_teammelee_4,
			game_option_teammelee_3,
			game_option_teammelee_2,
			game_option_teamffa_4,
			game_option_teamffa_3,
			game_option_teamffa_2,
			game_option_teamctf_4,
			game_option_teamctf_3,
			game_option_teamctf_2,
			game_option_topvbot_7,
			game_option_topvbot_6,
			game_option_topvbot_5,
			game_option_topvbot_4,
			game_option_topvbot_3,
			game_option_topvbot_2,
			game_option_topvbot_1
		} t_game_option;

		typedef enum
		{
			game_maptype_none,
			game_maptype_selfmade,
			game_maptype_blizzard,
			game_maptype_ladder,
			game_maptype_pgl,
			game_maptype_kbk,
			game_maptype_compusa
		} t_game_maptype;

		typedef enum
		{
			game_tileset_none,
			game_tileset_badlands,
			game_tileset_space,
			game_tileset_installation,
			game_tileset_ashworld,
			game_tileset_jungle,
			game_tileset_desert,
			game_tileset_ice,
			game_tileset_twilight
		} t_game_tileset;

		typedef enum
		{
			game_speed_none,
			game_speed_slowest,
			game_speed_slower,
			game_speed_slow,
			game_speed_normal,
			game_speed_fast,
			game_speed_faster,
			game_speed_fastest
		} t_game_speed;

		typedef enum
		{
			game_difficulty_none,
			game_difficulty_normal,
			game_difficulty_nightmare,
			game_difficulty_hell,
			game_difficulty_hardcore_normal,
			game_difficulty_hardcore_nightmare,
			game_difficulty_hardcore_hell
		} t_game_difficulty;

		typedef enum {
			game_flag_none,
			game_flag_private
		} t_game_flag;

		typedef struct game
#ifdef GAME_INTERNAL_ACCESS
		{
			char const *      name;
			char const *      pass;
			char const *      info;
			t_game_type       type;
			unsigned int      realm;
			char const *      realmname;
			t_clienttag       clienttag; /* type of client (STAR, SEXP, etc) */
			unsigned int      addr; /* host IP */
			unsigned short    port; /* host port */
			int               startver;
			unsigned long     version;
			t_game_status     status;
			unsigned int      ref; /* current number of players */
			unsigned int      count; /* max number of players */
			unsigned int      id;
			char const *      mapname;
			t_game_option     option;
			t_game_maptype    maptype;
			t_game_tileset    tileset;
			t_game_speed      speed;
			unsigned int      mapsize_x;
			unsigned int      mapsize_y;
			unsigned int      maxplayers;

			t_connection *    owner;
			t_connection * *  connections;
			t_account * *     players;
			t_game_result *   results;
			t_game_result * * reported_results;
			char const * *    report_heads;
			char const * *    report_bodies;

			t_channel *       channel; /* For Games with server-side chat support */

			std::time_t            create_time;
			std::time_t            start_time;
			std::time_t            lastaccess_time;
			int               bad; /* if 1, then the results will be ignored */
			unsigned          difficulty;
			char const *      description;
			t_game_flag       flag;
			t_elist	      glist_link;
		}
#endif
		t_game;

		typedef int(*t_glist_func)(t_game *, void *);

	}
}

#endif


#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_GAME_PROTOS
#define INCLUDED_GAME_PROTOS

#define MAX_GAME_EMPTY_TIME 300
#define STARTVER_UNKNOWN  0
#define STARTVER_GW1      1
#define STARTVER_GW3      3
#define STARTVER_GW4      4
#define STARTVER_REALM1 104

#define JUST_NEED_TYPES
#include "account.h"
#include "connection.h"
#include "channel.h"
#include "common/list.h"
#include "common/tag.h"
#include "common/packet.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{
		// game list structure with custom data
		struct glist_cbdata {
			const char * identifier;
			unsigned tcount, counter;
			t_connection *c;
			t_game_type gtype;
			t_packet *rpacket;
		};

		extern char const * game_type_get_str(t_game_type type);
		extern char const * game_status_get_str(t_game_status status);
		extern char const * game_result_get_str(t_game_result result);
		extern char const * game_option_get_str(t_game_option option);
		extern char const * game_maptype_get_str(t_game_maptype maptype);
		extern char const * game_tileset_get_str(t_game_tileset tileset);
		extern char const * game_speed_get_str(t_game_speed speed);
		extern char const * game_difficulty_get_str(unsigned difficulty);
		extern t_game * game_create(char const * name, char const * pass, char const * info, t_game_type type, int startver, t_clienttag clienttag, unsigned long gameversion);
		extern unsigned int game_get_id(t_game const * game);
		extern char const * game_get_name(t_game const * game);
		extern t_game_type game_get_type(t_game const * game);
		extern t_game_maptype game_get_maptype(t_game const * game);
		extern int game_set_maptype(t_game * game, t_game_maptype maptype);
		extern t_game_tileset game_get_tileset(t_game const * game);
		extern int game_set_tileset(t_game * game, t_game_tileset tileset);
		extern t_game_speed game_get_speed(t_game const * game);
		extern int game_set_speed(t_game * game, t_game_speed speed);
		extern unsigned int game_get_mapsize_x(t_game const * game);
		extern int game_set_mapsize_x(t_game * game, unsigned int x);
		extern unsigned int game_get_mapsize_y(t_game const * game);
		extern int game_set_mapsize_y(t_game * game, unsigned int y);
		extern unsigned int game_get_maxplayers(t_game const * game);
		extern int game_set_maxplayers(t_game * game, unsigned int maxplayers);
		extern unsigned int game_get_difficulty(t_game const * game);
		extern int game_set_difficulty(t_game * game, unsigned int difficulty);
		extern char const * game_get_description(t_game const * game);
		extern int game_set_description(t_game * game, char const * description);
		extern char const * game_get_pass(t_game const * game);
		extern char const * game_get_info(t_game const * game);
		extern unsigned long game_get_version(t_game const * game);
		extern int game_get_startver(t_game const * game);
		extern unsigned int game_get_ref(t_game const * game);
		extern unsigned int game_get_count(t_game const * game);
		extern void game_set_status(t_game * game, t_game_status status);
		extern t_game_status game_get_status(t_game const * game);
		extern unsigned int game_get_addr(t_game const * game);
		extern unsigned short game_get_port(t_game const * game);
		extern unsigned int game_get_latency(t_game const * game);
		extern t_connection * game_get_player_conn(t_game const * game, unsigned int i);
		extern t_clienttag game_get_clienttag(t_game const * game);
		extern int game_add_player(t_game * game, char const * pass, int startver, t_connection * c);
		extern int game_del_player(t_game * game, t_connection * c);
		extern t_account * game_get_player(t_game * game, unsigned int i);
		extern int game_set_report(t_game * game, t_account * account, char const * head, char const * body);
		extern int game_set_reported_results(t_game * game, t_account * account, t_game_result * results);
		extern int game_set_self_report(t_game * game, t_account * account, t_game_result result);
		extern t_game_result * game_get_reported_results(t_game * game, t_account * account);
		extern char const * game_get_mapname(t_game const * game);
		extern int game_set_mapname(t_game * game, char const * mapname);
		extern t_connection * game_get_owner(t_game const * game);
		extern std::time_t game_get_create_time(t_game const * game);
		extern std::time_t game_get_start_time(t_game const * game);
		extern int game_set_option(t_game * game, t_game_option option);
		extern t_game_option game_get_option(t_game const * game);
		extern int gamelist_create(void);
		extern int gamelist_destroy(void);
		extern t_elist * gamelist(void);
		extern int gamelist_get_length(void);
		extern t_game * gamelist_find_game(char const * name, t_clienttag ctag, t_game_type type);
		extern t_game * gamelist_find_game_available(char const * name, t_clienttag ctag, t_game_type type);
		extern t_game * gamelist_find_game_byid(unsigned int id);
		extern void gamelist_traverse(t_glist_func cb, void *data);
		extern int gamelist_total_games(void);
		extern int game_set_realm(t_game * game, unsigned int realm);
		extern unsigned int game_get_realm(t_game const * game);
		extern char const * game_get_realmname(t_game const * game);
		extern int game_set_realmname(t_game * game, char const * realmname);
		extern void gamelist_check_voidgame(void);
		extern void game_set_flag(t_game * game, t_game_flag flag);
		extern t_game_flag game_get_flag(t_game const * game);
		extern int game_get_count_by_clienttag(t_clienttag ct);
		extern int game_is_ladder(t_game *game);
		extern int game_discisloss(t_game *game);
		extern int game_set_channel(t_game * game, t_channel * channel);
		extern t_channel * game_get_channel(t_game * game);
	}

}

#endif
#endif
