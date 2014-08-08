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
#ifndef INCLUDED_STRCASECMP_PROTOS
#define INCLUDED_STRCASECMP_PROTOS

#include <cstring>

#ifndef HAVE_STRCASECMP

#ifdef HAVE_STRICMP
# define strcasecmp(s1, s2) stricmp(s1, s2)
# define HAVE_STRCASECMP
#else

namespace pvpgn
{

	extern int strcasecmp(char const * str1, char const * str2);

}

#endif

#endif

#endif
