/*
 * Copyright (C) 2002  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef HAVE_STRSEP
#include "strsep.h"

#include <cstring>

#include "common/setup_after.h"


namespace pvpgn
{

	extern char * strsep(char * * str, char const * delims)
	{
		char * begin;

		begin = *str;

		if (!begin)
			return NULL; /* already returned last token */

		/* FIXME: optimiz case of 1 char delims (maybe not worth the effort) */
		for (; **str != '\0'; (*str)++)
		if (std::strchr(delims, **str))
		{
			**str = '\0'; /* terminate token */
			(*str)++; /* remember the position of the next char */
			return begin;
		}

		*str = NULL;
		return begin; /* return the whole input string */
	}

}

#else
typedef int filenotempty; /* make ISO standard happy */
#endif
