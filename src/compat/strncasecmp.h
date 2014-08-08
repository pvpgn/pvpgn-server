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
#ifndef INCLUDED_STRNCASECMP_PROTOS
#define INCLUDED_STRNCASECMP_PROTOS

#include <cstring>

#ifndef HAVE_STRNCASECMP

#ifdef HAVE_STRNICMP
# define strncasecmp(s1, s2, cnt) strnicmp(s1, s2, cnt)
# define HAVE_STRNCASECMP /* don't include our own function */
#else

namespace pvpgn
{

	extern int strncasecmp(char const * str1, char const * str2, unsigned int cnt);

}

#endif

#endif

#endif
