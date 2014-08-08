/*
 * Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
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
#define BNETTIME_INTERNAL_ACCESS
#include "common/setup_before.h"
#include "common/bnettime.h"

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <ctime>

#include "compat/gettimeofday.h"
#include "common/eventlog.h"
#include "common/bn_type.h"
#include "common/setup_after.h"


namespace pvpgn
{

	/*
	 * By comparing the hex numbers from packet dumps with the times displayed
	 * on the login screen, bnettime seems to be in units of 10E-7 seconds. It
	 * is stored in a 64bit value, and represented as two distinct numbers when
	 * it is used as a string in the protocol.
	 *
	 * examples (client in GMT timezone):
	 * 11/04/99 05:16    0x01                  "29304451 3046165090"
	 * 01/03/99 11:19    0x01                  "29243146 3825346784"
	 * 08/02/99 21:28    0x01                  "29285677 4283311890"
	 * 12/12/99 01:55    0x01                  "29312068 151587081"
	 *
	 * The top number seems to be in units of about 7 minutes because the bottom
	 * number rolls over every 2^32 10E-7 seconds, which is:
	 * (2^32)*.0000001
	 * 429.4967296 // seconds
	 * 429/60
	 * 7.15827882666666666666 // minutes
	 *
	 * The epoch was determined using a binary search looking for Jan 1, 1970
	 * (GMT) to be displayed after setting the last game time to a pair of
	 * numbers. It was determined through all 64 bits by looking for the exact
	 * number where adding one pushes it over to 00:01. It was determined to be:
	 * "27111902 3577643008"
	 *
	 * It appears that this is the same format as is used in MS-DOS to store file
	 * times. The format is a 64 bit number telling how many 100 nanosecond
	 * intervals since Jan 1, 1601.
	 *
	 * If I was clever I should be able to do this without floating point math.
	 * I don't feel like being clever though :) (Refer to the function
	 * DOSFS_UnixTimeToFileTime() in dosfs.c in the WINE source code for an
	 * example that I wish I had seen before writing this).
	 *
	 * Update... from looking at the WINE sources VAP has determined that these
	 * are actually in MS-Windows FileTime format.
	 *
	 * "In the WINE listing, you can see that
	 *  FileTime = (UnixTime * 10 000 000) + 116 444 736 000 000 000 + remainder
	 *  These dates are represented from 1 Jan 1601."
	 */

#define SEC_PER_USEC .0000001

#define UNIX_EPOCH 11644473600. /* seconds the Unix epoch is from the bnettime epoch */

#define SEC_PER_UPPER 429.4967296 /* (2^32)*10E-7 */
#define SEC_PER_LOWER .0000001

#define UPPER_PER_SEC .00232830643653869628  /* 10E7/(2^32) */
#define LOWER_PER_SEC 10000000.


	extern t_bnettime secs_to_bnettime(double secs)
	{
		t_bnettime temp;

		temp.u = (unsigned int)(secs*UPPER_PER_SEC);
		temp.l = (unsigned int)(secs*LOWER_PER_SEC);

		return temp;
	}


	extern double bnettime_to_secs(t_bnettime bntime)
	{
		return ((double)bntime.u)*SEC_PER_UPPER +
			((double)bntime.l)*SEC_PER_LOWER;
	}


	extern t_bnettime time_to_bnettime(std::time_t stdtime, unsigned int usec)
	{
		return secs_to_bnettime((double)stdtime + (double)usec*SEC_PER_USEC + UNIX_EPOCH);
	}


	extern std::time_t bnettime_to_time(t_bnettime bntime)
	{
		return (std::time_t)(bnettime_to_secs(bntime) - UNIX_EPOCH);
	}


	/* return current time as bnettime */
	extern t_bnettime bnettime(void)
	{
		struct timeval tv;

		if (gettimeofday(&tv, NULL) < 0)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not get time (gettimeofday: %s)", std::strerror(errno));
			return time_to_bnettime(std::time(NULL), 0);
		}
		return time_to_bnettime((std::time_t)tv.tv_sec, tv.tv_usec);
	}


	/* FIXME: the string functions here should probably go in account_wrap */
	extern char const * bnettime_get_str(t_bnettime bntime)
	{
		static char temp[1024];

		std::sprintf(temp, "%u %u", bntime.u, bntime.l);

		return temp;
	}


	extern int bnettime_set_str(t_bnettime * bntime, char const * timestr)
	{
		if (!bntime)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL bntime");
			return -1;
		}
		if (!timestr)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL timestr");
			return -1;
		}

		if (std::sscanf(timestr, "%u %u", &bntime->u, &bntime->l) != 2)
			return -1;

		return 0;
	}


	extern void bnettime_to_bn_long(t_bnettime in, bn_long * out)
	{
		if (!out)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL out");
			return;
		}

		bn_long_set_a_b(out, in.u, in.l);
	}


	extern void bn_long_to_bnettime(bn_long in, t_bnettime * out)
	{
		if (!out)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL out");
			return;
		}

		out->u = bn_long_get_a(in);
		out->l = bn_long_get_b(in);
	}


	extern int local_tzbias(void) /* in minutes */
	{
		std::time_t      now;
		std::time_t      test;
		std::time_t      testloc;
		struct std::tm * temp;

		now = std::time(NULL);

		if (!(temp = std::gmtime(&now)))
			return 0;
		if ((test = std::mktime(temp)) == (std::time_t)(-1))
			return 0;

		if (!(temp = std::localtime(&now)))
			return 0;
		if ((testloc = std::mktime(temp)) == (std::time_t)(-1))
			return 0;

		if (testloc > test) /* std::time_t is probably unsigned... */
			return -(int)(testloc - test) / 60;
		return (int)(test - testloc) / 60;
	}


	extern t_bnettime bnettime_add_tzbias(t_bnettime bntime, int tzbias)
	{
		return secs_to_bnettime(bnettime_to_secs(bntime) - (double)tzbias*60.0);
	}

}
