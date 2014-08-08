/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2004  Donny Redmond (dredmond@linuxmail.org)
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
#include "common/hexdump.h"

#include "common/packet.h"
#include "common/eventlog.h"
#include "common/setup_after.h"

namespace pvpgn
{

	extern void hexdump(std::FILE * stream, void const * data, unsigned int len)
	{
		unsigned int i;
		char dst[100];
		unsigned char * datac;

		if (!data) {
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL data");
			return;
		}

		if (!stream) {
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL stream");
			return;
		}

		for (i = 0, datac = (unsigned char*)data; i < len; i += 16, datac += 16)
		{
			hexdump_string(datac, (len - i < 16) ? (len - i) : 16, dst, i);
			std::fprintf(stream, "%s\n", dst);
			std::fflush(stream);
		}
	}

	extern void hexdump_string(unsigned char * data, unsigned int datalen, char * dst, unsigned int counter)
	{
		unsigned int c;
		int tlen = 0;
		unsigned char *datatmp;

		datatmp = data;
		tlen += std::sprintf((dst + tlen), "%04X:   ", counter);

		for (c = 0; c < 8; c++) /* left half of hex dump */
		if (c < datalen)
			tlen += std::sprintf((dst + tlen), "%02X ", *(datatmp++));
		else
			tlen += std::sprintf((dst + tlen), "   "); /* pad if short line */

		tlen += std::sprintf((dst + tlen), "  ");

		for (c = 8; c < 16; c++) /* right half of hex dump */
		if (c < datalen)
			tlen += std::sprintf((dst + tlen), "%02X ", *(datatmp++));
		else
			tlen += std::sprintf((dst + tlen), "   "); /* pad if short line */

		tlen += std::sprintf((dst + tlen), "   ");

		for (c = 0, datatmp = data; c < 16; c++, datatmp++) /* ASCII dump */
		if (c < datalen) {
			if (*datatmp >= 32 && *datatmp < 127)
				tlen += std::sprintf((dst + tlen), "%c", *datatmp);
			else
				tlen += std::sprintf((dst + tlen), "."); /* put this for non-printables */
		}

	}

}
