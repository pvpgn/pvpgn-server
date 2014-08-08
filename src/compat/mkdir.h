/*
 * Copyright (C) 2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_MKDIR_PROTOS
#define INCLUDED_MKDIR_PROTOS

/* Unix puts this in unistd.h, Borland/Win32 puts it in dir.h, MSVC++/Win32 puts it in direct.h */
/* Windows and MacOS also typically take only one argument */
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_DIR_H
# include <dir.h>
#endif
#ifdef HAVE_DIRECT_H
# include <direct.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef MKDIR_TAKES_ONE_ARG
# ifdef HAVE_MKDIR
#  define p_mkdir(A,B) mkdir(A)
# else
#  ifdef HAVE__MKDIR
#   define p_mkdir(A,B) _mkdir(A)
#  else
#   error "This program requires either mkdir() or _mkdir()"
#  endif
# endif
#else
# ifdef HAVE_MKDIR
#  define p_mkdir mkdir
# else
#  ifdef HAVE__MKDIR
#   define p_mkdir _mkdir
#  else
#   error "This program requires either mkdir() or _mkdir()"
#  endif
# endif
#endif

#endif
