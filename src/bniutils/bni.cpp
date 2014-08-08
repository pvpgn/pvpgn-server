/*
	Copyright (C) 2000  Marco Ziech (mmz@gmx.net)
	Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)

	This program is xfree software; you can redistribute it and/or modify
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
#include "common/setup_before.h"
#include "bni.h"

#include "common/xalloc.h"
#include "fileio.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bni
	{

		extern t_bnifile * load_bni(std::FILE *f) {
			t_bnifile *b;
			unsigned int i;

			if (f == NULL) return NULL;
			b = (t_bnifile*)xmalloc(sizeof(t_bnifile));
			file_rpush(f);
			b->unknown1 = file_readd_le();
			if (b->unknown1 != 0x00000010)
				std::fprintf(stderr, "load_bni: field 1 is not 0x00000010. Data may be invalid!\n");
			b->unknown2 = file_readd_le();
			if (b->unknown2 != 0x00000001)
				std::fprintf(stderr, "load_bni: field 2 is not 0x00000001. Data may be invalid!\n");
			b->numicons = file_readd_le();
			if (b->numicons > BNI_MAXICONS) {
				std::fprintf(stderr, "load_bni: more than %d (BNI_MAXICONS) icons. Increase maximum number of icons in \"bni.h\".\n", BNI_MAXICONS);
				b->numicons = BNI_MAXICONS;
			}
			b->dataoffset = file_readd_le();
			if (b->numicons < 1) {
				std::fprintf(stderr, "load_bni: strange, no icons present in BNI file\n");
				b->icons = NULL;
			}
			else {
				b->icons = (struct bni_iconlist_struct*)xmalloc(b->numicons*sizeof(t_bniicon));
			}
			for (i = 0; i < b->numicons; i++) {
				b->icons->icon[i].id = file_readd_le();
				b->icons->icon[i].x = file_readd_le();
				b->icons->icon[i].y = file_readd_le();
				if (b->icons->icon[i].id == 0) {
					b->icons->icon[i].tag = file_readd_le();
				}
				else {
					b->icons->icon[i].tag = 0;
				}
				b->icons->icon[i].unknown = file_readd_le();
			}
			if (std::ftell(f) != (long)b->dataoffset)
				std::fprintf(stderr, "load_bni: Warning, %lu bytes of garbage after BNI header\n", (unsigned long)(b->dataoffset - std::ftell(f)));
			file_rpop();
			return b;
		}

		extern int write_bni(std::FILE *f, t_bnifile *b) {
			unsigned int i;

			if (f == NULL) return -1;
			if (b == NULL) return -1;
			file_wpush(f);
			file_writed_le(b->unknown1);
			if (b->unknown1 != 0x00000010)
				std::fprintf(stderr, "write_bni: field 1 is not 0x00000010. Data may be invalid!\n");
			file_writed_le(b->unknown2);
			if (b->unknown2 != 0x00000001)
				std::fprintf(stderr, "write_bni: field 2 is not 0x00000001. Data may be invalid!\n");
			file_writed_le(b->numicons);
			file_writed_le(b->dataoffset);
			for (i = 0; i < b->numicons; i++) {
				file_writed_le(b->icons->icon[i].id);
				file_writed_le(b->icons->icon[i].x);
				file_writed_le(b->icons->icon[i].y);
				if (b->icons->icon[i].id == 0) {
					file_writed_le(b->icons->icon[i].tag);
				}
				file_writed_le(b->icons->icon[i].unknown);
			}
			if (std::ftell(f) != (long)b->dataoffset)
				std::fprintf(stderr, "Warning: dataoffset is incorrect! (=0x%lx should be 0x%lx)\n", (unsigned long)b->dataoffset, (unsigned long)std::ftell(f));
			file_wpop();
			return 0;
		}

	}

}
