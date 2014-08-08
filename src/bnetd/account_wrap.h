/*
 * Copyright (C) 1998,1999  Ross Combs (rocombs@cs.nmsu.edu)
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


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_ACCOUNT_WRAP_PROTOS
#define INCLUDED_ACCOUNT_WRAP_PROTOS

#define JUST_NEED_TYPES
#include "account.h"
#include "connection.h"
#include "character.h"
#include "common/bnettime.h"
#include "ladder.h"
#include "game.h"
#include "common/tag.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		/* convenience functions */
		extern unsigned int account_get_numattr_real(t_account * account, char const * key, char const * fn, unsigned int ln);
#define account_get_numattr(A,K) account_get_numattr_real(A,K,__FILE__,__LINE__)
		extern int account_set_numattr(t_account * account, char const * key, unsigned int val);

		extern int account_get_boolattr_real(t_account * account, char const * key, char const * fn, unsigned int ln);
#define account_get_boolattr(A,K) account_get_boolattr_real(A,K,__FILE__,__LINE__)
		extern int account_set_boolattr(t_account * account, char const * key, int val);

		/* warning: unlike the other account_get_*attr functions this one allocates memory for returning data - its up to the caller to clean it up */
		extern char const * account_get_rawattr_real(t_account * account, char const * key, char const * fn, unsigned int ln);
#define account_get_rawattr(A,K) account_get_rawattr_real(A,K,__FILE__,__LINE__)
		extern int account_set_rawattr(t_account * account, char const * key, char const * val, int length);

		extern char const * account_get_pass(t_account * account);
		extern int account_set_pass(t_account * account, char const * passhash1);

		extern char const * account_get_salt(t_account * account);
		extern int account_set_salt(t_account * account, char const * salt);

		extern char const * account_get_verifier(t_account * account);
		extern int account_set_verifier(t_account * account, char const * verifier);

		/* authorization */
		extern int account_get_auth_admin(t_account * account, char const * channelname);
		extern int account_set_auth_admin(t_account * account, char const * channelname, int val);
		extern int account_get_auth_announce(t_account * account);
		extern int account_get_auth_botlogin(t_account * account);
		extern int account_get_auth_bnetlogin(t_account * account);
		extern int account_get_auth_operator(t_account * account, char const * channelname);
		extern int account_set_auth_operator(t_account * account, char const * channelname, int val);
		extern int account_get_auth_voice(t_account * account, char const * channelname);
		extern int account_set_auth_voice(t_account * account, char const * channelname, int val);
		extern int account_get_auth_changepass(t_account * account);
		extern int account_get_auth_changeprofile(t_account * account);
		extern int account_get_auth_createnormalgame(t_account * account);
		extern int account_get_auth_joinnormalgame(t_account * account);
		extern int account_get_auth_createladdergame(t_account * account);
		extern int account_get_auth_joinladdergame(t_account * account);
		extern int account_get_auth_lock(t_account * account);
		extern unsigned int account_get_auth_locktime(t_account * account);
		extern char const * account_get_auth_lockreason(t_account * account);
		extern char const * account_get_auth_lockby(t_account * account);
		extern int account_set_auth_lock(t_account * account, int val);
		extern int account_set_auth_locktime(t_account * account, unsigned int val);
		extern int account_set_auth_lockreason(t_account * account, char const * val);
		extern int account_set_auth_lockby(t_account * account, char const * val);
		extern int account_get_auth_mute(t_account * account);
		extern unsigned int account_get_auth_mutetime(t_account * account);
		extern char const * account_get_auth_mutereason(t_account * account);
		extern char const * account_get_auth_muteby(t_account * account);
		extern int account_set_auth_mute(t_account * account, int val);
		extern int account_set_auth_mutetime(t_account * account, unsigned int val);
		extern int account_set_auth_mutereason(t_account * account, char const * val);
		extern int account_set_auth_muteby(t_account * account, char const * val);
		extern std::string account_get_locktext(t_account * account, bool with_author = true, bool for_mute = false);
		extern std::string account_get_mutetext(t_account * account, bool with_author = true);

		/* profile */
		extern char const * account_get_sex(t_account * account); /* the profile attributes are updated directly in bnetd.c */
		extern char const * account_get_age(t_account * account);
		extern char const * account_get_loc(t_account * account);
		extern char const * account_get_desc(t_account * account);

		/* last login */
		extern unsigned int account_get_ll_ctime(t_account * account);
		extern unsigned int account_get_ll_time(t_account * account);
		extern int account_set_ll_time(t_account * account, unsigned int t);
		extern char const * account_get_ll_user(t_account * account);
		extern int account_set_ll_user(t_account * account, char const * user);
		extern t_clienttag account_get_ll_clienttag(t_account * account);
		extern int account_set_ll_clienttag(t_account * account, t_clienttag clienttag);
		extern char const * account_get_ll_owner(t_account * account);
		extern int account_set_ll_owner(t_account * account, char const * owner);
		extern char const * account_get_ll_ip(t_account * account);
		extern int account_set_ll_ip(t_account * account, char const * ip);

		/* normal stats */
		extern unsigned int account_get_normal_wins(t_account * account, t_clienttag clienttag);
		extern int account_inc_normal_wins(t_account * account, t_clienttag clienttag);
		extern int account_set_normal_wins(t_account * account, t_clienttag clienttag, unsigned wins);
		extern unsigned int account_get_normal_losses(t_account * account, t_clienttag clienttag);
		extern int account_inc_normal_losses(t_account * account, t_clienttag clienttag);
		extern int account_set_normal_losses(t_account * account, t_clienttag clienttag, unsigned losses);
		extern unsigned int account_get_normal_draws(t_account * account, t_clienttag clienttag);
		extern int account_inc_normal_draws(t_account * account, t_clienttag clienttag);
		extern int account_set_normal_draws(t_account * account, t_clienttag clienttag, unsigned draws);
		extern unsigned int account_get_normal_disconnects(t_account * account, t_clienttag clienttag);
		extern int account_inc_normal_disconnects(t_account * account, t_clienttag clienttag);
		extern int account_set_normal_disconnects(t_account * account, t_clienttag clienttag, unsigned discs);
		extern int account_set_normal_last_time(t_account * account, t_clienttag clienttag, t_bnettime t);
		extern int account_set_normal_last_result(t_account * account, t_clienttag clienttag, char const * result);

		/* ladder stats (active) */
		extern unsigned int account_get_ladder_active_wins(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_active_wins(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int wins);
		extern unsigned int account_get_ladder_active_losses(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_active_losses(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int losses);
		extern unsigned int account_get_ladder_active_draws(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_active_draws(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int draws);
		extern unsigned int account_get_ladder_active_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_active_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int disconnects);
		extern unsigned int account_get_ladder_active_rating(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_active_rating(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int rating);
		extern int account_get_ladder_active_rank(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_active_rank(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int rank);
		extern char const * account_get_ladder_active_last_time(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_active_last_time(t_account * account, t_clienttag clienttag, t_ladder_id id, t_bnettime t);

		/* ladder stats (current) */
		extern unsigned int account_get_ladder_wins(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_inc_ladder_wins(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_wins(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned wins);
		extern unsigned int account_get_ladder_losses(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_inc_ladder_draws(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_draws(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned draws);
		extern unsigned int account_get_ladder_draws(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_inc_ladder_losses(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_losses(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned losses);
		extern unsigned int account_get_ladder_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_inc_ladder_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned discs);
		extern unsigned int account_get_ladder_rating(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_rating(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned rating);
		extern int account_adjust_ladder_rating(t_account * account, t_clienttag clienttag, t_ladder_id id, int delta);
		extern int account_get_ladder_rank(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_rank(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int rank);
		extern unsigned int account_get_ladder_high_rating(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern unsigned int account_get_ladder_high_rank(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_last_time(t_account * account, t_clienttag clienttag, t_ladder_id id, t_bnettime t);
		extern char const * account_get_ladder_last_time(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_last_result(t_account * account, t_clienttag clienttag, t_ladder_id id, char const * result);

		/* Diablo normal stats */
		extern unsigned int account_get_normal_level(t_account * account, t_clienttag clienttag);
		extern int account_set_normal_level(t_account * account, t_clienttag clienttag, unsigned int level);
		extern unsigned int account_get_normal_class(t_account * account, t_clienttag clienttag);
		extern int account_set_normal_class(t_account * account, t_clienttag clienttag, unsigned int chclass);
		extern unsigned int account_get_normal_diablo_kills(t_account * account, t_clienttag clienttag);
		extern int account_set_normal_diablo_kills(t_account * account, t_clienttag clienttag, unsigned int diablo_kills);
		extern unsigned int account_get_normal_strength(t_account * account, t_clienttag clienttag);
		extern int account_set_normal_strength(t_account * account, t_clienttag clienttag, unsigned int strength);
		extern unsigned int account_get_normal_magic(t_account * account, t_clienttag clienttag);
		extern int account_set_normal_magic(t_account * account, t_clienttag clienttag, unsigned int magic);
		extern unsigned int account_get_normal_dexterity(t_account * account, t_clienttag clienttag);
		extern int account_set_normal_dexterity(t_account * account, t_clienttag clienttag, unsigned int dexterity);
		extern unsigned int account_get_normal_vitality(t_account * account, t_clienttag clienttag);
		extern int account_set_normal_vitality(t_account * account, t_clienttag clienttag, unsigned int vitality);
		extern unsigned int account_get_normal_gold(t_account * account, t_clienttag clienttag);
		extern int account_set_normal_gold(t_account * account, t_clienttag clienttag, unsigned int gold);

		/* Diablo II closed characters */
		extern char const * account_get_closed_characterlist(t_account * account, t_clienttag clienttag, char const * realmname);
		extern int account_set_closed_characterlist(t_account * account, t_clienttag clienttag, char const * charlist);
		extern int account_add_closed_character(t_account * account, t_clienttag clienttag, t_character * ch);
		extern int account_check_closed_character(t_account * account, t_clienttag clienttag, char const * realmname, char const * charname);


		extern int account_set_friend(t_account * account, int friendnum, unsigned int frienduid);
		extern unsigned int account_get_friend(t_account * account, int friendnum);
		extern int account_get_friendcount(t_account * account);
		extern int account_add_friend(t_account * my_acc, t_account * facc);
		extern int account_remove_friend(t_account * account, int friendnum);
		extern int account_remove_friend2(t_account * account, const char * friendname);

		extern char const * race_get_str(unsigned int race);
		extern int account_set_admin(t_account * account);
		extern int account_set_demoteadmin(t_account * account);

		extern unsigned int account_get_command_groups(t_account * account);
		extern int account_set_command_groups(t_account * account, unsigned int groups);

		extern int account_set_w3pgrace(t_account * account, t_clienttag clienttag, unsigned int race);
		extern int account_get_w3pgrace(t_account * account, t_clienttag clienttag);

		extern void account_get_raceicon(t_account * account, char * raceicon, unsigned int * raceiconnumber, unsigned int * wins, t_clienttag clienttag);
		//Used to call the save stats for all opponents
		extern int account_set_saveladderstats(t_account *a, unsigned int gametype, t_game_result result, unsigned int opponlevel, t_clienttag clienttag);

		extern int account_inc_racewins(t_account * account, unsigned int intrace, t_clienttag clienttag);
		extern int account_get_racewins(t_account * account, unsigned int intrace, t_clienttag clienttag);
		extern int account_inc_racelosses(t_account * account, unsigned int intrace, t_clienttag clienttag);
		extern int account_get_racelosses(t_account * account, unsigned int intrace, t_clienttag clienttag);

		extern int account_update_xp(t_account * account, t_clienttag clienttag, t_game_result gameresult, unsigned int opponlevel, int * xp_diff, t_ladder_id id);
		extern int account_get_ladder_xp(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_xp(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int xp);
		extern int account_get_ladder_level(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_level(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int level);
		extern int account_adjust_ladder_level(t_account * account, t_clienttag clienttag, t_ladder_id id);

		extern int account_set_currentatteam(t_account * account, unsigned int teamcount);
		extern int account_get_currentatteam(t_account * account);

		extern int account_get_highestladderlevel(t_account * account, t_clienttag clienttag); //func will compare levels for AT, Solo/Team Ladder and out the higest level of the 3

		// Determines the length of XP bar in profiles screen
		extern int account_get_profile_calcs(t_account * account, int xp, unsigned int level);
		extern unsigned int account_get_icon_profile(t_account * account, t_clienttag clienttag);

		extern int account_set_user_iconstash(t_account * account, t_clienttag clienttag, char const * value);
		extern char const * account_get_user_iconstash(t_account * account, t_clienttag clienttag);
		extern int account_set_user_icon(t_account * account, t_clienttag clienttag, char const * usericon);
		extern char const * account_get_user_icon(t_account * account, t_clienttag clienttag);
		extern unsigned int account_icon_to_profile_icon(char const * icon, t_account * account, t_clienttag ctag);
		extern char const * account_icon_default(t_account * account, t_clienttag clienttag);

		extern int account_is_operator_or_admin(t_account * account, char const * channel);

		extern int account_set_email(t_account * account, char const * email);
		extern char const * account_get_email(t_account * account);

		extern int account_set_userlang(t_account * account, const char * lang);
		extern char const * account_get_userlang(t_account * account);

		/*  Westwood Online Extensions */
		extern char const * account_get_wol_apgar(t_account * account);
		extern int account_set_wol_apgar(t_account * account, char const * apgar);
		extern int account_get_locale(t_account * account);
		extern int account_set_locale(t_account * account, int locale);
		extern int account_get_ladder_points(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int account_set_ladder_points(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int points);
	}

}

#endif
#endif
