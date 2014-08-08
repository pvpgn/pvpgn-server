/*
 * Copyright (C) 2003 Dizzy
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
#ifndef INCLUDED_PMMAP_PROTOS
#define INCLUDED_PMMAP_PROTOS

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_MMAN_H
# include <sys/mman.h>
#endif
#ifdef __BORLANDC__
# include <io.h>
#endif

#ifndef PROT_NONE
#define PROT_NONE       0x00    /* no permissions */
#endif
#ifndef PROT_READ
#define PROT_READ       0x01    /* pages can be read */
#endif
#ifndef PROT_WRITE
#define PROT_WRITE      0x02    /* pages can be written */
#endif
#ifndef PROT_EXEC
#define PROT_EXEC       0x04    /* pages can be executed */
#endif
#ifndef MAP_SHARED
#define MAP_SHARED      0x0001          /* share changes */
#endif
#ifndef MAP_PRIVATE
#define MAP_PRIVATE     0x0002          /* changes are private */
#endif
#ifndef MAP_COPY
#define MAP_COPY        MAP_PRIVATE     /* Obsolete */
#endif
#ifndef MAP_FAILED
#define MAP_FAILED      ((void *)-1)
#endif

#ifdef HAVE_MMAP
#define pmmap(a,b,c,d,e,f) mmap(a,b,c,d,e,f)
#define pmunmap(a,b) munmap(a,b)
#else

namespace pvpgn
{

	extern void * pmmap(void *addr, unsigned len, int prot, int flags, int fd, unsigned offset);
	extern int pmunmap(void *addr, unsigned len);

}

#endif

#endif
