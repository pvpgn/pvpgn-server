/*
 * Copyright (C) 2001		faster	(lqx@cic.tsinghua.edu.cn)
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
#include "setup.h"
#include "dbsdupecheck.h"

#include "common/bn_type.h"
#include "common/eventlog.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace d2dbs
	{

		const char * delimiter = "JM";

		int is_delimit(char * data)
		{
			if ((data[0] == delimiter[0]) && (data[1] == delimiter[1])) return 1;
			else return 0;
		}

		void * find_delimiter(char * data, unsigned int datalen)
		{
			char * datap = data;
			unsigned int count;
			for (count = 1; count < datalen; count++)
			{
				if (is_delimit(datap)) return datap;
			}
			return NULL;
		}

#define SKIPLEN 750

		extern int dbsdupecheck(char * data, unsigned int datalen)
		{
			char * pointer, *datap;
			unsigned int restlen;

			unsigned short itemcount, counter;
			unsigned long uid;

			// skip characters that have never played yet
			if (datalen < SKIPLEN) return DBSDUPECHECK_CONTAINS_NO_DUPE;

			// we skip the first SKIPLEN bytes containing various char infos wo don't care about
			// this a) speeds things up
			// and  b) prevents us from detecting our magic index in a bad charname ;-)
			datap = data + SKIPLEN;
			restlen = datalen - SKIPLEN;

			do
			{
				pointer = (char*)find_delimiter(datap, restlen);
				restlen -= (pointer - datap);
				datap = pointer;
			} while ((is_delimit(datap) != 1) || (is_delimit(datap + 4) != 1)); // now we should have found "JMxxJM"

			itemcount = bn_short_get((bn_basic*)&datap[2]);

			datap += 4;
			restlen -= 4;

			for (counter = 0; counter < itemcount; counter++)
			{
				if ((datap[4] & 0x20) == 0x20)
				{
					eventlog(eventlog_level_info, __FUNCTION__, "simple item");
					datap += 14;
					restlen -= 14;
				}
				else
				{
					eventlog(eventlog_level_info, __FUNCTION__, "extended item");
					uid = bn_int_get((bn_basic*)&datap[14]);
					eventlog(eventlog_level_info, __FUNCTION__, "unique ID: %lu", uid);
					pointer = (char*)find_delimiter(datap, restlen);
					restlen -= (pointer - datap);
					datap = pointer;
				}
			}

			return DBSDUPECHECK_CONTAINS_NO_DUPE;
		}

	}

}
