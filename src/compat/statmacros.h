/*
 * Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_STATMACROS_PROTOS
#define INCLUDED_STATMACROS_PROTOS

#ifdef STAT_MACROS_BROKEN
# ifdef S_ISREG
#  undef S_ISREG
# endif
# ifdef S_ISDIR
#  undef S_ISDIR
# endif
# ifdef S_ISCHR
#  undef S_ISCHR
# endif
# ifdef S_ISBLK
#  undef S_ISBLK
# endif
# ifdef S_ISFIFO
#  undef S_ISFIFO
# endif
# ifdef S_ISSOCK
#  undef S_ISSOCK
# endif
# ifdef S_ISLNK
#  undef S_ISLNK
# endif
#endif

#ifndef S_IFMT
# define S_IFMT 0170000
#endif

#ifndef S_ISREG
# ifdef S_IFREG
#  define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
# endif
#endif
#ifndef S_ISDIR
# ifdef S_IFDIR
#  define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
# endif
#endif
#ifndef S_ISCHR
# ifdef S_IFCHR
#  define S_ISCHR(m) (((m) & S_IFMT) == S_IFCHR)
# endif
#endif
#ifndef S_ISBLK
# ifdef S_IFBLK
#  define S_ISBLK(m) (((m) & S_IFMT) == S_IFBLK)
# endif
#endif
#ifndef S_ISFIFO
# ifdef S_IFIFO
#  define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
# endif
#endif
#ifndef S_ISLNK
# ifdef S_IFLNK
#  define S_ISLNK(m) (((m) & S_IFMT) == S_IFLNK)
# endif
#endif
#ifndef S_ISSOCK
# ifdef S_IFSOCK
#  define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)
# endif
#endif
#ifndef S_ISCDF
# ifdef S_CDF
#  define S_ISCDF(m) (S_ISDIR(m) && ((m) & S_CDF))
# endif
#endif
#ifndef S_ISMPB
# ifdef S_IFMPB /* V7 */
#  define S_ISMPB(m) (((m) & S_IFMT) == S_IFMPB)
#  define S_ISMPC(m) (((m) & S_IFMT) == S_IFMPC)
# endif
#endif
#ifndef S_ISNWK
# ifdef S_IFNWK /* HP/UX */
#  define S_ISNWK(m) (((m) & S_IFMT) == S_IFNWK)
# endif
#endif

#ifndef S_ISUID
# define S_ISUID 01000 /* set user ID */
#endif
#ifndef S_ISGID
# define S_ISGID 01000 /* set group ID */
#endif
#ifndef S_ISVTX
# define S_ISVTX 01000 /* sticky bit */
#endif

#ifndef S_IRUSR
# ifdef S_IREAD
#  define S_IRUSR S_IREAD
# else
#  define S_IRUSR 00400
# endif
#endif
#ifndef S_IWUSR
# ifdef S_IWRITE
#  define S_IWUSR S_IWRITE
# else
#  define S_IWUSR 00200
# endif
#endif
#ifndef S_IXUSR
# ifdef S_IEXEC
#  define S_IXUSR S_IEXEC
# else
#  define S_IXUSR 00100
# endif
#endif
#ifndef S_IRWXU
# define S_IRWXU (S_IRUSR|S_IWUSR|S_IXUSR)
#endif
#ifndef S_IRGRP
# define S_IRGRP (S_IRUSR >> 3)
#endif
#ifndef S_IWGRP
# define S_IWGRP (S_IWUSR >> 3)
#endif
#ifndef S_IXGRP
# define S_IXGRP (S_IXUSR >> 3)
#endif
#ifndef S_IRWXG
# define S_IRWXG (S_IRGRP|S_IWGRP|S_IXGRP)
#endif

#ifndef S_IROTH
# define S_IROTH (S_IRGRP >> 3)
#endif
#ifndef S_IWOTH
# define S_IWOTH (S_IWGRP >> 3)
#endif
#ifndef S_IXOTH
# define S_IXOTH (S_IXGRP >> 3)
#endif
#ifndef S_IRWXO
# define S_IRWXO (S_IROTH|S_IWOTH|S_IXOTH)
#endif

#endif
