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
#include "attrgroup.h"

#include <cassert>
#include <cstring>

#include "common/eventlog.h"
#include "common/flags.h"
#include "common/xalloc.h"
#include "compat/strcasecmp.h"
#include "compat/strncasecmp.h"
#include "attr.h"
#include "attrlayer.h"
#include "storage.h"
#include "prefs.h"
#include "server.h"
#include "common/setup_after.h"


namespace pvpgn
{

	namespace bnetd
	{

		static inline void attrgroup_set_accessed(t_attrgroup *attrgroup)
		{
			FLAG_SET(&attrgroup->flags, ATTRGROUP_FLAG_ACCESSED);
			attrgroup->lastaccess = now;
			attrlayer_accessed(attrgroup);
		}

		static inline void attrgroup_clear_accessed(t_attrgroup *attrgroup)
		{
			FLAG_CLEAR(&attrgroup->flags, ATTRGROUP_FLAG_ACCESSED);
		}

		static inline void attrgroup_set_dirty(t_attrgroup *attrgroup)
		{
			if (FLAG_ISSET(attrgroup->flags, ATTRGROUP_FLAG_DIRTY)) return;

			attrgroup->dirtytime = now;
			FLAG_SET(&attrgroup->flags, ATTRGROUP_FLAG_DIRTY);
			attrlayer_add_dirtylist(&attrgroup->dirtylist);
		}

		static inline void attrgroup_clear_dirty(t_attrgroup *attrgroup)
		{
			if (!FLAG_ISSET(attrgroup->flags, ATTRGROUP_FLAG_DIRTY)) return;

			FLAG_CLEAR(&attrgroup->flags, ATTRGROUP_FLAG_DIRTY);
			attrlayer_del_dirtylist(&attrgroup->dirtylist);
		}

		static inline void attrgroup_set_loaded(t_attrgroup *attrgroup)
		{
			if (FLAG_ISSET(attrgroup->flags, ATTRGROUP_FLAG_LOADED)) return;
			FLAG_SET(&attrgroup->flags, ATTRGROUP_FLAG_LOADED);

			attrlayer_add_loadedlist(&attrgroup->loadedlist);
		}

		static inline void attrgroup_clear_loaded(t_attrgroup *attrgroup)
		{
			if (!FLAG_ISSET(attrgroup->flags, ATTRGROUP_FLAG_LOADED)) return;

			/* clear this because they are not valid if attrgroup is unloaded */
			attrgroup_clear_dirty(attrgroup);
			attrgroup_clear_accessed(attrgroup);

			FLAG_CLEAR(&attrgroup->flags, ATTRGROUP_FLAG_LOADED);
			attrlayer_del_loadedlist(&attrgroup->loadedlist);
		}

		static t_attrgroup * attrgroup_create(void)
		{
			t_attrgroup *attrgroup;

			attrgroup = (t_attrgroup*)xmalloc(sizeof(t_attrgroup));

			hlist_init(&attrgroup->list);
			attrgroup->storage = NULL;
			attrgroup->flags = ATTRGROUP_FLAG_NONE;
			attrgroup->lastaccess = 0;
			attrgroup->dirtytime = 0;
			elist_init(&attrgroup->loadedlist);
			elist_init(&attrgroup->dirtylist);

			return attrgroup;
		}

		extern t_attrgroup * attrgroup_create_storage(t_storage_info *storage)
		{
			t_attrgroup *attrgroup;

			attrgroup = attrgroup_create();
			attrgroup->storage = storage;

			return attrgroup;
		}

		extern t_attrgroup * attrgroup_create_newuser(const char *name)
		{
			t_attrgroup *attrgroup;
			t_storage_info *stmp;

			stmp = storage->create_account(name);
			if (!stmp) {
				eventlog(eventlog_level_error, __FUNCTION__, "failed to add user '%s' to storage", name);
				return NULL;
			}

			attrgroup = attrgroup_create_storage(stmp);

			/* new accounts are born loaded */
			attrgroup_set_loaded(attrgroup);

			return attrgroup;
		}

		extern t_attrgroup * attrgroup_create_nameuid(const char *name, unsigned uid)
		{
			t_storage_info *info;
			t_attrgroup *attrgroup;

			info = storage->read_account(name, uid);
			if (!info) return NULL;

			attrgroup = attrgroup_create_storage(info);

			return attrgroup;
		}

		extern int attrgroup_destroy(t_attrgroup *attrgroup)
		{
			if (!attrgroup) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL attrgroup");
				return -1;
			}

			attrgroup_unload(attrgroup);
			if (attrgroup->storage) storage->free_info(attrgroup->storage);
			xfree(attrgroup);

			return 0;
		}

		extern int attrgroup_save(t_attrgroup *attrgroup, int flags)
		{
			if (!attrgroup) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL attrgroup");
				return -1;
			}

			if (!FLAG_ISSET(attrgroup->flags, ATTRGROUP_FLAG_LOADED))
				return 0;

			if (!FLAG_ISSET(attrgroup->flags, ATTRGROUP_FLAG_DIRTY))
				return 0;

			if (!FLAG_ISSET(flags, FS_FORCE) && now - attrgroup->dirtytime < prefs_get_user_sync_timer())
				return 0;

			assert(attrgroup->storage);

			storage->write_attrs(attrgroup->storage, &attrgroup->list);
			attrgroup_clear_dirty(attrgroup);

			return 1;
		}

		extern int attrgroup_flush(t_attrgroup *attrgroup, int flags)
		{
			t_attr *attr;
			t_hlist *curr, *save;

			if (!attrgroup) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL attrgroup");
				return -1;
			}

			if (!FLAG_ISSET(attrgroup->flags, ATTRGROUP_FLAG_LOADED))
				return 0;

			if (!FLAG_ISSET(flags, FS_FORCE) &&
				FLAG_ISSET(attrgroup->flags, ATTRGROUP_FLAG_ACCESSED) &&
				now - attrgroup->lastaccess < prefs_get_user_flush_timer())
				return 0;

			/* sync data to disk if dirty */
			attrgroup_save(attrgroup, FS_FORCE);

			hlist_for_each_safe(curr, &attrgroup->list, save) {
				attr = hlist_entry(curr, t_attr, link);
				attr_destroy(attr);
			}
			hlist_init(&attrgroup->list);	/* reset list */

			attrgroup_clear_loaded(attrgroup);

			return 1;
		}

		static int _cb_load_attr(const char *key, const char *val, void *data)
		{
			t_attrgroup *attrgroup = (t_attrgroup *)data;

			return attrgroup_set_attr(attrgroup, key, val);
		}

		extern int attrgroup_load(t_attrgroup *attrgroup)
		{
			assert(attrgroup);
			assert(attrgroup->storage);

			if (FLAG_ISSET(attrgroup->flags, ATTRGROUP_FLAG_LOADED))	/* already done */
				return 0;
			if (FLAG_ISSET(attrgroup->flags, ATTRGROUP_FLAG_DIRTY)) { /* if not loaded, how dirty ? */
				eventlog(eventlog_level_error, __FUNCTION__, "can't load modified account");
				return -1;
			}

			attrgroup_set_loaded(attrgroup);
			if (storage->read_attrs(attrgroup->storage, _cb_load_attr, attrgroup)) {
				eventlog(eventlog_level_error, __FUNCTION__, "got error loading attributes");
				return -1;
			}
			attrgroup_clear_dirty(attrgroup);

			return 0;
		}

		extern int attrgroup_unload(t_attrgroup *attrgroup)
		{
			attrgroup_flush(attrgroup, FS_FORCE);

			return 0;
		}

		typedef struct {
			void *data;
			t_attr_cb cb;
		} t_attr_cb_data;

		static int _cb_read_accounts(t_storage_info *info, void *data)
		{
			t_attrgroup *attrgroup;
			t_attr_cb_data *cbdata = (t_attr_cb_data*)data;

			attrgroup = attrgroup_create_storage(info);
			return cbdata->cb(attrgroup, cbdata->data);
		}

		extern int attrgroup_read_accounts(int flag, t_attr_cb cb, void *data)
		{
			t_attr_cb_data cbdata;

			cbdata.cb = cb;
			cbdata.data = data;

			return storage->read_accounts(flag, _cb_read_accounts, &cbdata);
		}

		static const char *attrgroup_escape_key(const char *key)
		{
			const char *newkey, *newkey2;
			char *tmp;

			newkey = key;

			if (!strncasecmp(key, "DynKey", 6)) {
				/* OLD COMMENT, MIGHT NOT BE VALID ANYMORE
				 * Recent Starcraft clients seems to query DynKey\*\1\rank instead of
				 * Record\*\1\rank. So replace Dynkey with Record for key lookup.
				 */
				tmp = xstrdup(key);
				std::strncpy(tmp, "Record", 6);
				newkey = tmp;
			}
			else if (!std::strncmp(key, "Star", 4)) {
				/* OLD COMMENT
				 * Starcraft clients query Star instead of STAR on logon screen.
				 */
				tmp = xstrdup(key);
				std::strncpy(tmp, "STAR", 4);
				newkey = tmp;
			}

			if (newkey != key) {
				newkey2 = storage->escape_key(newkey);
				if (newkey2 != newkey) {
					xfree((void*)newkey);
					newkey = newkey2;
				}
			}
			else newkey = storage->escape_key(key);

			return newkey;
		}

		static t_attr *attrgroup_find_attr(t_attrgroup *attrgroup, const char *pkey[], int escape)
		{
			const char *val;
			t_hlist *curr, *last, *last2;
			t_attr *attr;

			assert(attrgroup);
			assert(pkey);
			assert(*pkey);

			/* trigger loading of attributes if not loaded already */
			if (attrgroup_load(attrgroup)) return NULL;	/* eventlog happens earlier */

			/* only if the callers tell us to */
			if (escape) *pkey = attrgroup_escape_key(*pkey);

			/* we are doing attribute lookup so we are accessing it */
			attrgroup_set_accessed(attrgroup);

			last = &attrgroup->list;
			last2 = NULL;

			hlist_for_each(curr, &attrgroup->list) {
				attr = hlist_entry(curr, t_attr, link);

				if (!strcasecmp(attr_get_key(attr), *pkey)) {
					val = attr_get_val(attr);
					/* key found, promote it so it's found faster next time */
					hlist_promote(curr, last, last2);
					break;
				}

				last2 = last; last = curr;
			}

			if (curr == &attrgroup->list) {	/* no key found in cached list */
				attr = (t_attr*)storage->read_attr(attrgroup->storage, *pkey);
				if (attr) hlist_add(&attrgroup->list, &attr->link);
			}

			/* "attr" here can either have a proper value found in the cached list, or
			 * a value returned by storage->read_attr, or NULL */
			return attr;
		}

		/* low-level get attr, receives a flag to tell if it needs to escape key */
		static const char *attrgroup_get_attrlow(t_attrgroup *attrgroup, const char *key, int escape)
		{
			const char *val = NULL;
			const char *newkey = key;
			t_attr *attr;

			/* no need to check for attrgroup, key */

			attr = attrgroup_find_attr(attrgroup, &newkey, escape);

			if (attr) val = attr_get_val(attr);

			if (!val && attrgroup != attrlayer_get_defattrgroup())
				val = attrgroup_get_attrlow(attrlayer_get_defattrgroup(), newkey, 0);

			if (newkey != key) xfree((void*)newkey);

			return val;
		}

		extern const char *attrgroup_get_attr(t_attrgroup *attrgroup, const char *key)
		{
			if (!attrgroup) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL attrgroup");
				return NULL;
			}

			if (!key) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key");
				return NULL;
			}

			return attrgroup_get_attrlow(attrgroup, key, 1);
		}

		extern int attrgroup_set_attr(t_attrgroup *attrgroup, const char *key, const char *val)
		{
			t_attr *attr;
			const char *newkey = key;

			if (!attrgroup) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL attrgroup");
				return -1;
			}

			if (!key) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key");
				return -1;
			}

			attr = attrgroup_find_attr(attrgroup, &newkey, 1);

			if (attr) {
				if (attr_get_val(attr) == val ||
					(attr_get_val(attr) && val && !std::strcmp(attr_get_val(attr), val)))
					goto out;	/* no need to modify anything, values are the same */

				/* new value for existent key, replace the old one */
				attr_set_val(attr, val);
			}
			else {	/* unknown key so add new attr */
				attr = attr_create(newkey, val);
				hlist_add(&attrgroup->list, &attr->link);
			}

			/* we have modified this attr and attrgroup */
			attr_set_dirty(attr);
			attrgroup_set_dirty(attrgroup);

		out:
			if (newkey != key) xfree((void*)newkey);

			return 0;
		}

	}

}
