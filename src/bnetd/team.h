/*
 * (C) 2004	Olaf Freyer	(aaron@cs.tu-berlin.de)
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

#ifndef INCLUDED_TEAM_TYPES
#define INCLUDED_TEAM_TYPES

#include <ctime>

#ifndef JUST_NEED_TYPES
# define JUST_NEED_TYPES
# include "common/list.h"
# include "common/tag.h"
# include "account.h"
# undef JUST_NEED_TYPES
#else
# include "common/list.h"
# include "common/tag.h"
# include "account.h"
#endif

#define MAX_TEAMSIZE 4

namespace pvpgn
{

	namespace bnetd
	{

		typedef struct team
#ifdef TEAM_INTERNAL_ACCESS
		{
			unsigned char 	size;
			int		wins;
			int		losses;
			int		xp;
			int		level;
			int		rank;
			unsigned int	teamid;
			unsigned int	teammembers[MAX_TEAMSIZE];
			t_account *     members[MAX_TEAMSIZE];
			t_clienttag	clienttag;
			std::time_t	lastgame;
		}
#endif
		t_team;

	}

}

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_TEAM_PROTOS
#define INCLUDED_TEAM_PROTOS

namespace pvpgn
{

	namespace bnetd
	{

		extern int teamlist_load(void);
		extern int teamlist_unload(void);
		extern int teams_destroy(t_list *);

		extern t_team* create_team(t_account **accounts, t_clienttag clienttag);
		extern void dispose_team(t_team * team);

		extern t_team* teamlist_find_team_by_accounts(t_account **accounts, t_clienttag clienttag);
		extern t_team* teamlist_find_team_by_uids(unsigned int * uids, t_clienttag clienttag);
		extern t_team* teamlist_find_team_by_teamid(unsigned int teamid);

		extern t_team* _list_find_team_by_accounts(t_account **accounts, t_clienttag clienttag, t_list * list);
		extern t_team* _list_find_team_by_uids(unsigned int * uids, t_clienttag clienttag, t_list * list);
		extern t_team* _list_find_team_by_teamid(unsigned int teamid, t_list * list);

		extern unsigned int team_get_teamid(t_team * team);
		extern t_account * team_get_member(t_team * team, int count);
		extern unsigned int team_get_memberuid(t_team * team, int count);

		extern t_clienttag team_get_clienttag(t_team * team);
		extern unsigned char team_get_size(t_team * team);
		extern int team_get_wins(t_team * team);
		extern int team_get_losses(t_team * team);
		extern int team_get_xp(t_team * team);
		extern int team_get_level(t_team * team);
		extern int team_set_rank(t_team * team, unsigned int rank);
		extern int team_get_rank(t_team * team);
		extern std::time_t team_get_lastgame(t_team * team);

		extern int team_inc_wins(t_team * team);
		extern int team_inc_losses(t_team * team);

		extern int team_set_saveladderstats(t_team * team, unsigned int gametype, int result, unsigned int opponlevel, t_clienttag clienttag);

	}

}

#endif
#endif
