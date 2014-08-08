/*
 * Copyright (C) 1999  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2005 Dizzy
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
#ifndef PVPGN_BNETD_WATCH_H
#define PVPGN_BNETD_WATCH_H

#include <list>

#include "common/scoped_ptr.h"

#include "account.h"
#include "connection.h"
#include "common/tag.h"

namespace pvpgn
{

	namespace bnetd
	{

		class Watch
		{
		public:
			enum EventType
			{
				ET_login = 1,
				ET_logout = 2,
				ET_joingame = 4,
				ET_leavegame = 8
			};

			Watch(t_connection* owner_, t_account* who_, unsigned what_, t_clienttag ctag_);
			~Watch() throw();

			t_connection* getOwner() const;
			t_account* getAccount() const;
			unsigned getEventMask() const;
			t_clienttag getClientTag() const;

			void setEventMask(unsigned what_);

		private:
			t_connection * owner; /* who to notify */
			t_account *    who;   /* when this account */
			unsigned       what;  /* does one of these things */
			t_clienttag    ctag;  /* while logged in with this clienttag (0 for any) */
		};

		class WatchComponent
		{
		public:
			WatchComponent();
			~WatchComponent() throw();

			void add(t_connection * owner, t_account * who, t_clienttag clienttag, unsigned events);
			int del(t_connection * owner, t_account * who, t_clienttag clienttag, unsigned events);
			void del(t_connection * owner);
			int dispatch(t_account * who, char const * gamename, t_clienttag clienttag, Watch::EventType event) const;

		private:
			typedef std::list<Watch> WatchList;

			WatchList wlist;

			int dispatch_whisper(t_account *account, char const *gamename, t_clienttag clienttag, Watch::EventType event) const;
		};

		extern scoped_ptr<WatchComponent> watchlist;

	}

}

#endif
