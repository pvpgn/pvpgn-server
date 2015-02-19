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

#ifndef HAVE_STRDUP
#include "strdup.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "common/xalloc.h"

#include "common/setup_after.h"


namespace pvpgn
{

	extern char * strdup(char const * str)
	{
		char * out;

		if (!(out = (char *)xmalloc(std::strlen(str) + 1)))
			return NULL;
		std::strcpy(out, str);
		return out;
	}

}

#else
typedef int filenotempty; /* make ISO standard happy */
#endif
