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
#ifndef INCLUDED_GAME_H
#define INCLUDED_GAME_H

#include "d2gs.h"
#include "connection.h"
#include "bit.h"

namespace pvpgn
{

namespace d2cs
{

typedef struct
{
	unsigned char		chclass;
	unsigned char		level;
	char const		* charname;
} t_game_charinfo;

typedef struct
{
	unsigned int		id;
	char const 		* name;
	char const 		* pass;
	char const 		* desc;
	int			create_time;
	int			lastaccess_time;
	unsigned int		created;
	unsigned int		gameflag;
	unsigned int		charlevel;
	unsigned int		leveldiff;
	unsigned int		d2gs_gameid;
	unsigned int		maxchar;
	unsigned int		currchar;
	t_list			* charlist;
	t_d2gs			* d2gs;
} t_game;

#define D2_GAMEFLAG_BETA			0x00000001
#define D2_GAMEFLAG_RELEASE			0x00000002
#define D2_GAMEFLAG_TEMPLATE			0x00000008
#define D2_GAMEFLAG_SINGLE			0x00000010
#define D2_GAMEFLAG_HARDCORE			0x00000800
#define D2_GAMEFLAG_EXPANSION			0x00100000
#define D2_GAMEFLAG_LADDER			0x00200000

#define gameflag_get_difficulty(gameflag) ( (gameflag >> 0x0C) & 0x07 )
#define gameflag_get_expansion(gameflag)	BIT_TST_FLAG(gameflag, D2_GAMEFLAG_EXPANSION)
#define gameflag_get_ladder(gameflag)		BIT_TST_FLAG(gameflag, D2_GAMEFLAG_LADDER)
#define gameflag_get_hardcore(gameflag)		BIT_TST_FLAG(gameflag, D2_GAMEFLAG_HARDCORE)
#define gameflag_get_beta(gameflag)		BIT_TST_FLAG(gameflag, D2_GAMEFLAG_BETA)	/* true if the game is a beta version with limitations */
#define gameflag_get_release(gameflag)		BIT_TST_FLAG(gameflag, D2_GAMEFLAG_RELEASE)	/* this bit must always not be zero */
#define gameflag_get_single(gameflag)		BIT_TST_FLAG(gameflag, D2_GAMEFLAG_SINGLE)	/* true if the game is single mode without party */
#define gameflag_get_template(gameflag)		BIT_TST_FLAG(gameflag, D2_GAMEFLAG_TEMPLATE)	/* true if use template settings */

#define gameflag_set_difficulty(gameflag,n) ( gameflag |= ((n & 0x07) << 0x0C) )
#define gameflag_set_expansion(gameflag,n)  BIT_SET_CLR_FLAG(gameflag, D2_GAMEFLAG_EXPANSION, n)
#define gameflag_set_ladder(gameflag,n)     BIT_SET_CLR_FLAG(gameflag, D2_GAMEFLAG_LADDER, n)
#define gameflag_set_hardcore(gameflag,n)   BIT_SET_CLR_FLAG(gameflag, D2_GAMEFLAG_HARDCORE, n)

#define gameflag_create(l,e,h,d) (0x04|(e?D2_GAMEFLAG_EXPANSION:0) | (h?D2_GAMEFLAG_HARDCORE:0) | ((d & 0x07) << 0x0c) | (l?D2_GAMEFLAG_LADDER:0))


extern t_list * d2cs_gamelist(void);
extern int d2cs_gamelist_destroy(void);
extern int d2cs_gamelist_create(void);
extern t_game * d2cs_gamelist_find_game(char const * gamename);
extern t_game * gamelist_find_game_by_id(unsigned int id);
extern t_game * gamelist_find_game_by_d2gs_and_id(unsigned int d2gs_id, unsigned int d2gs_gameid);
extern void d2cs_gamelist_check_voidgame(void);
extern t_game * d2cs_game_create(char const * gamename, char const * gamepass, char const * gamedesc,
				unsigned int gameflag);
extern int game_destroy(t_game * game, t_elem ** elem);
extern int game_add_character(t_game * game, char const * charname,
				unsigned char chclass, unsigned char level);
extern int game_del_character(t_game * game, char const * charname);

extern t_d2gs * game_get_d2gs(t_game const * game);
extern int game_set_d2gs(t_game * game, t_d2gs * gs);
extern int game_set_d2gs_gameid(t_game * game, unsigned int gameid);
extern unsigned int game_get_d2gs_gameid(t_game const * game);
extern int game_set_id(t_game * game, unsigned int id);
extern unsigned int d2cs_game_get_id(t_game const * game);
extern int game_set_created(t_game * game, unsigned int created);
extern unsigned int game_get_created(t_game const * game);
extern int game_set_leveldiff(t_game * game, unsigned int leveldiff);
extern int game_set_charlevel(t_game * game, unsigned int charlevel);
extern unsigned int game_get_charlevel(t_game const * game);
extern unsigned int game_get_leveldiff(t_game const * game);
extern unsigned int game_get_maxlevel(t_game const * game);
extern unsigned int game_get_minlevel(t_game const * game);
extern int game_set_gameflag_ladder(t_game * game, unsigned int ladder);
extern int game_set_gameflag_expansion(t_game * game, unsigned int expansion);
extern int game_set_gameflag_hardcore(t_game * game, unsigned int hardcore);
extern int game_set_gameflag_difficulty(t_game * game, unsigned int difficulty);
extern unsigned int game_get_gameflag_ladder(t_game const * game);
extern unsigned int game_get_gameflag_expansion(t_game const * game);
extern unsigned int game_get_gameflag_hardcore(t_game const * game);
extern unsigned int game_get_gameflag_difficulty(t_game const * game);
extern int game_set_maxchar(t_game * game, unsigned int maxchar);
extern unsigned int game_get_maxchar(t_game const * game);
extern unsigned int game_get_currchar(t_game const * game);
extern char const * d2cs_game_get_name(t_game const * game);
extern char const * d2cs_game_get_pass(t_game const * game);
extern char const * game_get_desc(t_game const * game);
extern unsigned int game_get_gameflag(t_game const * game);
extern int d2cs_game_get_create_time(t_game const * game);
extern int game_set_create_time(t_game * game,int create_time);
extern t_list * game_get_charlist(t_game const * game);
extern t_game * gamelist_find_character(char const * charname);
extern unsigned int gamelist_get_totalgame(void);
extern t_elem const * gamelist_get_curr_elem(void);
extern void gamelist_set_curr_elem(t_elem const * elem);

}

}

#endif
