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
#ifndef INCLUDED_TERMIOS_TYPES
#define INCLUDED_TERMIOS_TYPES

/* FIXME: this might already exist even if termios.h doesn't... check
   for the type in autoconf */
# ifndef HAVE_TERMIOS_H
namespace pvpgn
{

	struct termios
	{
		int c_lflag;
		int c_cc[1];
	};

}

#  define ECHO   1
#  define ICANON 1
#  define VMIN   1
#  define VTIME  1

# endif

#endif


#ifndef INCLUDED_TERMIOS_PROTOS
#define INCLUDED_TERMIOS_PROTOS

/* FIXME: check for functions or macros (autoconf doesn't protect header inclusion for function tests so it isn't easy */
#ifndef HAVE_TERMIOS_H
# define tcgetattr(F,T) (-1)
#endif

/* FIXME: check for functions or macros (autoconf doesn't protect header inclusion for function tests so it isn't easy */
#ifndef HAVE_TERMIOS_H
# define tcsetattr(F,A,T) (-1)
#endif

#endif
