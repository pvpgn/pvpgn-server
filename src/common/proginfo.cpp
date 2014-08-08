/*
 * Copyright (C) 2000 Rob Crittenden (rcrit@greyoak.com)
 * Copyright (C) 2001 Ross Combs (ross@bnetd.org)
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
#include "common/proginfo.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "common/setup_after.h"


namespace pvpgn
{

	extern int verparts_to_vernum(unsigned short v1, unsigned short v2, unsigned short v3, unsigned short v4, unsigned long * vernum)
	{
		if (!vernum)
			return -1;

		*vernum = (((unsigned long)v4) << 24) |
			(((unsigned long)v3) << 16) |
			(((unsigned long)v2) << 8) |
			(((unsigned long)v1));
		return 0;
	}


	extern int verstr_to_vernum(char const * verstr, unsigned long * vernum)
	{
		unsigned long v1, v2, v3, v4;

		if (!vernum)
			return -1;

		if (std::strchr(verstr, '.'))
		{
			int count;

			count = std::sscanf(verstr, "%lu.%lu.%lu.%lu", &v4, &v3, &v2, &v1);
			if (count < 4)
			{
				v1 = 0;
				if (count < 3)
				{
					v2 = 0;
					if (count < 2)
						return -1; /* no data */
				}
			}
		}
		else
		{
			unsigned long temp;

			temp = std::strtoul(verstr, NULL, 10);
			v4 = (temp / 100);
			v3 = (temp / 10) % 10;
			v2 = (temp) % 10;
			v1 = 0;
		}

		if (v1 > 255 || v2 > 255 || v3 > 255 || v4 > 255)
			return -1;
		*vernum = (v4 << 24) | (v3 << 16) | (v2 << 8) | (v1);
		return 0;
	}


	extern char const * vernum_to_verstr(unsigned long vernum)
	{
		static char verstr[16];

		std::sprintf(verstr, "%lu.%lu.%lu.%lu",
			(vernum >> 24),
			(vernum >> 16) & 0xff,
			(vernum >> 8) & 0xff,
			(vernum)& 0xff);
		return verstr;
	}

}
