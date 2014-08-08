/*
 * Copyright (C) 2002,2003,2005 Dizzy
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

#ifndef INCLUDED_SQL_COMMON_TYPES
#define INCLUDED_SQL_COMMON_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		typedef unsigned int t_sql_info;

		/* used as a pointer to it */
#define t_sql_res void

		typedef char * t_sql_row;

		typedef char * t_sql_field;

		typedef struct {
			int(*init)(const char *host, const char *port, const char *socket, const char *name, const char *user, const char *pass);
			int(*close)(void);
			t_sql_res * (*query_res)(const char *);
			int(*query)(const char *);
			t_sql_row * (*fetch_row)(t_sql_res *);
			void(*free_result)(t_sql_res *);
			unsigned int(*num_rows)(t_sql_res *);
			unsigned int(*num_fields)(t_sql_res *);
			unsigned int(*affected_rows)(void);
			t_sql_field * (*fetch_fields)(t_sql_res *);
			int(*free_fields)(t_sql_field *);
			void(*escape_string)(char *, const char *, int);
		} t_sql_engine;

	}

}

#endif /* INCLUDED_SQL_COMMON_TYPES */

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_SQL_COMMON_PROTOS
#define INCLUDED_SQL_COMMON_PROTOS

#include "storage.h"

namespace pvpgn
{

	namespace bnetd
	{

		extern t_storage storage_sql;

#ifdef SQL_INTERNAL

#define CURRENT_DB_VERSION 150

#define DB_MAX_ATTRKEY	128
#define DB_MAX_ATTRVAL  180
#define DB_MAX_TAB	64

#define SQL_UID_FIELD		"uid"
#define STORAGE_SQL_DEFAULT_UID	0
#define SQL_DEFAULT_PREFIX	""

#define SQL_ON_DEMAND	1

		extern t_sql_engine *sql;
		extern unsigned int sql_defacct;
		extern const char* tab_prefix;

#ifndef SQL_ON_DEMAND
		extern char *sql_tables[];
#endif /* SQL_ON_DEMAND */

		extern int sql_init(const char *);
		extern int sql_close(void);
		extern unsigned sql_read_maxuserid(void);
		extern int sql_read_accounts(int flag, t_read_accounts_func cb, void *data);
		extern int sql_cmp_info(t_storage_info * info1, t_storage_info * info2);
		extern int sql_free_info(t_storage_info * info);
		extern t_storage_info *sql_get_defacct(void);
		extern int sql_load_clans(t_load_clans_func cb);
		extern int sql_write_clan(void *data);
		extern int sql_remove_clan(int clantag);
		extern int sql_remove_clanmember(int uid);
		extern int sql_load_teams(t_load_teams_func cb);
		extern int sql_write_team(void *data);
		extern int sql_remove_team(unsigned int teamid);

#endif /* SQL_INTERNAL */

	}

}

#endif /* INCLUDED_SQL_COMMON_PROTOS */
#endif /* JUST_NEED_TYPES */
