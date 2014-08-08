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


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_INTROTATE_PROTOS
#define INCLUDED_INTROTATE_PROTOS

/*
 * ROTL(x,n,w) rotates "w" bit wide value "x" by "n" bits to the left
 *
 * The expression passed in as x must have a type at least as wide as w.
 * The type should probably be unsigned for this to be guaranteed to work
 * properly.  If n or w is signed and larger than x remember that the
 * promotion rules would promote x to be of the same (signed) type.
 *
 * Unfortunately C doesn't have rotate operations and they can be difficult
 * to implement when handling rotates by zero, negative numbers, or numbers
 * greater or equal to the bit width of the number.  This is because ANSI/ISO
 * C makes weak guarantees about the left and right shift operators.  We
 * would like to not depend on word size, endianness, or how negative
 * integers are represented.  Unfortunately only some of those goals may be
 * achieved.  As for optimization, a really good compiler might be able to
 * recognize what we are doing and turn it into a single machine instruction.
 */

/* valid for 0<n<w and w>0 */
/*#define ROTL(x,n,w) (((x)<<(n)) | ((x)>>((w)-(n))))*/

/* valid for 0<=n<w and w>0 */
/*#define ROTL(x,n,w) (((x)<<(n)) | ((x)>>(((-(n))&(w-1)))))*/

/* valid for 0<=n and w>0 , depends on 2's complement */
#define ROTL(x,n,w) (((x)<<((n)&(w-1))) | ((x)>>(((-(n))&(w-1)))))

/* valid for 0<=n and w>0 , uses three mods and an ugly conditional */
/* FIXME: and also a bug because it doesn't work on PPC */
/*#define ROTL(x,n,w) (((n)%(w)) ? (((x)<<((n)%(w))) | ((x)>>((w)-((n)%(w))))) : (x))*/

#define ROTL32(x,n) ROTL(x,n,32)
#define ROTL16(x,n) ROTL(x,n,16)

#endif
#endif
