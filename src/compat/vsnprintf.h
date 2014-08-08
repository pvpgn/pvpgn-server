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
#ifndef INCLUDED_VSNPRINTF_PROTOS
#define INCLUDED_VSNPRINTF_PROTOS

#include <cstdio>

#if !defined(HAVE_VSNPRINTF)
#ifdef HAVE__VSNPRINTF
#define vsnprintf(str,size,format,ap) _vsnprintf(str,size,format,ap)
#else
#if defined(_IOSTRG) && defined(_IOSTRG) && defined(HAVE_DOPRNT)

#include <cstdarg>

namespace pvpgn
{

	extern int vsnprintf(char *str, int size, const char *format, va_list ap);

}
#else
#error "Your system lacks ANY kind of vsnprintf support!"
#endif
#endif
#endif

#endif /* INCLUDED_VSNPRINTF_PROTOS */
