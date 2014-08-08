/*
 * (C) 2004		Olaf Freyer	(aaron@cs.tu-berlin.de)
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
#define TEAM_INTERNAL_ACCESS
#include "team.h"

#include <cstring>
#include <cassert>

#include "common/eventlog.h"
#include "common/list.h"
#include "common/bnet_protocol.h"

#include "account.h"
#include "account_wrap.h"
#include "storage.h"
#include "ladder.h"
#include "server.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static t_list *teamlist_head = NULL;
		unsigned max_teamid = 0;
		int teamlist_add_team(t_team * team);

		/* callback function for storage use */

		static int _cb_load_teams(void *team)
		{
			if (teamlist_add_team((t_team*)team) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "failed to add team to teamlist");
				return -1;
			}

			if (((t_team *)team)->teamid > max_teamid)
				max_teamid = ((t_team *)team)->teamid;
			return 0;
		}


		int teamlist_add_team(t_team * team)
		{
			int i;

			if (!(team))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL team");
				return -1;
			}

			for (i = 0; i < team->size; i++)
			{
				if (!(team->members[i] = accountlist_find_account_by_uid(team->teammembers[i])))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "at least one non-existant member (id %d) in team %u - discarding team", team->teammembers[i], team->teamid);
					//FIXME: delete team file now???
					return team->teamid; //we return teamid even though we have an error, we don't want unintentional overwriting
				}
			}

			for (i = 0; i < team->size; i++)
				account_add_team(team->members[i], team);

			if (!(team->teamid))
				team->teamid = ++max_teamid;

			list_append_data(teamlist_head, team);

			return team->teamid;
		}


		int teamlist_load(void)
		{
			// make sure to unload previous teamlist before loading again
			if (teamlist_head)
				teamlist_unload();

			teamlist_head = list_create();

			storage->load_teams(_cb_load_teams);

			return 0;
		}


		int teamlist_unload(void)
		{
			t_elem *curr;
			t_team *team;

			if ((teamlist_head))
			{
				LIST_TRAVERSE(teamlist_head, curr)
				{
					if (!(team = (t_team*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
						continue;
					}
					xfree((void *)team);
					list_remove_elem(teamlist_head, &curr);
				}

				if (list_destroy(teamlist_head) < 0)
					return -1;

				teamlist_head = NULL;
			}

			return 0;
		}

		int teams_destroy(t_list * teams)
		{
			t_elem *curr;
			t_team *team;

			if ((teams))
			{
				LIST_TRAVERSE(teams, curr)
				{
					if (!(team = (t_team*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
						continue;
					}
					list_remove_elem(teams, &curr);
				}

				if (list_destroy(teams) < 0)
					return -1;
			}
			teams = NULL;

			return 0;
		}

		t_team* create_team(t_account **accounts, t_clienttag clienttag)
		{
			t_team * team;
			int i;
			unsigned char size;

			team = (t_team*)xmalloc(sizeof(t_team));
			std::memset(team, 0, sizeof(t_team));
			size = 0;

			for (i = 0; i < MAX_TEAMSIZE; i++)
			{
				team->members[i] = accounts[i];
				if ((accounts[i]))
				{
					team->teammembers[i] = account_get_uid(accounts[i]);
					size++;
				}
			}
			team->size = size;
			team->clienttag = clienttag;

			_cb_load_teams(team);

			storage->write_team(team);

			return team;
		}

		void dispose_team(t_team * team)
		{
			if ((team))
				xfree((void *)team);
			team = NULL;
		}

		t_team * teamlist_find_team_by_accounts(t_account **accounts, t_clienttag clienttag)
		{
			return _list_find_team_by_accounts(accounts, clienttag, teamlist_head);
		}

		t_team * _list_find_team_by_accounts(t_account **accounts, t_clienttag clienttag, t_list * teamlist)
		{
			t_elem *curr;
			t_team *cteam;
			int i, j, found;
			unsigned char size;

			assert(accounts);

			found = 0;

			if ((teamlist))
			{
				LIST_TRAVERSE(teamlist, curr)
				{
					if (!(cteam = (t_team*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
						continue;
					}
					size = 0;

					for (i = 0; i < MAX_TEAMSIZE; i++)
					{
						if (!(accounts[i]))
							break;

						size++;
						found = 0;
						for (j = 0; j < MAX_TEAMSIZE; j++)
						{
							if ((accounts[i] == cteam->members[j]))
							{
								found = 1;
								break;
							}
						}
						if (!(found)) break;
					}
					if ((found) && (clienttag == cteam->clienttag) && (size == cteam->size))
						return cteam;
				}

			}

			return NULL;
		}

		t_team * teamlist_find_team_by_uids(unsigned int * uids, t_clienttag clienttag)
		{
			return _list_find_team_by_uids(uids, clienttag, teamlist_head);
		}

		t_team * _list_find_team_by_uids(unsigned int * uids, t_clienttag clienttag, t_list * teamlist)
		{
			t_elem *curr;
			t_team *cteam;
			int i, j, found;
			unsigned char size;

			assert(uids);

			found = 0;

			if ((teamlist))
			{
				LIST_TRAVERSE(teamlist, curr)
				{
					if (!(cteam = (t_team*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
						continue;
					}
					size = 0;

					for (i = 0; i < MAX_TEAMSIZE; i++)
					{
						if (!(uids[i]))
							break;

						found = 0;
						for (j = 0; j < MAX_TEAMSIZE; j++)
						{
							if ((uids[i] == cteam->teammembers[j]))
							{
								found = 1;
								break;
							}
						}
						if (!(found)) break;
					}
					if ((found) && (clienttag == cteam->clienttag) && (size == cteam->size))
						return cteam;
				}

			}

			return NULL;
		}

		t_team * teamlist_find_team_by_teamid(unsigned int teamid)
		{
			return _list_find_team_by_teamid(teamid, teamlist_head);
		}

		t_team* _list_find_team_by_teamid(unsigned int teamid, t_list * teamlist)
		{
			t_elem * curr;
			t_team * cteam;

			assert(teamid);

			if ((teamlist))
			{
				LIST_TRAVERSE(teamlist, curr)
				{
					if (!(cteam = (t_team*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
						continue;
					}

					if ((cteam->teamid == teamid))
						return cteam;

				}
			}
			return NULL;
		}

		unsigned int team_get_teamid(t_team * team)
		{
			assert(team);

			return team->teamid;
		}

		t_account * team_get_member(t_team * team, int count)
		{
			assert(team);
			assert(count >= 0);
			assert(count < MAX_TEAMSIZE);

			return team->members[count];
		}

		unsigned int team_get_memberuid(t_team * team, int count)
		{
			assert(team);
			assert(count >= 0);
			assert(count < MAX_TEAMSIZE);

			return team->teammembers[count];
		}


		t_clienttag team_get_clienttag(t_team * team)
		{
			assert(team);

			return team->clienttag;
		}

		unsigned char team_get_size(t_team * team)
		{
			assert(team);

			return team->size;
		}

		int team_get_wins(t_team * team)
		{
			assert(team);

			return team->wins;
		}

		int team_get_losses(t_team * team)
		{
			assert(team);

			return team->losses;
		}

		int team_get_xp(t_team * team)
		{
			assert(team);

			return team->xp;
		}

		int team_get_level(t_team * team)
		{
			assert(team);

			return team->level;
		}

		int team_set_rank(t_team * team, unsigned int rank)
		{
			assert(team);
			team->rank = rank;
			return 0;
		}

		int team_get_rank(t_team * team)
		{
			assert(team);

			return team->rank;
		}

		time_t team_get_lastgame(t_team * team)
		{
			assert(team);

			return team->lastgame;
		}

		int team_inc_wins(t_team * team)
		{
			assert(team);

			team->wins++;
			return 0;
		}

		int team_inc_losses(t_team * team)
		{
			assert(team);

			team->losses++;
			return 0;
		}

		int team_update_lastgame(t_team * team)
		{
			assert(team);

			team->lastgame = now;
			return 0;
		}

		extern int team_update_xp(t_team * team, int gameresult, unsigned int opponlevel, int * xp_diff)
		{
			int xp;
			int mylevel;
			int xpdiff = 0, placeholder;

			xp = team->xp; //get current xp
			if (xp < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got negative XP");
				return -1;
			}

			mylevel = team->level; //get teams level
			if (mylevel > W3_XPCALC_MAXLEVEL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid level: %d", mylevel);
				return -1;
			}

			if (mylevel <= 0) //if level is 0 then set it to 1
				mylevel = 1;

			if (opponlevel < 1) opponlevel = 1;

			switch (gameresult)
			{
			case W3_GAMERESULT_WIN:
				ladder_war3_xpdiff(mylevel, opponlevel, &xpdiff, &placeholder); break;
			case W3_GAMERESULT_LOSS:
				ladder_war3_xpdiff(opponlevel, mylevel, &placeholder, &xpdiff); break;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid game result: %d", gameresult);
				return -1;
			}

			*xp_diff = xpdiff;
			xp += xpdiff;
			if (xp < 0) xp = 0;

			team->xp = xp;

			return 0;
		}

		int team_update_level(t_team * team)
		{
			int xp, mylevel;

			xp = team->xp;
			if (xp < 0) xp = 0;

			mylevel = team->level;
			if (mylevel < 1) mylevel = 1;

			if (mylevel > W3_XPCALC_MAXLEVEL) {
				eventlog(eventlog_level_error, "account_set_sololevel", "got invalid level: %d", mylevel);
				return -1;
			}

			mylevel = ladder_war3_updatelevel(mylevel, xp);

			team->level = mylevel;

			return 0;
		}

		int team_set_saveladderstats(t_team * team, unsigned int gametype, int result, unsigned int opponlevel, t_clienttag clienttag)
		{
			unsigned int intrace;
			int xpdiff;
			int i;
			t_account * account;

			if (!team)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL team");
				return -1;
			}

			//added for better tracking down of problems with gameresults
			eventlog(eventlog_level_trace, __FUNCTION__, "parsing game result for team: %u result: %s", team_get_teamid(team), (result == W3_GAMERESULT_WIN) ? "WIN" : "LOSS");


			if (result == W3_GAMERESULT_WIN)
			{
				team_inc_wins(team);
			}
			if (result == W3_GAMERESULT_LOSS)
			{
				team_inc_losses(team);
			}
			team_update_xp(team, result, opponlevel, &xpdiff);
			team_update_level(team);
			team_update_lastgame(team);

			LadderList* ladderList = ladders.getLadderList(LadderKey(ladder_id_ateam, clienttag, ladder_sort_default, ladder_time_default));
			LadderReferencedObject reference(team);
			ladderList->updateEntry(team->teamid, team->level, team->xp, 0, reference);

			storage->write_team(team);


			// now set currentatteam to 0 so we only get one report for each team
			for (i = 0; i < MAX_TEAMSIZE; i++)
			{
				if ((account = team->members[i]))
				{
					intrace = account_get_w3pgrace(account, clienttag);
					if (result == W3_GAMERESULT_WIN)
					{
						account_inc_racewins(account, intrace, clienttag);
					}
					if (result == W3_GAMERESULT_LOSS)
					{
						account_inc_racelosses(account, intrace, clienttag);
					}
					account_set_currentatteam(account, 0);
				}
			}

			return 0;
		}

	}

}
