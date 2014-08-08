/*
 * Copyright (C) 2005  Dizzy
 *
 * xstr is a module trying to offer some high-level string functionality
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

#ifndef INCLUDED_XSTR_TYPES
#define INCLUDED_XSTR_TYPES

namespace pvpgn
{

	typedef struct {
		unsigned alen, ulen;
		char *str;
	} t_xstr;

}

#define DECLARE_XSTR(var) \
	t_xstr var = { 0, 0, NULL };

#endif /* INCLUDED_XSTR_TYPES */

#ifndef INCLUDED_XSTR_PROTOS
#define INCLUDED_XSTR_PROTOS

/* for NULL */
#include <cstddef>

namespace pvpgn
{

	extern t_xstr* xstr_alloc(void);
	extern void xstr_free(t_xstr*);
	extern t_xstr* xstr_cpy_str(t_xstr* dst, const char* src);
	extern t_xstr* xstr_cat_xstr(t_xstr* dst, const t_xstr* src);
	extern t_xstr* xstr_cat_str(t_xstr* dst, const char* src);
	extern t_xstr* xstr_ncat_str(t_xstr* dst, const char* src, int len);
	extern t_xstr* xstr_cat_char(t_xstr* dst, const char ch);

	static inline void xstr_init(t_xstr* xstr)
	{
		xstr->alen = xstr->ulen = 0;
		xstr->str = NULL;
	}

	static inline const char* xstr_get_str(t_xstr* xstr)
	{
		if (xstr->alen && xstr->ulen) return xstr->str;
		return NULL;
	}

	static inline unsigned xstr_get_len(t_xstr* xstr)
	{
		return xstr->alen ? xstr->ulen : 0;
	}

	static inline t_xstr* xstr_clear(t_xstr* xstr)
	{
		/* reset the string content, don't touch the allocation space */
		xstr->ulen = 0;

		return xstr;
	}

}

#endif /* INCLUDED_STR_PROTOS */

