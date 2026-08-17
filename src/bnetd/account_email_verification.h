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


#ifndef INCLUDED_EMAIL_VERIFICATION_H
#define INCLUDED_EMAIL_VERIFICATION_H

#ifndef JUST_NEED_TYPES
#define JUST_NEED_TYPES

#include "account.h"

#undef JUST_NEED_TYPES
#endif

#include <string>

namespace pvpgn
{

    namespace bnetd
    {

        enum class AccountVerifyEmailStatus
        {
            Success,
            FailureCodeExpired,
            FailureCodeIncorrect,
            FailureOther
        };

        bool account_email_verification_load(const char* filepath, const char* prefs_servername, const char* prefs_verify_account_email_from_address, const char* prefs_verify_account_email_from_name);
        void account_email_verification_unload();
        AccountVerifyEmailStatus account_verify_email(t_account* account, const std::string& code);
        bool account_generate_email_verification_code(t_account* account);

    }

}

#endif