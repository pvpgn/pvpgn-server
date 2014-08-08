/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
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
#include "d2char_checksum.h"
#include "common/setup_after.h"

namespace pvpgn
{

	extern int d2charsave_checksum(unsigned char const * data, unsigned int len, unsigned int offset)
	{
		int		checksum;
		unsigned int	i;
		unsigned int	ch;

		if (!data) return 0;
		checksum = 0;
		for (i = 0; i < len; i++) {
			if (i >= offset && i < offset + sizeof(int)) ch = 0;
			else ch = *data;
			ch += (checksum < 0);
			checksum = 2 * checksum + ch;
			data++;
		}
		return checksum;
	}

}
