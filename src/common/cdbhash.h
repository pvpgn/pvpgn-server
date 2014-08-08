/*
 * Copyright (C) 2005 Dizzy
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

#ifndef __CDBHASH_H_INCLUDED__
#define __CDBHASH_H_INCLUDED__

namespace pvpgn
{

	typedef unsigned int t_cdbhash;

	static inline t_cdbhash cdb_hash(const void* data, std::size_t len)
	{
		t_cdbhash h;
		const char* p = (const char*)data;

		for (h = 5381; len > 0; --len, ++p) {
			h += h << 5;
			h ^= *p;
		}
		return h;
	}

}

#endif /* __CDBHASH_H_INCLUDED__ */

