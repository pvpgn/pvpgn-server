/*
 * Copyright (C) 1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#include "tick.h"

#include <cerrno>
#include <cstddef>
#include <cstring>

#include "compat/gettimeofday.h"
#include "common/eventlog.h"

#include "common/setup_after.h"


namespace pvpgn
{

	namespace bnetd
	{

		/*
		 * This routine returns the number of miliseconds that have passed since one second
		 * before it is first called. This is used for timing fields in some packets.
		 */
		extern unsigned int get_ticks(void)
		{
			static int first = 1;
			static long beginsec;
			struct timeval tv;

			if (gettimeofday(&tv, NULL) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not get std::time (gettimeofday: %s)", std::strerror(errno));
				return 0;
			}
			if (first)
			{
				beginsec = tv.tv_sec - 1;
				first = 0;
			}

			return (unsigned int)((tv.tv_sec - beginsec) * 1000 + tv.tv_usec / 1000);
		}

	}

}
