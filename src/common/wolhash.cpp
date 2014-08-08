/*
 * Original hash funcions Copyright (C)  Luigi Auriemma (aluigi@autistici.org)
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

#include "common/setup_before.h"
#include "common/wolhash.h"

#include <cstdlib>
#include <cstring>

#include "common/eventlog.h"
#include "common/setup_after.h"

namespace pvpgn
{


	extern int wol_hash(t_wolhash * hashout, unsigned int size, void const * datain) {
		unsigned char pwd1[9];
		unsigned char pwd2[9];
		unsigned char edx;
		unsigned char esi;
		unsigned char i;
		unsigned char *p1;
		unsigned char *p2;

		/**
		 * Original NOTE by Luigi Auriemma:
		 * Original algorithm starts at offset 0x0041d440 of wchat.dat of 4.221 US version
		 * The algorithm is one-way, so the encoded password can NOT be completely decoded!
		 */

		if (size > 8) {
			//        ERROR1("Westwood passwords are max 8 bytes long: \"%.8s\"",size);
			return -1;
		}

		if (!datain) {
			ERROR0("got NULL datain");
			return -1;
		}

		std::memset(pwd1, 0, sizeof(pwd1));
		std::memset(pwd2, 0, sizeof(pwd2));
		std::memcpy(pwd1, datain, size);

		esi = size;
		p1 = pwd1;
		p2 = pwd2;
		for (i = 0; i < size; i++) {
			if (*p1 & 1) {
				edx = *p1 << 1;
				edx &= *(pwd1 + esi);
			}
			else {
				edx = *p1 ^ *(pwd1 + esi);
			}
			*p2++ = edx;
			esi--;
			p1++;
		}

		p1 = pwd1;
		p2 = pwd2;
		for (i = 0; i < 8; i++) {
			edx = *p2++ & 0x3f;
			*p1++ = WOL_HASH_CHAR[edx];
		}

		std::memcpy(hashout, pwd1, sizeof(pwd1));
		return 0;
	}

	/*extern int wolhash_eq(t_wolhash const h1, t_wolhash const h2)
	{
	unsigned int i;

	if (!h1 || !h2)
	{
	eventlog(eventlog_level_error,__FUNCTION__,"got NULL hash");
	return -1;
	}

	for (i=0; i<5; i++)
	if (h1[i]!=h2[i])
	return 0;

	return 1;
	}*/

}



