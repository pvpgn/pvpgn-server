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
#ifndef INCLUDED_UINT_TYPES
#define INCLUDED_UINT_TYPES

#ifdef HAVE_STDINT_H

# include <stdint.h>
namespace pvpgn
{

	typedef uint8_t  t_uint8;
	typedef uint16_t t_uint16;
	typedef uint32_t t_uint32;
	typedef uint64_t t_uint64;

	typedef int8_t   t_int8;
	typedef int16_t  t_int16;
	typedef int32_t  t_int32;
	typedef int64_t  t_int64;

}

# define HAVE_UINT64_T

#else
#ifdef HAVE_MODE_ATTR
namespace pvpgn
{

	typedef unsigned int t_uint8   MODE_ATTR(__QI__);
	typedef unsigned int t_uint16  MODE_ATTR(__HI__);
	typedef unsigned int t_uint32  MODE_ATTR(__SI__);
	typedef unsigned int t_uint64  MODE_ATTR(__DI__); /* FIXME: I guess DI is always available... Is there a way to check? */
	typedef signed int   t_int8    MODE_ATTR(__QI__);
	typedef signed int   t_int16   MODE_ATTR(__HI__);
	typedef signed int   t_int32   MODE_ATTR(__SI__);
	typedef signed int   t_int64   MODE_ATTR(__DI__); /* FIXME: I guess DI is always available... Is there a way to check? */

}
#  define HAVE_UINT64_T

# else

#  ifdef HAVE_LIMITS_H
#   include <limits.h>
#  endif
#  ifndef CHAR_BIT
#   define MY_CHAR_BIT 8 /* well, this is usually true :) */
#  else
#   define MY_CHAR_BIT CHAR_BIT
#  endif

namespace pvpgn
{

#  if SIZEOF_UNSIGNED_CHAR*MY_CHAR_BIT == 8
	typedef unsigned char      t_uint8;
#  else
# error "Unable to find 8-bit integer type"
#  endif

#  if SIZEOF_SIGNED_CHAR*MY_CHAR_BIT == 8
	typedef signed char        t_int8;
#  else
# error "Unable to find 8-bit integer type"
#  endif

#  if SIZEOF_UNSIGNED_SHORT*MY_CHAR_BIT == 16
	typedef unsigned short     t_uint16;
#  else
#   if SIZEOF_UNSIGNED_INT*MY_CHAR_BIT == 16
	typedef unsigned int       t_uint16;
#   else
#    error "Unable to find 16-bit integer type"
#   endif
#  endif

#  if SIZEOF_SIGNED_SHORT*MY_CHAR_BIT == 16
	typedef signed short       t_int16;
#  else
#   if SIZEOF_SIGNED_INT*MY_CHAR_BIT == 16
	typedef signed int         t_int16;
#   else
#    error "Unable to find 16-bit integer type"
#   endif
#  endif

#  if   SIZEOF_UNSIGNED_SHORT*MY_CHAR_BIT == 32
	typedef unsigned short     t_uint32;
#  else
#   if SIZEOF_UNSIGNED_INT*MY_CHAR_BIT == 32
	typedef unsigned int       t_uint32;
#   else
#    if SIZEOF_UNSIGNED_LONG*MY_CHAR_BIT == 32
	typedef unsigned long      t_uint32;
#    else
#     error "Unable to find 32-bit integer type"
#    endif
#   endif
#  endif

#  if   SIZEOF_SIGNED_SHORT*MY_CHAR_BIT == 32
	typedef signed short       t_int32;
#  else
#   if SIZEOF_SIGNED_INT*MY_CHAR_BIT == 32
	typedef signed int         t_int32;
#   else
#    if SIZEOF_SIGNED_LONG*MY_CHAR_BIT == 32
	typedef signed long        t_int32;
#    else
#     error "Unable to find 32-bit integer type"
#    endif
#   endif
#  endif

#  if SIZEOF_UNSIGNED_INT*MY_CHAR_BIT == 64
	typedef unsigned int       t_uint64;
#   define HAVE_UINT64_T
#  else
#   if SIZEOF_UNSIGNED_LONG*MY_CHAR_BIT == 64
	typedef unsigned long      t_uint64;
#    define HAVE_UINT64_T
#   else
#    if SIZEOF_UNSIGNED_LONG_LONG*MY_CHAR_BIT == 64
	typedef unsigned long long t_uint64;
#     define HAVE_UINT64_T
#    endif
#   endif
#  endif

#  if SIZEOF_SIGNED_INT*MY_CHAR_BIT == 64
	typedef signed int         t_int64;
#   define HAVE_INT64_T
#  else
#   if SIZEOF_SIGNED_LONG*MY_CHAR_BIT == 64
	typedef signed long        t_int64;
#    define HAVE_INT64_T
#   else
#    if SIZEOF_SIGNED_LONG_LONG*MY_CHAR_BIT == 64
	typedef signed long long   t_int64;
#     define HAVE_INT64_T
#    endif
#   endif
#  endif

}

#  undef MY_CHAR_BIT

# endif
#endif

#endif
