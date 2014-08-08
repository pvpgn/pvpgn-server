/*
 * Copyright (C) 2001,2006  Dizzy
 * Copyright (C) 2004  Donny Redmond (dredmond@linuxmail.org)
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
#include "mail.h"

#include <sstream>
#include <fstream>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <algorithm>
#include <vector>

#include "compat/strcasecmp.h"
#include "compat/mkdir.h"
#include "compat/statmacros.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/xstring.h"
#include "account.h"
#include "message.h"
#include "prefs.h"
#include "connection.h"
#include "helpfile.h"
#include "command.h"
#include "i18n.h"
#include "common/setup_after.h"


namespace pvpgn
{

	namespace bnetd
	{

		static int identify_mail_function(const std::string&);
		static void mail_usage(t_connection*);
		static void mail_func_send(t_connection*, char const *, char const *);
		static void mail_func_read(t_connection*, std::string);
		static void mail_func_delete(t_connection*, std::string);
		static unsigned get_mail_quota(t_account *);

		/* Mail API */
		/* for now this functions are only for internal use */

		Mail::Mail(const std::string& sender, const std::string& message, const std::time_t timestamp)
			:sender_(sender), message_(message), timestamp_(timestamp)
		{
		}

		Mail::~Mail() throw()
		{
		}

		const std::string&
			Mail::sender() const
		{
				return sender_;
			}

		const std::string&
			Mail::message() const
		{
				return message_;
			}

		const std::time_t&
			Mail::timestamp() const
		{
				return timestamp_;
			}

		Mailbox::Mailbox(unsigned uid_)
			:uid(uid_), path(buildPath(prefs_get_maildir())), mdir(path, true)
		{
		}

		std::string
			Mailbox::buildPath(const std::string& root) const
		{
				std::ostringstream ostr;
				ostr << root << "/" << std::setfill('0') << std::setw(6) << uid;
				return ostr.str();
			}

		void
			Mailbox::createOpenDir()
		{
				p_mkdir(prefs_get_maildir(), S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
				p_mkdir(path.c_str(), S_IRWXU | S_IXGRP | S_IRGRP | S_IROTH | S_IXOTH);
				mdir.open(path, false);
			}

		Mailbox::~Mailbox() throw()
		{
		}

		unsigned
			Mailbox::size() const
		{
				mdir.rewind();

				unsigned count = 0;
				while ((mdir.read())) count++;
				return count;
			}

		bool
			Mailbox::empty() const
		{
				mdir.rewind();

				if (mdir.read()) return false;
				return true;
			}

		void
			Mailbox::deliver(const std::string& sender, const std::string& mess)
		{
				if (!mdir)
					try {
					createOpenDir();
				}
				catch (const Directory::OpenError&) {
					ERROR1("could not (re)open directory: '%s'", path.c_str());
					throw DeliverError("could not (re)open directory: " + path);
				}

				std::ostringstream ostr;
				ostr << path << '/' << std::setfill('0') << std::setw(15) << static_cast<unsigned long>(std::time(0));

				std::ofstream fd(ostr.str().c_str());
				if (!fd) {
					ERROR1("error opening mail file. check permissions: '%s'", path.c_str());
					throw DeliverError("error opening mail file. check permissions: " + path);
				}

				fd << sender << std::endl << mess << std::endl;
			}

		Mail
			Mailbox::read(const std::string& fname, const std::time_t& timestamp) const
		{
				std::ifstream fd(fname.c_str());
				if (!fd) {
					ERROR1("error opening mail file: %s", fname.c_str());
					throw ReadError("error opening mail file: " + fname);
				}

				std::string sender;
				std::getline(fd, sender);

				std::string message;
				std::getline(fd, message);

				return Mail(sender, message, timestamp);
			}

		Mail
			Mailbox::read(unsigned int idx) const
		{
				mdir.rewind();
				const char * dentry = mdir.read();
				for (unsigned i = 0; i < idx && (dentry = mdir.read());)
				if (dentry[0] != '.') ++i;
				if (!dentry) {
					INFO0("mail not found");
					throw ReadError("mail not found");
				}

				std::string fname(path);
				fname += '/';
				fname += dentry;

				return read(fname, std::atoi(dentry));
			}

		void
			Mailbox::readAll(MailList& dest) const
		{
				mdir.rewind();

				std::string fname(path);
				fname += '/';

				const char* dentry;
				while ((dentry = mdir.read())) {
					if (dentry[0] == '.') continue;

					try {
						dest.push_back(read(fname + dentry, std::atoi(dentry)));
					}
					catch (const ReadError&) {
						/* ignore ReadError in reading a specific message and try to read as much as we can */
					}
				}
			}

		void
			Mailbox::erase(unsigned int idx)
		{
				mdir.rewind();
				const char* dentry = mdir.read();
				for (unsigned i = 0; i < idx && (dentry = mdir.read());)
				if (dentry[0] != '.') ++i;

				if (!dentry) {
					WARN0("index out of range");
					return;
				}

				std::string fname(path);
				fname += '/';
				fname += dentry;

				if (std::remove(fname.c_str()) < 0)
					INFO2("could not remove file \"%s\" (std::remove: %s)", fname.c_str(), std::strerror(errno));
			}

		void
			Mailbox::clear()
		{
				std::string fname(path);
				fname += '/';

				mdir.rewind();

				const char* dentry;
				while ((dentry = mdir.read())) {
					std::remove((fname + dentry).c_str());
				}
			}

		extern int handle_mail_command(t_connection * c, char const * text)
		{
			if (!prefs_get_mail_support()) {
				message_send_text(c, message_type_error, c, localize(c, "This server has NO mail support."));
				return -1;
			}

			std::istringstream istr(text);
			std::string token;

			char const *subcommand;

			std::vector<std::string> args = split_command(text, 3);
			if (args[1].empty())
			{
				describe_command(c, args[0].c_str());
				return -1;
			}
			subcommand = args[1].c_str(); // subcommand

			switch (identify_mail_function(subcommand)) {
			case MAIL_FUNC_SEND:
				if (args[3].empty())
				{
					describe_command(c, args[0].c_str());
					return -1;
				}
				mail_func_send(c, args[2].c_str(), args[3].c_str());
				break;
			case MAIL_FUNC_READ:
				mail_func_read(c, args[2]);
				break;
			case MAIL_FUNC_DELETE:
				if (args[2].empty())
				{
					describe_command(c, args[0].c_str());
					return -1;
				}
				mail_func_delete(c, args[2]);
				break;
			default:
				describe_command(c, args[0].c_str());
			}

			return 0;
		}

		static int identify_mail_function(const std::string& funcstr)
		{
			if (funcstr.empty() || !strcasecmp(funcstr.c_str(), "read") || !strcasecmp(funcstr.c_str(), "r"))
				return MAIL_FUNC_READ;
			if (!strcasecmp(funcstr.c_str(), "send") || !strcasecmp(funcstr.c_str(), "s"))
				return MAIL_FUNC_SEND;
			if (!strcasecmp(funcstr.c_str(), "delete") || !strcasecmp(funcstr.c_str(), "del"))
				return MAIL_FUNC_DELETE;

			return MAIL_FUNC_UNKNOWN;
		}

		static unsigned get_mail_quota(t_account * user)
		{
			int quota;
			char const * user_quota = account_get_strattr(user, "BNET\\auth\\mailquota");

			if (!user_quota) quota = prefs_get_mail_quota();
			else {
				quota = std::atoi(user_quota);
				if (quota < 1) quota = 1;
				if (quota > MAX_MAIL_QUOTA) quota = MAX_MAIL_QUOTA;
			}

			return quota;
		}

		static void mail_func_send(t_connection * c, char const * receiver, char const * message)
		{
			if (!c) {
				ERROR0("got NULL connection");
				return;
			}

			t_account * recv = accountlist_find_account(receiver);
			if (!recv) {
				message_send_text(c, message_type_error, c, localize(c, "Receiver UNKNOWN!"));
				return;
			}

			Mailbox mbox(account_get_uid(recv));
			if (get_mail_quota(recv) <= mbox.size()) {
				message_send_text(c, message_type_error, c, localize(c, "Receiver has reached his mail quota. Your message will NOT be sent."));
				return;
			}

			try {
				mbox.deliver(conn_get_username(c), message);
				message_send_text(c, message_type_info, c, localize(c, "Your mail has been sent successfully."));
			}
			catch (const Mailbox::DeliverError&) {
				message_send_text(c, message_type_error, c, localize(c, "There was an error completing your request!"));
			}
		}

		bool NonNumericChar(const char ch)
		{
			if (ch < '0' || ch > '9') return true;
			return false;
		}

		static void mail_func_read(t_connection * c, std::string token)
		{
			if (!c) {
				ERROR0("got NULL connection");
				return;
			}

			t_account * user = conn_get_account(c);
			Mailbox mbox(account_get_uid(user));

			if (token.empty()) { /* user wants to see the mail summary */
				if (mbox.empty()) {
					message_send_text(c, message_type_info, c, localize(c, "You have no mail."));
					return;
				}

				MailList mlist;
				mbox.readAll(mlist);

				std::ostringstream ostr;
				ostr << "You have " << mbox.size() << " messages. Your mail quote is set to " << get_mail_quota(user) << '.';
				message_send_text(c, message_type_info, c, ostr.str().c_str());
				message_send_text(c, message_type_info, c, localize(c, "ID    Sender          Date"));
				message_send_text(c, message_type_info, c, "-------------------------------------");

				for (MailList::const_iterator it(mlist.begin()); it != mlist.end(); ++it) {
					ostr.str("");
					ostr << std::setfill('0') << std::setw(2) << std::right << (it - mlist.begin()) << "    "
						<< std::setfill(' ') << std::setw(14) << std::left << it->sender() << ' ';
					char buff[128];
					std::strftime(buff, sizeof(buff), "%a %b %d %H:%M:%S %Y", std::localtime(&it->timestamp()));
					ostr << buff;
					message_send_text(c, message_type_info, c, ostr.str().c_str());
				}

				message_send_text(c, message_type_info, c, localize(c, "Use /mail read <ID> to read the content of any message"));
			}
			else { /* user wants to read a message */
				if (std::find_if(token.begin(), token.end(), NonNumericChar) != token.end()) {
					message_send_text(c, message_type_error, c, localize(c, "Invalid index. Please use /mail read <index> where <index> is a number."));
					return;
				}

				try {
					unsigned idx = std::atoi(token.c_str());
					Mail mail(mbox.read(idx));

					std::ostringstream ostr;
					ostr << "Message #" << idx << " from " << mail.sender() << " on ";
					char buff[128];
					std::strftime(buff, sizeof(buff), "%a %b %d %H:%M:%S %Y", std::localtime(&mail.timestamp()));
					ostr << buff << ':';
					message_send_text(c, message_type_info, c, ostr.str().c_str());
					message_send_text(c, message_type_info, c, mail.message().c_str());
				}
				catch (const Mailbox::ReadError&) {
					message_send_text(c, message_type_error, c, localize(c, "There was an error completing your request."));
				}
			}
		}

		static void mail_func_delete(t_connection * c, std::string token)
		{
			if (!c) {
				ERROR0("got NULL connection");
				return;
			}

			if (token.empty()) {
				message_send_text(c, message_type_error, c, localize(c, "Please specify which message to delete. Use the following syntax: /mail delete {<index>|all} ."));
				return;
			}

			t_account * user = conn_get_account(c);
			Mailbox mbox(account_get_uid(user));

			if (token == "all") {
				mbox.clear();
				message_send_text(c, message_type_info, c, localize(c, "Successfully deleted messages."));
			}
			else {
				if (std::find_if(token.begin(), token.end(), NonNumericChar) != token.end()) {
					message_send_text(c, message_type_error, c, localize(c, "Invalid index. Please use /mail delete {<index>|all} where <index> is a number."));
					return;
				}

				mbox.erase(std::atoi(token.c_str()));
				message_send_text(c, message_type_info, c, localize(c, "Succesfully deleted message."));
			}
		}


		extern unsigned check_mail(t_connection const * c)
		{
			if (!c) {
				ERROR0("got NULL connection");
				return 0;
			}

			if (t_account * account = conn_get_account(c))
				return Mailbox(account_get_uid(account)).size();

			return 0;
		}

	}

}
