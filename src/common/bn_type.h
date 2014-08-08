/*
 * Copyright (C) 1998,1999  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_BN_TYPE_TYPES
#define INCLUDED_BN_TYPE_TYPES

#ifdef JUST_NEED_TYPES
# include "compat/uint.h"
#else
# define JUST_NEED_TYPES
# include "compat/uint.h"
# undef JUST_NEED_TYPES
#endif

namespace pvpgn
{

	typedef t_uint8  bn_basic;
	typedef bn_basic bn_byte[1];
	typedef bn_basic bn_short[2];
	typedef bn_basic bn_int[4];
	typedef bn_basic bn_long[8];

}

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_BN_TYPE_PROTOS
#define INCLUDED_BN_TYPE_PROTOS

#define JUST_NEED_TYPES
# include "compat/uint.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	extern int bn_byte_tag_get(bn_byte const * src, char * dst, unsigned int len);
	extern int bn_short_tag_get(bn_short const * src, char * dst, unsigned int len);
	extern int bn_int_tag_get(bn_int const * src, char * dst, unsigned int len);
	extern int bn_long_tag_get(bn_long const * src, char * dst, unsigned int len);

	extern int bn_byte_tag_set(bn_byte * dst, char const * tag);
	extern int bn_short_tag_set(bn_short * dst, char const * tag);
	extern int bn_int_tag_set(bn_int * dst, char const * tag);
	extern int bn_long_tag_set(bn_long * dst, char const * tag);

	extern t_uint8 bn_byte_get(bn_byte const src);
	extern t_uint16 bn_short_get(bn_short const src);
	extern t_uint16 bn_short_nget(bn_short const src);
	extern t_uint32 bn_int_get(bn_int const src);
	extern t_uint32 bn_int_nget(bn_int const src);
#ifdef HAVE_T_UINT64
	extern t_uint64 bn_long_get(bn_long const src);
#endif
	extern t_uint32 bn_long_get_a(bn_long const src);
	extern t_uint32 bn_long_get_b(bn_long const src);

	extern int bn_byte_set(bn_byte * dst, t_uint8 src);
	extern int bn_short_set(bn_short * dst, t_uint16 src);
	extern int bn_short_nset(bn_short * dst, t_uint16 src);
	extern int bn_int_set(bn_int * dst, t_uint32 src);
	extern int bn_int_nset(bn_int * dst, t_uint32 src);
#ifdef HAVE_T_UINT64
	extern int bn_long_set(bn_long * dst, t_uint64 src);
	extern int bn_long_nset(bn_long * dst, t_uint64 src);
#endif
	extern int bn_long_set_a_b(bn_long * dst, t_uint32 srca, t_uint32 srcb);
	extern int bn_long_nset_a_b(bn_long * dst, t_uint32 srca, t_uint32 srcb);

	extern int bn_raw_set(void * dst, void const * src, unsigned int len);

	extern int bn_byte_tag_eq(bn_byte const src, char const * tag);
	extern int bn_short_tag_eq(bn_short const src, char const * tag);
	extern int bn_int_tag_eq(bn_int const src, char const * tag);
	extern int bn_long_tag_eq(bn_long const src, char const * tag);

	extern int uint32_to_int(t_uint32 num);

}

#endif
#endif
