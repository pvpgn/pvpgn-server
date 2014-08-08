/*
 * Copyright (C) 2004 Dizzy
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
#define ATTRGROUP_INTERNAL_ACCESS
#include "attrlayer.h"
#include "common/eventlog.h"
#include "common/flags.h"
#include "attr.h"
#include "attrgroup.h"
#include "storage.h"
#include "prefs.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static t_attrgroup *defattrs = NULL;

		static DECLARE_ELIST_INIT(loadedlist);
		static DECLARE_ELIST_INIT(dirtylist);

		static int attrlayer_unload_default(void);

		extern int attrlayer_init(void)
		{
			elist_init(&loadedlist);
			elist_init(&dirtylist);
			attrlayer_load_default();

			return 0;
		}

		extern int attrlayer_cleanup(void)
		{
			attrlayer_flush(FS_FORCE | FS_ALL);
			attrlayer_unload_default();

			return 0;
		}

		extern int attrlayer_load_default(void)
		{
			if (defattrs) attrlayer_unload_default();

			defattrs = attrgroup_create_storage(storage->get_defacct());
			if (!defattrs) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not create attrgroup");
				return -1;
			}

			return attrgroup_load(defattrs);
		}

		static int attrlayer_unload_default(void)
		{
			attrgroup_destroy(defattrs);
			defattrs = NULL;

			return 0;
		}

		/* FIXME: dont copy most of the functionality, a good place for a C++ template ;) */

		extern int attrlayer_flush(int flags)
		{
			static t_elist *curr = &loadedlist;
			static t_elist *next = NULL;
			t_attrgroup *attrgroup;
			unsigned int fcount;
			unsigned int tcount;

			fcount = tcount = 0;
			if (curr == &loadedlist || FLAG_ISSET(flags, FS_ALL)) {
				curr = elist_next(&loadedlist);
				next = elist_next(curr);
			}

			/* elist_for_each_safe splitted into separate startup for userstep function */
			for (; curr != &loadedlist; curr = next, next = elist_next(curr)) {
				if (!FLAG_ISSET(flags, FS_ALL) && tcount >= prefs_get_user_step()) break;

				attrgroup = elist_entry(curr, t_attrgroup, loadedlist);
				switch (attrgroup_flush(attrgroup, flags)) {
				case 0:
					/* stop on the first account not flushed (ie accessed too early) */
					goto loopout;
				case 1:
					fcount++;
					break;
				case -1:
					eventlog(eventlog_level_error, __FUNCTION__, "could not flush account");
					break;
				default:
					break;
				}
				tcount++;
			}

		loopout:
			if (fcount > 0)
				eventlog(eventlog_level_debug, __FUNCTION__, "flushed %u user accounts", fcount);

			if (!FLAG_ISSET(flags, FS_ALL) && curr != &loadedlist) return 1;

			return 0;
		}

		extern int attrlayer_save(int flags)
		{
			static t_elist *curr = &dirtylist;
			static t_elist *next = NULL;
			t_attrgroup *attrgroup;
			unsigned int scount;
			unsigned int tcount;

			scount = tcount = 0;
			if (curr == &dirtylist || FLAG_ISSET(flags, FS_ALL)) {
				curr = elist_next(&dirtylist);
				next = elist_next(curr);
			}

			/* elist_for_each_safe splitted into separate startup for userstep function */
			for (; curr != &dirtylist; curr = next, next = elist_next(curr)) {
				if (!FLAG_ISSET(flags, FS_ALL) && tcount >= prefs_get_user_step()) break;

				attrgroup = elist_entry(curr, t_attrgroup, dirtylist);
				switch (attrgroup_save(attrgroup, flags)) {
				case 0:
					/* stop on the first account not saved (ie dirty too early) */
					goto loopout;
				case 1:
					scount++;
					break;
				case -1:
					eventlog(eventlog_level_error, __FUNCTION__, "could not save account");
					break;
				default:
					break;
				}
				tcount++;
			}

		loopout:
			if (scount > 0)
				eventlog(eventlog_level_debug, __FUNCTION__, "saved %u user accounts", scount);

			if (!FLAG_ISSET(flags, FS_ALL) && curr != &dirtylist) return 1;

			return 0;
		}

		extern void attrlayer_add_loadedlist(t_elist *what)
		{
			elist_add_tail(&loadedlist, what);
		}

		extern void attrlayer_del_loadedlist(t_elist *what)
		{
			elist_del(what);
		}

		extern void attrlayer_add_dirtylist(t_elist *what)
		{
			elist_add_tail(&dirtylist, what);
		}

		extern void attrlayer_del_dirtylist(t_elist *what)
		{
			elist_del(what);
		}

		extern t_attrgroup * attrlayer_get_defattrgroup(void)
		{
			return defattrs;
		}

		extern void attrlayer_accessed(t_attrgroup* attrgroup)
		{
			/* move the attrgroup at the end of loaded list for the "flush" loop */
			elist_del(&attrgroup->loadedlist);
			elist_add_tail(&loadedlist, &attrgroup->loadedlist);
		}

	}

}
