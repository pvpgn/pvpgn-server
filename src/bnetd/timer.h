/*
 * Copyright (C) 1999  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_TIMER_TYPES
#define INCLUDED_TIMER_TYPES

#include <ctime>

#ifdef JUST_NEED_TYPES
# include "connection.h"
#else
# define JUST_NEED_TYPES
# include "connection.h"
# undef JUST_NEED_TYPES
#endif
#include "common/elist.h"

namespace pvpgn
{

	namespace bnetd
	{

		typedef union
		{
			unsigned long n;
			void *        p;
		} t_timer_data;

		typedef void(*t_timer_cb)(t_connection * owner, std::time_t when, t_timer_data data);

		typedef struct timer_struct
#ifdef TIMER_INTERNAL_ACCESS
		{
			t_connection * owner; 	/* who to notify */
			std::time_t         when;  	/* when the timer expires */
			t_timer_cb     cb;    	/* what to call */
			t_timer_data   data;  	/* data argument */
			t_elist	   owners;	/* list to the setup timers of same owner */
			t_elist	   timers;	/* timers list, used for final cleaning */
		}
#endif
		t_timer;

	}

}

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_TIMER_PROTOS
#define INCLUDED_TIMER_PROTOS

#include <ctime>

#define JUST_NEED_TYPES
#include "connection.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern int timerlist_create(void);
		extern int timerlist_destroy(void);
		extern int timerlist_add_timer(t_connection * owner, std::time_t when, t_timer_cb cb, t_timer_data data);
		extern int timerlist_del_all_timers(t_connection * owner);
		extern int timerlist_check_timers(std::time_t when);

	}

}

#endif
#endif
