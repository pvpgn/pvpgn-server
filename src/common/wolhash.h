/*
 * Copyright (C) 2007  Pelish (pelish@gmail.com)
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
#ifndef INCLUDED_WOLHASH_TYPES
#define INCLUDED_WOLHASH_TYPES

#ifdef JUST_NEED_TYPES
# include "compat/uint.h"
#else
# define JUST_NEED_TYPES
# include "compat/uint.h"
# undef JUST_NEED_TYPES
#endif

namespace pvpgn
{

	typedef char t_wolhash[9];

}

#endif

/*****/
#define WOL_HASH_CHAR    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./"

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_WOLHASH_PROTOS
#define INCLUDED_WOLHASH_PROTOS


namespace pvpgn
{

	extern int wol_hash(t_wolhash * hashout, unsigned int size, void const * datain);
	//extern int wolhash_eq(t_wolhash const h1, t_wolhash const h2) ;

}

#endif
#endif
