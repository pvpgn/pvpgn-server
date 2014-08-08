/*
  * Copyright (C) 2002,2003 Dizzy
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

#ifndef INCLUDED_STORAGE_TYPES
#define INCLUDED_STORAGE_TYPES

#include "common/elist.h"
#include "attr.h"

/* flags to tell if calling codes really want all accounts loading */
#define ST_NONE		0
#define ST_FORCE	1

namespace pvpgn
{

	namespace bnetd
	{

		typedef const void t_storage_info;
		typedef int(*t_read_attr_func)(const char *, const char *, void *);
		typedef int(*t_read_accounts_func)(t_storage_info *, void*);
		typedef int(*t_load_clans_func)(void*);
		typedef int(*t_load_teams_func)(void*);

		typedef struct {
			int(*init)(const char *);
			int(*close)(void);
			unsigned(*read_maxuserid)(void);
			t_storage_info * (*create_account)(char const *);
			t_storage_info * (*get_defacct)(void);
			int(*free_info)(t_storage_info *);
			int(*read_attrs)(t_storage_info *, t_read_attr_func, void *);
			int(*write_attrs)(t_storage_info *, const t_hlist *);
			t_attr * (*read_attr)(t_storage_info *, const char *);
			int(*read_accounts)(int, t_read_accounts_func, void *);
			t_storage_info * (*read_account)(const char *, unsigned);
			int(*cmp_info)(t_storage_info *, t_storage_info *);
			const char * (*escape_key)(const char *);
			int(*load_clans)(t_load_clans_func);
			int(*write_clan)(void *);
			int(*remove_clan)(int);
			int(*remove_clanmember)(int);
			int(*load_teams)(t_load_teams_func);
			int(*write_team)(void *);
			int(*remove_team)(unsigned int);
		} t_storage;

	}

}

#endif /* INCLUDED_STORAGE_TYPES */

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_STORAGE_PROTOS
#define INCLUDED_STORAGE_PROTOS

namespace pvpgn
{

	namespace bnetd
	{

		extern t_storage *storage;

		extern int storage_init(const char *);
		extern void storage_close(void);

	}

}

#endif /* INCLUDED_STORAGE_PROTOS */
#endif /* JUST_NEED_TYPES */
