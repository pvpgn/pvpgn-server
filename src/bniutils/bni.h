/*
	Copyright (C) 2000  Marco Ziech
	Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
	*/
#ifndef INCLUDED_BNI_H
#define INCLUDED_BNI_H

#include <cstdio>

#ifndef BNI_MAXICONS
#define BNI_MAXICONS 4096
#endif

namespace pvpgn
{

	namespace bni
	{

		typedef struct {
			unsigned int id;		/* Icon ID */
			unsigned int x, y;	/* width and height */
			unsigned int tag;	/* if ID == 0 */
			unsigned int unknown;	/* 0x00000000 */
		} t_bniicon;

		struct bni_iconlist_struct {
			t_bniicon icon[BNI_MAXICONS];
		}; /* The icons */

		typedef struct {
			unsigned int unknown1;	/* 0x00000010 */
			unsigned int unknown2;	/* 0x00000001 */
			unsigned int numicons;	/* Number of icons */
			unsigned int dataoffset;	/* Start of TGA-File */
			struct bni_iconlist_struct *icons; /* The icons */
		} t_bnifile;


		extern t_bnifile * load_bni(std::FILE *f);
		extern int write_bni(std::FILE *f, t_bnifile *b);

	}

}

#endif
