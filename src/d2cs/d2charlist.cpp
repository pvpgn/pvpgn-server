/*
 * Copyright (C) 2004      ls_sons  (ls@gamelife.org)
 * Copyright (C) 2004	Olaf Freyer (aaron@cs.tu-berlin.de)
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
#include "d2charlist.h"

#include <cstring>

#include "compat/strcasecmp.h"
#include "compat/strncasecmp.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "prefs.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace d2cs
	{

		extern int d2charlist_add_char(t_elist * list_head, t_d2charinfo_file * charinfo, unsigned int expiration_time)
		{
			t_d2charlist * charlist, *ccharlist;
			char const * d2char_sort;
			t_elist * curr;

			d2char_sort = prefs_get_charlist_sort();
			charlist = (t_d2charlist*)xmalloc(sizeof(t_d2charlist));
			charlist->charinfo = charinfo;
			charlist->expiration_time = expiration_time;

			if (elist_empty(list_head))
				elist_add(list_head, &charlist->list);
			else
			{
				if (strcasecmp(d2char_sort, "name") == 0)
				{
					elist_for_each(curr, list_head)
					{
						ccharlist = elist_entry(curr, t_d2charlist, list);
						if (strncasecmp((char*)charinfo->header.charname, (char*)ccharlist->charinfo->header.charname, std::strlen((char*)charinfo->header.charname)) < 0)
							break;
					}
					elist_add_tail(curr, &charlist->list);
				}
				else if (strcasecmp(d2char_sort, "ctime") == 0)
				{
					elist_for_each(curr, list_head)
					{
						ccharlist = elist_entry(curr, t_d2charlist, list);
						if (bn_int_get(charinfo->header.create_time) < bn_int_get(ccharlist->charinfo->header.create_time))
							break;
					}
					elist_add_tail(curr, &charlist->list);
				}
				else if (strcasecmp(d2char_sort, "mtime") == 0)
				{
					elist_for_each(curr, list_head)
					{
						ccharlist = elist_entry(curr, t_d2charlist, list);
						if (bn_int_get(charinfo->header.last_time) < bn_int_get(ccharlist->charinfo->header.last_time))
							break;
					}
					elist_add_tail(curr, &charlist->list);
				}
				else if (strcasecmp(d2char_sort, "level") == 0)
				{
					elist_for_each(curr, list_head)
					{
						ccharlist = elist_entry(curr, t_d2charlist, list);
						if (bn_int_get(charinfo->summary.experience) < bn_int_get(ccharlist->charinfo->summary.experience))
							break;
					}
					elist_add_tail(curr, &charlist->list);
				}
				else
				{
					eventlog(eventlog_level_debug, __FUNCTION__, "unsorted");
					elist_add_tail(list_head, &charlist->list);
				}
			}
			return 0;
		}

	}

}
