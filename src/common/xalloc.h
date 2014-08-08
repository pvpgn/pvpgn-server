/*
 * Copyright (C) 2004 Dizzy
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

#ifndef INCLUDED_XALLOC_TYPES

namespace pvpgn
{

	/* out of memory callback function */
	typedef int(*t_oom_cb)(void);

}

#define INCLUDED_XALLOC_TYPES

#endif /* INCLUDED_XALLOC_TYPES */

#ifndef INCLUDED_XALLOC_PROTOS
#define INCLUDED_XALLOC_PROTOS

#ifndef XALLOC_SKIP
#include <cstdlib>

namespace pvpgn
{


#define xmalloc(size) xmalloc_real(size,__FILE__,__LINE__)
	void *xmalloc_real(std::size_t size, const char *fn, unsigned ln);
#define xcalloc(no,size) xcalloc_real(no,size,__FILE__,__LINE__)
	void *xcalloc_real(std::size_t nmemb, std::size_t size, const char *fn, unsigned ln);
#define xrealloc(ptr,size) xrealloc_real(ptr,size,__FILE__,__LINE__)
	void *xrealloc_real(void *ptr, std::size_t size, const char *fn, unsigned ln);
#define xstrdup(str) xstrdup_real(str,__FILE__,__LINE__)
	char *xstrdup_real(const char *str, const char *fn, unsigned ln);
#define xfree(ptr) xfree_real(ptr,__FILE__,__LINE__)
	void xfree_real(void *ptr, const char *fn, unsigned ln);
	void xalloc_setcb(t_oom_cb cb);

}

#else /* XALLOC_SKIP */

#define xmalloc(size) malloc(size)
#define xcalloc(no,size) calloc(no,size)
#define xrealloc(ptr,size) std::realloc(ptr,size)
#define xstrdup(str) strdup(str)
#define xfree(ptr) free(ptr)
#define xalloc_setcb(cb)

#endif

#endif /* INCLUDED_XALLOC_PROTOS */
