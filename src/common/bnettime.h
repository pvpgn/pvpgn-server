/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
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
#ifndef INCLUDED_BNETTIME_TYPES
#define INCLUDED_BNETTIME_TYPES

namespace pvpgn
{

	typedef struct
	{
#ifdef BNETTIME_INTERNAL_ACCESS
		unsigned int u;
		unsigned int l;
#else
		unsigned int _private1;
		unsigned int _private2;
#endif
	} t_bnettime;

}

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_BNETTIME_PROTOS
#define INCLUDED_BNETTIME_PROTOS

#include <ctime>
#define JUST_NEED_TYPES
#include "common/bn_type.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	extern t_bnettime secs_to_bnettime(double secs);
	extern double bnettime_to_secs(t_bnettime bntime);
	extern t_bnettime time_to_bnettime(std::time_t stdtime, unsigned int usec);
	extern std::time_t bnettime_to_time(t_bnettime bntime);
	extern t_bnettime bnettime(void);
	extern char const * bnettime_get_str(t_bnettime bntime);
	extern int bnettime_set_str(t_bnettime * bntime, char const * timestr);
	extern int local_tzbias(void);
	extern t_bnettime bnettime_add_tzbias(t_bnettime bntime, int tzbias);
	extern void bnettime_to_bn_long(t_bnettime in, bn_long * out);
	extern void bn_long_to_bnettime(bn_long in, t_bnettime * out);

}

#endif
#endif
