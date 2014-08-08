/*
 * Copyright (C) 2001  Ross Combs (ross@bnetd.org)
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
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_PROGINFO_PROTOS
#define INCLUDED_PROGINFO_PROTOS

namespace pvpgn
{

	extern int verparts_to_vernum(unsigned short v1, unsigned short v2, unsigned short v3, unsigned short v4, unsigned long * vernum);
	extern int verstr_to_vernum(char const * verstr, unsigned long * vernum);
	extern char const * vernum_to_verstr(unsigned long vernum);

}

#endif
#endif
