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
#ifndef HAVE_STRTOUL

#include <ctype.h>
#include "strtoul.h"
#include "common/setup_after.h"


namespace pvpgn
{

	extern unsigned long strtoul(char const * str, char * * endptr, int base)
	{
		unsigned long val;
		char symbolval;
		char * pos;

		if (!str)
			return 0; /* EINVAL */

		for (pos = (char *)str; *pos == ' ' || *pos == '\t'; pos++);
		if (*pos == '-' || *pos == '+')
			pos++;
		if ((base == 0 || base == 16) && *pos == '0' && (*(pos + 1) == 'x' || *(pos + 1) == 'X'))
		{
			base = 16;
			pos += 2; /* skip 0x prefix */
		}
		else if ((base == 0 || base == 8) && *pos == '0')
		{
			base = 8;
			pos += 1;
		}
		else if (base == 0)
		{
			base = 10;
		}

		if (base<2 || base>16) /* sorry, not complete emulation (should do up to 36) */
			return 0; /* EINVAL */

		val = 0;
		for (; *pos != '\0'; pos++)
		{
			val *= base;

			if (isxdigit(*pos))
			{
				symbolval = isdigit(*pos) ? *pos - '0' : tolower(*pos) - 'a' + 10;
				if (base > symbolval)
					val += symbolval;
				else
					break;

			}
		}

		if (endptr)
			*endptr = (void *)pos; /* avoid warning */

		return val;
	}

}

#else
typedef int filenotempty; /* make ISO standard happy */
#endif
