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
#define CHARACTER_INTERNAL_ACCESS
#include "common/setup_before.h"
#include "account_wrap.h"

#include <cstring>

#include "compat/snprintf.h"
#include "compat/strcasecmp.h"
#include "compat/strncasecmp.h"

#include "common/bnet_protocol.h"
#include "common/bnettime.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "common/tag.h"
#include "common/util.h"

#include "account.h"
#include "anongame_infos.h"
#include "character.h"
#include "clan.h"
#include "command.h"
#include "connection.h"
#include "friends.h"
#include "i18n.h"
#include "ladder.h"
#include "prefs.h"
#include "server.h"
#include "team.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static unsigned int char_icon_to_uint(const char * icon);

		extern unsigned int account_get_numattr_real(t_account * account, char const * key, char const * fn, unsigned int ln)
		{
			char const * temp;
			unsigned int val;

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account (from %s:%u)", fn, ln);
				return 0;
			}
			if (!key)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key (from %s:%u)", fn, ln);
				return 0;
			}

			if (!(temp = account_get_strattr(account, key)))
				return 0;

			if (str_to_uint(temp, &val) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "not a numeric string \"%s\" for key \"%s\"", temp, key);
				return 0;
			}

			return val;
		}


		extern int account_set_numattr(t_account * account, char const * key, unsigned int val)
		{
			char temp[32]; /* should be more than enough room */

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}
			if (!key)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key");
				return -1;
			}

			snprintf(temp, sizeof(temp), "%u", val);
			return account_set_strattr(account, key, temp);
		}


		extern int account_get_boolattr_real(t_account * account, char const * key, char const * fn, unsigned int ln)
		{
			char const * temp;

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account (from %s:%u)", fn, ln);
				return -1;
			}
			if (!key)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key (from %s:%u)", fn, ln);
				return -1;
			}

			if (!(temp = account_get_strattr(account, key)))
				return -1;

			switch (str_get_bool(temp))
			{
			case 1:
				return 1;
			case 0:
				return 0;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "bad boolean value \"%s\" for key \"%s\"", temp, key);
				return -1;
			}
		}


		extern int account_set_boolattr(t_account * account, char const * key, int val)
		{
			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}
			if (!key)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key");
				return -1;
			}

			return account_set_strattr(account, key, val ? "true" : "false");
		}

		extern char const * account_get_rawattr_real(t_account * account, char const * key, char const * fn, unsigned int ln)
		{
			char const * temp;
			char * result;
			int length;

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account (from %s:%u)", fn, ln);
				return NULL;
			}
			if (!key)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key (from %s:%u)", fn, ln);
				return NULL;
			}

			if (!(temp = account_get_strattr(account, key)))
				return NULL;

			length = std::strlen(temp) / 3;

			if (!(result = (char *)xmalloc(length)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "failed to create result");
				return NULL;
			}
			hex_to_str(temp, result, length);

			return result;
		}

		extern int account_set_rawattr(t_account * account, char const * key, char const * val, int length)
		{
			char * temp_buffer;
			int result;

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}
			if (!key)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key");
				return -1;
			}

			if (!(temp_buffer = (char *)xmalloc(length * 3 + 1)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "failed to create temp_buffer");
				return -1;
			}

			str_to_hex(temp_buffer, val, length);
			result = account_set_strattr(account, key, temp_buffer);
			xfree((void *)temp_buffer);
			return result;
		}


		/****************************************************************/


		extern char const * account_get_pass(t_account * account)
		{
			return account_get_strattr(account, "BNET\\acct\\passhash1");
		}

		extern int account_set_pass(t_account * account, char const * passhash1)
		{
			return account_set_strattr(account, "BNET\\acct\\passhash1", passhash1);
		}

		extern char const * account_get_salt(t_account * account)
		{
			return account_get_rawattr(account, "BNET\\acct\\salt");
		}

		extern int account_set_salt(t_account * account, char const * salt)
		{
			return account_set_rawattr(account, "BNET\\acct\\salt", salt, 32);
		}

		extern char const * account_get_verifier(t_account * account)
		{
			return account_get_rawattr(account, "BNET\\acct\\verifier");
		}

		extern int account_set_verifier(t_account * account, char const * verifier)
		{
			return account_set_rawattr(account, "BNET\\acct\\verifier", verifier, 32);
		}


		/****************************************************************/


		extern int account_get_auth_admin(t_account * account, char const * channelname)
		{
			char temp[256];

			if (!channelname)
				return account_get_boolattr(account, "BNET\\auth\\admin");

			snprintf(temp, sizeof(temp), "BNET\\auth\\admin\\%.100s", channelname);
			return account_get_boolattr(account, temp);
		}


		extern int account_set_auth_admin(t_account * account, char const * channelname, int val)
		{
			char temp[256];

			if (!channelname)
				return account_set_boolattr(account, "BNET\\auth\\admin", val);

			snprintf(temp, sizeof(temp), "BNET\\auth\\admin\\%.100s", channelname);
			return account_set_boolattr(account, temp, val);
		}


		extern int account_get_auth_announce(t_account * account)
		{
			return account_get_boolattr(account, "BNET\\auth\\announce");
		}


		extern int account_get_auth_botlogin(t_account * account)
		{
			return account_get_boolattr(account, "BNET\\auth\\botlogin");
		}


		extern int account_get_auth_bnetlogin(t_account * account)
		{
			return account_get_boolattr(account, "BNET\\auth\\normallogin");
		}


		extern int account_get_auth_operator(t_account * account, char const * channelname)
		{
			char temp[256];

			if (!channelname)
				return account_get_boolattr(account, "BNET\\auth\\operator");

			snprintf(temp, sizeof(temp), "BNET\\auth\\operator\\%.100s", channelname);
			return account_get_boolattr(account, temp);
		}

		extern int account_set_auth_operator(t_account * account, char const * channelname, int val)
		{
			char temp[256];

			if (!channelname)
				return account_set_boolattr(account, "BNET\\auth\\operator", val);

			snprintf(temp, sizeof(temp), "BNET\\auth\\operator\\%.100s", channelname);
			return account_set_boolattr(account, temp, val);
		}

		extern int account_get_auth_voice(t_account * account, char const * channelname)
		{
			char temp[256];

			snprintf(temp, sizeof(temp), "BNET\\auth\\voice\\%.100s", channelname);
			return account_get_boolattr(account, temp);
		}

		extern int account_set_auth_voice(t_account * account, char const * channelname, int val)
		{
			char temp[256];

			snprintf(temp, sizeof(temp), "BNET\\auth\\voice\\%.100s", channelname);
			return account_set_boolattr(account, temp, val);
		}

		extern int account_get_auth_changepass(t_account * account)
		{
			return account_get_boolattr(account, "BNET\\auth\\changepass");
		}


		extern int account_get_auth_changeprofile(t_account * account)
		{
			return account_get_boolattr(account, "BNET\\auth\\changeprofile");
		}


		extern int account_get_auth_createnormalgame(t_account * account)
		{
			return account_get_boolattr(account, "BNET\\auth\\createnormalgame");
		}


		extern int account_get_auth_joinnormalgame(t_account * account)
		{
			return account_get_boolattr(account, "BNET\\auth\\joinnormalgame");
		}


		extern int account_get_auth_createladdergame(t_account * account)
		{
			return account_get_boolattr(account, "BNET\\auth\\createladdergame");
		}


		extern int account_get_auth_joinladdergame(t_account * account)
		{
			return account_get_boolattr(account, "BNET\\auth\\joinladdergame");
		}


		extern int account_get_auth_lock(t_account * account)
		{

#ifdef WIN32
			// do not allow login with illegal Windows filenames
			// 1) account will not created on plain file storage
			// 2) it may cause a crash when writing a userlog commands into a file with that name
			for (int i = 0; i < (sizeof(ILLEGALFILENAMES) / sizeof(*ILLEGALFILENAMES)); i++)
			{
				if (strcasecmp(account_get_name(account), ILLEGALFILENAMES[i]) == 0)
				{
					account_set_auth_lockreason(account, "user name is illegal");
					eventlog(eventlog_level_debug, __FUNCTION__, "user name is invalid (reserved file name on Windows)");
					return true;
				}
			}
#endif

			// check for unlock
			if (unsigned int locktime = account_get_auth_locktime(account))
			{
				if ((locktime - std::time(NULL)) < 0)
				{
					account_set_auth_lock(account, 0);
					account_set_auth_locktime(account, 0);
					account_set_auth_lockreason(account, "");
					account_set_auth_lockby(account, "");
				}
			}
			return account_get_boolattr(account, "BNET\\auth\\lock");
		}
		extern unsigned int account_get_auth_locktime(t_account * account)
		{
			return account_get_numattr(account, "BNET\\auth\\locktime");
		}
		extern char const * account_get_auth_lockreason(t_account * account)
		{
			return account_get_strattr(account, "BNET\\auth\\lockreason");
		}
		extern char const * account_get_auth_lockby(t_account * account)
		{
			return account_get_strattr(account, "BNET\\auth\\lockby");
		}

		extern int account_set_auth_lock(t_account * account, int val)
		{
			return account_set_boolattr(account, "BNET\\auth\\lock", val);
		}
		extern int account_set_auth_locktime(t_account * account, unsigned int val)
		{
			return account_set_numattr(account, "BNET\\auth\\locktime", val);
		}
		extern int account_set_auth_lockreason(t_account * account, char const * val)
		{
			return account_set_strattr(account, "BNET\\auth\\lockreason", val);
		}
		extern int account_set_auth_lockby(t_account * account, char const * val)
		{
			return account_set_strattr(account, "BNET\\auth\\lockby", val);
		}


		extern int account_get_auth_mute(t_account * account)
		{
			// check for unmute
			if (unsigned int locktime = account_get_auth_mutetime(account))
			{
				if ((locktime - std::time(NULL)) < 0)
				{
					account_set_auth_mute(account, 0);
					account_set_auth_mutetime(account, 0);
					account_set_auth_mutereason(account, "");
					account_set_auth_muteby(account, "");
				}
			}
			return account_get_boolattr(account, "BNET\\auth\\mute");
		}
		extern unsigned int account_get_auth_mutetime(t_account * account)
		{
			return account_get_numattr(account, "BNET\\auth\\mutetime");
		}
		extern char const * account_get_auth_mutereason(t_account * account)
		{
			return account_get_strattr(account, "BNET\\auth\\mutereason");
		}
		extern char const * account_get_auth_muteby(t_account * account)
		{
			return account_get_strattr(account, "BNET\\auth\\muteby");
		}

		extern int account_set_auth_mute(t_account * account, int val)
		{
			return account_set_boolattr(account, "BNET\\auth\\mute", val);
		}
		extern int account_set_auth_mutetime(t_account * account, unsigned int val)
		{
			return account_set_numattr(account, "BNET\\auth\\mutetime", val);
		}
		extern int account_set_auth_mutereason(t_account * account, char const * val)
		{
			return account_set_strattr(account, "BNET\\auth\\mutereason", val);
		}
		extern int account_set_auth_muteby(t_account * account, char const * val)
		{
			return account_set_strattr(account, "BNET\\auth\\muteby", val);
		}


		/* Return text with account lock */
		extern std::string account_get_locktext(t_connection * c, t_account * account, bool with_author, bool for_mute)
		{
			std::string msgtemp;

			// append author of ban
			if (with_author)
			{
				if (char const * author = (for_mute) ? account_get_auth_muteby(account) : account_get_auth_lockby(account))
				if (author && author[0] != '\0')
				{
					msgtemp += localize(c, " by {}", author);
				}
			}

			// append remaining time
			if (unsigned int locktime = (for_mute) ? account_get_auth_mutetime(account) : account_get_auth_locktime(account))
				msgtemp += localize(c, " for {}", seconds_to_timestr(locktime - now));
			else
				msgtemp += localize(c, " permanently");

			// append reason
			char const * reason = (for_mute) ? account_get_auth_mutereason(account) : account_get_auth_lockreason(account);
			if (reason && reason[0] != '\0')
			{
				msgtemp += localize(c, " with a reason \"{}\"", reason);
			}
			return msgtemp;
		}
		extern std::string account_get_mutetext(t_connection * c, t_account * account, bool with_author)
		{
			return account_get_locktext(c, account, with_author, true);
		}
		/****************************************************************/


		extern char const * account_get_sex(t_account * account)
		{
			char const * temp;

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return NULL;
			}

			if (!(temp = account_get_strattr(account, "profile\\sex")))
				return "";
			return temp;
		}


		extern char const * account_get_age(t_account * account)
		{
			char const * temp;

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return NULL;
			}

			if (!(temp = account_get_strattr(account, "profile\\age")))
				return "";
			return temp;
		}


		extern char const * account_get_loc(t_account * account)
		{
			char const * temp;

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return NULL;
			}

			if (!(temp = account_get_strattr(account, "profile\\location")))
				return "";
			return temp;
		}


		extern char const * account_get_desc(t_account * account)
		{
			char const * temp;

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return NULL;
			}

			if (!(temp = account_get_strattr(account, "profile\\description")))
				return "";
			return temp;
		}


		/****************************************************************/

		/* Account creation time */
		extern unsigned int account_get_ll_ctime(t_account * account)
		{
			return account_get_numattr(account, "BNET\\acct\\ctime");
		}

		extern unsigned int account_get_ll_time(t_account * account)
		{
			return account_get_numattr(account, "BNET\\acct\\lastlogin_time");
		}


		extern int account_set_ll_time(t_account * account, unsigned int t)
		{
			return account_set_numattr(account, "BNET\\acct\\lastlogin_time", t);
		}



		extern t_clienttag account_get_ll_clienttag(t_account * account)
		{
			char const * clienttag;
			t_clienttag clienttag_uint;

			clienttag = account_get_strattr(account, "BNET\\acct\\lastlogin_clienttag");
			clienttag_uint = tag_str_to_uint(clienttag);

			return clienttag_uint;
		}

		extern int account_set_ll_clienttag(t_account * account, t_clienttag clienttag)
		{
			char clienttag_str[5];

			return account_set_strattr(account, "BNET\\acct\\lastlogin_clienttag", tag_uint_to_str(clienttag_str, clienttag));
		}


		extern char const * account_get_ll_user(t_account * account)
		{
			return account_get_strattr(account, "BNET\\acct\\lastlogin_user");
		}


		extern int account_set_ll_user(t_account * account, char const * user)
		{
			return account_set_strattr(account, "BNET\\acct\\lastlogin_user", user);
		}


		extern char const * account_get_ll_owner(t_account * account)
		{
			return account_get_strattr(account, "BNET\\acct\\lastlogin_owner");
		}


		extern int account_set_ll_owner(t_account * account, char const * owner)
		{
			return account_set_strattr(account, "BNET\\acct\\lastlogin_owner", owner);
		}


		extern char const * account_get_ll_ip(t_account * account)
		{
			return account_get_strattr(account, "BNET\\acct\\lastlogin_ip");
		}


		extern int account_set_ll_ip(t_account * account, char const * ip)
		{
			return account_set_strattr(account, "BNET\\acct\\lastlogin_ip", ip);
		}

		/****************************************************************/


		extern unsigned int account_get_normal_wins(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\wins", tag_uint_to_str(clienttag_str, clienttag));
			return account_get_numattr(account, key);
		}


		extern int account_inc_normal_wins(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\wins", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, account_get_normal_wins(account, clienttag) + 1);
		}


		extern int account_set_normal_wins(t_account * account, t_clienttag clienttag, unsigned wins)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			snprintf(key, sizeof(key), "Record\\%s\\0\\wins", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, wins);
		}


		extern unsigned int account_get_normal_losses(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\losses", tag_uint_to_str(clienttag_str, clienttag));
			return account_get_numattr(account, key);
		}


		extern int account_inc_normal_losses(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\losses", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, account_get_normal_losses(account, clienttag) + 1);
		}


		extern int account_set_normal_losses(t_account * account, t_clienttag clienttag, unsigned losses)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\losses", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, losses);
		}


		extern unsigned int account_get_normal_draws(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\draws", tag_uint_to_str(clienttag_str, clienttag));
			return account_get_numattr(account, key);
		}


		extern int account_inc_normal_draws(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\draws", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, account_get_normal_draws(account, clienttag) + 1);
		}


		extern int account_set_normal_draws(t_account * account, t_clienttag clienttag, unsigned draws)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\draws", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, draws);
		}



		extern unsigned int account_get_normal_disconnects(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\disconnects", tag_uint_to_str(clienttag_str, clienttag));
			return account_get_numattr(account, key);
		}


		extern int account_inc_normal_disconnects(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\disconnects", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, account_get_normal_disconnects(account, clienttag) + 1);
		}


		extern int account_set_normal_disconnects(t_account * account, t_clienttag clienttag, unsigned discs)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\disconnects", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, discs);
		}


		extern int account_set_normal_last_time(t_account * account, t_clienttag clienttag, t_bnettime t)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\last game", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_strattr(account, key, bnettime_get_str(t));
		}


		extern int account_set_normal_last_result(t_account * account, t_clienttag clienttag, char const * result)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\last game result", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_strattr(account, key, result);
		}


		/****************************************************************/


		extern unsigned int account_get_ladder_active_wins(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\active wins", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_get_numattr(account, key);
		}


		extern int account_set_ladder_active_wins(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int wins)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\active wins", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_set_numattr(account, key, wins);
		}


		extern unsigned int account_get_ladder_active_losses(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\active losses", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_get_numattr(account, key);
		}


		extern int account_set_ladder_active_losses(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int losses)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\active losses", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_set_numattr(account, key, losses);
		}


		extern unsigned int account_get_ladder_active_draws(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\active draws", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_get_numattr(account, key);
		}


		extern int account_set_ladder_active_draws(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int draws)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\active draws", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_set_numattr(account, key, draws);
		}


		extern unsigned int account_get_ladder_active_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\active disconnects", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_get_numattr(account, key);
		}


		extern int account_set_ladder_active_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int disconnects)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\active disconnects", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_set_numattr(account, key, disconnects);
		}


		extern unsigned int account_get_ladder_active_rating(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\active rating", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_get_numattr(account, key);
		}


		extern int account_set_ladder_active_rating(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int rating)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\active rating", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_set_numattr(account, key, rating);
		}


		extern int account_get_ladder_active_rank(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\active rank", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_get_numattr(account, key);
		}


		extern int account_set_ladder_active_rank(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int rank)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\active rank", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_set_numattr(account, key, rank);
		}


		extern char const * account_get_ladder_active_last_time(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return NULL;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\active last game", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_get_strattr(account, key);
		}


		extern int account_set_ladder_active_last_time(t_account * account, t_clienttag clienttag, t_ladder_id id, t_bnettime t)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\active last game", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_set_strattr(account, key, bnettime_get_str(t));
		}

		/****************************************************************/


		extern unsigned int account_get_ladder_wins(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%s\\wins", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_get_numattr(account, key);
		}


		extern int account_inc_ladder_wins(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%s\\wins", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_set_numattr(account, key, account_get_ladder_wins(account, clienttag, id) + 1);
		}


		extern int account_set_ladder_wins(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned wins)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%s\\wins", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_set_numattr(account, key, wins);
		}


		extern unsigned int account_get_ladder_losses(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%s\\losses", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_get_numattr(account, key);
		}


		extern int account_inc_ladder_losses(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%s\\losses", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_set_numattr(account, key, account_get_ladder_losses(account, clienttag, id) + 1);
		}


		extern int account_set_ladder_losses(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned losses)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%s\\losses", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_set_numattr(account, key, losses);
		}


		extern unsigned int account_get_ladder_draws(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%s\\draws", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_get_numattr(account, key);
		}


		extern int account_inc_ladder_draws(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%s\\draws", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_set_numattr(account, key, account_get_ladder_draws(account, clienttag, id) + 1);
		}


		extern int account_set_ladder_draws(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned draws)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%s\\draws", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_set_numattr(account, key, draws);
		}


		extern unsigned int account_get_ladder_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%s\\disconnects", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_get_numattr(account, key);
		}


		extern int account_inc_ladder_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%s\\disconnects", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_set_numattr(account, key, account_get_ladder_disconnects(account, clienttag, id) + 1);
		}


		extern int account_set_ladder_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned discs)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%s\\disconnects", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_set_numattr(account, key, discs);
		}


		extern unsigned int account_get_ladder_rating(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\rating", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_get_numattr(account, key);
		}


		extern int account_set_ladder_rating(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned rating)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\rating", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_set_numattr(account, key, rating);
		}


		extern int account_adjust_ladder_rating(t_account * account, t_clienttag clienttag, t_ladder_id id, int delta)
		{
			char         key[256];
			char clienttag_str[5];
			unsigned int oldrating;
			unsigned int newrating;
			int          retval = 0;

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\rating", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			/* don't allow rating to go below 1 */
			oldrating = account_get_ladder_rating(account, clienttag, id);
			if (delta < 0 && oldrating <= (unsigned int)-delta)
				newrating = 1;
			else
				newrating = oldrating + delta;
			if (account_set_numattr(account, key, newrating)<0)
				retval = -1;

			if (newrating>account_get_ladder_high_rating(account, clienttag, id))
			{
				snprintf(key, sizeof(key), "Record\\%s\\%d\\high rating", tag_uint_to_str(clienttag_str, clienttag), (int)id);
				if (account_set_numattr(account, key, newrating) < 0)
					retval = -1;
			}

			return retval;
		}



		extern int account_get_ladder_rank(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%s\\rank", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_get_numattr(account, key);
		}


		extern int account_set_ladder_rank(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int rank)
		{
			char         key[256];
			char clienttag_str[5];
			unsigned int oldrank;
			int          retval = 0;

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			// if (rank==0)
			//    eventlog(eventlog_level_warn,__FUNCTION__,"setting rank to zero?");
			snprintf(key, sizeof(key), "Record\\%s\\%s\\rank", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			if (account_set_numattr(account, key, rank) < 0)
				retval = -1;

			oldrank = account_get_ladder_high_rank(account, clienttag, id);
			if (oldrank == 0 || rank < oldrank)
			{
				snprintf(key, sizeof(key), "Record\\%s\\%s\\high rank", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
				if (account_set_numattr(account, key, rank) < 0)
					retval = -1;
			}
			return retval;
		}

		extern unsigned int account_get_ladder_high_rating(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\high rating", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_get_numattr(account, key);
		}


		extern unsigned int account_get_ladder_high_rank(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\high rank", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_get_numattr(account, key);
		}


		extern int account_set_ladder_last_time(t_account * account, t_clienttag clienttag, t_ladder_id id, t_bnettime t)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\last game", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_set_strattr(account, key, bnettime_get_str(t));
		}


		extern char const * account_get_ladder_last_time(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return NULL;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\last game", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_get_strattr(account, key);
		}


		extern int account_set_ladder_last_result(t_account * account, t_clienttag clienttag, t_ladder_id id, char const * result)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\%d\\last game result", tag_uint_to_str(clienttag_str, clienttag), (int)id);
			return account_set_strattr(account, key, result);
		}


		/****************************************************************/


		extern unsigned int account_get_normal_level(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\level", tag_uint_to_str(clienttag_str, clienttag));
			return account_get_numattr(account, key);
		}


		extern int account_set_normal_level(t_account * account, t_clienttag clienttag, unsigned int level)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\level", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, level);
		}


		extern unsigned int account_get_normal_class(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\class", tag_uint_to_str(clienttag_str, clienttag));
			return account_get_numattr(account, key);
		}


		extern int account_set_normal_class(t_account * account, t_clienttag clienttag, unsigned int chclass)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\class", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, chclass);
		}


		extern unsigned int account_get_normal_diablo_kills(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\diablo kills", tag_uint_to_str(clienttag_str, clienttag));
			return account_get_numattr(account, key);
		}


		extern int account_set_normal_diablo_kills(t_account * account, t_clienttag clienttag, unsigned int diablo_kills)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\diablo kills", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, diablo_kills);
		}


		extern unsigned int account_get_normal_strength(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\strength", tag_uint_to_str(clienttag_str, clienttag));
			return account_get_numattr(account, key);
		}


		extern int account_set_normal_strength(t_account * account, t_clienttag clienttag, unsigned int strength)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\strength", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, strength);
		}


		extern unsigned int account_get_normal_magic(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\magic", tag_uint_to_str(clienttag_str, clienttag));
			return account_get_numattr(account, key);
		}


		extern int account_set_normal_magic(t_account * account, t_clienttag clienttag, unsigned int magic)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\magic", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, magic);
		}


		extern unsigned int account_get_normal_dexterity(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\dexterity", tag_uint_to_str(clienttag_str, clienttag));
			return account_get_numattr(account, key);
		}


		extern int account_set_normal_dexterity(t_account * account, t_clienttag clienttag, unsigned int dexterity)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\dexterity", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, dexterity);
		}


		extern unsigned int account_get_normal_vitality(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\vitality", tag_uint_to_str(clienttag_str, clienttag));
			return account_get_numattr(account, key);
		}


		extern int account_set_normal_vitality(t_account * account, t_clienttag clienttag, unsigned int vitality)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\vitality", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, vitality);
		}


		extern unsigned int account_get_normal_gold(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\gold", tag_uint_to_str(clienttag_str, clienttag));
			return account_get_numattr(account, key);
		}


		extern int account_set_normal_gold(t_account * account, t_clienttag clienttag, unsigned int gold)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}
			snprintf(key, sizeof(key), "Record\\%s\\0\\gold", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_numattr(account, key, gold);
		}


		/****************************************************************/


		extern int account_check_closed_character(t_account * account, t_clienttag clienttag, char const * realmname, char const * charname)
		{
			char const * charlist = account_get_closed_characterlist(account, clienttag, realmname);
			char         tempname[32];

			if (charlist == NULL)
			{
				eventlog(eventlog_level_debug, __FUNCTION__, "no characters in Realm %s", realmname);
			}
			else
			{
				char const * start;
				char const * next_char;
				int    list_len;
				int    name_len;
				int    i;

				eventlog(eventlog_level_debug, __FUNCTION__, "got characterlist \"%s\" for Realm %s", charlist, realmname);

				list_len = std::strlen(charlist);
				start = charlist;
				next_char = start;
				for (i = 0; i < list_len; i++, next_char++)
				{
					if (',' == *next_char)
					{
						name_len = next_char - start;

						std::strncpy(tempname, start, name_len);
						tempname[name_len] = '\0';

						eventlog(eventlog_level_debug, __FUNCTION__, "found character \"%s\"", tempname);

						if (std::strcmp(tempname, charname) == 0)
							return 1;

						start = next_char + 1;
					}
				}

				name_len = next_char - start;

				std::strncpy(tempname, start, name_len);
				tempname[name_len] = '\0';

				eventlog(eventlog_level_debug, __FUNCTION__, "found tail character \"%s\"", tempname);

				if (std::strcmp(tempname, charname) == 0)
					return 1;
			}

			return 0;
		}


		extern char const * account_get_closed_characterlist(t_account * account, t_clienttag clienttag, char const * realmname)
		{
			char realmkey[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return NULL;
			}

			if (!realmname)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL realmname");
				return NULL;
			}

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return NULL;
			}

			snprintf(realmkey, sizeof(realmkey), "BNET\\CharacterList\\%s\\%s\\0", tag_uint_to_str(clienttag_str, clienttag), realmname);
			eventlog(eventlog_level_debug, __FUNCTION__, "looking for '%s'", realmkey);

			return account_get_strattr(account, realmkey);
		}


		extern int account_set_closed_characterlist(t_account * account, t_clienttag clienttag, char const * charlist)
		{
			char key[256];
			char clienttag_str[5];

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "clienttag='%s', charlist='%s'", tag_uint_to_str(clienttag_str, clienttag), charlist);

			snprintf(key, sizeof(key), "BNET\\Characters\\%s\\0", tag_uint_to_str(clienttag_str, clienttag));
			return account_set_strattr(account, key, charlist);
		}

		extern int account_add_closed_character(t_account * account, t_clienttag clienttag, t_character * ch)
		{
			char key[256];
			char clienttag_str[5];
			char hex_buffer[356];
			char chars_in_realm[256];
			char const * old_list;

			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			if (!ch)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL character");
				return -1;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "clienttag=\"%s\", realm=\"%s\", name=\"%s\"", tag_uint_to_str(clienttag_str, clienttag), ch->realmname, ch->name);

			snprintf(key, sizeof(key), "BNET\\CharacterList\\%s\\%s\\0", tag_uint_to_str(clienttag_str, clienttag), ch->realmname);
			old_list = account_get_strattr(account, key);
			if (old_list)
			{
				snprintf(chars_in_realm, sizeof(chars_in_realm), "%s,%s", old_list, ch->name);
			}
			else
			{
				snprintf(chars_in_realm, sizeof(chars_in_realm), "%s", ch->name);
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "new character list for realm \"%s\" is \"%s\"", ch->realmname, chars_in_realm);
			account_set_strattr(account, key, chars_in_realm);

			snprintf(key, sizeof(key), "BNET\\Characters\\%s\\%s\\%s\\0", tag_uint_to_str(clienttag_str, clienttag), ch->realmname, ch->name);
			str_to_hex(hex_buffer, (char*)ch->data, ch->datalen);
			account_set_strattr(account, key, hex_buffer);

			/*
			eventlog(eventlog_level_debug,__FUNCTION__,"key \"%s\"", key);
			eventlog(eventlog_level_debug,__FUNCTION__,"value \"%s\"", hex_buffer);
			*/

			return 0;
		}

		extern int account_set_friend(t_account * account, int friendnum, unsigned int frienduid)
		{
			char key[256];
			if (frienduid == 0 || friendnum < 0 || friendnum >= prefs_get_max_friends())
			{
				return -1;
			}
			snprintf(key, sizeof(key), "friend\\%d\\uid", friendnum);

			return account_set_numattr(account, key, frienduid);
		}

		extern unsigned int account_get_friend(t_account * account, int friendnum)
		{
			char key[256];
			int tmp;
			char const * name;
			t_account * acct;

			if (friendnum < 0 || friendnum >= prefs_get_max_friends()) {
				// bogus name (user himself) instead of NULL, otherwise clients might crash
				eventlog(eventlog_level_error, __FUNCTION__, "invalid friendnum %d (max: %d)", friendnum, prefs_get_max_friends());
				return 0;
			}

			snprintf(key, sizeof(key), "friend\\%d\\uid", friendnum);
			tmp = account_get_numattr(account, key);
			if (!tmp) {
				// ok, looks like we have a problem. Maybe friends still stored in old format?

				snprintf(key, sizeof(key), "friend\\%d\\name", friendnum);
				name = account_get_strattr(account, key);

				if (name)
				{
					if ((acct = accountlist_find_account(name)) != NULL)
					{
						tmp = account_get_uid(acct);
						account_set_friend(account, friendnum, tmp);
						account_set_strattr(account, key, NULL); //remove old username-based friend now

						return tmp;
					}
					account_set_strattr(account, key, NULL); //remove old username-based friend now
					eventlog(eventlog_level_warn, __FUNCTION__, "unexistant friend name ('%s') in old storage format", name);
					return 0;
				}

				eventlog(eventlog_level_error, __FUNCTION__, "could not find friend (friendno: %d of '%s')", friendnum, account_get_name(account));
				return 0;
			}

			return tmp;
		}

		static int account_set_friendcount(t_account * account, int count)
		{
			if (count < 0 || count > prefs_get_max_friends())
			{
				return -1;
			}

			return account_set_numattr(account, "friend\\count", count);
		}

		extern int account_get_friendcount(t_account * account)
		{
			return account_get_numattr(account, "friend\\count");
		}

		extern int account_add_friend(t_account * my_acc, t_account * facc)
		{
			unsigned my_uid;
			unsigned fuid;
			int nf;
			t_list *flist;

			if (my_acc == NULL || facc == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			my_uid = account_get_uid(my_acc);
			fuid = account_get_uid(facc);

			if (my_acc == facc) return -2;

			nf = account_get_friendcount(my_acc);
			if (nf >= prefs_get_max_friends()) return -3;

			flist = account_get_friends(my_acc);
			if (flist == NULL) return -1;
			if (friendlist_find_account(flist, facc) != NULL) return -4;

			account_set_friend(my_acc, nf, fuid);
			account_set_friendcount(my_acc, nf + 1);
			if (account_check_mutual(facc, my_uid) == 0)
				friendlist_add_account(flist, facc, 1);
			else
				friendlist_add_account(flist, facc, 0);

			return 0;
		}

		extern int account_remove_friend(t_account * account, int friendnum)
		{
			int i;
			int n = account_get_friendcount(account);

			if (account == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			if (friendnum < 0 || friendnum >= n) {
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid friendnum (friendnum: %d max: %d)", friendnum, n);
				return -1;
			}

			for (i = friendnum; i < n - 1; i++)
				account_set_friend(account, i, account_get_friend(account, i + 1));

			account_set_friend(account, n - 1, 0); /* FIXME: should delete the attribute */
			account_set_friendcount(account, n - 1);

			return 0;
		}

		extern int account_remove_friend2(t_account * account, const char * frienduid)
		{
			t_list *flist;
			t_friend *fr;
			unsigned uid;
			int i, n;

			if (account == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			if (frienduid == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL friend username");
				return -1;
			}

			if ((flist = account_get_friends(account)) == NULL)
				return -1;

			if ((fr = friendlist_find_username(flist, frienduid)) == NULL) return -2;

			n = account_get_friendcount(account);
			uid = account_get_uid(friend_get_account(fr));
			for (i = 0; i < n; i++)
			if (account_get_friend(account, i) == uid) {
				t_list * fflist;
				t_friend * ffr;
				t_account * facc;

				account_remove_friend(account, i);
				if ((facc = friend_get_account(fr)) &&
					(fflist = account_get_friends(facc)) &&
					(ffr = friendlist_find_account(fflist, account)))
					friend_set_mutual(ffr, 0);
				friendlist_remove_friend(flist, fr);
				return i;
			}

			return -2;
		}

		// Some Extra Commands for REAL admins to promote other users to Admins
		// And Moderators of channels
		extern int account_set_admin(t_account * account)
		{
			return account_set_strattr(account, "BNET\\auth\\admin", "true");
		}
		extern int account_set_demoteadmin(t_account * account)
		{
			return account_set_strattr(account, "BNET\\auth\\admin", "false");
		}

		extern unsigned int account_get_command_groups(t_account * account)
		{
			return account_get_numattr(account, "BNET\\auth\\command_groups");
		}
		extern int account_set_command_groups(t_account * account, unsigned int groups)
		{
			return account_set_numattr(account, "BNET\\auth\\command_groups", groups);
		}

		// WAR3 Play Game & Profile Funcs

		extern char const * race_get_str(unsigned int race)
		{
			switch (race) {
			case W3_RACE_ORCS:
				return "orcs";
			case W3_RACE_HUMANS:
				return "humans";
			case W3_RACE_UNDEAD:
			case W3_ICON_UNDEAD:
				return "undead";
			case W3_RACE_NIGHTELVES:
				return "nightelves";
			case W3_RACE_RANDOM:
			case W3_ICON_RANDOM:
				return "random";
			case W3_RACE_DEMONS:
			case W3_ICON_DEMONS:
				return "demons";
			default:
				eventlog(eventlog_level_warn, __FUNCTION__, "unknown race: %x", race);
				return NULL;
			}
		}

		extern int account_inc_racewins(t_account * account, unsigned int intrace, t_clienttag clienttag)
		{
			char table[256];
			char clienttag_str[5];
			unsigned int wins;
			char const * race = race_get_str(intrace);

			if (!race)
				return -1;

			snprintf(table, sizeof(table), "Record\\%s\\%s\\wins", tag_uint_to_str(clienttag_str, clienttag), race);
			wins = account_get_numattr(account, table);
			wins++;

			return account_set_numattr(account, table, wins);
		}

		extern int account_get_racewins(t_account * account, unsigned int intrace, t_clienttag clienttag)
		{
			char table[256];
			char clienttag_str[5];
			char const *race = race_get_str(intrace);

			if (!race)
				return 0;

			snprintf(table, sizeof(table), "Record\\%s\\%s\\wins", tag_uint_to_str(clienttag_str, clienttag), race);
			return account_get_numattr(account, table);
		}

		extern int account_inc_racelosses(t_account * account, unsigned int intrace, t_clienttag clienttag)
		{
			char table[256];
			char clienttag_str[5];
			unsigned int losses;
			char const *race = race_get_str(intrace);

			if (!race)
				return -1;

			snprintf(table, sizeof(table), "Record\\%s\\%s\\losses", tag_uint_to_str(clienttag_str, clienttag), race);

			losses = account_get_numattr(account, table);

			losses++;

			return account_set_numattr(account, table, losses);

		}

		extern int account_get_racelosses(t_account * account, unsigned int intrace, t_clienttag clienttag)
		{
			char table[256];
			char clienttag_str[5];
			char const *race = race_get_str(intrace);

			if (!race)
				return 0;

			snprintf(table, sizeof(table), "Record\\%s\\%s\\losses", tag_uint_to_str(clienttag_str, clienttag), race);

			return account_get_numattr(account, table);

		}

		extern int account_update_xp(t_account * account, t_clienttag clienttag, t_game_result gameresult, unsigned int opponlevel, int * xp_diff, t_ladder_id id)
		{
			int xp;
			int mylevel;
			int xpdiff = 0, placeholder;

			xp = account_get_ladder_xp(account, clienttag, id); //get current xp
			if (xp < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got negative XP");
				return -1;
			}

			mylevel = account_get_ladder_level(account, clienttag, id); //get accounts level
			if (mylevel > W3_XPCALC_MAXLEVEL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid level: %d", mylevel);
				return -1;
			}

			if (mylevel <= 0) //if level is 0 then set it to 1
				mylevel = 1;

			if (opponlevel < 1) opponlevel = 1;

			switch (gameresult)
			{
			case game_result_win:
				ladder_war3_xpdiff(mylevel, opponlevel, &xpdiff, &placeholder); break;
			case game_result_loss:
				ladder_war3_xpdiff(opponlevel, mylevel, &placeholder, &xpdiff); break;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid game result: %d", gameresult);
				return -1;
			}

			*xp_diff = xpdiff;
			xp += xpdiff;
			if (xp < 0) xp = 0;

			return account_set_ladder_xp(account, clienttag, id, xp);
		}

		extern int account_get_ladder_xp(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			snprintf(key, sizeof(key), "Record\\%s\\%s\\xp", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_get_numattr(account, key);
		}

		extern int account_set_ladder_xp(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int xp)
		{
			char         key[256];
			char clienttag_str[5];

			snprintf(key, sizeof(key), "Record\\%s\\%s\\xp", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_set_numattr(account, key, xp);
		}

		extern int account_get_ladder_level(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			snprintf(key, sizeof(key), "Record\\%s\\%s\\level", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_get_numattr(account, key);
		}

		extern int account_set_ladder_level(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int level)
		{
			char         key[256];
			char clienttag_str[5];

			snprintf(key, sizeof(key), "Record\\%s\\%s\\level", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_set_numattr(account, key, level);
		}


		extern int account_adjust_ladder_level(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			int xp, mylevel;

			xp = account_get_ladder_xp(account, clienttag, id);
			if (xp < 0) xp = 0;

			mylevel = account_get_ladder_level(account, clienttag, id);
			if (mylevel < 1) mylevel = 1;

			if (mylevel > W3_XPCALC_MAXLEVEL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid level: %d", mylevel);
				return -1;
			}

			mylevel = ladder_war3_updatelevel(mylevel, xp);

			return account_set_ladder_level(account, clienttag, id, mylevel);
		}


		//Other funcs used in profiles and PG saving
		extern void account_get_raceicon(t_account * account, char * raceicon, unsigned int * raceiconnumber, unsigned int * wins, t_clienttag clienttag) //Based of wins for each race, Race with most wins, gets shown in chat channel
		{
			unsigned int humans;
			unsigned int orcs;
			unsigned int undead;
			unsigned int nightelf;
			unsigned int random;
			unsigned int i;

			random = account_get_racewins(account, W3_RACE_RANDOM, clienttag);
			humans = account_get_racewins(account, W3_RACE_HUMANS, clienttag);
			orcs = account_get_racewins(account, W3_RACE_ORCS, clienttag);
			undead = account_get_racewins(account, W3_RACE_UNDEAD, clienttag);
			nightelf = account_get_racewins(account, W3_RACE_NIGHTELVES, clienttag);
			if (orcs >= humans && orcs >= undead && orcs >= nightelf && orcs >= random) {
				*raceicon = 'O';
				*wins = orcs;
			}
			else if (humans >= orcs && humans >= undead && humans >= nightelf && humans >= random) {
				*raceicon = 'H';
				*wins = humans;
			}
			else if (nightelf >= humans && nightelf >= orcs && nightelf >= undead && nightelf >= random) {
				*raceicon = 'N';
				*wins = nightelf;
			}
			else if (undead >= humans && undead >= orcs && undead >= nightelf && undead >= random) {
				*raceicon = 'U';
				*wins = undead;
			}
			else {
				*raceicon = 'R';
				*wins = random;
			}
			i = 1;
			while ((signed)*wins >= anongame_infos_get_ICON_REQ(i, clienttag) && anongame_infos_get_ICON_REQ(i, clienttag) > 0) i++;
			*raceiconnumber = i;
		}

		extern int account_get_profile_calcs(t_account * account, int xp, unsigned int Level)
		{
			int xp_min;
			int xp_max;
			unsigned int i;
			int  t;
			unsigned int startlvl;

			if (Level == 1) startlvl = 1;
			else startlvl = Level - 1;
			for (i = startlvl; i < W3_XPCALC_MAXLEVEL; i++) {
				xp_min = ladder_war3_get_min_xp(i);
				xp_max = ladder_war3_get_min_xp(i + 1);
				if ((xp >= xp_min) && (xp < xp_max)) {
					t = (int)((((double)xp - (double)xp_min)
						/ ((double)xp_max - (double)xp_min)) * 128);
					if (i < Level) {
						return 128 + t;
					}
					else {
						return t;
					}
				}
			}
			return 0;
		}

		extern int account_set_saveladderstats(t_account * account, unsigned int gametype, t_game_result result, unsigned int opponlevel, t_clienttag clienttag)
		{
			unsigned int intrace;
			int xpdiff;
			unsigned int uid, xp, level;
			t_ladder_id id;

			if (!account) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}


			//added for better tracking down of problems with gameresults
			eventlog(eventlog_level_trace, __FUNCTION__, "parsing game result for player: %s result: %s", account_get_name(account), (result == game_result_win) ? "WIN" : "LOSS");

			intrace = account_get_w3pgrace(account, clienttag);
			uid = account_get_uid(account);

			switch (gametype)
			{
			case ANONGAME_TYPE_1V1: //1v1
			{
										id = ladder_id_solo;
										break;
			}
			case ANONGAME_TYPE_2V2:
			case ANONGAME_TYPE_3V3:
			case ANONGAME_TYPE_4V4:
			case ANONGAME_TYPE_5V5:
			case ANONGAME_TYPE_6V6:
			case ANONGAME_TYPE_2V2V2:
			case ANONGAME_TYPE_3V3V3:
			case ANONGAME_TYPE_4V4V4:
			case ANONGAME_TYPE_2V2V2V2:
			case ANONGAME_TYPE_3V3V3V3:
			{
										  id = ladder_id_team;
										  break;
			}

			case ANONGAME_TYPE_SMALL_FFA:
			{
											id = ladder_id_ffa;
											break;
			}
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "Invalid Gametype? %u", gametype);
				return -1;
			}

			if (result == game_result_win) //win
			{
				account_inc_ladder_wins(account, clienttag, id);
				account_inc_racewins(account, intrace, clienttag);
			}
			if (result == game_result_loss) //loss
			{
				account_inc_ladder_losses(account, clienttag, id);
				account_inc_racelosses(account, intrace, clienttag);
			}

			account_update_xp(account, clienttag, result, opponlevel, &xpdiff, id);
			account_adjust_ladder_level(account, clienttag, id);
			level = account_get_ladder_level(account, clienttag, id);
			xp = account_get_ladder_xp(account, clienttag, id);
			LadderList* ladderList = ladders.getLadderList(LadderKey(id, clienttag, ladder_sort_default, ladder_time_default));
			LadderReferencedObject reference(account);

			//consider using wins count for tertiary attribute ?
			ladderList->updateEntry(uid, level, xp, 0, reference);

			return 0;
		}

		extern int account_set_w3pgrace(t_account * account, t_clienttag clienttag, unsigned int race)
		{
			char key[256];
			char clienttag_str[5];

			snprintf(key, sizeof(key), "Record\\%s\\w3pgrace", tag_uint_to_str(clienttag_str, clienttag));

			return account_set_numattr(account, key, race);
		}

		extern int account_get_w3pgrace(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char clienttag_str[5];

			snprintf(key, sizeof(key), "Record\\%s\\w3pgrace", tag_uint_to_str(clienttag_str, clienttag));

			return account_get_numattr(account, key);
		}
		// Arranged Team Functions
		extern int account_set_currentatteam(t_account * account, unsigned int teamcount)
		{
			return account_set_numattr(account, "BNET\\current_at_team", teamcount);
		}

		extern int account_get_currentatteam(t_account * account)
		{
			return account_get_numattr(account, "BNET\\current_at_team");
		}

		extern int account_get_highestladderlevel(t_account * account, t_clienttag clienttag)
		{
			t_elem * curr;
			t_team * team;

			unsigned int sololevel = account_get_ladder_level(account, clienttag, ladder_id_solo);
			unsigned int teamlevel = account_get_ladder_level(account, clienttag, ladder_id_team);
			unsigned int ffalevel = account_get_ladder_level(account, clienttag, ladder_id_ffa);
			unsigned int atlevel = 0;
			unsigned int t;

			if (account_get_teams(account))
			{
				LIST_TRAVERSE(account_get_teams(account), curr)
				{
					if (!(team = (t_team*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
						continue;
					}
					if ((team_get_clienttag(team) != clienttag))
						continue;

					if ((t = team_get_level(team)) > atlevel)
						atlevel = t;
				}
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "Checking for highest level in Solo,Team,FFA,AT Ladder Stats");
			eventlog(eventlog_level_debug, __FUNCTION__, "Solo Level: %d, Team Level %d, FFA Level %d, Highest AT Team Level: %d", sololevel, teamlevel, ffalevel, atlevel);

			if (sololevel >= teamlevel && sololevel >= atlevel && sololevel >= ffalevel)
				return sololevel;
			if (teamlevel >= sololevel && teamlevel >= atlevel && teamlevel >= ffalevel)
				return teamlevel;
			if (atlevel >= sololevel && atlevel >= teamlevel && atlevel >= ffalevel)
				return atlevel;
			return ffalevel;

			// we should never get here

			return -1;
		}


		/* value = icons delimeted by space */
		extern int account_set_user_iconstash(t_account * account, t_clienttag clienttag, char const * value)
		{
			char key[256];
			char clienttag_str[5];

			snprintf(key, sizeof(key), "Record\\%s\\iconstash", tag_uint_to_str(clienttag_str, clienttag));
			if (value)
				return account_set_strattr(account, key, value);
			else
				return account_set_strattr(account, key, "NULL");
		}

		extern char const * account_get_user_iconstash(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char const * retval;
			char clienttag_str[5];

			snprintf(key, sizeof(key), "Record\\%s\\iconstash", tag_uint_to_str(clienttag_str, clienttag));
			retval = account_get_strattr(account, key);

			if ((retval) && ((std::strcmp(retval, "NULL") != 0)))
				return retval;
			else
				return NULL;
		}

		//BlacKDicK 04/20/2003
		extern int account_set_user_icon(t_account * account, t_clienttag clienttag, char const * usericon)
		{
			char key[256];
			char clienttag_str[5];

			snprintf(key, sizeof(key), "Record\\%s\\userselected_icon", tag_uint_to_str(clienttag_str, clienttag));
			if (usericon)
				return account_set_strattr(account, key, usericon);
			else
				return account_set_strattr(account, key, "NULL");
		}

		extern char const * account_get_user_icon(t_account * account, t_clienttag clienttag)
		{
			char key[256];
			char const * retval;
			char clienttag_str[5];

			snprintf(key, sizeof(key), "Record\\%s\\userselected_icon", tag_uint_to_str(clienttag_str, clienttag));
			retval = account_get_strattr(account, key);

			if ((retval) && ((std::strcmp(retval, "NULL") != 0)))
				return retval;
			else
				return NULL;
		}

		// Ramdom - Nothing, Grean Dragon Whelp, Azure Dragon (Blue Dragon), Red Dragon, Deathwing, Nothing
		// Humans - Peasant, Footman, Knight, Archmage, Medivh, Nothing
		// Orcs - Peon, Grunt, Tauren, Far Seer, Thrall, Nothing
		// Undead - Acolyle, Ghoul, Abomination, Lich, Tichondrius, Nothing
		// Night Elves - Wisp, Archer, Druid of the Claw, Priestess of the Moon, Furion Stormrage, Nothing
		// Demons - Nothing, ???(wich unit is nfgn), Infernal, Doom Guard, Pit Lord/Manaroth, Archimonde
		// ADDED TFT ICON BY DJP 07/16/2003
		static const char * profile_code[12][6] = {
			{ NULL, "ngrd", "nadr", "nrdr", "nbwm", NULL },
			{ "hpea", "hfoo", "hkni", "Hamg", "nmed", NULL },
			{ "opeo", "ogru", "otau", "Ofar", "Othr", NULL },
			{ "uaco", "ugho", "uabo", "Ulic", "Utic", NULL },
			{ "ewsp", "earc", "edoc", "Emoo", "Efur", NULL },
			{ NULL, "nfng", "ninf", "nbal", "Nplh", "Uwar" }, /* not used by RoC */
			{ NULL, "nmyr", "nnsw", "nhyc", "Hvsh", "Eevm" },
			{ "hpea", "hrif", "hsor", "hspt", "Hblm", "Hjai" },
			{ "opeo", "ohun", "oshm", "ospw", "Oshd", "Orex" },
			{ "uaco", "ucry", "uban", "uobs", "Ucrl", "Usyl" },
			{ "ewsp", "esen", "edot", "edry", "Ekee", "Ewrd" },
			{ NULL, "nfgu", "ninf", "nbal", "Nplh", "Uwar" }
		};

		extern unsigned int account_get_icon_profile(t_account * account, t_clienttag clienttag)
		{
			unsigned int humans = account_get_racewins(account, W3_RACE_HUMANS, clienttag);		//  1;
			unsigned int orcs = account_get_racewins(account, W3_RACE_ORCS, clienttag); 		        //  2;
			unsigned int nightelf = account_get_racewins(account, W3_RACE_NIGHTELVES, clienttag);	        //  4;
			unsigned int undead = account_get_racewins(account, W3_RACE_UNDEAD, clienttag);		//  8;
			unsigned int random = account_get_racewins(account, W3_RACE_RANDOM, clienttag);		// 32;
			unsigned int race; 	     // 0 = Humans, 1 = Orcs, 2 = Night Elves, 3 = Undead, 4 = Ramdom
			unsigned int level = 0; // 0 = under 25, 1 = 25 to 249, 2 = 250 to 499, 3 = 500 to 1499, 4 = 1500 or more (wins)
			int wins;
			int number_ctag = 0;

			/* moved the check for orcs in the first place so people with 0 wins get peon */
			if (orcs >= humans && orcs >= undead && orcs >= nightelf && orcs >= random) {
				wins = orcs;
				race = 2;
			}
			else if (humans >= orcs && humans >= undead && humans >= nightelf && humans >= random) {
				wins = humans;
				race = 1;
			}
			else if (nightelf >= humans && nightelf >= orcs && nightelf >= undead && nightelf >= random) {
				wins = nightelf;
				race = 4;
			}
			else if (undead >= humans && undead >= orcs && undead >= nightelf && undead >= random) {
				wins = undead;
				race = 3;
			}
			else {
				wins = random;
				race = 0;
			}

			while (wins >= anongame_infos_get_ICON_REQ(level + 1, clienttag) && anongame_infos_get_ICON_REQ(level + 1, clienttag) > 0) level++;
			if (clienttag == CLIENTTAG_WAR3XP_UINT)
				number_ctag = 6;

			eventlog(eventlog_level_info, __FUNCTION__, "race -> %u; level -> %u; wins -> %u; profileicon -> %s", race, level, wins, profile_code[race + number_ctag][level]);

			return char_icon_to_uint(profile_code[race + number_ctag][level]);
		}

		extern unsigned int account_icon_to_profile_icon(char const * icon, t_account * account, t_clienttag ctag)
		{
			char tmp_icon[4];
			const char * result;
			int number_ctag = 0;

			if (icon == NULL) return account_get_icon_profile(account, ctag);
			if (sizeof(icon) >= 4){
				std::strncpy(tmp_icon, icon, 4);
				tmp_icon[0] = tmp_icon[0] - 48;
				if (ctag == CLIENTTAG_WAR3XP_UINT) {
					number_ctag = 6;
				}
				if (tmp_icon[0] >= 1) {
					if (tmp_icon[1] == 'R'){
						result = profile_code[0 + number_ctag][tmp_icon[0] - 1];
					}
					else if (tmp_icon[1] == 'H'){
						result = profile_code[1 + number_ctag][tmp_icon[0] - 1];
					}
					else if (tmp_icon[1] == 'O'){
						result = profile_code[2 + number_ctag][tmp_icon[0] - 1];
					}
					else if (tmp_icon[1] == 'U'){
						result = profile_code[3 + number_ctag][tmp_icon[0] - 1];
					}
					else if (tmp_icon[1] == 'N'){
						result = profile_code[4 + number_ctag][tmp_icon[0] - 1];
					}
					else if (tmp_icon[1] == 'D'){
						result = profile_code[5 + number_ctag][tmp_icon[0] - 1];
					}
					else{
						eventlog(eventlog_level_warn, __FUNCTION__, "got unrecognized race on [%s] icon ", icon);
						result = profile_code[2][0];
					} /* "opeo" */
				}
				else{
					eventlog(eventlog_level_warn, __FUNCTION__, "got race_level<1 on [%s] icon ", icon);
					result = NULL;
				}
			}
			else{
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid icon lenght [%s] icon ", icon);
				result = NULL;
			}
			//eventlog(eventlog_level_debug,__FUNCTION__,"from [%4.4s] icon returned [0x%X]",icon,char_icon_to_uint(result));
			return char_icon_to_uint(result);
		}

		extern int account_is_operator_or_admin(t_account * account, char const * channel)
		{
			if ((account_get_auth_operator(account, channel) == 1) || (account_get_auth_operator(account, NULL) == 1) ||
				(account_get_auth_admin(account, channel) == 1) || (account_get_auth_admin(account, NULL) == 1))
				return 1;
			else
				return 0;

		}

		static unsigned int char_icon_to_uint(const char * icon)
		{
			unsigned int value;

			if (!icon) return 0;
			if (std::strlen(icon) != 4) return 0;

			value = ((unsigned int)icon[0]) << 24;
			value |= ((unsigned int)icon[1]) << 16;
			value |= ((unsigned int)icon[2]) << 8;
			value |= ((unsigned int)icon[3]);

			return value;
		}

		extern char const * account_get_email(t_account * account)
		{
			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Unable to set account flag to TRUE");
				return NULL;
			}
			return account_get_strattr(account, "BNET\\acct\\email");
		}

		extern int account_set_email(t_account * account, char const * email)
		{
			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Unable to set account flag to TRUE");
				return -1;
			}
			return account_set_strattr(account, "BNET\\acct\\email", email);
		}

		extern int account_set_userlang(t_account * account, const char * lang)
		{
			if (lang)
				return account_set_strattr(account, "BNET\\acct\\userlang", lang);
			else
				return account_set_strattr(account, "BNET\\acct\\userlang", "NULL");
		}

		extern char const * account_get_userlang(t_account * account)
		{
			char const * retval;
			retval = account_get_strattr(account, "BNET\\acct\\userlang");

			if ((retval) && ((std::strcmp(retval, "NULL") != 0)))
				return retval;
			else
				return NULL;
		}

		/**
		*  Westwood Online Extensions
		*/
		extern char const * account_get_wol_apgar(t_account * account)
		{
			if (!account) {
				ERROR0("got NULL account");
				return NULL;
			}

			return account_get_strattr(account, "WOL\\auth\\apgar");
		}

		extern int account_set_wol_apgar(t_account * account, char const * apgar)
		{
			if (!account) {
				ERROR0("got NULL account");
				return -1;
			}
			if (!apgar) {
				ERROR0("got NULL apgar");
				return -1;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "[** WOL **] WOL\\auth\\apgar = %s", apgar);
			return account_set_strattr(account, "WOL\\auth\\apgar", apgar);
		}

		extern int account_get_locale(t_account * account)
		{
			if (!account) {
				ERROR0("got NULL account");
				return 0;
			}

			return account_get_numattr(account, "WOL\\acct\\locale");
		}

		extern int account_set_locale(t_account * account, int locale)
		{
			if (!account) {
				ERROR0("got NULL account");
				return -1;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "[** WOL **] WOL\\acct\\locale = %u", locale);
			return account_set_numattr(account, "WOL\\acct\\locale", locale);
		}

		extern int account_get_ladder_points(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			char key[256];
			char clienttag_str[5];

			snprintf(key, sizeof(key), "Record\\%s\\%s\\points", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_get_numattr(account, key);
		}

		extern int account_set_ladder_points(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int points)
		{
			char key[256];
			char clienttag_str[5];

			snprintf(key, sizeof(key), "Record\\%s\\%s\\points", tag_uint_to_str(clienttag_str, clienttag), ladder_id_str[(int)id]);
			return account_set_numattr(account, key, points);
		}

	}

}
