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
#include <memory>
#include <string>

#include <fmt/format.h>

#include "compat/strcasecmp.h"

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
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account (from {}:{})", fn, ln);
				return 0;
			}
			if (key == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key (from {}:{})", fn, ln);
				return 0;
			}
			char const * temp = account_get_strattr(account, key);
			if (temp == nullptr)
				return 0;

			unsigned int val;
			if (str_to_uint(temp, &val) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "not a numeric string \"{}\" for key \"{}\"", temp, key);
				return 0;
			}

			return val;
		}


		extern int account_set_numattr(t_account * account, char const * key, unsigned int val)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}
			if (key == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key");
				return -1;
			}

			return account_set_strattr(account, key, std::to_string(val).c_str());
		}


		extern int account_get_boolattr_real(t_account * account, char const * key, char const * fn, unsigned int ln)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account (from {}:{})", fn, ln);
				return -1;
			}
			if (key == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key (from {}:{})", fn, ln);
				return -1;
			}

			char const * temp = account_get_strattr(account, key);
			if (temp == nullptr)
				return -1;

			switch (str_get_bool(temp))
			{
			case 1:
				return 1;
			case 0:
				return 0;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "bad boolean value \"{}\" for key \"{}\"", temp, key);
				return -1;
			}
		}


		extern int account_set_boolattr(t_account * account, char const * key, int val)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}
			if (key == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key");
				return -1;
			}

			return account_set_strattr(account, key, val ? "true" : "false");
		}

		extern char const * account_get_rawattr_real(t_account * account, char const * key, char const * fn, unsigned int ln)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account (from {}:{})", fn, ln);
				return nullptr;
			}
			if (key == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key (from {}:{})", fn, ln);
				return nullptr;
			}

			char const * temp = account_get_strattr(account, key);
			if (temp == nullptr)
				return nullptr;

			size_t length = std::strlen(temp) / 3;
			char * result = (char *)xmalloc(length);
			if (result == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "failed to create result");
				return nullptr;
			}

			hex_to_str(temp, result, length);

			return result;
		}

		extern int account_set_rawattr(t_account * account, char const * key, char const * val, int length)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}
			if (key == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key");
				return -1;
			}

			char * temp_buffer = (char *)xmalloc(length * 3 + 1);

			str_to_hex(temp_buffer, val, length);
			int result = account_set_strattr(account, key, temp_buffer);

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
			if (channelname == nullptr)
				return account_get_boolattr(account, "BNET\\auth\\admin");
			
			std::string temp("BNET\\auth\\admin\\" + std::string(channelname));

			return account_get_boolattr(account, temp.c_str());
		}
		extern int account_get_auth_admin(t_account * account, std::string channelname)
		{
			if (channelname.empty())
				return account_get_boolattr(account, "BNET\\auth\\admin");

			std::string temp("BNET\\auth\\admin\\" + channelname);

			return account_get_boolattr(account, temp.c_str());
		}


		extern int account_set_auth_admin(t_account * account, char const * channelname, int val)
		{
			if (channelname == nullptr)
				return account_set_boolattr(account, "BNET\\auth\\admin", val);

			std::string temp("BNET\\auth\\admin\\" + std::string(channelname));

			return account_set_boolattr(account, temp.c_str(), val);
		}
		extern int account_set_auth_admin(t_account * account, std::string channelname, int val)
		{
			if (channelname.empty())
				return account_set_boolattr(account, "BNET\\auth\\admin", val);

			std::string temp("BNET\\auth\\admin\\" + channelname);

			return account_set_boolattr(account, temp.c_str(), val);
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
			if (channelname == nullptr)
				return account_get_boolattr(account, "BNET\\auth\\operator");

			std::string temp("BNET\\auth\\operator\\" + std::string(channelname));

			return account_get_boolattr(account, temp.c_str());
		}
		extern int account_get_auth_operator(t_account * account, std::string channelname)
		{
			if (channelname.empty())
				return account_get_boolattr(account, "BNET\\auth\\operator");

			std::string temp("BNET\\auth\\operator\\" + channelname);

			return account_get_boolattr(account, temp.c_str());
		}

		extern int account_set_auth_operator(t_account * account, char const * channelname, int val)
		{
			if (channelname == nullptr)
				return account_set_boolattr(account, "BNET\\auth\\operator", val);

			std::string temp("BNET\\auth\\operator\\" + std::string(channelname));

			return account_set_boolattr(account, temp.c_str(), val);
		}
		extern int account_set_auth_operator(t_account * account, std::string channelname, int val)
		{
			if (channelname.empty())
				return account_set_boolattr(account, "BNET\\auth\\operator", val);

			std::string temp("BNET\\auth\\operator\\" + channelname);

			return account_set_boolattr(account, temp.c_str(), val);
		}

		extern int account_get_auth_voice(t_account * account, char const * channelname)
		{
			std::string temp("BNET\\auth\\voice\\" + std::string(channelname));

			return account_get_boolattr(account, temp.c_str());
		}
		extern int account_get_auth_voice(t_account * account, std::string channelname)
		{
			std::string temp("BNET\\auth\\voice\\" + channelname);

			return account_get_boolattr(account, temp.c_str());
		}

		extern int account_set_auth_voice(t_account * account, char const * channelname, int val)
		{
			std::string temp("BNET\\auth\\voice\\" + std::string(channelname));

			return account_set_boolattr(account, temp.c_str(), val);
		}
		extern int account_set_auth_voice(t_account * account, std::string channelname, int val)
		{
			std::string temp("BNET\\auth\\voice\\" + channelname);

			return account_set_boolattr(account, temp.c_str(), val);
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


		extern std::string account_get_sex(t_account * account)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return std::string();
			}

			char const * temp = account_get_strattr(account, "profile\\sex");
			if (temp == nullptr)
				return std::string();

			return std::string(temp);
		}


		extern std::string account_get_age(t_account * account)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return std::string();
			}
			char const * temp = account_get_strattr(account, "profile\\age");
			if (temp == nullptr)
				return std::string();

			return std::string(temp);
		}


		extern std::string account_get_loc(t_account * account)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return std::string();
			}
			char const * temp = account_get_strattr(account, "profile\\location");
			if (temp == nullptr)
				return std::string();

			return std::string(temp);
		}


		extern std::string account_get_desc(t_account * account)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return std::string();
			}

			char const * temp = account_get_strattr(account, "profile\\description");
			if (temp == nullptr)
				return std::string();

			return std::string(temp);
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
			return account_set_strattr(account, "BNET\\acct\\lastlogin_clienttag", tag_uint_to_str2(clienttag).c_str());
		}


		extern char const * account_get_ll_user(t_account * account)
		{
			return account_get_strattr(account, "BNET\\acct\\lastlogin_user");
		}


		extern int account_set_ll_user(t_account * account, char const * user)
		{
			return account_set_strattr(account, "BNET\\acct\\lastlogin_user", user);
		}
		extern int account_set_ll_user(t_account * account, std::string user)
		{
			return account_set_strattr(account, "BNET\\acct\\lastlogin_user", user.c_str());
		}


		extern char const * account_get_ll_owner(t_account * account)
		{
			return account_get_strattr(account, "BNET\\acct\\lastlogin_owner");
		}


		extern int account_set_ll_owner(t_account * account, char const * owner)
		{
			return account_set_strattr(account, "BNET\\acct\\lastlogin_owner", owner);
		}
		extern int account_set_ll_owner(t_account * account, std::string owner)
		{
			return account_set_strattr(account, "BNET\\acct\\lastlogin_owner", owner.c_str());
		}


		extern char const * account_get_ll_ip(t_account * account)
		{
			return account_get_strattr(account, "BNET\\acct\\lastlogin_ip");
		}


		extern int account_set_ll_ip(t_account * account, char const * ip)
		{
			return account_set_strattr(account, "BNET\\acct\\lastlogin_ip", ip);
		}
		extern int account_set_ll_ip(t_account * account, std::string ip)
		{
			return account_set_strattr(account, "BNET\\acct\\lastlogin_ip", ip.c_str());
		}

		/****************************************************************/


		extern unsigned int account_get_normal_wins(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\wins");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_inc_normal_wins(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\wins");

			return account_set_numattr(account, key.c_str(), account_get_normal_wins(account, clienttag) + 1);
		}


		extern int account_set_normal_wins(t_account * account, t_clienttag clienttag, unsigned wins)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\wins");

			return account_set_numattr(account, key.c_str(), wins);
		}


		extern unsigned int account_get_normal_losses(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\losses");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_inc_normal_losses(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\losses");

			return account_set_numattr(account, key.c_str(), account_get_normal_losses(account, clienttag) + 1);
		}


		extern int account_set_normal_losses(t_account * account, t_clienttag clienttag, unsigned losses)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\losses");

			return account_set_numattr(account, key.c_str(), losses);
		}


		extern unsigned int account_get_normal_draws(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\draws");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_inc_normal_draws(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\draws");

			return account_set_numattr(account, key.c_str(), account_get_normal_draws(account, clienttag) + 1);
		}


		extern int account_set_normal_draws(t_account * account, t_clienttag clienttag, unsigned draws)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\draws");

			return account_set_numattr(account, key.c_str(), draws);
		}



		extern unsigned int account_get_normal_disconnects(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\disconnects");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_inc_normal_disconnects(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\disconnects");

			return account_set_numattr(account, key.c_str(), account_get_normal_disconnects(account, clienttag) + 1);
		}


		extern int account_set_normal_disconnects(t_account * account, t_clienttag clienttag, unsigned discs)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\disconnects");

			return account_set_numattr(account, key.c_str(), discs);
		}


		extern int account_set_normal_last_time(t_account * account, t_clienttag clienttag, t_bnettime t)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\last game");

			return account_set_strattr(account, key.c_str(), bnettime_get_str(t));
		}


		extern int account_set_normal_last_result(t_account * account, t_clienttag clienttag, char const * result)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\last game result");

			return account_set_strattr(account, key.c_str(), result);
		}
		extern int account_set_normal_last_result(t_account * account, t_clienttag clienttag, std::string result)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\last game result");

			return account_set_strattr(account, key.c_str(), result.c_str());
		}


		/****************************************************************/


		extern unsigned int account_get_ladder_active_wins(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\active wins");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_ladder_active_wins(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int wins)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) +"\\" + std::to_string(id) + "\\active wins");

			return account_set_numattr(account, key.c_str(), wins);
		}


		extern unsigned int account_get_ladder_active_losses(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\active losses");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_ladder_active_losses(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int losses)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\active losses");

			return account_set_numattr(account, key.c_str(), losses);
		}


		extern unsigned int account_get_ladder_active_draws(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\active draws");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_ladder_active_draws(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int draws)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\active draws");

			return account_set_numattr(account, key.c_str(), draws);
		}


		extern unsigned int account_get_ladder_active_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\active disconnects");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_ladder_active_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int disconnects)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\active disconnects");

			return account_set_numattr(account, key.c_str(), disconnects);
		}


		extern unsigned int account_get_ladder_active_rating(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\active rating");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_ladder_active_rating(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int rating)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\active rating");

			return account_set_numattr(account, key.c_str(), rating);
		}


		extern int account_get_ladder_active_rank(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\active rank");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_ladder_active_rank(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int rank)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\active rank");

			return account_set_numattr(account, key.c_str(), rank);
		}


		extern char const * account_get_ladder_active_last_time(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return nullptr;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\active last game");

			return account_get_strattr(account, key.c_str());
		}


		extern int account_set_ladder_active_last_time(t_account * account, t_clienttag clienttag, t_ladder_id id, t_bnettime t)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\active last game");

			return account_set_strattr(account, key.c_str(), bnettime_get_str(t));
		}

		/****************************************************************/


		extern unsigned int account_get_ladder_wins(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\wins");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_inc_ladder_wins(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\wins");

			return account_set_numattr(account, key.c_str(), account_get_ladder_wins(account, clienttag, id) + 1);
		}


		extern int account_set_ladder_wins(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned wins)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\wins");

			return account_set_numattr(account, key.c_str(), wins);
		}


		extern unsigned int account_get_ladder_losses(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\losses");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_inc_ladder_losses(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\losses");

			return account_set_numattr(account, key.c_str(), account_get_ladder_losses(account, clienttag, id) + 1);
		}


		extern int account_set_ladder_losses(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned losses)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\losses");

			return account_set_numattr(account, key.c_str(), losses);
		}


		extern unsigned int account_get_ladder_draws(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\draws");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_inc_ladder_draws(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\draws");

			return account_set_numattr(account, key.c_str(), account_get_ladder_draws(account, clienttag, id) + 1);
		}


		extern int account_set_ladder_draws(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned draws)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\draws");

			return account_set_numattr(account, key.c_str(), draws);
		}


		extern unsigned int account_get_ladder_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\disconnects");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_inc_ladder_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\disconnects");

			return account_set_numattr(account, key.c_str(), account_get_ladder_disconnects(account, clienttag, id) + 1);
		}


		extern int account_set_ladder_disconnects(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned discs)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\disconnects");

			return account_set_numattr(account, key.c_str(), discs);
		}


		extern unsigned int account_get_ladder_rating(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\rating");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_ladder_rating(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned rating)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\rating");

			return account_set_numattr(account, key.c_str(), rating);
		}


		extern int account_adjust_ladder_rating(t_account * account, t_clienttag clienttag, t_ladder_id id, int delta)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\rating");

			/* don't allow rating to go below 1 */
			unsigned int oldrating = account_get_ladder_rating(account, clienttag, id);
			unsigned int newrating;
			if (delta < 0 && oldrating <= (unsigned int)-delta)
				newrating = 1;
			else
				newrating = oldrating + delta;

			int retval = 0;
			if (account_set_numattr(account, key.c_str(), newrating) < 0)
				retval = -1;

			if (newrating > account_get_ladder_high_rating(account, clienttag, id))
			{
				key = "Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\high rating";

				if (account_set_numattr(account, key.c_str(), newrating) < 0)
					retval = -1;
			}

			return retval;
		}



		extern int account_get_ladder_rank(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\rank");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_ladder_rank(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int rank)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\rank");

			int retval = 0;
			if (account_set_numattr(account, key.c_str(), rank) < 0)
				retval = -1;

			unsigned int oldrank = account_get_ladder_high_rank(account, clienttag, id);
			if (oldrank == 0 || rank < oldrank)
			{
				key = "Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\high rank";

				if (account_set_numattr(account, key.c_str(), rank) < 0)
					retval = -1;
			}
			return retval;
		}

		extern unsigned int account_get_ladder_high_rating(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\high rating");

			return account_get_numattr(account, key.c_str());
		}


		extern unsigned int account_get_ladder_high_rank(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\high rank");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_ladder_last_time(t_account * account, t_clienttag clienttag, t_ladder_id id, t_bnettime t)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\last game");

			return account_set_strattr(account, key.c_str(), bnettime_get_str(t));
		}


		extern char const * account_get_ladder_last_time(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return nullptr;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\last game");

			return account_get_strattr(account, key.c_str());
		}


		extern int account_set_ladder_last_result(t_account * account, t_clienttag clienttag, t_ladder_id id, char const * result)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + std::to_string(id) + "\\last game result");

			return account_set_strattr(account, key.c_str(), result);
		}


		/****************************************************************/


		extern unsigned int account_get_normal_level(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\level");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_normal_level(t_account * account, t_clienttag clienttag, unsigned int level)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\level");

			return account_set_numattr(account, key.c_str(), level);
		}


		extern unsigned int account_get_normal_class(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\class");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_normal_class(t_account * account, t_clienttag clienttag, unsigned int chclass)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\class");

			return account_set_numattr(account, key.c_str(), chclass);
		}


		extern unsigned int account_get_normal_diablo_kills(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\diablo kills");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_normal_diablo_kills(t_account * account, t_clienttag clienttag, unsigned int diablo_kills)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\diablo kills");

			return account_set_numattr(account, key.c_str(), diablo_kills);
		}


		extern unsigned int account_get_normal_strength(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\strength");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_normal_strength(t_account * account, t_clienttag clienttag, unsigned int strength)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\strength");

			return account_set_numattr(account, key.c_str(), strength);
		}


		extern unsigned int account_get_normal_magic(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\magic");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_normal_magic(t_account * account, t_clienttag clienttag, unsigned int magic)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\magic");

			return account_set_numattr(account, key.c_str(), magic);
		}


		extern unsigned int account_get_normal_dexterity(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\dexterity");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_normal_dexterity(t_account * account, t_clienttag clienttag, unsigned int dexterity)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\dexterity");

			return account_set_numattr(account, key.c_str(), dexterity);
		}


		extern unsigned int account_get_normal_vitality(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\vitality");
			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_normal_vitality(t_account * account, t_clienttag clienttag, unsigned int vitality)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\vitality");

			return account_set_numattr(account, key.c_str(), vitality);
		}


		extern unsigned int account_get_normal_gold(t_account * account, t_clienttag clienttag)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return 0;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\gold");

			return account_get_numattr(account, key.c_str());
		}


		extern int account_set_normal_gold(t_account * account, t_clienttag clienttag, unsigned int gold)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\0\\gold");

			return account_set_numattr(account, key.c_str(), gold);
		}


		/****************************************************************/


		extern int account_check_closed_character(t_account * account, t_clienttag clienttag, char const * realmname, char const * charname)
		{
			char const * charlist = account_get_closed_characterlist(account, clienttag, realmname);
			if (charlist == nullptr)
			{
				eventlog(eventlog_level_debug, __FUNCTION__, "no characters in Realm {}", realmname);
				return 0;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "got characterlist \"{}\" for Realm {}", charlist, realmname);

			size_t list_len = std::strlen(charlist);
			char const * start = charlist;
			char const * next_char = start;


			char tempname[32];
			int name_len;

			for (std::size_t i = 0; i < list_len; i++, next_char++)
			{
				if (*next_char == ',')
				{
					name_len = next_char - start;

					std::strncpy(tempname, start, name_len);
					tempname[name_len] = '\0';

					eventlog(eventlog_level_debug, __FUNCTION__, "found character \"{}\"", tempname);

					if (std::strcmp(tempname, charname) == 0)
						return 1;

					start = next_char + 1;
				}
			}

			name_len = next_char - start;

			std::strncpy(tempname, start, name_len);
			tempname[name_len] = '\0';

			eventlog(eventlog_level_debug, __FUNCTION__, "found tail character \"{}\"", tempname);

			if (std::strcmp(tempname, charname) == 0)
				return 1;

			return 0;
		}


		extern char const * account_get_closed_characterlist(t_account * account, t_clienttag clienttag, char const * realmname)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return nullptr;
			}

			if (!realmname)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL realmname");
				return nullptr;
			}

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return nullptr;
			}

			std::string realmkey("BNET\\CharacterList\\" + tag_uint_to_str2(clienttag) + "\\" + std::string(realmname) + "\\0");
			
			eventlog(eventlog_level_trace, __FUNCTION__, "looking for '{}'", realmkey.c_str());

			return account_get_strattr(account, realmkey.c_str());
		}
		extern char const * account_get_closed_characterlist(t_account * account, t_clienttag clienttag, std::string realmname)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return nullptr;
			}

			if (realmname.empty())
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL realmname");
				return nullptr;
			}

			if (!account)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return nullptr;
			}

			std::string realmkey("BNET\\CharacterList\\" + tag_uint_to_str2(clienttag) + "\\" + realmname + "\\0");

			eventlog(eventlog_level_trace, __FUNCTION__, "looking for '{}'", realmkey.c_str());

			return account_get_strattr(account, realmkey.c_str());
		}


		extern int account_set_closed_characterlist(t_account * account, t_clienttag clienttag, char const * charlist)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "clienttag='{}', charlist='{}'", tag_uint_to_str2(clienttag).c_str(), charlist);
			std::string key("BNET\\Characters\\" + tag_uint_to_str2(clienttag) + "\\0");

			return account_set_strattr(account, key.c_str(), charlist);
		}
		extern int account_set_closed_characterlist(t_account * account, t_clienttag clienttag, std::string charlist)
		{
			if (!clienttag)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got bad clienttag");
				return -1;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "clienttag='{}', charlist='{}'", tag_uint_to_str2(clienttag).c_str(), charlist.c_str());
			std::string key("BNET\\Characters\\" + tag_uint_to_str2(clienttag) + "\\0");

			return account_set_strattr(account, key.c_str(), charlist.c_str());
		}

		extern int account_add_closed_character(t_account * account, t_clienttag clienttag, t_character * ch)
		{
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

			eventlog(eventlog_level_debug, __FUNCTION__, "clienttag=\"{}\", realm=\"{}\", name=\"{}\"", tag_uint_to_str2(clienttag).c_str(), ch->realmname, ch->name);
			std::string key("BNET\\CharacterList\\" + tag_uint_to_str2(clienttag) + "\\" + std::string(ch->realmname) + "\\0");
			
			std::string chars_in_realm;
			char const * old_list = account_get_strattr(account, key.c_str());
			if (old_list)
				chars_in_realm = std::string(old_list) + "," + std::string(ch->name);
			else
				chars_in_realm = std::string(ch->name);

			eventlog(eventlog_level_debug, __FUNCTION__, "new character list for realm \"{}\" is \"{}\"", ch->realmname, chars_in_realm.c_str());
			account_set_strattr(account, key.c_str(), chars_in_realm.c_str());

			key = "BNET\\Characters\\" + tag_uint_to_str2(clienttag) + "\\" + std::string(ch->realmname) + "\\" + std::string(ch->name) + "\\0";
			char hex_buffer[356];
			str_to_hex(hex_buffer, (char*)ch->data, ch->datalen);
			account_set_strattr(account, key.c_str(), hex_buffer);

			/*
			eventlog(eventlog_level_debug,__FUNCTION__,"key \"%s\"", key);
			eventlog(eventlog_level_debug,__FUNCTION__,"value \"%s\"", hex_buffer);
			*/

			return 0;
		}

		extern int account_set_friend(t_account * account, int friendnum, unsigned int frienduid)
		{
			if (frienduid == 0 || friendnum < 0 || friendnum >= prefs_get_max_friends())
				return -1;

			std::string key("friend\\" + std::to_string(friendnum) + "\\uid");

			return account_set_numattr(account, key.c_str(), frienduid);
		}

		extern unsigned int account_get_friend(t_account * account, int friendnum)
		{
			if (friendnum < 0 || friendnum >= prefs_get_max_friends())
			{
				// bogus name (user himself) instead of NULL, otherwise clients might crash
				eventlog(eventlog_level_error, __FUNCTION__, "invalid friendnum {} (max: {})", friendnum, prefs_get_max_friends());
				return 0;
			}

			std::string key("friend\\" + std::to_string(friendnum) + "\\uid");

			int tmp = account_get_numattr(account, key.c_str());
			if (!tmp)
			{
				// ok, looks like we have a problem. Maybe friends still stored in old format?

				key = "friend\\" + std::to_string(friendnum) + "\\name";
				char const * name = account_get_strattr(account, key.c_str());
				if (!name)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not find friend (friendno: {} of '{}')", friendnum, account_get_name(account));
					return 0;
				}

				t_account * acct = accountlist_find_account(name);
				if (acct != nullptr)
				{
					tmp = account_get_uid(acct);
					account_set_friend(account, friendnum, tmp);
					account_set_strattr(account, key.c_str(), nullptr); //remove old username-based friend now

					return tmp;
				}
				account_set_strattr(account, key.c_str(), nullptr); //remove old username-based friend now
				eventlog(eventlog_level_warn, __FUNCTION__, "unexistant friend name ('{}') in old storage format", name);
				return 0;
			}

			return tmp;
		}

		static int account_set_friendcount(t_account * account, int count)
		{
			if (count < 0 || count > prefs_get_max_friends())
				return -1;

			return account_set_numattr(account, "friend\\count", count);
		}

		extern int account_get_friendcount(t_account * account)
		{
			return account_get_numattr(account, "friend\\count");
		}

		extern int account_add_friend(t_account * my_acc, t_account * facc)
		{
			if (my_acc == nullptr || facc == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			unsigned int my_uid = account_get_uid(my_acc);
			unsigned int fuid = account_get_uid(facc);

			if (my_acc == facc)
				return -2;

			int nf = account_get_friendcount(my_acc);
			if (nf >= prefs_get_max_friends())
				return -3;

			t_list *flist = account_get_friends(my_acc);
			if (flist == nullptr)
				return -1;
			if (friendlist_find_account(flist, facc) != nullptr)
				return -4;

			account_set_friend(my_acc, nf, fuid);
			account_set_friendcount(my_acc, nf + 1);

			if (account_check_mutual(facc, my_uid) == 0)
				friendlist_add_account(flist, facc, FRIEND_ISMUTUAL);
			else
				friendlist_add_account(flist, facc, FRIEND_NOTMUTUAL);

			return 0;
		}

		extern int account_remove_friend(t_account * account, int friendnum)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			int n = account_get_friendcount(account);
			if (friendnum < 0 || friendnum >= n)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid friendnum (friendnum: {} max: {})", friendnum, n);
				return -1;
			}

			for (auto i = friendnum; i < n - 1; i++)
				account_set_friend(account, i, account_get_friend(account, i + 1));

			account_set_friend(account, n - 1, 0); /* FIXME: should delete the attribute */
			account_set_friendcount(account, n - 1);

			return 0;
		}

		extern int account_remove_friend2(t_account * account, const char * frienduid)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			if (frienduid == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL friend username");
				return -1;
			}
			t_list *flist = account_get_friends(account);
			if (flist == nullptr)
				return -1;

			t_friend *fr = friendlist_find_username(flist, frienduid);
			if (fr == nullptr)
				return -2;

			int n = account_get_friendcount(account);
			unsigned int uid = account_get_uid(friend_get_account(fr));
			for (auto i = 0; i < n; i++)
			{
				if (account_get_friend(account, i) == uid)
				{
					t_account * facc = friend_get_account(fr);
					t_list * fflist = account_get_friends(facc);
					t_friend * ffr = friendlist_find_account(fflist, account);

					account_remove_friend(account, i);
					if (facc && fflist && ffr)
						friend_set_mutual(ffr, FRIEND_NOTMUTUAL);

					friendlist_remove_friend(flist, fr);
					return i;
				}
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

		extern std::string race_get_str(unsigned int race)
		{
			switch (race)
			{
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
				eventlog(eventlog_level_warn, __FUNCTION__, "unknown race: {}", race);
				return std::string();
			}
		}

		extern int account_inc_racewins(t_account * account, unsigned int intrace, t_clienttag clienttag)
		{
			std::string race = race_get_str(intrace);
			if (race.empty())
				return -1;

			std::string table("Record\\" + tag_uint_to_str2(clienttag) + "\\" + race + "\\wins");
			unsigned int wins = account_get_numattr(account, table.c_str()); wins++;

			return account_set_numattr(account, table.c_str(), wins);
		}

		extern int account_get_racewins(t_account * account, unsigned int intrace, t_clienttag clienttag)
		{
			std::string race = race_get_str(intrace);
			if (race.empty())
				return 0;

			std::string table("Record\\" + tag_uint_to_str2(clienttag) + "\\" + race + "\\wins");

			return account_get_numattr(account, table.c_str());
		}

		extern int account_inc_racelosses(t_account * account, unsigned int intrace, t_clienttag clienttag)
		{
			std::string race = race_get_str(intrace);
			if (race.empty())
				return -1;

			std::string table("Record\\" + tag_uint_to_str2(clienttag) + "\\" + race + "\\losses");

			unsigned int losses = account_get_numattr(account, table.c_str()); losses++;

			return account_set_numattr(account, table.c_str(), losses);

		}

		extern int account_get_racelosses(t_account * account, unsigned int intrace, t_clienttag clienttag)
		{
			std::string race = race_get_str(intrace);
			if (race.empty())
				return 0;

			std::string table("Record\\" + tag_uint_to_str2(clienttag) + "\\" + race + "\\losses");

			return account_get_numattr(account, table.c_str());

		}

		extern int account_update_xp(t_account * account, t_clienttag clienttag, t_game_result gameresult, unsigned int opponlevel, int * xp_diff, t_ladder_id id)
		{
			int xp = account_get_ladder_xp(account, clienttag, id); //get current xp
			if (xp < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got negative XP");
				return -1;
			}

			int mylevel = account_get_ladder_level(account, clienttag, id); //get accounts level
			if (mylevel > W3_XPCALC_MAXLEVEL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid level: {}", mylevel);
				return -1;
			}

			if (mylevel <= 0) //if level is negative or 0 then set it to 1
				mylevel = 1;

			if (opponlevel < 1) opponlevel = 1;

			int xpdiff = 0, placeholder;

			switch (gameresult)
			{
			case game_result_win:
				ladder_war3_xpdiff(mylevel, opponlevel, &xpdiff, &placeholder); break;
			case game_result_loss:
				ladder_war3_xpdiff(opponlevel, mylevel, &placeholder, &xpdiff); break;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid game result: {}", gameresult);
				return -1;
			}

			*xp_diff = xpdiff;
			xp += xpdiff;
			if (xp < 0) xp = 0;

			return account_set_ladder_xp(account, clienttag, id, xp);
		}

		extern int account_get_ladder_xp(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\xp");

			return account_get_numattr(account, key.c_str());
		}

		extern int account_set_ladder_xp(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int xp)
		{
			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\xp");

			return account_set_numattr(account, key.c_str(), xp);
		}

		extern int account_get_ladder_level(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\level");

			return account_get_numattr(account, key.c_str());
		}

		extern int account_set_ladder_level(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int level)
		{
			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\level");

			return account_set_numattr(account, key.c_str(), level);
		}


		extern int account_adjust_ladder_level(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			int xp = account_get_ladder_xp(account, clienttag, id);
			if (xp < 0) xp = 0;

			int mylevel = account_get_ladder_level(account, clienttag, id);
			if (mylevel < 1) mylevel = 1;

			if (mylevel > W3_XPCALC_MAXLEVEL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid level: {}", mylevel);
				return -1;
			}

			mylevel = ladder_war3_updatelevel(mylevel, xp);

			return account_set_ladder_level(account, clienttag, id, mylevel);
		}


		//Other funcs used in profiles and PG saving
		extern void account_get_raceicon(t_account * account, char * raceicon, unsigned int * raceiconnumber, unsigned int * wins, t_clienttag clienttag) //Based of wins for each race, Race with most wins, gets shown in chat channel
		{
			unsigned int random = account_get_racewins(account, W3_RACE_RANDOM, clienttag);
			unsigned int humans = account_get_racewins(account, W3_RACE_HUMANS, clienttag);
			unsigned int orcs = account_get_racewins(account, W3_RACE_ORCS, clienttag);
			unsigned int undead = account_get_racewins(account, W3_RACE_UNDEAD, clienttag);
			unsigned int nightelf = account_get_racewins(account, W3_RACE_NIGHTELVES, clienttag);
			if (orcs >= humans && orcs >= undead && orcs >= nightelf && orcs >= random)
			{
				*raceicon = 'O';
				*wins = orcs;
			}
			else if (humans >= orcs && humans >= undead && humans >= nightelf && humans >= random)
			{
				*raceicon = 'H';
				*wins = humans;
			}
			else if (nightelf >= humans && nightelf >= orcs && nightelf >= undead && nightelf >= random)
			{
				*raceicon = 'N';
				*wins = nightelf;
			}
			else if (undead >= humans && undead >= orcs && undead >= nightelf && undead >= random)
			{
				*raceicon = 'U';
				*wins = undead;
			}
			else
			{
				*raceicon = 'R';
				*wins = random;
			}
			unsigned int i = 1;
			while ((signed)*wins >= anongame_infos_get_ICON_REQ(i, clienttag) && anongame_infos_get_ICON_REQ(i, clienttag) > 0) i++;
			*raceiconnumber = i;
		}

		extern int account_get_profile_calcs(t_account * account, int xp, unsigned int Level)
		{
			unsigned int startlvl;
			if (Level == 1)
				startlvl = 1;
			else
				startlvl = Level - 1;

			for (auto i = startlvl; i < W3_XPCALC_MAXLEVEL; i++)
			{
				int xp_min = ladder_war3_get_min_xp(i);
				int xp_max = ladder_war3_get_min_xp(i + 1);
				if ((xp >= xp_min) && (xp < xp_max))
				{
					//FIXME.......
					int t = (int)((((double)xp - (double)xp_min)
						/ ((double)xp_max - (double)xp_min)) * 128);

					if (i < Level)
						return 128 + t;
					else
						return t;
				}
			}

			return 0;
		}

		extern int account_set_saveladderstats(t_account * account, unsigned int gametype, t_game_result result, unsigned int opponlevel, t_clienttag clienttag)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			//added for better tracking down of problems with gameresults
			eventlog(eventlog_level_trace, __FUNCTION__, "parsing game result for player: {} result: {}", account_get_name(account), (result == game_result_win) ? "WIN" : "LOSS");

			unsigned int intrace = account_get_w3pgrace(account, clienttag);
			unsigned int uid = account_get_uid(account);
			t_ladder_id id;

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
				eventlog(eventlog_level_error, __FUNCTION__, "Invalid Gametype? {}", gametype);
				return -1;
			}

			if (result == game_result_win)
			{
				account_inc_ladder_wins(account, clienttag, id);
				account_inc_racewins(account, intrace, clienttag);
			}
			if (result == game_result_loss)
			{
				account_inc_ladder_losses(account, clienttag, id);
				account_inc_racelosses(account, intrace, clienttag);
			}

			int xpdiff;
			account_update_xp(account, clienttag, result, opponlevel, &xpdiff, id);
			account_adjust_ladder_level(account, clienttag, id);
			unsigned int level = account_get_ladder_level(account, clienttag, id);
			unsigned int xp = account_get_ladder_xp(account, clienttag, id);
			LadderList* ladderList = ladders.getLadderList(LadderKey(id, clienttag, ladder_sort_default, ladder_time_default));
			LadderReferencedObject reference(account);

			//consider using wins count for tertiary attribute ?
			ladderList->updateEntry(uid, level, xp, 0, reference);

			return 0;
		}

		extern int account_set_w3pgrace(t_account * account, t_clienttag clienttag, unsigned int race)
		{
			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\w3pgrace");

			return account_set_numattr(account, key.c_str(), race);
		}

		extern int account_get_w3pgrace(t_account * account, t_clienttag clienttag)
		{
			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\w3pgrace");

			return account_get_numattr(account, key.c_str());
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
			unsigned int sololevel = account_get_ladder_level(account, clienttag, ladder_id_solo);
			unsigned int teamlevel = account_get_ladder_level(account, clienttag, ladder_id_team);
			unsigned int ffalevel = account_get_ladder_level(account, clienttag, ladder_id_ffa);
			unsigned int atlevel = 0;
			unsigned int t;

			if (account_get_teams(account))
			{
				t_elem * curr = nullptr;
				t_team * team = nullptr;
				LIST_TRAVERSE(account_get_teams(account), curr)
				{
					team = (t_team*)elem_get_data(curr);
					if (team == nullptr)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
						continue;
					}
					if (team_get_clienttag(team) != clienttag)
						continue;

					t = team_get_level(team);
					if (t > atlevel)
						atlevel = t;
				}
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "Checking for highest level in Solo,Team,FFA,AT Ladder Stats");
			eventlog(eventlog_level_debug, __FUNCTION__, "Solo Level: {}, Team Level {}, FFA Level {}, Highest AT Team Level: {}", sololevel, teamlevel, ffalevel, atlevel);

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


		/* value = icons delimited by space */
		extern int account_set_user_iconstash(t_account * account, t_clienttag clienttag, char const * value)
		{
			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\iconstash");

			if (value)
				return account_set_strattr(account, key.c_str(), value);
			else
				return account_set_strattr(account, key.c_str(), "NULL");
		}
		extern int account_set_user_iconstash(t_account * account, t_clienttag clienttag, std::string value)
		{
			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\iconstash");

			if (!value.empty())
				return account_set_strattr(account, key.c_str(), value.c_str());
			else
				return account_set_strattr(account, key.c_str(), "NULL");
		}

		extern char const * account_get_user_iconstash(t_account * account, t_clienttag clienttag)
		{
			std::string key ("Record\\" + tag_uint_to_str2(clienttag) + "\\iconstash");

			char const * retval = account_get_strattr(account, key.c_str());
			if ((retval != nullptr) && (std::strcmp(retval, "NULL") != 0))
				return retval;
			else
				return nullptr;
		}

		//BlacKDicK 04/20/2003
		extern int account_set_user_icon(t_account * account, t_clienttag clienttag, char const * usericon)
		{
			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\userselected_icon");

			if (usericon)
				return account_set_strattr(account, key.c_str(), usericon);
			else
				return account_set_strattr(account, key.c_str(), "NULL");
		}
		extern int account_set_user_icon(t_account * account, t_clienttag clienttag, std::string usericon)
		{
			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\userselected_icon");

			if (!usericon.empty())
				return account_set_strattr(account, key.c_str(), usericon.c_str());
			else
				return account_set_strattr(account, key.c_str(), "NULL");
		}

		extern char const * account_get_user_icon(t_account * account, t_clienttag clienttag)
		{
			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\userselected_icon");

			char const * retval = account_get_strattr(account, key.c_str());
			if ((retval != nullptr) && ((std::strcmp(retval, "NULL") != 0)))
				return retval;
			else
				return nullptr;
		}

		// Ramdom - Nothing, Grean Dragon Whelp, Azure Dragon (Blue Dragon), Red Dragon, Deathwing, Nothing
		// Humans - Peasant, Footman, Knight, Archmage, Medivh, Nothing
		// Orcs - Peon, Grunt, Tauren, Far Seer, Thrall, Nothing
		// Undead - Acolyle, Ghoul, Abomination, Lich, Tichondrius, Nothing
		// Night Elves - Wisp, Archer, Druid of the Claw, Priestess of the Moon, Furion Stormrage, Nothing
		// Demons - Nothing, ???(which unit is nfgn), Infernal, Doom Guard, Pit Lord/Manaroth, Archimonde
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
			if (orcs >= humans && orcs >= undead && orcs >= nightelf && orcs >= random)
			{
				wins = orcs;
				race = 2;
			}
			else if (humans >= orcs && humans >= undead && humans >= nightelf && humans >= random)
			{
				wins = humans;
				race = 1;
			}
			else if (nightelf >= humans && nightelf >= orcs && nightelf >= undead && nightelf >= random)
			{
				wins = nightelf;
				race = 4;
			}
			else if (undead >= humans && undead >= orcs && undead >= nightelf && undead >= random)
			{
				wins = undead;
				race = 3;
			}
			else
			{
				wins = random;
				race = 0;
			}

			while (wins >= anongame_infos_get_ICON_REQ(level + 1, clienttag) && anongame_infos_get_ICON_REQ(level + 1, clienttag) > 0) level++;
			if (clienttag == CLIENTTAG_WAR3XP_UINT)
				number_ctag = 6;

			eventlog(eventlog_level_info, __FUNCTION__, "race -> {}; level -> {}; wins -> {}; profileicon -> {}", race, level, wins, profile_code[race + number_ctag][level]);

			return char_icon_to_uint(profile_code[race + number_ctag][level]);
		}

		extern unsigned int account_icon_to_profile_icon(char const * icon, t_account * account, t_clienttag ctag)
		{
			if (icon == nullptr)
				return account_get_icon_profile(account, ctag);


			const char * result;
			if (sizeof(icon) >= 4)
			{
				char tmp_icon[4];
				int number_ctag = 0;

				std::strncpy(tmp_icon, icon, 4);
				tmp_icon[0] = tmp_icon[0] - 48;
				if (ctag == CLIENTTAG_WAR3XP_UINT)
					number_ctag = 6;

				if (tmp_icon[0] >= 1)
				{
					if (tmp_icon[1] == 'R')
						result = profile_code[0 + number_ctag][tmp_icon[0] - 1];
					else if (tmp_icon[1] == 'H')
						result = profile_code[1 + number_ctag][tmp_icon[0] - 1];
					else if (tmp_icon[1] == 'O')
						result = profile_code[2 + number_ctag][tmp_icon[0] - 1];
					else if (tmp_icon[1] == 'U')
						result = profile_code[3 + number_ctag][tmp_icon[0] - 1];
					else if (tmp_icon[1] == 'N')
						result = profile_code[4 + number_ctag][tmp_icon[0] - 1];
					else if (tmp_icon[1] == 'D')
						result = profile_code[5 + number_ctag][tmp_icon[0] - 1];
					else
					{
						eventlog(eventlog_level_warn, __FUNCTION__, "got unrecognized race on [{}] icon ", icon);
						result = profile_code[2][0];
					} /* "opeo" */
				}
				else
				{
					eventlog(eventlog_level_warn, __FUNCTION__, "got race_level<1 on [{}] icon ", icon);
					result = nullptr;
				}
			}
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid icon length [{}] icon ", icon);
				result = nullptr;
			}

			return char_icon_to_uint(result);
		}

		extern int account_is_operator_or_admin(t_account * account, char const * channel)
		{
			if ((account_get_auth_operator(account, channel) == 1) || (account_get_auth_operator(account, nullptr) == 1) ||
				(account_get_auth_admin(account, channel) == 1) || (account_get_auth_admin(account, nullptr) == 1))
				return 1;
			else
				return 0;

		}

		static unsigned int char_icon_to_uint(const char * icon)
		{
			if (icon == nullptr)
				return 0;

			if (std::strlen(icon) != 4)
				return 0;

			unsigned int value;
			value = ((unsigned int)icon[0]) << 24;
			value |= ((unsigned int)icon[1]) << 16;
			value |= ((unsigned int)icon[2]) << 8;
			value |= ((unsigned int)icon[3]);

			return value;
		}

		extern char const * account_get_email(t_account * account)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return nullptr;
			}

			return account_get_strattr(account, "BNET\\acct\\email");
		}

		extern int account_set_email(t_account * account, char const * email)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			return account_set_strattr(account, "BNET\\acct\\email", email);
		}
		extern int account_set_email(t_account * account, std::string email)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			return account_set_strattr(account, "BNET\\acct\\email", email.c_str());
		}

		extern int account_get_email_verified(t_account* account)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			return account_get_boolattr(account, "BNET\\acct\\email\\verified");
		}

		extern int account_set_email_verified(t_account* account, bool is_verified)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			return account_set_boolattr(account, "BNET\\acct\\email\\verified", is_verified ? 1 : 0);
		}

		extern char const * account_get_emailverification_code(t_account* account)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return nullptr;
			}

			return account_get_strattr(account, "emailverification\\code");
		}

		extern int account_set_emailverification_code(t_account* account, char const * verification_code)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			if (verification_code == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL verification_code");
				return -1;
			}

			return account_set_strattr(account, "emailverification\\code", verification_code);
		}

		extern unsigned int account_get_emailverification_expiration(t_account* account)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return 0;
			}

			return account_get_numattr(account, "emailverification\\expiration");
		}

		extern int account_set_emailverification_expiration(t_account* account, unsigned int expiration_date)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}

			return account_set_numattr(account, "emailverification\\expiration", expiration_date);
		}

		extern int account_set_userlang(t_account * account, const char * lang)
		{
			if (lang)
				return account_set_strattr(account, "BNET\\acct\\userlang", lang);
			else
				return account_set_strattr(account, "BNET\\acct\\userlang", "NULL");
		}
		extern int account_set_userlang(t_account * account, std::string lang)
		{
			if (!lang.empty())
				return account_set_strattr(account, "BNET\\acct\\userlang", lang.c_str());
			else
				return account_set_strattr(account, "BNET\\acct\\userlang", "NULL");
		}

		extern char const * account_get_userlang(t_account * account)
		{
			char const * retval = account_get_strattr(account, "BNET\\acct\\userlang");

			if ((retval != nullptr) && (std::strcmp(retval, "NULL") != 0))
				return retval;
			else
				return nullptr;
		}

		/**
		*  Westwood Online Extensions
		*/
		extern char const * account_get_wol_apgar(t_account * account)
		{
			if (account == nullptr)
			{
				ERROR0("got NULL account");
				return nullptr;
			}

			return account_get_strattr(account, "WOL\\auth\\apgar");
		}

		extern int account_set_wol_apgar(t_account * account, char const * apgar)
		{
			if (account == nullptr)
			{
				ERROR0("got NULL account");
				return -1;
			}
			if (apgar == nullptr)
			{
				ERROR0("got NULL apgar");
				return -1;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "[** WOL **] WOL\\auth\\apgar = {}", apgar);
			return account_set_strattr(account, "WOL\\auth\\apgar", apgar);
		}
		extern int account_set_wol_apgar(t_account * account, std::string apgar)
		{
			if (account == nullptr)
			{
				ERROR0("got NULL account");
				return -1;
			}
			if (apgar.empty())
			{
				ERROR0("got NULL apgar");
				return -1;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "[** WOL **] WOL\\auth\\apgar = {}", apgar.c_str());
			return account_set_strattr(account, "WOL\\auth\\apgar", apgar.c_str());
		}

		extern int account_get_locale(t_account * account)
		{
			if (account == nullptr)
			{
				ERROR0("got NULL account");
				return 0;
			}

			return account_get_numattr(account, "WOL\\acct\\locale");
		}

		extern int account_set_locale(t_account * account, int locale)
		{
			if (account == nullptr)
			{
				ERROR0("got NULL account");
				return -1;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "[** WOL **] WOL\\acct\\locale = {}", locale);
			return account_set_numattr(account, "WOL\\acct\\locale", locale);
		}

		extern int account_get_ladder_points(t_account * account, t_clienttag clienttag, t_ladder_id id)
		{
			std::string key("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\points");

			return account_get_numattr(account, key.c_str());
		}

		extern int account_set_ladder_points(t_account * account, t_clienttag clienttag, t_ladder_id id, unsigned int points)
		{
			std::string key ("Record\\" + tag_uint_to_str2(clienttag) + "\\" + ladder_id_str.at(static_cast<size_t>(id)) + "\\points");
			return account_set_numattr(account, key.c_str(), points);
		}

	}

}
