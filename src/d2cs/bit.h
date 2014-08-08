/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
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
#ifndef INCLUDED_BIT_H
#define INCLUDED_BIT_H

#define BIT_POS(n)		( 1u << (n) )
#define BIT_SET_FLAG(n,f)	( (n) |= (f) )
#define BIT_CLR_FLAG(n,f)	( (n) &= ~(f) )
#define BIT_TST_FLAG(n,f)		( ((n) & (f)) ? 1: 0 )
#define BIT_SET_CLR_FLAG(n, f, v)	( (v)?(BIT_SET_FLAG(n,f)):(BIT_CLR_FLAG(n,f)) )

#define BIT_RANGE(n,m)		( BIT_POS((m)+1-(n)) - BIT_POS(n) )
#define BIT_LSHIFT(n,m)		( (n) << (m) )
#define BIT_RSHIFT(n,m)		( (n) >> (m) )

#define BIT_GET(n, m)			( ((n) >> (m)) & 1)
#define BIT_GET_RANGE(n, m, l)		( ((n) >> (m)) & (1u << (l) -1))
#define BIT_GET_RANGE_MASK(n, m, mask)	( ((n) >> (m)) & (mask))

#endif
