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
#include "account_email_verification.h"

#include <fstream>
#include <iterator>
#include <limits>
#include <random>
#include <string>

#include <curl/curl.h>

#include "common/eventlog.h"

#include "account.h"
#include "account_wrap.h"
#include "prefs.h"
#include "server.h"
#include "smtp.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static std::string message;
		static std::string verify_account_email_from_address;
		static std::string verify_account_email_from_name;

		bool account_email_verification_load(const char* filepath, const char* prefs_servername, const char* prefs_verify_account_email_from_address, const char* prefs_verify_account_email_from_name)
		{
			if (filepath == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL filepath");
				return false;
			}

			if (prefs_servername == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL prefs_servername");
				return false;
			}

			if (prefs_verify_account_email_from_address == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL prefs_verify_account_email_from_address");
				return false;
			}

			if (prefs_verify_account_email_from_name == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL prefs_verify_account_email_from_name");
				return false;
			}

			std::ifstream infile{ filepath };
			if (!infile.is_open())
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Could not open email verification message file ({})", filepath);
				return false;
			}

			try
			{
				std::string raw_message{ std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>() };
				message = fmt::format(raw_message, fmt::arg("pvpgn_server_name", prefs_servername));

				eventlog(eventlog_level_info, __FUNCTION__, "Successfully loaded email verification message ({} bytes)", message.length());
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Failed to load email verification message ({})", e.what());
				return false;
			}

			verify_account_email_from_address = prefs_verify_account_email_from_address;
			verify_account_email_from_name = prefs_verify_account_email_from_name;

			return true;
		}

		void account_email_verification_unload()
		{
			message.clear();
			verify_account_email_from_address.clear();
		}

		/**
		* If successful, sets "BNET\\acct\\email\\verified" attribute to true and sets "emailverification\\code"
		* attribute and "emailverification\\expiration" attribute to 0 for the given account.
		*/
		AccountVerifyEmailStatus account_verify_email(t_account* account, const std::string& code)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return AccountVerifyEmailStatus::FailureOther;
			}

			const char* account_code = account_get_emailverification_code(account);
			if (account_code == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Could not retrieve email verification code for account uid {}", account_get_uid(account));
				return AccountVerifyEmailStatus::FailureOther;
			}

			unsigned int account_expiration = account_get_emailverification_expiration(account);
			if (account_expiration == 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Could not retrieve email verification expiration date for account uid {}", account_get_uid(account));
				return AccountVerifyEmailStatus::FailureOther;
			}

			if (now >= account_expiration)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Failed to verify email for account uid {} (Email verification code expired)", account_get_uid(account));
				return AccountVerifyEmailStatus::FailureCodeExpired;
			}

			if (code.compare(account_code) != 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Failed to verify email for account uid {} (Incorrect email verification code)", account_get_uid(account));
				return AccountVerifyEmailStatus::FailureCodeIncorrect;
			}

			int verified = account_set_email_verified(account, true);
			if (verified != 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Could not verify email code for account uid {}", account_get_uid(account));
				return AccountVerifyEmailStatus::FailureOther;
			}

			account_set_emailverification_code(account, "");
			account_set_emailverification_expiration(account, 0);

			eventlog(eventlog_level_info, __FUNCTION__, "Successfully verified email address ({}) account uid {}", account_get_email(account), account_get_uid(account));

			return AccountVerifyEmailStatus::Success;
		}

		/**
		* Generates an email verification code and saves it in the "emailverification\\code" attribute for the given account. Sets the "emailverification\\expiration" attribute
		* to X minutes from current time, where X is the value of 'verify_account_email_expiration' in bnetd.conf.
		* Sends an email containing the email verification code to the registered email address of the account.
		*/
		bool account_generate_email_verification_code(t_account* account)
		{
			if (account == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return false;
			}

			static std::random_device rdevice;
			static std::default_random_engine rengine(rdevice());
			static std::uniform_int_distribution<unsigned int> uniform_dist(100000, 999999);

			std::time_t expiration = now + (60ull * prefs_get_verify_account_email_expiration());
			std::string code = fmt::to_string(uniform_dist(rengine));

			int a = account_set_emailverification_expiration(account, expiration);
			int b = account_set_emailverification_code(account, code.c_str());
			if (!(a == 0 && b == 0))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Could not generate an email verification code for account uid {}", account_get_uid(account));
				return false;
			}

			std::string personalized_message = fmt::format(message, fmt::arg("account_name", account_get_name(account)), fmt::arg("account_email_verification_code", code), fmt::arg("account_email_verification_expiration", prefs_get_verify_account_email_expiration()));

			eventlog(eventlog_level_debug, __FUNCTION__, "Sending email verification code to {} for account uid {}", account_get_email(account), account_get_uid(account));
			smtp_send_email(account_get_email(account), verify_account_email_from_address, verify_account_email_from_name, "Email Verification", personalized_message);

			return true;
		}

	}
}