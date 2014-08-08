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
#include "common/setup_before.h"
#include "tournament.h"

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <cstdlib>

#include "common/list.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/util.h"
#include "common/tag.h"
/*
#include "common/packet.h"

*/
#include "anongame_maplists.h"
#include "account.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static t_tournament_info * tournament_info = NULL;
		static t_list * tournament_head = NULL;

		static int tournamentlist_create(void);
		static int gamelist_destroy(void);
		static t_tournament_user * tournament_get_user(t_account * account);
		//static int tournament_get_in_game_status(t_account * account);

		/*****/
		static int tournamentlist_create(void)
		{
			tournament_head = list_create();
			return 0;
		}

		static int gamelist_destroy(void)
		{
			t_elem *		curr;
			t_tournament_user * user;

			if (tournament_head) {
				LIST_TRAVERSE(tournament_head, curr)
				{
					if (!(user = (t_tournament_user*)elem_get_data(curr))) {
						eventlog(eventlog_level_error, __FUNCTION__, "tournament list contains NULL item");
						continue;
					}

					if (list_remove_elem(tournament_head, &curr) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "could not remove item from list");

					if (user->name)
						xfree((void *)user->name); /* avoid warning */

					xfree(user);

				}

				if (list_destroy(tournament_head) < 0)
					return -1;
				tournament_head = NULL;
			}
			return 0;
		}

		/*****/
		extern int tournament_check_client(t_clienttag clienttag)
		{
			if (clienttag == CLIENTTAG_WAR3XP_UINT && tournament_info->game_client == 2)
				return 1;
			if (clienttag == CLIENTTAG_WARCRAFT3_UINT && tournament_info->game_client == 1)
				return 1;

			return -1;
		}

		extern int tournament_signup_user(t_account * account)
		{
			t_tournament_user * user;

			if (!(account))
				return -1;

			if ((user = tournament_get_user(account))) {
				eventlog(eventlog_level_info, __FUNCTION__, "user \"%s\" already signed up in tournament", account_get_name(account));
				return 0;
			}

			user = (t_tournament_user*)xmalloc(sizeof(t_tournament_user));
			user->name = xstrdup(account_get_name(account));
			user->wins = 0;
			user->losses = 0;
			user->ties = 0;
			user->in_game = 0;
			user->in_finals = 0;

			list_prepend_data(tournament_head, user);

			eventlog(eventlog_level_info, __FUNCTION__, "added user \"%s\" to tournament", account_get_name(account));
			return 0;
		}

		static t_tournament_user * tournament_get_user(t_account * account)
		{
			t_elem const * curr;
			t_tournament_user * user;

			if (tournament_head)
				LIST_TRAVERSE(tournament_head, curr)
			{
					user = (t_tournament_user*)elem_get_data(curr);
					if (std::strcmp(user->name, account_get_name(account)) == 0)
						return user;
				}

			return NULL;
		}

		extern int tournament_user_signed_up(t_account * account)
		{
			if (!(tournament_get_user(account)))
				return -1;

			return 0;
		}


		extern int tournament_add_stat(t_account * account, int stat)
		{
			t_tournament_user * user;

			if (!(user = tournament_get_user(account)))
				return -1;

			if (stat == 1)
				user->wins++;
			if (stat == 2)
				user->losses++;
			if (stat == 3)
				user->ties++;

			return 0;
		}

		extern int tournament_get_stat(t_account * account, int stat)
		{
			t_tournament_user * user;

			if (!(user = tournament_get_user(account)))
				return 0;

			if (stat == 1)
				return user->wins;
			if (stat == 2)
				return user->losses;
			if (stat == 3)
				return user->ties;

			return 0;
		}

		extern int tournament_get_player_score(t_account * account)
		{
			t_tournament_user * user;
			int score;

			if (!(user = tournament_get_user(account)))
				return 0;

			score = user->wins * 3 + user->ties - user->losses;

			if (score < 0)
				return 0;

			return score;
		}

		extern int tournament_set_in_game_status(t_account * account, int status)
		{
			t_tournament_user * user;

			if (!(user = tournament_get_user(account)))
				return -1;

			user->in_game = status;

			return 0;
		}
		/*
		static int tournament_get_in_game_status(t_account * account)
		{
		t_tournament_user * user;

		if (!(user = tournament_get_user(account)))
		return 0;

		return user->in_game;
		}
		*/
		extern int tournament_get_in_finals_status(t_account * account)
		{
			t_tournament_user * user;

			if (!(user = tournament_get_user(account)))
				return 0;

			return user->in_finals;
		}

		extern int tournament_get_game_in_progress(void)
		{
			t_elem const * curr;
			t_tournament_user * user;

			if (tournament_head)
				LIST_TRAVERSE_CONST(tournament_head, curr)
			{
					user = (t_tournament_user*)elem_get_data(curr);
					if (user->in_game == 1)
						return 1;
				}

			return 0;
		}

		extern int tournament_is_arranged(void)
		{
			if (tournament_info->game_selection == 2)
				return 1;
			else
				return 0;
		}

		extern int tournament_get_totalplayers(void)
		{
			return tournament_info->game_type * 2;
		}

		void tournament_check_date(unsigned int *mon, unsigned int *mday, unsigned int *year, unsigned int *hour, unsigned int *min, unsigned int *sec, char const * caller)
		{
			if (*mon > 12)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid month (%u) in %s", *mon, caller);
				*mon = 12;
			}
			if (*mday > 31)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid mday (%u) in %s", *mday, caller);
				*mday = 31;
			}
			if (*hour > 23)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid hour (%u) from %s", *hour, caller);
				*hour = 23;
			}
			if (*min > 59)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid min (%u) from %s", *min, caller);
				*min = 59;
			}
			if (*sec > 59)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid sec (%u) from %s", *sec, caller);
				*sec = 59;
			}
			return;
		}

		/*****/
		extern int tournament_init(char const * filename)
		{
			std::FILE * fp;
			unsigned int line, pos, mon, day, year, hour, min, sec;
			char *buff, *temp, *pointer, *value;
			char format[30];
			char *variable;
			char *sponsor = NULL;
			char *have_sponsor = NULL;
			char *have_icon = NULL;
			struct std::tm * timestamp = (struct std::tm*)xmalloc(sizeof(struct std::tm));

			std::sprintf(format, "%%02u/%%02u/%%04u %%02u:%%02u:%%02u");

			tournament_info = (t_tournament_info*)xmalloc(sizeof(t_tournament_info));
			tournament_info->start_preliminary = 0;
			tournament_info->end_signup = 0;
			tournament_info->end_preliminary = 0;
			tournament_info->start_round_1 = 0;
			tournament_info->start_round_2 = 0;
			tournament_info->start_round_3 = 0;
			tournament_info->start_round_4 = 0;
			tournament_info->tournament_end = 0;
			tournament_info->game_selection = 1; /* Default to PG */
			tournament_info->game_type = 1; /* Default to 1v1 */
			tournament_info->game_client = 2; /* Default to FT */
			tournament_info->races = 0x3F; /* Default to all races */
			tournament_info->format = xstrdup("");
			tournament_info->sponsor = xstrdup("");
			tournament_info->thumbs_down = 0;

			anongame_tournament_maplists_destroy();

			if (!filename) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
				xfree((void *)timestamp);
				return -1;
			}

			if (!(fp = std::fopen(filename, "r"))) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for reading (std::fopen: %s)", filename, std::strerror(errno));
				xfree((void *)timestamp);
				return -1;
			}

			for (line = 1; (buff = file_get_line(fp)); line++) {
				for (pos = 0; buff[pos] == '\t' || buff[pos] == ' '; pos++);
				if (buff[pos] == '\0' || buff[pos] == '#') {
					continue;
				}
				if ((temp = std::strrchr(buff, '#'))) {
					unsigned int len;
					unsigned int endpos;

					*temp = '\0';
					len = std::strlen(buff) + 1;
					for (endpos = len - 1; buff[endpos] == '\t' || buff[endpos] == ' '; endpos--);
					buff[endpos + 1] = '\0';
				}

				if (std::strcmp(buff, "[MAPS]") == 0) {
					char *clienttag, *mapname, *mname;
					t_clienttag ctag;

					for (; (buff = file_get_line(fp));) {
						for (pos = 0; buff[pos] == '\t' || buff[pos] == ' '; pos++);
						if (buff[pos] == '\0' || buff[pos] == '#') {
							continue;
						}
						if ((temp = std::strrchr(buff, '#'))) {
							unsigned int len;
							unsigned int endpos;

							*temp = '\0';
							len = std::strlen(buff) + 1;
							for (endpos = len - 1; buff[endpos] == '\t' || buff[endpos] == ' '; endpos--);
							buff[endpos + 1] = '\0';
						}
						/* FIXME: use next_token() */
						if (!(clienttag = std::strtok(buff, " \t"))) { /* std::strtok modifies the string it is passed */
							continue;
						}
						if (std::strlen(clienttag) != 4) {
							continue;
						}
						if (std::strcmp(buff, "[ENDMAPS]") == 0) {
							break;
						}
						if (!(mapname = std::strtok(NULL, " \t"))) {
							continue;
						}
						if (!tag_check_client((ctag = tag_case_str_to_uint(clienttag)))) {
							continue;
						}
						mname = xstrdup(mapname);

						anongame_add_tournament_map(ctag, mname);
						eventlog(eventlog_level_trace, __FUNCTION__, "added tournament map \"%s\" for %s", mname, clienttag);
						xfree(mname);
					}
				}
				else {
					variable = buff;
					pointer = std::strchr(variable, '=');
					for (pointer--; pointer[0] == ' ' || pointer[0] == '\t'; pointer--);
					pointer[1] = '\0';
					pointer++;
					pointer++;
					pointer = std::strchr(pointer, '=');
					pointer++;

					if (std::strcmp(variable, "start_preliminary") == 0) {
						pointer = std::strchr(pointer, '\"');
						pointer++;
						value = pointer;
						pointer = std::strchr(pointer, '\"');
						pointer[0] = '\0';

						std::sscanf(value, format, &mon, &day, &year, &hour, &min, &sec);

						tournament_check_date(&mon, &day, &year, &hour, &min, &sec, variable);

						timestamp->tm_mon = mon - 1;
						timestamp->tm_mday = day;
						timestamp->tm_year = year - 1900;
						timestamp->tm_hour = hour;
						timestamp->tm_min = min;
						timestamp->tm_sec = sec;
						timestamp->tm_isdst = -1;

						tournament_info->start_preliminary = std::mktime(timestamp);
					}
					else if (std::strcmp(variable, "end_signup") == 0) {
						pointer = std::strchr(pointer, '\"');
						pointer++;
						value = pointer;
						pointer = std::strchr(pointer, '\"');
						pointer[0] = '\0';

						std::sscanf(value, format, &mon, &day, &year, &hour, &min, &sec);

						tournament_check_date(&mon, &day, &year, &hour, &min, &sec, variable);

						timestamp->tm_mon = mon - 1;
						timestamp->tm_mday = day;
						timestamp->tm_year = year - 1900;
						timestamp->tm_hour = hour;
						timestamp->tm_min = min;
						timestamp->tm_sec = sec;
						timestamp->tm_isdst = -1;

						tournament_info->end_signup = std::mktime(timestamp);
					}
					else if (std::strcmp(variable, "end_preliminary") == 0) {
						pointer = std::strchr(pointer, '\"');
						pointer++;
						value = pointer;
						pointer = std::strchr(pointer, '\"');
						pointer[0] = '\0';

						std::sscanf(value, format, &mon, &day, &year, &hour, &min, &sec);

						tournament_check_date(&mon, &day, &year, &hour, &min, &sec, variable);

						timestamp->tm_mon = mon - 1;
						timestamp->tm_mday = day;
						timestamp->tm_year = year - 1900;
						timestamp->tm_hour = hour;
						timestamp->tm_min = min;
						timestamp->tm_sec = sec;
						timestamp->tm_isdst = -1;

						tournament_info->end_preliminary = std::mktime(timestamp);
					}
					else if (std::strcmp(variable, "start_round_1") == 0) {
						pointer = std::strchr(pointer, '\"');
						pointer++;
						value = pointer;
						pointer = std::strchr(pointer, '\"');
						pointer[0] = '\0';

						std::sscanf(value, format, &mon, &day, &year, &hour, &min, &sec);

						tournament_check_date(&mon, &day, &year, &hour, &min, &sec, variable);

						timestamp->tm_mon = mon - 1;
						timestamp->tm_mday = day;
						timestamp->tm_year = year - 1900;
						timestamp->tm_hour = hour;
						timestamp->tm_min = min;
						timestamp->tm_sec = sec;
						timestamp->tm_isdst = -1;

						tournament_info->start_round_1 = std::mktime(timestamp);
					}
					else if (std::strcmp(variable, "start_round_2") == 0) {
						pointer = std::strchr(pointer, '\"');
						pointer++;
						value = pointer;
						pointer = std::strchr(pointer, '\"');
						pointer[0] = '\0';

						std::sscanf(value, format, &mon, &day, &year, &hour, &min, &sec);

						tournament_check_date(&mon, &day, &year, &hour, &min, &sec, variable);

						timestamp->tm_mon = mon - 1;
						timestamp->tm_mday = day;
						timestamp->tm_year = year - 1900;
						timestamp->tm_hour = hour;
						timestamp->tm_min = min;
						timestamp->tm_sec = sec;
						timestamp->tm_isdst = -1;

						tournament_info->start_round_2 = std::mktime(timestamp);
					}
					else if (std::strcmp(variable, "start_round_3") == 0) {
						pointer = std::strchr(pointer, '\"');
						pointer++;
						value = pointer;
						pointer = std::strchr(pointer, '\"');
						pointer[0] = '\0';

						std::sscanf(value, format, &mon, &day, &year, &hour, &min, &sec);

						tournament_check_date(&mon, &day, &year, &hour, &min, &sec, variable);

						timestamp->tm_mon = mon - 1;
						timestamp->tm_mday = day;
						timestamp->tm_year = year - 1900;
						timestamp->tm_hour = hour;
						timestamp->tm_min = min;
						timestamp->tm_sec = sec;
						timestamp->tm_isdst = -1;

						tournament_info->start_round_3 = std::mktime(timestamp);
					}
					else if (std::strcmp(variable, "start_round_4") == 0) {
						pointer = std::strchr(pointer, '\"');
						pointer++;
						value = pointer;
						pointer = std::strchr(pointer, '\"');
						pointer[0] = '\0';

						std::sscanf(value, format, &mon, &day, &year, &hour, &min, &sec);

						tournament_check_date(&mon, &day, &year, &hour, &min, &sec, variable);

						timestamp->tm_mon = mon - 1;
						timestamp->tm_mday = day;
						timestamp->tm_year = year - 1900;
						timestamp->tm_hour = hour;
						timestamp->tm_min = min;
						timestamp->tm_sec = sec;
						timestamp->tm_isdst = -1;

						tournament_info->start_round_4 = std::mktime(timestamp);
					}
					else if (std::strcmp(variable, "tournament_end") == 0) {
						pointer = std::strchr(pointer, '\"');
						pointer++;
						value = pointer;
						pointer = std::strchr(pointer, '\"');
						pointer[0] = '\0';

						std::sscanf(value, format, &mon, &day, &year, &hour, &min, &sec);

						tournament_check_date(&mon, &day, &year, &hour, &min, &sec, variable);

						timestamp->tm_mon = mon - 1;
						timestamp->tm_mday = day;
						timestamp->tm_year = year - 1900;
						timestamp->tm_hour = hour;
						timestamp->tm_min = min;
						timestamp->tm_sec = sec;
						timestamp->tm_isdst = -1;

						tournament_info->tournament_end = std::mktime(timestamp);
					}
					else if (std::strcmp(variable, "game_selection") == 0) {
						if (std::atoi(pointer) >= 1 && std::atoi(pointer) <= 2)
							tournament_info->game_selection = std::atoi(pointer);
					}
					else if (std::strcmp(variable, "game_type") == 0) {
						if (std::atoi(pointer) >= 1 && std::atoi(pointer) <= 4)
							tournament_info->game_type = std::atoi(pointer);
					}
					else if (std::strcmp(variable, "game_client") == 0) {
						if (std::atoi(pointer) >= 1 && std::atoi(pointer) <= 2)
							tournament_info->game_client = std::atoi(pointer);
					}
					else if (std::strcmp(variable, "format") == 0) {
						pointer = std::strchr(pointer, '\"');
						pointer++;
						value = pointer;
						pointer = std::strchr(pointer, '\"');
						pointer[0] = '\0';

						if (tournament_info->format) xfree((void *)tournament_info->format);
						tournament_info->format = xstrdup(value);
					}
					else if (std::strcmp(variable, "races") == 0) {
						unsigned int intvalue = 0;
						unsigned int i;

						pointer = std::strchr(pointer, '\"');
						pointer++;
						value = pointer;
						pointer = std::strchr(pointer, '\"');
						pointer[0] = '\0';

						for (i = 0; i < std::strlen(value); i++) {
							if (value[i] == 'H') intvalue = intvalue | 0x01;
							if (value[i] == 'O') intvalue = intvalue | 0x02;
							if (value[i] == 'N') intvalue = intvalue | 0x04;
							if (value[i] == 'U') intvalue = intvalue | 0x08;
							if (value[i] == 'R') intvalue = intvalue | 0x20;
						}

						if (intvalue == 0 || intvalue == 0x2F)
							intvalue = 0x3F; /* hack to make all races availiable */

						tournament_info->races = intvalue;
					}
					else if (std::strcmp(variable, "sponsor") == 0) {
						pointer = std::strchr(pointer, '\"');
						pointer++;
						value = pointer;
						pointer = std::strchr(pointer, '\"');
						pointer[0] = '\0';

						have_sponsor = xstrdup(value);
					}
					else if (std::strcmp(variable, "icon") == 0) {
						pointer = std::strchr(pointer, '\"');
						pointer++;
						value = pointer;
						pointer = std::strchr(pointer, '\"');
						pointer[0] = '\0';

						have_icon = xstrdup(value);
					}
					else if (std::strcmp(variable, "thumbs_down") == 0) {
						tournament_info->thumbs_down = std::atoi(pointer);
					}
					else
						eventlog(eventlog_level_error, __FUNCTION__, "bad option \"%s\" in \"%s\"", variable, filename);

					if (have_sponsor && have_icon) {
						sponsor = (char*)xmalloc(std::strlen(have_sponsor) + 6);

						if (std::strlen(have_icon) == 4)
							std::sprintf(sponsor, "%c%c%c%c,%s", have_icon[3], have_icon[2], have_icon[1], have_icon[0], have_sponsor);
						else if (std::strlen(have_icon) == 2)
							std::sprintf(sponsor, "%c%c3W,%s", have_icon[1], have_icon[0], have_sponsor);
						else {
							std::sprintf(sponsor, "PX3W,%s", have_sponsor); /* default to standard FT icon */
							eventlog(eventlog_level_warn, __FUNCTION__, "bad icon length, using W3XP");
						}

						if (tournament_info->sponsor)
							xfree((void *)tournament_info->sponsor);

						tournament_info->sponsor = xstrdup(sponsor);
						xfree((void *)have_sponsor);
						xfree((void *)have_icon);
						xfree((void *)sponsor);
						have_sponsor = NULL;
						have_icon = NULL;
					}
				}
			}
			if (have_sponsor) xfree((void *)have_sponsor);
			if (have_icon) xfree((void *)have_icon);
			xfree((void *)timestamp);
			file_get_line(NULL); // clear file_get_line buffer
			std::fclose(fp);

			/* check if we have timestamps for all the times */
			/* if not disable tournament by setting "start_preliminary" to 0 */
			if (tournament_info->end_signup == 0 || tournament_info->end_preliminary == 0 ||
				tournament_info->start_round_1 == 0 || tournament_info->start_round_2 == 0 ||
				tournament_info->start_round_3 == 0 || tournament_info->start_round_4 == 0 ||
				tournament_info->tournament_end == 0) {
				tournament_info->start_preliminary = 0;
				eventlog(eventlog_level_warn, __FUNCTION__, "one or more timestamps for tournaments is not valid, tournament has been disabled");
			}
			else {
				tournamentlist_create();
			}

			return 0;
		}

		extern int tournament_destroy(void)
		{
			if (tournament_info->format) xfree((void *)tournament_info->format);
			if (tournament_info->sponsor) xfree((void *)tournament_info->sponsor);
			if (tournament_info) xfree((void *)tournament_info);
			tournament_info = NULL;
			gamelist_destroy();
			return 0;
		}

		extern int tournament_reload(char const * filename)
		{
			std::time_t tm;
			std::time(&tm);
			if ((tm >= tournament_info->start_preliminary) && (tm <= tournament_info->tournament_end))
			{
				eventlog(eventlog_level_info, __FUNCTION__, "unable to reload tournament, tournament is in process");
				return -1;
			}
			tournament_destroy();
			return tournament_init(filename);
		}
		/*****/
		extern unsigned int tournament_get_start_preliminary(void)
		{
			return tournament_info->start_preliminary;
		}

		extern unsigned int tournament_get_end_signup(void)
		{
			return tournament_info->end_signup;
		}

		extern unsigned int tournament_get_end_preliminary(void)
		{
			return tournament_info->end_preliminary;
		}

		extern unsigned int tournament_get_start_round_1(void)
		{
			return tournament_info->start_round_1;
		}

		extern unsigned int tournament_get_start_round_2(void)
		{
			return tournament_info->start_round_2;
		}

		extern unsigned int tournament_get_start_round_3(void)
		{
			return tournament_info->start_round_3;
		}

		extern unsigned int tournament_get_start_round_4(void)
		{
			return tournament_info->start_round_4;
		}

		extern unsigned int tournament_get_tournament_end(void)
		{
			return tournament_info->tournament_end;
		}

		extern unsigned int tournament_get_game_selection(void)
		{
			return tournament_info->game_selection;
		}

		extern unsigned int tournament_get_game_type(void)
		{
			return tournament_info->game_type;
		}

		extern unsigned int tournament_get_races(void)
		{
			return tournament_info->races;
		}

		extern char * tournament_get_format(void)
		{
			return tournament_info->format;
		}

		extern char * tournament_get_sponsor(void)
		{
			return tournament_info->sponsor;
		}

		extern unsigned int tournament_get_thumbs_down(void)
		{
			return tournament_info->thumbs_down;
		}

	}

}

/****/
