/*
 * Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
 * Copyright (C) 1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 1999,2000  D.Moreaux (vapula@linuxbe.org)
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
#define LADDER_INTERNAL_ACCESS
#include "ladder.h"

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <list>
#include <map>
#include <cmath>
#include <algorithm>

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#include "compat/strerror.h"
#include "common/tag.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "common/hashtable.h"
#include "common/bnettime.h"
#include "common/bnet_protocol.h"

#include "account.h"
#include "account_wrap.h"
#include "prefs.h"
#include "ladder_calc.h"
#include "team.h"
#include "common/setup_after.h"

#define MaxRankKeptInLadder 1000

namespace pvpgn
{

	namespace bnetd
	{

		/* for War3 XP computations */
		static t_xpcalc_entry  * xpcalc;
		static t_xplevel_entry * xplevels;
		int w3_xpcalc_maxleveldiff;

		const char * ladder_id_str[] = { "0", "1", "", "3", "", "solo", "team", "ffa" };
		const char * bin_ladder_id_str[] = { "", "", "", "I", "", "SOLO", "TEAM", "FFA", "AT" };
		const char * bin_ladder_sort_str[] = { "R", "W", "G", "" };
		const char * bin_ladder_time_str[] = { "A", "C", "" };

		Ladders ladders;


		/*
		 * Prepare an account for first ladder play if necessary.
		 */
		extern int ladder_init_account(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			unsigned int uid, rating;

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			if (account_get_ladder_rating(account, clienttag, id) == 0)
			{
				if (account_get_ladder_wins(account, clienttag, id) +
					account_get_ladder_losses(account, clienttag, id) > 0) /* no ladder games so far... */
				{
					eventlog(eventlog_level_warn, __FUNCTION__, "account for \"%s\" (%s) has %u wins and %u losses but has zero rating", account_get_name(account), clienttag_uint_to_str(clienttag), account_get_ladder_wins(account, clienttag, id), account_get_ladder_losses(account, clienttag, id));
					return -1;
				}
				account_adjust_ladder_rating(account, clienttag, id, prefs_get_ladder_init_rating());

				uid = account_get_uid(account);
				rating = account_get_ladder_rating(account, clienttag, id);

				LadderList* ladderlist_cr = ladders.getLadderList(LadderKey(id, clienttag, ladder_sort_highestrated, ladder_time_current));
				LadderList* ladderlist_cw = ladders.getLadderList(LadderKey(id, clienttag, ladder_sort_mostwins, ladder_time_current));
				LadderList* ladderlist_cg = ladders.getLadderList(LadderKey(id, clienttag, ladder_sort_mostgames, ladder_time_current));

				LadderReferencedObject reference(account);

				ladderlist_cr->updateEntry(uid, rating, 0, 0, reference);
				ladderlist_cg->updateEntry(uid, 0, rating, 0, reference);
				ladderlist_cw->updateEntry(uid, 0, rating, 0, reference);

				INFO2("initialized account for \"%s\" for \"%s\" ladder", account_get_name(account), clienttag_uint_to_str(clienttag));
			}

			return 0;
		}

		extern int ladder_init_account_wol(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			unsigned int uid, rating;

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			if ((account_get_ladder_points(account, clienttag, id) == 0) && (account_get_ladder_wins(account, clienttag, id) == 0)
				&& (account_get_ladder_losses(account, clienttag, id) == 0))
			{
				uid = account_get_uid(account);

				LadderList* ladderlist = ladders.getLadderList(LadderKey(id, clienttag, ladder_sort_default, ladder_time_default));

				LadderReferencedObject reference(account);

				ladderlist->updateEntry(uid, 0, 0, 0, reference);

				INFO2("initialized WOL account for \"%s\" for \"%s\" ladder", account_get_name(account), clienttag_uint_to_str(clienttag));
			}

			return 0;
		}

		/*
		 * Update player ratings, rankings, etc due to game results.
		 */
		extern int ladder_update(t_clienttag clienttag, t_ladder_id id, unsigned int count, t_account * * players, t_game_result * results, t_ladder_info * info)
		{
			unsigned int curr;
			unsigned int ratio = 0;
			int uid;

			if (count<1 || count>8)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid player count %u", count);
				return -1;
			}
			if (!players)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL players");
				return -1;
			}
			if (!results)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL results");
				return -1;
			}
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			if (!info)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL info");
				return -1;
			}

			if (ladder_calc_info(clienttag, id, count, players, results, info) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "unable to calculate info from game results");
				return -1;
			}

			for (curr = 0; curr < count; curr++)
			{
				unsigned int wins, games;
				t_account * account;

				account = players[curr];
				uid = account_get_uid(account);
				wins = account_get_ladder_wins(account, clienttag, id);
				games = wins + account_get_ladder_losses(account, clienttag, id) +
					account_get_ladder_disconnects(account, clienttag, id) +
					account_get_ladder_draws(account, clienttag, id);

				//reasonable close to 1000 ;-)
				ratio = (wins << 10) / games;

				account_adjust_ladder_rating(account, clienttag, id, info[curr].adj);
				unsigned int rating = account_get_ladder_rating(account, clienttag, id);

				LadderReferencedObject reference(account);

				LadderList* ladderlist = ladders.getLadderList(LadderKey(id, clienttag, ladder_sort_highestrated, ladder_time_current));
				ladderlist->updateEntry(uid, rating, wins, ratio, reference);

				ladderlist = ladders.getLadderList(LadderKey(id, clienttag, ladder_sort_mostgames, ladder_time_current));
				ladderlist->updateEntry(uid, games, rating, ratio, reference);

				ladderlist = ladders.getLadderList(LadderKey(id, clienttag, ladder_sort_mostwins, ladder_time_current));
				ladderlist->updateEntry(uid, wins, rating, ratio, reference);
			}

			ladders.update();
			return 0;
		}

		static int _cb_count_points(int * points_win, int * points_loss, int pl1_points, int pl2_points)
		{
			*points_win = static_cast<int>(64 * (1 - 1 / (powf(10, static_cast<float>(pl2_points - pl1_points) / 400) + 1)) + 0.5);
			*points_loss = std::min(64 - *points_win, pl1_points / 10);

			return 0;
		}

		extern int ladder_update_wol(t_clienttag clienttag, t_ladder_id id, t_account * * players, t_game_result * results)
		{
			unsigned int wins, losses, points;
			int curr;
			int uid;
			int pl1_points = account_get_ladder_points(players[0], clienttag, id);
			int pl2_points = account_get_ladder_points(players[1], clienttag, id);

			if (!players) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL players");
				return -1;
			}

			if (!results) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL results");
				return -1;
			}

			if (!clienttag) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			if (((results[0]) && (results[1])) && ((results[0] == results[1]) && (results[0] = game_result_disconnect))) {
				DEBUG0("Both players got game_result_disconnect - points counting terminated");
				return 0;
			}

			for (curr = 0; curr < 2; curr++) {
				t_account * account = players[curr];
				int mypoints = account_get_ladder_points(players[curr], clienttag, id);
				int points_win = 0;
				int points_loss = 0;
				uid = account_get_uid(account);

				if (results[curr] == game_result_win) {
					_cb_count_points(&points_win, &points_loss, mypoints, (mypoints == pl1_points ? pl2_points : pl1_points));
					account_set_ladder_points(players[curr], clienttag, id, mypoints + points_win);
					DEBUG3("Player %s WIN, had %u points and now have %u points", account_get_name(account), mypoints, mypoints + points_win);
				}
				else {
					_cb_count_points(&points_win, &points_loss, mypoints, (mypoints == pl1_points ? pl2_points : pl1_points));
					account_set_ladder_points(players[curr], clienttag, id, mypoints - points_loss);
					DEBUG3("Player %s LOSS, had %u points and now have %u points", account_get_name(account), mypoints, mypoints - points_loss);
				}

				LadderReferencedObject reference(account);
				wins = account_get_ladder_wins(account, clienttag, id);
				losses = account_get_ladder_losses(account, clienttag, id);
				points = account_get_ladder_points(account, clienttag, id);

				LadderList* ladderlist = ladders.getLadderList(LadderKey(id, clienttag, ladder_sort_default, ladder_time_default));
				ladderlist->updateEntry(uid, points, wins, 0, reference);
			}

			ladders.update();
			return 0;
		}

		extern int ladder_check_map(char const * mapname, t_game_maptype maptype, t_clienttag clienttag)
		{
			if (!mapname)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL mapname");
				return -1;
			}
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "checking mapname \"%s\" maptype=%d", mapname, (int)maptype);
			if (maptype == game_maptype_ladder) /* FIXME: what about Ironman? */
				return 1;

			return 0;
		}

		/* *********************************************************************************************************************
		 * handling of bnxpcalc and bnxplevel stuff (old code)
		 * *********************************************************************************************************************/

		extern int ladder_createxptable(const char *xplevelfile, const char *xpcalcfile)
		{
			std::FILE *fd1, *fd2;
			char buffer[256];
			char *p;
			t_xpcalc_entry * newxpcalc;
			int len, i, j;
			int level, startxp, neededxp, mingames;
			float lossfactor;
			int minlevel, leveldiff, higher_xpgained, higher_xplost, lower_xpgained, lower_xplost = 10;

			if (xplevelfile == NULL || xpcalcfile == NULL) {
				eventlog(eventlog_level_error, "ladder_createxptable", "got NULL filename(s)");
				return -1;
			}

			/* first lets open files */
			if ((fd1 = std::fopen(xplevelfile, "rt")) == NULL) {
				eventlog(eventlog_level_error, "ladder_createxptable", "could not open XP level file : \"%s\"", xplevelfile);
				return -1;
			}

			if ((fd2 = std::fopen(xpcalcfile, "rt")) == NULL) {
				eventlog(eventlog_level_error, "ladder_createxptable", "could not open XP calc file : \"%s\"", xpcalcfile);
				std::fclose(fd1);
				return -1;
			}

			/* then lets allocate mem for all the arrays */
			xpcalc = (t_xpcalc_entry*)xmalloc(sizeof(t_xpcalc_entry)* W3_XPCALC_MAXLEVEL); //presume the maximal leveldiff is level number

			w3_xpcalc_maxleveldiff = -1;
			std::memset(xpcalc, 0, sizeof(t_xpcalc_entry)* W3_XPCALC_MAXLEVEL);
			xplevels = (t_xplevel_entry*)xmalloc(sizeof(t_xplevel_entry)* W3_XPCALC_MAXLEVEL);
			std::memset(xplevels, 0, sizeof(t_xplevel_entry)* W3_XPCALC_MAXLEVEL);

			/* finally, lets read from the files */

			while (std::fgets(buffer, 256, fd1)) {
				len = std::strlen(buffer);
				if (len < 2) continue;
				if (buffer[len - 1] == '\n') buffer[len - 1] = '\0';

				/* support comments */
				for (p = buffer; *p && *p != '#'; p++);
				if (*p == '#') *p = '\0';

				if (std::sscanf(buffer, "%d %d %d %f %d", &level, &startxp, &neededxp, &lossfactor, &mingames) != 5)
					continue;

				if (level < 1 || level > W3_XPCALC_MAXLEVEL) { /* invalid level */
					eventlog(eventlog_level_error, "ladder_createxptable", "read INVALID player level : %d", level);
					continue;
				}

				level--; /* the index in a C array starts from 0 */
				xplevels[level].startxp = startxp;
				xplevels[level].neededxp = neededxp;
				xplevels[level].lossfactor = (int)(lossfactor * 100); /* we store the loss factor as % */
				xplevels[level].mingames = mingames;
				eventlog(eventlog_level_trace, "ladder_createxptable", "inserting level XP info (level: %d, startxp: %d neededxp: %d lossfactor: %d mingames: %d)", level + 1, xplevels[level].startxp, xplevels[level].neededxp, xplevels[level].lossfactor, xplevels[level].mingames);
			}
			std::fclose(fd1);

			while (std::fgets(buffer, 256, fd2)) {
				len = std::strlen(buffer);
				if (len < 2) continue;
				if (buffer[len - 1] == '\n') buffer[len - 1] = '\0';

				/* support comments */
				for (p = buffer; *p && *p != '#'; p++);
				if (*p == '#') *p = '\0';

				if (std::sscanf(buffer, " %d %d %d %d %d %d ", &minlevel, &leveldiff, &higher_xpgained, &higher_xplost, &lower_xpgained, &lower_xplost) != 6)
					continue;

				eventlog(eventlog_level_trace, "ladder_createxptable", "parsed xpcalc leveldiff : %d", leveldiff);

				if (leveldiff <0) {
					eventlog(eventlog_level_error, "ladder_createxptable", "got invalid level diff : %d", leveldiff);
					continue;
				}

				if (leveldiff>(w3_xpcalc_maxleveldiff + 1)) {
					eventlog(eventlog_level_error, __FUNCTION__, "expected entry for leveldiff=%u but found %u", w3_xpcalc_maxleveldiff + 1, leveldiff);
					continue;
				}

				w3_xpcalc_maxleveldiff = leveldiff;

				xpcalc[leveldiff].higher_winxp = higher_xpgained;
				xpcalc[leveldiff].higher_lossxp = higher_xplost;
				xpcalc[leveldiff].lower_winxp = lower_xpgained;
				xpcalc[leveldiff].lower_lossxp = lower_xplost;
			}
			std::fclose(fd2);

			newxpcalc = (t_xpcalc_entry*)xrealloc(xpcalc, sizeof(t_xpcalc_entry)* (w3_xpcalc_maxleveldiff + 1));
			xpcalc = newxpcalc;

			/* OK, now we need to test couse if the user forgot to put some values
			 * lots of profiles could get screwed up
			 */

			if (w3_xpcalc_maxleveldiff < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "found no valid entries for WAR3 xp calculation");
				ladder_destroyxptable();
				return -1;
			}

			eventlog(eventlog_level_info, __FUNCTION__, "set war3 xpcalc maxleveldiff to %u", w3_xpcalc_maxleveldiff);

			for (j = 0; j <= w3_xpcalc_maxleveldiff; j++)
			if (xpcalc[j].higher_winxp == 0 || xpcalc[j].higher_lossxp == 0 ||
				xpcalc[j].lower_winxp == 0 || xpcalc[j].lower_lossxp == 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "I found 0 for a win/loss XP, please check your config file");
				ladder_destroyxptable();
				return -1;
			}

			for (i = 0; i<W3_XPCALC_MAXLEVEL; i++)
			if ((i > 0 && xplevels[i].neededxp == 0) || xplevels[i].lossfactor == 0
				|| (i > 0 && (xplevels[i].startxp <= xplevels[i - 1].startxp || xplevels[i].neededxp < xplevels[i - 1].neededxp))) {
				eventlog(eventlog_level_error, __FUNCTION__, "I found 0 for a level XP, please check your config file (level: %d neededxp: %d lossfactor: %d)", i + 1, xplevels[i].neededxp, xplevels[i].lossfactor);
				ladder_destroyxptable();
				return -1;
			}

			return 0;
		}

		extern void ladder_destroyxptable()
		{
			if (xpcalc != NULL) xfree(xpcalc);
			if (xplevels != NULL) xfree(xplevels);
		}

		extern int war3_get_maxleveldiff()
		{
			return w3_xpcalc_maxleveldiff;
		}


		extern int ladder_war3_xpdiff(unsigned int winnerlevel, unsigned int looserlevel, int *winxpdiff, int *loosxpdiff)
		{
			int diff, absdiff;

			diff = winnerlevel - looserlevel;
			absdiff = (diff < 0) ? (-diff) : diff;

			if (absdiff > w3_xpcalc_maxleveldiff) {
				eventlog(eventlog_level_error, "ladder_war3_xpdiff", "got invalid level difference : %d", absdiff);
				return -1;
			}

			if (winnerlevel > W3_XPCALC_MAXLEVEL || looserlevel > W3_XPCALC_MAXLEVEL || winnerlevel < 1 || looserlevel < 1) {
				eventlog(eventlog_level_error, "ladder_war3_xpdiff", "got invalid account levels (win: %d loss: %d)", winnerlevel, looserlevel);
				return -1;
			}

			if (winxpdiff == NULL || loosxpdiff == NULL) {
				eventlog(eventlog_level_error, "ladder_war3_xpdiff", "got NULL winxpdiff, loosxpdiff");
				return -1;
			}
			/* we return the xp difference for the winner and the looser
			 * we compute that from the xp charts also applying the loss factor for
			 * lower level profiles
			 * FIXME: ?! loss factor doesnt keep the sum of xp won/lost constant
			 * DON'T CARE, cause current win/loss values aren't symetrical any more
			 */
			if (diff >= 0) {
				*winxpdiff = xpcalc[absdiff].higher_winxp;
				*loosxpdiff = -(xpcalc[absdiff].lower_lossxp * xplevels[looserlevel - 1].lossfactor) / 100;
			}
			else {
				*winxpdiff = xpcalc[absdiff].lower_winxp;
				*loosxpdiff = -(xpcalc[absdiff].higher_lossxp * xplevels[looserlevel - 1].lossfactor) / 100;
			}

			return 0;
		}

		extern int ladder_war3_updatelevel(unsigned int oldlevel, int xp)
		{
			int i, mylevel;

			if (oldlevel < 1 || oldlevel > W3_XPCALC_MAXLEVEL) {
				eventlog(eventlog_level_error, "ladder_war3_updatelevel", "got invalid level: %d", oldlevel);
				return oldlevel;
			}

			if (xp <= 0) return 1;

			mylevel = oldlevel;

			for (i = mylevel; i < W3_XPCALC_MAXLEVEL; i++)
			if (xplevels[i].startxp > xp) { mylevel = i; break; }

			for (i = mylevel - 1; i > 0; i--)
			if (xplevels[i - 1].startxp < xp) { mylevel = i + 1; break; }

			return mylevel;
		}

		extern int ladder_war3_get_min_xp(unsigned int Level)
		{
			if (Level < 1 || Level > W3_XPCALC_MAXLEVEL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid Level %d", Level);
				return -1;
			}
			return xplevels[Level - 1].startxp;
		}


		/* *********************************************************************************************************************
		 * start of new ladder codes
		 * *********************************************************************************************************************/


		LadderKey::LadderKey(t_ladder_id ladderId_, t_clienttag clienttag_, t_ladder_sort ladderSort_, t_ladder_time ladderTime_)
			:ladderId(ladderId_), clienttag(clienttag_), ladderSort(ladderSort_), ladderTime(ladderTime_)
		{
		}


		LadderKey::~LadderKey() throw ()
		{}


		bool
			LadderKey::operator== (const LadderKey& right) const
		{
				return ((ladderId == right.ladderId) &&
					(clienttag == right.clienttag) &&
					(ladderSort == right.ladderSort) &&
					(ladderTime == right.ladderTime));
			}


		bool
			LadderKey::operator< (const LadderKey& right) const
		{
				if (ladderId != right.ladderId)
					return (ladderId < right.ladderId);
				else if (clienttag != right.clienttag)
					return (clienttag < right.clienttag);
				else if (ladderSort != right.ladderSort)
					return ladderSort < right.ladderSort;
				else
					return (ladderTime < right.ladderTime);
			}


		t_ladder_id
			LadderKey::getLadderId() const
		{
				return ladderId;
			}


		t_clienttag
			LadderKey::getClienttag() const
		{
				return clienttag;
			}


		t_ladder_sort
			LadderKey::getLadderSort() const
		{
				return ladderSort;
			}


		t_ladder_time
			LadderKey::getLadderTime() const
		{
				return ladderTime;
			}

		LadderKey
			LadderKey::getOpposingKey() const
		{
				if (ladderTime == ladder_time_active)
					return LadderKey(ladderId, clienttag, ladderSort, ladder_time_current);
				else if (ladderTime == ladder_time_current)
					return LadderKey(ladderId, clienttag, ladderSort, ladder_time_active);
				else
					return LadderKey(ladderId, clienttag, ladderSort, ladderTime);;
			}


		LadderReferencedObject::LadderReferencedObject(t_account *account_)
			:referenceType(referenceTypeAccount), account(account_), team(NULL)
		{
		}


		LadderReferencedObject::LadderReferencedObject(t_team *team_)
			: referenceType(referenceTypeTeam), account(NULL), team(team_)
		{
		}


		LadderReferencedObject::~LadderReferencedObject() throw()
		{
		}


		bool
			LadderReferencedObject::getData(const LadderKey& ladderKey_, unsigned int& uid_, unsigned int& primary_, unsigned int& secondary_, unsigned int& tertiary_) const
		{
				// returns false in case of failures - and also when no need to add this referencedObject to ladder
				t_clienttag clienttag = ladderKey_.getClienttag();
				t_ladder_id ladderId = ladderKey_.getLadderId();

				if (referenceType == referenceTypeAccount)
				{
					uid_ = account_get_uid(account);
					if (clienttag == CLIENTTAG_WARCRAFT3_UINT || clienttag == CLIENTTAG_WAR3XP_UINT) {
						if (!(primary_ = account_get_ladder_level(account, clienttag, ladderId)))
							return false;
						secondary_ = account_get_ladder_xp(account, clienttag, ladderId);
						tertiary_ = 0;
						return true;
					}
					else if (tag_check_wolv1(clienttag) || tag_check_wolv2(clienttag)) {
						if (!(primary_ = account_get_ladder_points(account, clienttag, ladderId)))
							return false;
						secondary_ = account_get_ladder_wins(account, clienttag, ladderId);
						tertiary_ = 0;
						return true;
					}
					else{
						t_ladder_sort ladderSort = ladderKey_.getLadderSort();
						unsigned int rating, wins, games;
						// current ladders
						if (ladderKey_.getLadderTime() == ladder_time_current)
						{
							rating = account_get_ladder_rating(account, clienttag, ladderId);
							if (!rating) return false;
							wins = account_get_ladder_wins(account, clienttag, ladderId);
							games = wins +
								account_get_ladder_losses(account, clienttag, ladderId) +
								account_get_ladder_disconnects(account, clienttag, ladderId) +
								account_get_ladder_draws(account, clienttag, ladderId);
						}
						else{ // active ladders
							rating = account_get_ladder_active_rating(account, clienttag, ladderId);
							if (!rating) return false;
							wins = account_get_ladder_active_wins(account, clienttag, ladderId);
							games = wins +
								account_get_ladder_active_losses(account, clienttag, ladderId) +
								account_get_ladder_active_disconnects(account, clienttag, ladderId) +
								account_get_ladder_active_draws(account, clienttag, ladderId);
						}
						unsigned int ratio = (wins << 10) / games;
						switch (ladderSort)
						{
						case ladder_sort_highestrated:
							primary_ = rating;
							secondary_ = wins;
							tertiary_ = ratio;
							return true;
						case ladder_sort_mostwins:
							primary_ = wins;
							secondary_ = rating;
							tertiary_ = ratio;
							return true;
						case ladder_sort_mostgames:
							primary_ = games;
							secondary_ = rating;
							tertiary_ = ratio;
							return true;
						default:
							return false;
						}
					}
				}
				else if (referenceType == referenceTypeTeam)
				{
					uid_ = team_get_teamid(team);
					if (!(primary_ = team_get_level(team)))
						return false;
					secondary_ = team_get_xp(team);
					tertiary_ = 0;
					return true;
				}

				return false;
			}

		unsigned int
			LadderReferencedObject::getRank(const LadderKey& ladderKey_) const
		{
				t_clienttag clienttag = ladderKey_.getClienttag();
				t_ladder_id ladderId = ladderKey_.getLadderId();
				t_ladder_time ladderTime = ladderKey_.getLadderTime();
				t_ladder_sort ladderSort = ladderKey_.getLadderSort();

				if (referenceType == referenceTypeAccount)
				{
					if (ladderSort == ladder_sort_default || ladderSort == ladder_sort_highestrated)
					{
						if (ladderTime == ladder_time_active)
						{
							return account_get_ladder_active_rank(account, clienttag, ladderId);
						}
						else{
							return account_get_ladder_rank(account, clienttag, ladderId);
						}
					}
					else{
						// the account rank is only determined by highestrated/default ladder sort
						return 0;
					}
				}
				else if (referenceType == referenceTypeTeam)
				{
					return team_get_rank(team);
				}
				return 0;
			}

		bool
			LadderReferencedObject::setRank(const LadderKey& ladderKey_, unsigned int rank_) const
		{
				t_clienttag clienttag = ladderKey_.getClienttag();
				t_ladder_id ladderId = ladderKey_.getLadderId();
				t_ladder_time ladderTime = ladderKey_.getLadderTime();
				t_ladder_sort ladderSort = ladderKey_.getLadderSort();

				if (referenceType == referenceTypeAccount)
				{
					if (ladderSort == ladder_sort_default || ladderSort == ladder_sort_highestrated)
					{
						if (ladderTime == ladder_time_active)
						{
							account_set_ladder_active_rank(account, clienttag, ladderId, rank_);
						}
						else{
							account_set_ladder_rank(account, clienttag, ladderId, rank_);
						}
					}
					else{
						// the account rank is only determined by highestrated/default ladder sort
						return false;
					}
				}
				else if (referenceType == referenceTypeTeam)
				{
					team_set_rank(team, rank_);
				}
				return true;
			}


		void
			LadderReferencedObject::activate(const LadderKey& ladderKey_) const
		{
				if (referenceType == referenceTypeAccount)
				{
					if (ladderKey_.getLadderSort() == ladder_sort_highestrated)
					{

						t_clienttag clienttag = ladderKey_.getClienttag();
						t_ladder_id ladderId = ladderKey_.getLadderId();
						char const * timestr;
						t_bnettime bt;

						account_set_ladder_active_wins(account, clienttag, ladderId,
							account_get_ladder_wins(account, clienttag, ladderId));
						account_set_ladder_active_losses(account, clienttag, ladderId,
							account_get_ladder_losses(account, clienttag, ladderId));
						account_set_ladder_active_draws(account, clienttag, ladderId,
							account_get_ladder_draws(account, clienttag, ladderId));
						account_set_ladder_active_disconnects(account, clienttag, ladderId,
							account_get_ladder_disconnects(account, clienttag, ladderId));
						account_set_ladder_active_rating(account, clienttag, ladderId,
							account_get_ladder_rating(account, clienttag, ladderId));
						account_set_ladder_active_rank(account, clienttag, ladderId,
							account_get_ladder_rank(account, clienttag, ladderId));
						if (!(timestr = account_get_ladder_last_time(account, clienttag, ladderId)))
							timestr = BNETD_LADDER_DEFAULT_TIME;
						bnettime_set_str(&bt, timestr);
						account_set_ladder_active_last_time(account, clienttag, ladderId, bt);
					}
				}
			}


		const t_referenceType
			LadderReferencedObject::getReferenceType() const
		{
				return referenceType;
			}


		t_account *
			LadderReferencedObject::getAccount() const
		{
				if (referenceType == referenceTypeAccount)
					return account;
				else
					return NULL;
			}

		t_team *
			LadderReferencedObject::getTeam() const
		{
				if (referenceType == referenceTypeTeam)
					return team;
				else
					return NULL;
			}


		LadderEntry::LadderEntry(unsigned int uid_, unsigned int primary_, unsigned int secondary_, unsigned int tertiary_, LadderReferencedObject referencedObject_)
			:uid(uid_), primary(primary_), secondary(secondary_), tertiary(tertiary_), rank(0), referencedObject(referencedObject_)
		{
		}


		LadderEntry::~LadderEntry() throw()
		{
		}


		unsigned int
			LadderEntry::getUid() const
		{
				return uid;
			}


		unsigned int
			LadderEntry::getPrimary() const
		{
				return primary;
			}


		unsigned int
			LadderEntry::getSecondary() const
		{
				return secondary;
			}


		unsigned int
			LadderEntry::getTertiary() const
		{
				return tertiary;
			}


		unsigned int
			LadderEntry::getRank() const
		{
				return rank;
			}


		const LadderReferencedObject&
			LadderEntry::getReferencedObject() const
		{
				return referencedObject;
			}


		bool
			LadderEntry::setRank(unsigned int rank_, const LadderKey& ladderKey_)
		{
				if (referencedObject.getRank(ladderKey_) != rank_)
				{
					rank = rank_;
					return referencedObject.setRank(ladderKey_, rank_);
				}
				else{
					return false;
				}
			}


		void
			LadderEntry::update(unsigned int primary_, unsigned int secondary_, unsigned int tertiary_)
		{
				primary = primary_;
				secondary = secondary_;
				tertiary = tertiary_;
			}

		std::string
			LadderEntry::status() const
		{
				std::ostringstream result;
				result << uid << "," << primary << "," << secondary << "," << tertiary;

				return result.str();
			}

		bool
			LadderEntry::operator== (const LadderEntry& right) const
		{
				return ((uid == right.uid) &&
					(primary == right.primary) &&
					(secondary == right.secondary) &&
					(tertiary == right.tertiary));
			}


		bool
			LadderEntry::operator< (const LadderEntry& right) const
		{
				if (primary != right.primary)
					return (primary>right.primary);
				else if (secondary != right.secondary)
					return (secondary > right.secondary);
				else if (tertiary != right.tertiary)
					return (tertiary > right.tertiary);
				else
					return (uid > right.uid);
			}



		LadderList::LadderList(LadderKey ladderKey_, t_referenceType referenceType_)
			:ladderKey(ladderKey_), dirty(true), saved(false), referenceType(referenceType_)
		{
			ladderFilename = clienttag_uint_to_str(ladderKey_.getClienttag());
			ladderFilename += "_";
			ladderFilename += bin_ladder_time_str[ladderKey_.getLadderTime()];
			ladderFilename += bin_ladder_sort_str[ladderKey_.getLadderSort()];
			ladderFilename += bin_ladder_id_str[ladderKey_.getLadderId()];
			ladderFilename += "_LADDER";
		}


		LadderList::~LadderList() throw ()
		{
		}


		bool
			LadderList::load()
		{
				return loadBinary();
			}


		bool
			LadderList::save()
		{
				sortAndUpdate();
				return saveBinary();
			}


		void
			LadderList::sortAndUpdate()
		{
				if (!(dirty))
					return;

				unsigned int rank = 1;
				unsigned int changed = 0;
				LList::iterator endMarker = ladder.end();

				sort(ladder.begin(), ladder.end());

				for (LList::iterator lit(ladder.begin()); lit != ladder.end(); lit++, rank++)
				{
					if (rank <= MaxRankKeptInLadder)
					{
						if (lit->getRank() != rank)
						{
							if (lit->setRank(rank, ladderKey))
								changed++;
						}
					}
					else
					{
						lit->setRank(0, ladderKey);
						if (endMarker == ladder.end())
							endMarker = lit;
					}
				}

				if (endMarker != ladder.end())
					ladder.erase(endMarker, ladder.end());

				if ((changed))
					eventlog(eventlog_level_trace, __FUNCTION__, "adjusted rank for %u accounts", changed);
				saved = false;
				dirty = false;
			}


		const unsigned int  magick = 0xdeadc0de;


		inline void
			LadderList::readdata(std::ifstream &fp, unsigned int &data)
		{
				fp.read((char *)(&data), sizeof(unsigned int));
			}


		inline void
			LadderList::readdata(std::ifstream &fp, unsigned int data[], unsigned int membercount)
		{
				fp.read((char *)(data), sizeof(*data)*membercount);
			}


		bool
			LadderList::loadBinary()
		{
				std::string filename = prefs_get_ladderdir();
				filename += "/";
				filename += ladderFilename;

				std::ifstream fp(filename.c_str(), std::ios::in | std::ios::binary);

				if (!(fp))
				{
					eventlog(eventlog_level_info, __FUNCTION__, "could not open ladder file \"%s\" - maybe ladder still empty (std::ifstream: %s)", filename.c_str(), std::strerror(errno));
					return false;
				}

				unsigned int checksum = 0;
				readdata(fp, checksum);

				if (checksum != magick)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "%s not starting with magick", ladderFilename.c_str());
					return false;
				}

				struct stat sfile;
				if (stat(filename.c_str(), &sfile) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "failed to retrieve size of %s", ladderFilename.c_str());
					return false;
				}

				unsigned int filesize = sfile.st_size;
				if (filesize%sizeof(unsigned int) != 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "%s has unexpected size of %u bytes (not multiple of sizeof(unsigned int)", ladderFilename.c_str(), filesize);
					return false;
				}

				unsigned int noe = (filesize) / sizeof(unsigned int)-2;

				if (noe % 4 != 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "%s has unexpected count of entries (%u) ", ladderFilename.c_str(), noe);
					return false;
				}

				checksum = 0;
				unsigned int values[4];
				unsigned int uid, primary, secondary, tertiary;

				while (noe >= 4)
				{
					readdata(fp, values, 4);
					noe -= 4;

					//handle differently dependant on ladderKey->ladderId
					if (t_account* account = accountlist_find_account_by_uid(values[0]))
					{
						uid = values[0];
						primary = values[2];
						secondary = values[1];
						tertiary = values[3];
						LadderReferencedObject reference(account);
						addEntry(uid, primary, secondary, tertiary, reference);
					}
					else{
						eventlog(eventlog_level_debug, __FUNCTION__, "no known entry for uid %u", values[0]);
					}
					for (int count = 0; count < 4; count++) checksum += values[count];
				}

				unsigned int filechecksum = 0;
				readdata(fp, filechecksum);

				if (filechecksum != checksum)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "%s has invalid checksum... fall back to old loading mode", ladderFilename.c_str());
					ladder.clear();
					return false;
				}

				eventlog(eventlog_level_info, __FUNCTION__, "successfully loaded %s", filename.c_str());
				return true;
			}


		inline void
			LadderList::writedata(std::ofstream &fp, unsigned int &data)
		{
				fp.write((char *)(&data), sizeof(unsigned int));
			}


		inline void
			LadderList::writedata(std::ofstream &fp, const unsigned int &data)
		{
				fp.write((char *)(&data), sizeof(unsigned int));
			}


		inline void
			LadderList::writedata(std::ofstream &fp, unsigned int data[], unsigned int membercount)
		{
				fp.write((char *)(data), sizeof(*data)*membercount);
			}


		bool
			LadderList::saveBinary()
		{

				if (saved)
					return true;

				std::string filename = prefs_get_ladderdir();
				filename += "/";
				filename += ladderFilename;

				std::ofstream fp(filename.c_str(), std::ios::out | std::ios::binary);

				if (!(fp))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for writing (std::ofstream: %s)", filename.c_str(), std::strerror(errno));
					return false;
				}

				writedata(fp, magick); //write the magick int as header

				unsigned int checksum = 0;
				unsigned int results[4];

				for (LList::const_iterator lit(ladder.begin()); lit != ladder.end(); lit++)
				{
					results[0] = lit->getUid();
					results[1] = lit->getSecondary();
					results[2] = lit->getPrimary();
					results[3] = 0;
					writedata(fp, results, 4);

					//calculate a checksum over saved data
					for (int count = 0; count < 4; count++) checksum += results[count];
				}

				writedata(fp, checksum); // add checksum at the en

				saved = true;
				eventlog(eventlog_level_info, __FUNCTION__, "successfully saved %s", filename.c_str());
				return true;
			}


		void
			LadderList::addEntry(unsigned int uid_, unsigned int primary_, unsigned int secondary_, unsigned int tertiary_, const LadderReferencedObject& referencedObject_)
		{
				if (referenceType != referencedObject_.getReferenceType())
				{
					eventlog(eventlog_level_error, __FUNCTION__, "referenceType of LadderList and referencedObject do mismatch");
					return;
				}

				LadderEntry entry(uid_, primary_, secondary_, tertiary_, referencedObject_);
				ladder.push_back(entry);
				dirty = true;
			}


		void
			LadderList::updateEntry(unsigned int uid_, unsigned int primary_, unsigned int secondary_, unsigned int tertiary_, const LadderReferencedObject& referencedObject_)
		{
				if (referenceType != referencedObject_.getReferenceType())
				{
					eventlog(eventlog_level_error, __FUNCTION__, "referenceType of LadderList and referencedObject do mismatch");
					return;
				}

				LList::iterator lit(ladder.begin());
				for (; lit != ladder.end() && lit->getUid() != uid_; lit++);

				if (lit == ladder.end())
				{
					LadderEntry entry(uid_, primary_, secondary_, tertiary_, referencedObject_);
					ladder.push_back(entry);
				}
				else{
					lit->update(primary_, secondary_, tertiary_);
				}
				dirty = true;
			}


		bool
			LadderList::delEntry(unsigned int uid_)
		{
				LList::iterator lit(ladder.begin());
				for (; lit != ladder.end() && lit->getUid() != uid_; lit++);

				if (lit == ladder.end())
					return false; //account not on ladder
				else{

					lit->setRank(0, ladderKey);
					ladder.erase(lit);
					dirty = true;
					return true;
				}
			}


		const LadderReferencedObject*
			LadderList::getReferencedObject(unsigned int rank_) const
		{
				if (rank_ > ladder.size())
					return 0;
				else
					return &ladder[rank_ - 1].getReferencedObject();
			}


		unsigned int
			LadderList::getRank(unsigned int uid_) const
		{
				unsigned int rank;
				LList::const_iterator lit(ladder.begin());
				for (rank = 1; lit != ladder.end() && lit->getUid() != uid_; lit++);

				if (lit == ladder.end())
					return 0;
				else
					return lit->getRank();
			}


		const LadderKey&
			LadderList::getLadderKey() const
		{
				return ladderKey;
			}


		const t_referenceType
			LadderList::getReferenceType() const
		{

				return referenceType;
			}


		void
			LadderList::activateFrom(const LadderList * currentLadder_)
		{
				const LList* currentList = &currentLadder_->ladder;
				for (LList::const_iterator lit(currentList->begin()); lit != currentList->end(); lit++)
				{
					const LadderReferencedObject& referencedObject = lit->getReferencedObject();
					updateEntry(lit->getUid(), lit->getPrimary(), lit->getSecondary(), lit->getTertiary(), referencedObject);
					referencedObject.activate(ladderKey);
				}
				return;
			}


		void
			LadderList::writeStatusfile() const
		{
				std::string filename;

				filename = prefs_get_outputdir();
				filename += "/";
				filename += ladderFilename;

				std::ofstream fp(filename.c_str());

				if (!(fp))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for writing (std::ofstream: %s)", filename.c_str(), std::strerror(errno));
					return;
				}

				unsigned int rank = 1;
				for (LList::const_iterator lit(ladder.begin()); lit != ladder.end(); lit++, rank++)
				{
					fp << rank << "," << lit->status() << "\n";
				}

			}


		Ladders::Ladders()
		{
			// WAR3 ladders
			LadderKey WAR3_solo(ladder_id_solo, CLIENTTAG_WARCRAFT3_UINT, ladder_sort_default, ladder_time_default);
			ladderMap.insert(std::make_pair(WAR3_solo, LadderList(WAR3_solo, referenceTypeAccount)));

			LadderKey WAR3_team(ladder_id_team, CLIENTTAG_WARCRAFT3_UINT, ladder_sort_default, ladder_time_default);
			ladderMap.insert(std::make_pair(WAR3_team, LadderList(WAR3_team, referenceTypeAccount)));

			LadderKey WAR3_ffa(ladder_id_ffa, CLIENTTAG_WARCRAFT3_UINT, ladder_sort_default, ladder_time_default);
			ladderMap.insert(std::make_pair(WAR3_ffa, LadderList(WAR3_ffa, referenceTypeAccount)));

			LadderKey WAR3_ateam(ladder_id_ateam, CLIENTTAG_WARCRAFT3_UINT, ladder_sort_default, ladder_time_default);
			ladderMap.insert(std::make_pair(WAR3_ateam, LadderList(WAR3_ateam, referenceTypeTeam)));


			//W3XP ladders
			LadderKey W3XP_solo(ladder_id_solo, CLIENTTAG_WAR3XP_UINT, ladder_sort_default, ladder_time_default);
			ladderMap.insert(std::make_pair(W3XP_solo, LadderList(W3XP_solo, referenceTypeAccount)));

			LadderKey W3XP_team(ladder_id_team, CLIENTTAG_WAR3XP_UINT, ladder_sort_default, ladder_time_default);
			ladderMap.insert(std::make_pair(W3XP_team, LadderList(W3XP_team, referenceTypeAccount)));

			LadderKey W3XP_ffa(ladder_id_ffa, CLIENTTAG_WAR3XP_UINT, ladder_sort_default, ladder_time_default);
			ladderMap.insert(std::make_pair(W3XP_ffa, LadderList(W3XP_ffa, referenceTypeAccount)));

			LadderKey W3XP_ateam(ladder_id_ateam, CLIENTTAG_WAR3XP_UINT, ladder_sort_default, ladder_time_default);
			ladderMap.insert(std::make_pair(W3XP_ateam, LadderList(W3XP_ateam, referenceTypeTeam)));

			//STAR ladders
			LadderKey STAR_ar(ladder_id_normal, CLIENTTAG_STARCRAFT_UINT, ladder_sort_highestrated, ladder_time_active);
			ladderMap.insert(std::make_pair(STAR_ar, LadderList(STAR_ar, referenceTypeAccount)));

			LadderKey STAR_aw(ladder_id_normal, CLIENTTAG_STARCRAFT_UINT, ladder_sort_mostwins, ladder_time_active);
			ladderMap.insert(std::make_pair(STAR_aw, LadderList(STAR_aw, referenceTypeAccount)));

			LadderKey STAR_ag(ladder_id_normal, CLIENTTAG_STARCRAFT_UINT, ladder_sort_mostgames, ladder_time_active);
			ladderMap.insert(std::make_pair(STAR_ag, LadderList(STAR_ag, referenceTypeAccount)));

			LadderKey STAR_cr(ladder_id_normal, CLIENTTAG_STARCRAFT_UINT, ladder_sort_highestrated, ladder_time_current);
			ladderMap.insert(std::make_pair(STAR_cr, LadderList(STAR_cr, referenceTypeAccount)));

			LadderKey STAR_cw(ladder_id_normal, CLIENTTAG_STARCRAFT_UINT, ladder_sort_mostwins, ladder_time_current);
			ladderMap.insert(std::make_pair(STAR_cw, LadderList(STAR_cw, referenceTypeAccount)));

			LadderKey STAR_cg(ladder_id_normal, CLIENTTAG_STARCRAFT_UINT, ladder_sort_mostgames, ladder_time_current);
			ladderMap.insert(std::make_pair(STAR_cg, LadderList(STAR_cg, referenceTypeAccount)));

			//SEXP ladders
			LadderKey SEXP_ar(ladder_id_normal, CLIENTTAG_BROODWARS_UINT, ladder_sort_highestrated, ladder_time_active);
			ladderMap.insert(std::make_pair(SEXP_ar, LadderList(SEXP_ar, referenceTypeAccount)));

			LadderKey SEXP_aw(ladder_id_normal, CLIENTTAG_BROODWARS_UINT, ladder_sort_mostwins, ladder_time_active);
			ladderMap.insert(std::make_pair(SEXP_aw, LadderList(SEXP_aw, referenceTypeAccount)));

			LadderKey SEXP_ag(ladder_id_normal, CLIENTTAG_BROODWARS_UINT, ladder_sort_mostgames, ladder_time_active);
			ladderMap.insert(std::make_pair(SEXP_ag, LadderList(SEXP_ag, referenceTypeAccount)));

			LadderKey SEXP_cr(ladder_id_normal, CLIENTTAG_BROODWARS_UINT, ladder_sort_highestrated, ladder_time_current);
			ladderMap.insert(std::make_pair(SEXP_cr, LadderList(SEXP_cr, referenceTypeAccount)));

			LadderKey SEXP_cw(ladder_id_normal, CLIENTTAG_BROODWARS_UINT, ladder_sort_mostwins, ladder_time_current);
			ladderMap.insert(std::make_pair(SEXP_cw, LadderList(SEXP_cw, referenceTypeAccount)));

			LadderKey SEXP_cg(ladder_id_normal, CLIENTTAG_BROODWARS_UINT, ladder_sort_mostgames, ladder_time_current);
			ladderMap.insert(std::make_pair(SEXP_cg, LadderList(SEXP_cg, referenceTypeAccount)));

			//W2BN ladders
			LadderKey W2BN_ar(ladder_id_normal, CLIENTTAG_WARCIIBNE_UINT, ladder_sort_highestrated, ladder_time_active);
			ladderMap.insert(std::make_pair(W2BN_ar, LadderList(W2BN_ar, referenceTypeAccount)));

			LadderKey W2BN_aw(ladder_id_normal, CLIENTTAG_WARCIIBNE_UINT, ladder_sort_mostwins, ladder_time_active);
			ladderMap.insert(std::make_pair(W2BN_aw, LadderList(W2BN_aw, referenceTypeAccount)));

			LadderKey W2BN_ag(ladder_id_normal, CLIENTTAG_WARCIIBNE_UINT, ladder_sort_mostgames, ladder_time_active);
			ladderMap.insert(std::make_pair(W2BN_ag, LadderList(W2BN_ag, referenceTypeAccount)));

			LadderKey W2BN_cr(ladder_id_normal, CLIENTTAG_WARCIIBNE_UINT, ladder_sort_highestrated, ladder_time_current);
			ladderMap.insert(std::make_pair(W2BN_cr, LadderList(W2BN_cr, referenceTypeAccount)));

			LadderKey W2BN_cw(ladder_id_normal, CLIENTTAG_WARCIIBNE_UINT, ladder_sort_mostwins, ladder_time_current);
			ladderMap.insert(std::make_pair(W2BN_cw, LadderList(W2BN_cw, referenceTypeAccount)));

			LadderKey W2BN_cg(ladder_id_normal, CLIENTTAG_WARCIIBNE_UINT, ladder_sort_mostgames, ladder_time_current);
			ladderMap.insert(std::make_pair(W2BN_cg, LadderList(W2BN_cg, referenceTypeAccount)));

			LadderKey W2BN_ari(ladder_id_ironman, CLIENTTAG_WARCIIBNE_UINT, ladder_sort_highestrated, ladder_time_active);
			ladderMap.insert(std::make_pair(W2BN_ari, LadderList(W2BN_ari, referenceTypeAccount)));

			LadderKey W2BN_awi(ladder_id_ironman, CLIENTTAG_WARCIIBNE_UINT, ladder_sort_mostwins, ladder_time_active);
			ladderMap.insert(std::make_pair(W2BN_awi, LadderList(W2BN_awi, referenceTypeAccount)));

			LadderKey W2BN_agi(ladder_id_ironman, CLIENTTAG_WARCIIBNE_UINT, ladder_sort_mostgames, ladder_time_active);
			ladderMap.insert(std::make_pair(W2BN_agi, LadderList(W2BN_agi, referenceTypeAccount)));

			LadderKey W2BN_cri(ladder_id_ironman, CLIENTTAG_WARCIIBNE_UINT, ladder_sort_highestrated, ladder_time_current);
			ladderMap.insert(std::make_pair(W2BN_cri, LadderList(W2BN_cri, referenceTypeAccount)));

			LadderKey W2BN_cwi(ladder_id_ironman, CLIENTTAG_WARCIIBNE_UINT, ladder_sort_mostwins, ladder_time_current);
			ladderMap.insert(std::make_pair(W2BN_cwi, LadderList(W2BN_cwi, referenceTypeAccount)));

			LadderKey W2BN_cgi(ladder_id_ironman, CLIENTTAG_WARCIIBNE_UINT, ladder_sort_mostgames, ladder_time_current);
			ladderMap.insert(std::make_pair(W2BN_cgi, LadderList(W2BN_cgi, referenceTypeAccount)));

			//TSUN ladders
			LadderKey TSUN_solo(ladder_id_solo, CLIENTTAG_TIBERNSUN_UINT, ladder_sort_default, ladder_time_default);
			ladderMap.insert(std::make_pair(TSUN_solo, LadderList(TSUN_solo, referenceTypeAccount)));

			//TSXP ladders
			LadderKey TSXP_solo(ladder_id_solo, CLIENTTAG_TIBSUNXP_UINT, ladder_sort_default, ladder_time_default);
			ladderMap.insert(std::make_pair(TSXP_solo, LadderList(TSXP_solo, referenceTypeAccount)));

			//RAL2 ladders
			LadderKey RAL2_solo(ladder_id_solo, CLIENTTAG_REDALERT2_UINT, ladder_sort_default, ladder_time_default);
			ladderMap.insert(std::make_pair(RAL2_solo, LadderList(RAL2_solo, referenceTypeAccount)));

			//YURI ladders
			LadderKey YURI_solo(ladder_id_solo, CLIENTTAG_YURISREV_UINT, ladder_sort_default, ladder_time_default);
			ladderMap.insert(std::make_pair(YURI_solo, LadderList(YURI_solo, referenceTypeAccount)));

		}

		Ladders::~Ladders() throw ()
		{
		}


		LadderList*
			Ladders::getLadderList(const LadderKey& ladderKey_)
		{
				KeyLadderMap::iterator kit(ladderMap.begin());
				for (; kit != ladderMap.end() && (!(ladderKey_ == (*kit).first)); kit++);

				if (kit == ladderMap.end())
				{
					eventlog(eventlog_level_error, __FUNCTION__, "found no matching ladderlist");
					return NULL;
				}
				else{
					return &(*kit).second;
				}
			}


		void
			Ladders::load()
		{
				std::list<LadderList*> laddersToRebuild;
				for (KeyLadderMap::iterator kit(ladderMap.begin()); kit != ladderMap.end(); kit++)
				{
					if (!kit->second.load())
					{
						//loading failed, add to rebuild list
						laddersToRebuild.push_back(&kit->second);
					}
				}

				if (!laddersToRebuild.empty())
					rebuild(laddersToRebuild);

				update();
				save();
			}

		void
			Ladders::rebuild(std::list<LadderList*>& laddersToRebuild)
		{
				t_entry * curr;
				t_account * account;
				unsigned int uid, primary, secondary, tertiary;

				eventlog(eventlog_level_debug, __FUNCTION__, "start rebuilding ladders");

				if (accountlist_load_all(ST_FORCE)) {
					eventlog(eventlog_level_error, __FUNCTION__, "error loading all accounts");
					return;
				}

				HASHTABLE_TRAVERSE(accountlist(), curr)
				{
					if ((account = ((t_account *)entry_get_data(curr))))
					{

						LadderReferencedObject referencedObject(account);
						for (std::list<LadderList*>::iterator lit(laddersToRebuild.begin()); lit != laddersToRebuild.end(); lit++)
						{
							// only do handle referenceTypeAccount ladders here
							if ((*lit)->getReferenceType() != referenceTypeAccount)
							{
								continue;
							}

							if (referencedObject.getData((*lit)->getLadderKey(), uid, primary, secondary, tertiary))
							{
								(*lit)->addEntry(uid, primary, secondary, tertiary, referencedObject);
							}
						}
					}
				}

				// now we would need to traverse teamlist, too.
				// how about comletly moving this code into team?

				eventlog(eventlog_level_debug, __FUNCTION__, "done rebuilding ladders");

			}

		void
			Ladders::update()
		{
				for (KeyLadderMap::iterator kit(ladderMap.begin()); kit != ladderMap.end(); kit++)
				{
					kit->second.sortAndUpdate();
				}
			}


		void
			Ladders::activate()
		{
				for (KeyLadderMap::iterator kit(ladderMap.begin()); kit != ladderMap.end(); kit++)
				{
					if (kit->second.getLadderKey().getLadderTime() == ladder_time_active)
					{
						LadderList* activeLadder = &kit->second;
						LadderList* currentLadder = getLadderList(activeLadder->getLadderKey().getOpposingKey());

						if (currentLadder == NULL){
							eventlog(eventlog_level_error, __FUNCTION__, "found active ladder, but no matching current ladder");
							continue;
						}
						activeLadder->activateFrom(currentLadder);
					}
				}
				update();
			}


		void
			Ladders::save()
		{
				for (KeyLadderMap::iterator kit(ladderMap.begin()); kit != ladderMap.end(); kit++)
				{
					kit->second.save();
				}
			}


		void
			Ladders::status() const
		{
				for (KeyLadderMap::const_iterator kit(ladderMap.begin()); kit != ladderMap.end(); kit++)
				{
					kit->second.writeStatusfile();
				}
			}


	}

}

