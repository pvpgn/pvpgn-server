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
#include "common/setup_before.h"
#include "common/bnethashconv.h"

#include "common/bnethash.h"
#include "common/bn_type.h"
#include "common/eventlog.h"
#include "common/setup_after.h"

namespace pvpgn
{

	extern void bnhash_to_hash(bn_int const * bnhash, t_hash * hash)
	{
		unsigned int i;

		if (!bnhash)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL bnhash");
			return;
		}
		if (!hash)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL hash");
			return;
		}

		for (i = 0; i < 5; i++)
			(*hash)[i] = bn_int_get(bnhash[i]);
	}


	extern void hash_to_bnhash(t_hash const * hash, bn_int * bnhash)
	{
		unsigned int i;

		if (!bnhash)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL bnhash");
			return;
		}
		if (!hash)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL hash");
			return;
		}

		for (i = 0; i < 5; i++)
			bn_int_set(&bnhash[i], (*hash)[i]);
	}

}
