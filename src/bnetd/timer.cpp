/*
 * Copyright (C) 1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2004 Dizzy
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
#define TIMER_INTERNAL_ACCESS
#include "common/setup_before.h"
#include "timer.h"
#include <cstdlib>
#include "common/elist.h"
#include "connection.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static t_elist timerlist_head;


		extern int timerlist_add_timer(t_connection * owner, std::time_t when, t_timer_cb cb, t_timer_data data)
		{
			t_timer * timer, *ctimer;
			t_elist * curr;

			if (!owner)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL owner");
				return -1;
			}

			timer = (t_timer*)xmalloc(sizeof(t_timer));
			timer->owner = owner;
			timer->when = when;
			timer->cb = cb;
			timer->data = data;

			/* try to preserve a when based order */
			elist_for_each(curr, &timerlist_head)
			{
				ctimer = elist_entry(curr, t_timer, timers);
				if (ctimer->when > when) break;
			}
			elist_add_tail(curr, &timer->timers);

			/* add it to the t_conn timers list */
			elist_add_tail(conn_get_timer(owner), &timer->owners);

			return 0;
		}


		extern int timerlist_del_all_timers(t_connection * owner)
		{
			t_elist * curr, *save;
			t_timer * timer;

			if (!owner)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL owner");
				return -1;
			}

			elist_for_each_safe(curr, conn_get_timer(owner), save)
			{
				timer = elist_entry(curr, t_timer, owners);
				if (timer->cb)
					timer->cb(timer->owner, (std::time_t)0, timer->data);
				elist_del(&timer->owners);
				elist_del(&timer->timers);
				xfree((void*)timer);
			}

			return 0;
		}


		extern int timerlist_check_timers(std::time_t when)
		{
			t_elist * curr, *save;
			t_timer * timer;

			elist_for_each_safe(curr, &timerlist_head, save)
			{
				timer = elist_entry(curr, t_timer, timers);
				if (timer->owner && timer->when < when)
				{
					if (timer->cb)
						timer->cb(timer->owner, timer->when, timer->data);
					elist_del(&timer->owners);
					elist_del(&timer->timers);
					xfree((void*)timer);
				}
				else break; /* beeing sorted there is no need to go beyond this point */
			}

			return 0;
		}

		extern int timerlist_create(void)
		{
			elist_init(&timerlist_head);
			return 0;
		}


		extern int timerlist_destroy(void)
		{
			t_elist * curr, *save;
			t_timer * timer;

			elist_for_each_safe(curr, &timerlist_head, save)
			{
				timer = elist_entry(curr, t_timer, timers);
				elist_del(&timer->owners);
				elist_del(&timer->timers);
				xfree((void*)timer);
			}
			elist_init(&timerlist_head);

			return 0;
		}

	}

}
