/*
 * Copyright (C) 2001,2006       Dizzy
 * Copyright (C) 2004            Donny Redmond (dredmond@linuxmail.org)
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
#ifndef INCLUDED_PVPGN_BNETD_MAIL
#define INCLUDED_PVPGN_BNETD_MAIL


#define MAX_FUNC_LEN 10
#define MAX_MAIL_QUOTA 10
#define MAIL_FUNC_SEND 1
#define MAIL_FUNC_READ 2
#define MAIL_FUNC_DELETE 3
#define MAIL_FUNC_UNKNOWN 4


#include <string>
#include <stdexcept>
#include <deque>
#include <ctime>

#include "compat/pdir.h"
#include "connection.h"

namespace pvpgn
{

	namespace bnetd
	{

		class Mail
		{
		public:
			Mail(const std::string& sender, const std::string& mess, const std::time_t timestamp);
			~Mail() throw();

			const std::string& sender() const;
			const std::string& message() const;
			const std::time_t& timestamp() const;

		private:
			const std::string sender_;
			const std::string message_;
			const std::time_t timestamp_;
		};

		typedef std::deque<Mail> MailList;

		class Mailbox {
		public:
			class ReadError : public std::runtime_error {
			public:
				ReadError(const std::string& mess)
					:std::runtime_error(mess) {}
				~ReadError() throw() {}
			};

			class DeliverError : public std::runtime_error {
			public:
				DeliverError(const std::string& mess)
					:std::runtime_error(mess) {}
				~DeliverError() throw() {}
			};

			explicit Mailbox(unsigned uid);
			~Mailbox() throw();

			unsigned size() const;
			bool empty() const;
			void deliver(const std::string& sender, const std::string& mess);
			Mail read(unsigned int) const;
			void readAll(MailList& dest) const;
			void erase(unsigned int);
			void clear();

		private:
			unsigned uid;
			const std::string path;
			mutable Directory mdir;

			std::string buildPath(const std::string& root) const;
			void createOpenDir();
			Mail read(const std::string& fname, const std::time_t& timestamp) const;

			Mailbox(const Mailbox&);
			Mailbox& operator=(const Mailbox&);
		};


		extern int handle_mail_command(t_connection *, char const *);
		extern unsigned check_mail(t_connection const * c);

	}

}

#endif
