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
#include "ladder_calc.h"

#include <cmath>

#include "common/eventlog.h"

#include "account.h"
#include "account_wrap.h"
#include "common/setup_after.h"


namespace pvpgn
{

	namespace bnetd
	{

		static double probability(unsigned int a, unsigned int b);
		static int coefficient(t_account * account, t_clienttag clienttag, t_ladder_id id);

		static double two_player(unsigned int *rating);

		static double three_player(unsigned int *rating);

		static double four_player(unsigned int *rating);

		static double five_player(unsigned int *rating);
		static double five_f1(int a, int b, int c, int d, int e);
		static double five_f2(int a, int b, int c);

		static double six_player(unsigned int *rating);
		static double six_f1(int a, int b, int c, int d, int e, int f);
		static double six_f2(int a, int b, int c, int d, int e, int f);
		static double six_f3(int a, int b, int c, int d);

		static double seven_player(unsigned int *rating);
		static double seven_f1(int a, int b, int c, int d, int e, int f, int g);
		static double seven_f2(int a, int b, int c, int d, int e, int f, int g);

		static double eight_player(unsigned int *rating);
		static double eight_f1(int a, int b, int c, int d, int e, int f, int g);
		static double eight_f2(int a, int b, int c, int d, int e, int f, int g);
		static double eight_f3(int a, int b, int c, int d, int e);

		/*
		 * Compute probability of winning using the Elo system
		 *
		 * The formula is:
		 *
		 *  D = rating(player a) - rating(player b)
		 *
		 *                    1
		 *  Pwin(D) = ------------------
		 *                   -(D / 400)
		 *             1 + 10
		 */
		static double probability(unsigned int a, unsigned int b)
		{
			double i, j;

			i = (((double)a) - ((double)b)) / 400.0;

			j = std::pow(10.0, -i);

			return (1.0 / (1.0 + j));
		}


		/*
		 *  This is the coefficient k which is meant to enhance the
		 *  effect of the Elo std::system where more experienced players
		 *  will gain fewer points when playing against newbies, and
		 *  newbies will gain massive points if they win against an
		 *  experienced player. It also helps stabilize a player's
		 *  rating after they have played 30 games or so.
		 *
		 *  K=50 for new players
		 *  K=30 for players who have played 30 or more ladder games
		 *  K=20 for players who have attained a rating of 2400 or higher
		 */
		static int coefficient(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			int const total_ladder_games = account_get_ladder_wins(account, clienttag, id) +
				account_get_ladder_losses(account, clienttag, id) +
				account_get_ladder_disconnects(account, clienttag, id);

			if (total_ladder_games < 30)
				return 50;

			if (account_get_ladder_rating(account, clienttag, id) < 2400)
				return 30;

			return 20;
		}


		/*
		 * The Elo std::system only handles 2 players, these functions extend
		 * the calculation to different numbers of players as if they were
		 * in a tournament. It turns out the math for this is really ugly,
		 * so we have hardcoded the equations for every number of players.
		 */
		static double two_player(unsigned int *rating)
		{
			unsigned int a, b;
			double       ab;

			a = rating[0];
			b = rating[1];

			ab = probability(a, b);

			return ab;
		}

		static double three_player(unsigned int *rating)
		{
			unsigned int a, b, c;
			double       ab, ac, bc, cb;

			a = rating[0];
			b = rating[1];
			c = rating[2];

			ab = probability(a, b);
			ac = probability(a, c);
			bc = probability(b, c);
			cb = 1.0 - bc;

			return (2 * (ab*ac) + (bc*ab) + (cb*ac)) / 3;
		}

		static double four_player(unsigned int *rating)
		{
			unsigned int a, b, c, d;
			double       ab, ac, ad, bc, bd, cb, cd, db, dc;

			a = rating[0];
			b = rating[1];
			c = rating[2];
			d = rating[3];


			ab = probability(a, b);
			ac = probability(a, c);
			ad = probability(a, d);
			bc = probability(b, c);
			bd = probability(b, d);
			cd = probability(c, d);
			cb = 1.0 - bc;
			db = 1.0 - bd;
			dc = 1.0 - cd;

			return (ab*ac*(cd + bd) + ac*ad*(db + cb) + ab*ad*(dc + bc)) / 3;
		}


		/* [Denis MOREAUX <vapula@linuxbe.org>, 10 Apr 2000]
		 *
		 *     C D E          A D E   The winner may be in the
		 * A B  C E       B C  A E    2 players or the 3 players
		 *  A    C         B    A     group. In either case, a
		 *    A              A        second player must be choosen
		 *                            to be either the one playing
		 * against A in the 2-players subtree or being the winner
		 * of the 2-player subtree if A is in the 3-players subtree.
		 */

		static double five_player(unsigned int *rating)
		{
			unsigned int a, b, c, d, e;

			a = rating[0];
			b = rating[1];
			c = rating[2];
			d = rating[3];
			e = rating[4];

			return (five_f1(a, b, c, d, e) + five_f1(a, c, d, e, b) +
				five_f1(a, d, e, b, c) + five_f1(a, e, b, c, d)) / 30;
		}

		/* [Denis MOREAUX <vapula@linuxbe.org>, 10 Apr 2000
		 *
		 * Two cases to treat : AB-CDE and BC-ADE.
		 * in both cases, A win against B.
		 * In the first case, A win over the winner of a 3-players game
		 * (3 possible winners).
		 * In the second case, B win over one of the three other and A is in
		 * the 3-players game.
		 */

		static double five_f1(int a, int b, int c, int d, int e)
		{
			double ab, ac, ad, ae, bc, bd, be;

			ab = probability(a, b);
			ac = probability(a, c);
			ad = probability(a, d);
			ae = probability(a, e);
			bc = probability(b, c);
			bd = probability(b, d);
			be = probability(b, e);

			return ab*(ac*five_f2(c, d, e) + ad*five_f2(d, e, c) + ae*five_f2(e, c, d) +
				bc*five_f2(a, d, e) + bd*five_f2(a, c, e) + be*five_f2(a, c, d));
		}

		static double five_f2(int a, int b, int c)
		{
			double       ab, ac, bc, cb;

			ab = probability(a, b);
			ac = probability(a, c);
			bc = probability(b, c);
			cb = 1.0 - bc;

			return (2 * (ab*ac) + bc*ab + cb*ac);
		}


		static double six_player(unsigned int *rating)
		{
			unsigned int a, b, c, d, e, f;

			a = rating[0];
			b = rating[1];
			c = rating[2];
			d = rating[3];
			e = rating[4];
			f = rating[5];

			/* A B C D
			 *  A   C    E F
			 *    A       E
			 *        A
			 */

			return (six_f1(a, b, c, d, e, f) +  /* A is in group of 4 */
				six_f1(a, b, c, e, d, f) +
				six_f1(a, b, e, d, c, f) +
				six_f1(a, e, c, d, b, f) +
				six_f1(a, b, c, f, d, e) +
				six_f1(a, b, f, d, c, e) +
				six_f1(a, f, c, d, b, e) +
				six_f1(a, e, f, b, c, d) +
				six_f1(a, e, f, c, b, d) +
				six_f1(a, e, f, d, b, c) +
				six_f2(a, b, c, d, e, f) +   /* A is in group of 2 */
				six_f2(a, c, b, d, e, f) +
				six_f2(a, d, b, c, e, f) +
				six_f2(a, e, b, c, d, f) +
				six_f2(a, f, b, c, d, e)) / 45;
		}


		/* ABCD = group of 4, EF = group of 2, A must win */
		/* D.Moreaux, 10 Apr 2000: changed double to int for the parameters */

		static double six_f1(int a, int b, int c, int d, int e, int f)
		{
			double ab, ac, ad, bc, bd, cb, cd, db, dc, ef, fe, ae, af;

			ab = probability(a, b);
			ac = probability(a, c);
			ad = probability(a, d);
			ae = probability(a, e);
			af = probability(a, f);
			bc = probability(b, c);
			bd = probability(b, d);
			cd = probability(c, d);
			ef = probability(e, f);
			cb = 1.0 - bc;
			db = 1.0 - bd;
			dc = 1.0 - cd;
			fe = 1.0 - ef;
			return (ab*ac*(cd + bd) + ac*ad*(db + cb) + ab*ad*(dc + bc))*(ef*ae + fe*af);
		}


		/* AB is group of 2, CDEF is group of 4, A must win */

		static double six_f2(int a, int b, int c, int d, int e, int f)
		{
			double ab, ac, ad, ae, af;

			ab = probability(a, b);
			ac = probability(a, c);
			ad = probability(a, d);
			ae = probability(a, e);
			af = probability(a, f);

			return (six_f3(c, d, e, f)*ab*ac +
				six_f3(d, c, e, f)*ab*ad +
				six_f3(e, c, d, f)*ab*ae +
				six_f3(f, c, d, e)*ab*af);
		}


		/* ABCD is group of 4, A win */

		static double six_f3(int a, int b, int c, int d)
		{
			double ab, ac, ad, bc, bd, cb, cd, db, dc;

			ab = probability(a, b);
			ac = probability(a, c);
			ad = probability(a, d);
			bc = probability(b, c);
			bd = probability(b, d);
			cd = probability(c, d);
			cb = 1.0 - bc;
			db = 1.0 - bd;
			dc = 1.0 - cd;

			return (ab*ac*(cd + bd) + ac*ad*(db + cb) + ab*ad*(dc + bc));
		}


		static double seven_player(unsigned int *rating)
		{
			unsigned int a, b, c, d, e, f, g;

			a = rating[0];
			b = rating[1];
			c = rating[2];
			d = rating[3];
			e = rating[4];
			f = rating[5];
			g = rating[6];

			return (seven_f1(a, b, c, d, e, f, g) + seven_f1(a, c, b, d, e, f, g) +
				seven_f1(a, d, c, b, e, f, g) + seven_f1(a, e, c, d, b, f, g) +
				seven_f1(a, f, c, d, e, b, g) + seven_f1(a, g, c, d, e, f, b)) / 45;
		}

		static double seven_f1(int a, int b, int c, int d, int e, int f, int g)
		{

			return seven_f2(a, b, c, d, e, f, g) + seven_f2(a, b, d, c, e, f, g) +
				seven_f2(a, b, e, d, c, f, g) + seven_f2(a, b, f, d, e, c, g) +
				seven_f2(a, b, g, d, e, f, c);
		}

		static double seven_f2(int a, int b, int c, int d, int e, int f, int g)
		{
			double ab, ac, ad, ae, af, ag, bc, bd, be, bf, bg, cd, ce, cf, cg;
			double de, df, dg, ed, ef, eg, fd, fe, fg, gd, ge, gf;
			ab = probability(a, b);
			ac = probability(a, c);
			ad = probability(a, d);
			ae = probability(a, e);
			af = probability(a, f);
			ag = probability(a, g);
			bc = probability(b, c);
			bd = probability(b, d);
			be = probability(b, e);
			bf = probability(b, f);
			bg = probability(b, g);
			cd = probability(c, d);
			ce = probability(c, e);
			cf = probability(c, f);
			cg = probability(c, g);
			de = probability(d, e);
			df = probability(d, f);
			dg = probability(d, g);
			ef = probability(e, f);
			eg = probability(e, g);
			fg = probability(f, g);
			ed = 1.0 - de;
			fd = 1.0 - df;
			gd = 1.0 - dg;
			fe = 1.0 - ef;
			ge = 1.0 - eg;
			gf = 1.0 - fg;

			return
				ab*(
				(ac + bc)*
				(ad*(de*df*(fg + eg) + df*dg*(ge + fe) + de*dg*(gf + ef)) +   /* 4:d win */
				ae*(ed*ef*(fg + dg) + ef*eg*(gd + fd) + ed*eg*(gf + df)) +   /* 4:e win */
				af*(fe*fd*(dg + eg) + fd*fg*(ge + de) + fe*fg*(gd + ed)) +   /* 4:f win */
				ag*(ge*gf*(fd + ed) + gf*gd*(de + fe) + ge*gd*(df + ef))) +  /* 4:g win */
				bc*
				((bd + cd)*(ae*af*(fg + eg) + af*ag*(ge + fe) + ae*ag*(gf + ef)) +   /* 3:d */
				(be + ce)*(ad*af*(fg + dg) + af*ag*(ad + ad) + ad*ag*(gf + df)) +   /* 3:e */
				(bf + cf)*(ae*ad*(dg + eg) + ad*ag*(ge + de) + ae*ag*(gd + ed)) +   /* 3:f */
				(bg + cg)*(ae*af*(fd + ed) + af*ad*(de + fe) + ae*ad*(df + ef))));  /* 3:g */

		}

		static double eight_player(unsigned int *rating)
		{
			unsigned int a, b, c, d, e, f, g, h;
			double       ab, ac, ad, ae, af, ag, ah;

			a = rating[0];
			b = rating[1];
			c = rating[2];
			d = rating[3];
			e = rating[4];
			f = rating[5];
			g = rating[6];
			h = rating[7];

			ab = probability(a, b);
			ac = probability(a, c);
			ad = probability(a, d);
			ae = probability(a, e);
			af = probability(a, f);
			ag = probability(a, g);
			ah = probability(a, h);

			/* First against A may be one from seven */

			return (eight_f1(a, c, d, e, f, g, h)*ab +
				eight_f1(a, b, d, e, f, g, h)*ac +
				eight_f1(a, b, c, e, f, g, h)*ad +
				eight_f1(a, b, c, d, f, g, h)*ae +
				eight_f1(a, b, c, d, e, g, h)*af +
				eight_f1(a, b, c, d, e, f, h)*ag +
				eight_f1(a, b, c, d, e, f, g)*ah) / 315;

		}


		static double eight_f1(int a, int b, int c, int d, int e, int f, int g)
		{
			/* The winner of the second group, who'll then play against A, may be one
			   from six possible players */

			return eight_f2(a, b, c, d, e, f, g) +
				eight_f2(a, c, b, d, e, f, g) +
				eight_f2(a, d, b, c, e, f, g) +
				eight_f2(a, e, b, c, d, f, g) +
				eight_f2(a, f, b, c, d, e, g) +
				eight_f2(a, g, b, c, d, e, f);
		}

		static double eight_f2(int a, int b, int c, int d, int e, int f, int g)
		{
			double ab, bc, bd, be, bf, bg;

			ab = probability(a, b);
			bc = probability(b, c);
			bd = probability(b, d);
			be = probability(b, e);
			bf = probability(b, f);
			bg = probability(b, g);

			/* There are 5 player who may play against the 3rd. The third (b) will win
			   over them and lose against a */

			return ab*(eight_f3(a, d, e, f, g)*bc +
				eight_f3(a, c, e, f, g)*bd +
				eight_f3(a, c, d, f, g)*be +
				eight_f3(a, c, d, e, g)*bf +
				eight_f3(a, c, d, e, f)*bg);

		}

		/* D.Moreaux, 10 Apr 2000: changed double to int for the parameters */

		static double eight_f3(int a, int b, int c, int d, int e)
		{
			double ab, ac, ad, ae, bc, bd, be, cb, cd, ce, db, dc, de, eb, ec, ed;

			ab = probability(a, b);
			ac = probability(a, c);
			ad = probability(a, d);
			ae = probability(a, e);
			bc = probability(b, c);
			bd = probability(b, d);
			be = probability(b, e);
			cd = probability(c, d);
			ce = probability(c, e);
			de = probability(d, e);
			cb = 1.0 - bc;
			db = 1.0 - bd;
			dc = 1.0 - cd;
			eb = 1.0 - be;
			ec = 1.0 - ce;
			ed = 1.0 - de;

			/* expansion then factorisation (this function is called 210 times)
			 * gain 4 func_call
			 *      24 *
			 *      30 probability
			 */
			return	(bc*de + be*dc)*((ab - ad)*bd + ad) +
				(bd*ce + be*cd)*((ab - ac)*bc + ac) +
				(cb*de + db*ce)*((ac - ad)*cd + ad) +
				(cd*eb + cb*ed)*((ac - ae)*ce + ae) +
				(dc*eb + db*ec)*((ad - ae)*de + ae) +
				(bd*ec + bc*ed)*((ae - ab)*eb + ab);
		}

		/* Determine changes in ratings due to game results. */
		extern int ladder_calc_info(t_clienttag clienttag, t_ladder_id id, unsigned int count, t_account * * players, t_game_result * results, t_ladder_info * info)
		{
			unsigned int curr;
			unsigned int *rating;
			unsigned int *sorted;

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

			rating = (unsigned int*)xmalloc(sizeof(unsigned int)*count);
			sorted = (unsigned int*)xmalloc(sizeof(unsigned int)*count);

			for (curr = 0; curr < count; curr++)
				rating[curr] = account_get_ladder_rating(players[curr], clienttag, id);

			for (curr = 0; curr < count; curr++)
			{
				double k;
				double prob;
				double delta;
				t_game_result myresult;
				unsigned int  opponent_count;
				unsigned int  team_members;

				k = coefficient(players[curr], clienttag, id);
				opponent_count = 0;
				myresult = results[curr];

				{
					unsigned int i, j;

					/* Put the current user into slot 0, his opponents into other slots
					   order is not important for the other players */
					for (i = 0, j = 1; i < count; i++)
					if (i == curr)
						sorted[0] = rating[i];
					else
					{
						if (results[i] != myresult)
						{
							sorted[j++] = rating[i];
							opponent_count++;
						}
					}
				}

				team_members = count - opponent_count;

				switch (opponent_count)
				{
				case 1:
					prob = two_player(sorted);
					break;
				case 2:
					prob = three_player(sorted);
					break;
				case 3:
					prob = four_player(sorted);
					break;
				case 4:
					prob = five_player(sorted);
					break;
				case 5:
					prob = six_player(sorted);
					break;
				case 6:
					prob = seven_player(sorted);
					break;
				case 7:
					prob = eight_player(sorted);
					break;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "sorry, unsupported number of ladder opponents (%u)", opponent_count);
					xfree((void *)rating);
					xfree((void *)sorted);
					return -1;
				}

				if (results[curr] == game_result_win)
					delta = std::fabs(k * (1.0 - prob) / team_members); /* better the chance of winning -> fewer points added */
				else
					delta = -std::fabs(k * prob); /* better the chance of winning -> more points subtracted */

				eventlog(eventlog_level_debug, __FUNCTION__, "computed probability=%g, k=%g, deltar=%+g", prob, k, delta);

				info[curr].prob = prob;
				info[curr].k = (unsigned int)k;
				info[curr].adj = (int)delta;
				info[curr].oldrating = rating[curr];
				info[curr].oldrank = account_get_ladder_rank(players[curr], clienttag, id);
			}

			xfree((void *)rating);
			xfree((void *)sorted);

			return 0;
		}

	}

}
