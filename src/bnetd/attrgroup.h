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

#ifndef __ATTRGROUP_H_INCLUDED__
#define __ATTRGROUP_H_INCLUDED__

#include <ctime>
#include "common/elist.h"

#ifndef JUST_NEED_TYPES
#define JUST_NEED_TYPES
#include "storage.h"
#undef JUST_NEED_TYPES
#else
#include "storage.h"
#endif

#define ATTRGROUP_FLAG_NONE	0
#define ATTRGROUP_FLAG_LOADED	1
#define ATTRGROUP_FLAG_ACCESSED	2
#define ATTRGROUP_FLAG_DIRTY	4

namespace pvpgn
{

	namespace bnetd
	{

		/* attrgroup represents a group of attributes which are read/saved/flush together
		 * ex: each account stores it's data into a attrgroup */
		typedef struct attrgroup_struct
#ifdef ATTRGROUP_INTERNAL_ACCESS
		{
			t_hlist		list;
			t_storage_info	*storage;
			int			flags;
			std::time_t		lastaccess;
			std::time_t		dirtytime;
			t_elist		loadedlist;
			t_elist		dirtylist;
		}
#endif
		t_attrgroup;

		typedef int(*t_attr_cb)(t_attrgroup *, void *);

		extern t_attrgroup *attrgroup_create_storage(t_storage_info *storage);
		extern t_attrgroup *attrgroup_create_newuser(const char *name);
		extern t_attrgroup *attrgroup_create_nameuid(const char *name, unsigned uid);
		extern int attrgroup_destroy(t_attrgroup *attrgroup);
		extern int attrgroup_load(t_attrgroup *attrgroup);
		extern int attrgroup_unload(t_attrgroup *attrgroup);
		extern int attrgroup_read_accounts(int flag, t_attr_cb cb, void *data);
		extern const char *attrgroup_get_attr(t_attrgroup *attrgroup, const char *key);
		extern int attrgroup_set_attr(t_attrgroup *attrgroup, const char *key, const char *val);
		extern int attrgroup_save(t_attrgroup *attrgroup, int flags);
		extern int attrgroup_flush(t_attrgroup *attrgroup, int flags);

	}

}

#endif /* __ATTRGROUP_H_INCLUDED__ */
