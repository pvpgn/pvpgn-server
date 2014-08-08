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
#include "common/setup_before.h"
#ifndef HAVE_MMAP
#include <cstdlib>
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include "common/xalloc.h"
#include "mmap.h"
#ifdef WIN32
# include <windows.h>
# include <io.h>
#endif
#include "common/setup_after.h"

namespace pvpgn
{

	extern void * pmmap(void *addr, unsigned len, int prot, int flags, int fd, unsigned offset)
	{
		void *mem;
#ifdef WIN32
		HANDLE	hFile, hMapping;

		/* under win32 we only support readonly mappings, the only ones used in pvpgn now :) */
		if (prot != PROT_READ) return NULL;
		hFile = (HANDLE)_get_osfhandle(fd);
		if (hFile == (HANDLE)-1) return MAP_FAILED;
		hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (!hMapping) return MAP_FAILED;
		mem = MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
#else /* systems without mmap or win32 */
		unsigned pos;
		int res;

		mem = xmalloc(len);
		pos = 0;
		while (pos < len) {
			res = read(fd, (char *)mem + pos, len - pos);
			if (res < 0) {
				xfree(mem);
				return MAP_FAILED;
			}
			pos += res;
		}
#endif

		return mem;
	}

	extern int pmunmap(void *addr, unsigned len)
	{
#ifdef WIN32
		UnmapViewOfFile(addr);
#else
		xfree(addr);
#endif
		return 0;
	}

}

#else
typedef int filenotempty; /* make ISO standard happy */
#endif
