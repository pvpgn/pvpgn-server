/*
 * Copyright (C) 2004  Dizzy
 *
 * lstr is a structure to be used for cached string lengths for speed
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

#ifndef INCLUDED_LSTR_TYPES
#define INCLUDED_LSTR_TYPES

namespace pvpgn
{

	typedef struct lstr {
		char *str;
		unsigned len;
	} t_lstr;

}

#endif /* INCLUDED_LSTR_TYPES */

#ifndef INCLUDED_LSTR_PROTOS
#define INCLUDED_LSTR_PROTOS

namespace pvpgn
{

	static inline void lstr_set_str(t_lstr *lstr, char *str)
	{
		lstr->str = str;
	}

	static inline char *lstr_get_str(t_lstr *lstr)
	{
		return lstr->str;
	}

	static inline void lstr_set_len(t_lstr *lstr, unsigned len)
	{
		lstr->len = len;
	}

	static inline unsigned lstr_get_len(t_lstr *lstr)
	{
		return lstr->len;
	}

}

#endif /* INCLUDED_LSTR_PROTOS */
