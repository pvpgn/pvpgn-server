/*
 * Copyright (C) 1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#include "common/setup_before.h"
#include "watch.h"
#include "common/field_sizes.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "message.h"
#include "friends.h"
#include "prefs.h"
#include "common/setup_after.h"


namespace pvpgn
{

	namespace bnetd
	{

		scoped_ptr<WatchComponent> watchlist;

		Watch::Watch(t_connection* owner_, t_account* who_, unsigned what_, t_clienttag ctag_)
			:owner(owner_), who(who_), what(what_), ctag(ctag_)
		{
		}

		Watch::~Watch() throw()
		{
		}

		t_connection*
			Watch::getOwner() const
		{
				return owner;
			}

		t_account*
			Watch::getAccount() const
		{
				return who;
			}

		unsigned
			Watch::getEventMask() const
		{
				return what;
			}

		t_clienttag
			Watch::getClientTag() const
		{
				return ctag;
			}

		void
			Watch::setEventMask(unsigned what_)
		{
				what = what_;
			}

		/* who == NULL means anybody */
		void
			WatchComponent::add(t_connection * owner, t_account * who, t_clienttag clienttag, unsigned events)
		{
				for (WatchList::iterator it(wlist.begin()); it != wlist.end(); ++it)
				{
					if (owner == it->getOwner() && who == it->getAccount() && clienttag == it->getClientTag())
					{
						it->setEventMask(it->getEventMask() | events);
						return;
					}
				}

				wlist.push_back(Watch(owner, who, events, clienttag));
			}


		/* who == NULL means anybody */
		int
			WatchComponent::del(t_connection * owner, t_account * who, t_clienttag clienttag, unsigned events)
		{
				for (WatchList::iterator it(wlist.begin()); it != wlist.end(); ++it)
				{
					if (owner == it->getOwner() && who == it->getAccount() && (!clienttag || clienttag == it->getClientTag()))
					{
						unsigned evmask = it->getEventMask() & (~events);
						if (!evmask) wlist.erase(it);
						else it->setEventMask(evmask);
						return 0;
					}
				}

				return -1;
			}


		/* this differs from del_events because it doesn't return an error if nothing was found */
		void
			WatchComponent::del(t_connection * owner)
		{
				for (WatchList::iterator it(wlist.begin()); it != wlist.end();)
				{
					if (owner == it->getOwner())
						it = wlist.erase(it);
					else ++it;
				}
			}

		int
			WatchComponent::dispatch_whisper(t_account *account, char const *gamename, t_clienttag clienttag, Watch::EventType event) const
		{
				t_elem const * curr;
				char msg[512];
				int cnt = 0;
				char const *myusername;
				t_list * flist;
				t_connection * dest_c, *my_c;
				t_friend * fr;
				char const * game_title;

				if (!(myusername = account_get_name(account)))
				{
					ERROR0("got NULL account name");
					return -1;
				}

				my_c = account_get_conn(account);

				game_title = clienttag_get_title(clienttag);

				/* mutual friends handling */
				flist = account_get_friends(account);
				if (flist)
				{
					switch (event)
					{
					case Watch::ET_joingame:
						if (gamename)
							std::sprintf(msg, "Your friend %s has entered a %s game named \"%s\".", myusername, game_title, gamename);
						else
							std::sprintf(msg, "Your friend %s has entered a %s game", myusername, game_title);
						break;
					case Watch::ET_leavegame:
						std::sprintf(msg, "Your friend %s has left a %s game.", myusername, game_title);
						break;
					case Watch::ET_login:
						std::sprintf(msg, "Your friend %s has entered %s.", myusername, prefs_get_servername());
						break;
					case Watch::ET_logout:
						std::sprintf(msg, "Your friend %s has left %s.", myusername, prefs_get_servername());
						break;
					}
					LIST_TRAVERSE(flist, curr)
					{
						if (!(fr = (t_friend*)elem_get_data(curr)))
						{
							ERROR0("found NULL entry in list");
							continue;
						}

						dest_c = connlist_find_connection_by_account(fr->friendacc);

						if (dest_c == NULL) /* If friend is offline, go on to next */
							continue;
						else {
							cnt++;	/* keep track of successful whispers */
							if (friend_get_mutual(fr))
								message_send_text(dest_c, message_type_whisper, my_c, msg);
						}
					}
				}

				if (cnt) DEBUG2("notified %d friends about %s", cnt, myusername);

				/* watchlist handling */
				switch (event)
				{
				case Watch::ET_joingame:
					if (gamename)
						std::sprintf(msg, "Watched user %s has entered a %s game named \"%s\".", myusername, game_title, gamename);
					else
						std::sprintf(msg, "Watched user %s has entered a %s game", myusername, game_title);
					break;
				case Watch::ET_leavegame:
					std::sprintf(msg, "Watched user %s has left a %s game.", myusername, game_title);
					break;
				case Watch::ET_login:
					std::sprintf(msg, "Watched user %s has entered %s.", myusername, prefs_get_servername());
					break;
				case Watch::ET_logout:
					std::sprintf(msg, "Watched user %s has left %s", myusername, prefs_get_servername());
					break;
				}

				for (WatchList::const_iterator it(wlist.begin()); it != wlist.end(); ++it)
				{
					if (it->getOwner() && (!it->getAccount() || it->getAccount() == account) && (!it->getClientTag() || (clienttag == it->getClientTag())) && (it->getEventMask() & event))
					{
						message_send_text(it->getOwner(), message_type_whisper, my_c, msg);
					}
				}

				return 0;
			}

		int
			WatchComponent::dispatch(t_account * who, char const * gamename, t_clienttag clienttag, Watch::EventType event) const
		{
				switch (event)
				{
				case Watch::ET_login:
				case Watch::ET_logout:
				case Watch::ET_joingame:
				case Watch::ET_leavegame:
					return dispatch_whisper(who, gamename, clienttag, event);
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "got unknown event %u", (unsigned int)event);
					return -1;
				}
				return 0;
			}


		WatchComponent::WatchComponent()
			:wlist()
		{
		}


		WatchComponent::~WatchComponent() throw ()
		{
		}

	}

}
