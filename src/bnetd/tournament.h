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
#ifndef INCLUDED_TOURNAMENT_TYPES
#define INCLUDED_TOURNAMENT_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		typedef struct
		{
			unsigned int start_preliminary;
			unsigned int end_signup;
			unsigned int end_preliminary;
			unsigned int start_round_1;
			unsigned int start_round_2;
			unsigned int start_round_3;
			unsigned int start_round_4;
			unsigned int tournament_end;
			unsigned int game_selection;
			unsigned int game_type;
			unsigned int game_client;
			unsigned int races;
			char *       format;
			char *       sponsor;       /* format: "ricon,sponsor"
										 * ricon = W3+icon reversed , if 2 char icon is selected
										 *  or reversed icon if 4 char icon is selected
										 * ie. "4R3W,The PvPGN Team"
										 */
			unsigned int thumbs_down;
		} t_tournament_info;

		typedef struct
		{
			char * name;
			unsigned int wins;
			unsigned int losses;
			unsigned int ties;
			int in_game;
			int in_finals;
		} t_tournament_user;

	}

}

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_TOURNAMENT_PROTOS
#define INCLUDED_TOURNAMENT_PROTOS

#define JUST_NEED_TYPES
# include "account.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern int tournament_signup_user(t_account * account);
		extern int tournament_user_signed_up(t_account * account);
		extern int tournament_add_stat(t_account * account, int stat);
		extern int tournament_get_stat(t_account * account, int stat);
		extern int tournament_get_player_score(t_account * account);
		extern int tournament_set_in_game_status(t_account * account, int status);
		extern int tournament_get_in_finals_status(t_account * account);
		extern int tournament_get_game_in_progress(void);
		extern int tournament_check_client(t_clienttag clienttag);

		extern int tournament_get_totalplayers(void);
		extern int tournament_is_arranged(void);

		/*****/
		extern int tournament_init(char const * filename);
		extern int tournament_destroy(void);
		extern int tournament_reload(char const * filename);

		/*****/
		extern unsigned int tournament_get_start_preliminary(void);
		extern unsigned int tournament_get_end_signup(void);
		extern unsigned int tournament_get_end_preliminary(void);
		extern unsigned int tournament_get_start_round_1(void);
		extern unsigned int tournament_get_start_round_2(void);
		extern unsigned int tournament_get_start_round_3(void);
		extern unsigned int tournament_get_start_round_4(void);
		extern unsigned int tournament_get_tournament_end(void);
		extern unsigned int tournament_get_game_selection(void);
		extern unsigned int tournament_get_game_type(void);
		extern unsigned int tournament_get_races(void);
		extern char * tournament_get_format(void);
		extern char * tournament_get_sponsor(void);
		extern unsigned int tournament_get_thumbs_down(void);

	}

}

/*****/
#endif
#endif
