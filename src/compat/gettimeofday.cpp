/*
 * Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef HAVE_GETTIMEOFDAY

#include "gettimeofday.h"
#include <cerrno>
#include <ctime>

#ifdef HAVE_SYS_TIMEB_H
# include <sys/timeb.h>
#endif



#include "common/setup_after.h"

namespace pvpgn
{

	extern int gettimeofday(struct timeval * tv, struct timezone * tz)
	{
#ifdef HAVE_FTIME
		struct timeb tb;
#endif

		if (!tv)
		{
			errno = EFAULT;
			return -1;
		}

#ifdef HAVE_FTIME
		tb.millitm = 0; /* apparently the MS CRT version of this doesn't set this member */
		/* FIXME: what would be a more appropriate function for that platform? */
		ftime(&tb); /* FIXME: some versions are void return others int */

		tv->tv_sec = tb.time;
		tv->tv_usec = ((long)tb.millitm) * 1000;
		if (tz)
		{
			tz->tz_minuteswest = 0;
			tz->tz_dsttime = 0;
		}

		return 0;
#else
# error "This program requires either gettimeofday() or ftime()"
#endif
	}

}

#else
typedef int filenotempty; /* make ISO standard happy */
#endif
