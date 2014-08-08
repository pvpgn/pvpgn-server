/*
 * Copyright (C) 1999  Descolada (dyn1-tnt9-237.chicago.il.ameritech.net)
 * Copyright (C) 1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_BNETHASH_TYPES
#define INCLUDED_BNETHASH_TYPES

#ifdef JUST_NEED_TYPES
# include "compat/uint.h"
#else
# define JUST_NEED_TYPES
# include "compat/uint.h"
# undef JUST_NEED_TYPES
#endif

namespace pvpgn
{

	typedef t_uint32 t_hash[5];

}

#endif

/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_BNETHASH_PROTOS
#define INCLUDED_BNETHASH_PROTOS

namespace pvpgn
{

	extern int bnet_hash(t_hash * hashout, unsigned int size, void const * data);
	extern int sha1_hash(t_hash * hashout, unsigned int size, void const * data);
	extern int little_endian_sha1_hash(t_hash * hashout, unsigned int size, void const * data);
	extern int hash_eq(t_hash const h1, t_hash const h2);
	extern char const * hash_get_str(t_hash const hash);
	extern char const * little_endian_hash_get_str(t_hash const hash);
	extern int hash_set_str(t_hash * hash, char const * str);

}

#endif
#endif
