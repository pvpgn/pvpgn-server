/*
 * Copyright (C) 2005 Dizzy
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
#ifdef WITH_SQL
#include "storage_sql2.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "compat/snprintf.h"

#include "common/eventlog.h"
#include "common/util.h"

#define SQL_INTERNAL
# include "sql_common.h"
#undef SQL_INTERNAL
#include "account.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static t_storage_info *sql2_create_account(char const *);
		static int sql2_read_attrs(t_storage_info *, t_read_attr_func, void *);
		static t_attr *sql2_read_attr(t_storage_info *, const char *);
		static int sql2_write_attrs(t_storage_info *, const t_hlist *);
		static t_storage_info * sql2_read_account(const char *, unsigned);
		static const char *sql2_escape_key(const char *);

		t_storage storage_sql2 = {
			sql_init,
			sql_close,
			sql_read_maxuserid,
			sql2_create_account,
			sql_get_defacct,
			sql_free_info,
			sql2_read_attrs,
			sql2_write_attrs,
			sql2_read_attr,
			sql_read_accounts,
			sql2_read_account,
			sql_cmp_info,
			sql2_escape_key,
			sql_load_clans,
			sql_write_clan,
			sql_remove_clan,
			sql_remove_clanmember,
			sql_load_teams,
			sql_write_team,
			sql_remove_team
		};

		static char query[512];

#ifndef SQL_ON_DEMAND

		static const char *_db_add_tab(const char *tab, const char *key)
		{
			static char nkey[DB_MAX_ATTRKEY];

			std::strncpy(nkey, tab, sizeof(nkey)-1);
			nkey[std::strlen(nkey) + 1] = '\0';
			nkey[std::strlen(nkey)] = '_';
			std::strncpy(nkey + std::strlen(nkey), key, sizeof(nkey)-std::strlen(nkey));
			return nkey;
		}

#endif				/* SQL_ON_DEMAND */

		static int _db_get_tab(const char *key, char **ptab, char **pcol)
		{
			static char tab[DB_MAX_ATTRKEY];
			static char col[DB_MAX_ATTRKEY];
			char *p;

			std::strncpy(tab, key, DB_MAX_TAB - 1);
			tab[DB_MAX_TAB - 1] = 0;

			if (!(p = std::strchr(tab, '\\')))
				return -1;
			*p = 0;

			std::strncpy(col, key + std::strlen(tab) + 1, DB_MAX_ATTRKEY - 1);
			col[DB_MAX_ATTRKEY - 1] = 0;
			/* return tab and col as 2 static buffers */
			*ptab = tab;
			*pcol = col;
			return 0;
		}

		static t_storage_info *sql2_create_account(char const *username)
		{
			t_sql_res *result = NULL;
			t_sql_row *row;
			int uid = maxuserid + 1;
			t_storage_info *info;
			char *user;

			if (!sql)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql layer not initilized");
				return NULL;
			}

			user = xstrdup(username);
			strlower(user);
			snprintf(query, sizeof(query), "SELECT count(*) FROM %sBNET WHERE name = 'username' and value = '%s'", tab_prefix, user);

			if ((result = sql->query_res(query)) != NULL)
			{
				int num;

				row = sql->fetch_row(result);
				if (row == NULL || row[0] == NULL)
				{
					sql->free_result(result);
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL count");
					goto err_dup;
				}
				num = std::atol(row[0]);
				sql->free_result(result);
				if (num > 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got existant username");
					goto err_dup;
				}
			}
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "error trying query: \"%s\"", query);
				goto err_dup;
			}

			info = xmalloc(sizeof(t_sql_info));
			*((unsigned int *)info) = uid;
			snprintf(query, sizeof(query), "DELETE FROM %sBNET WHERE " SQL_UID_FIELD " = '%u'", tab_prefix, uid);
			sql->query(query);
			snprintf(query, sizeof(query), "INSERT INTO %sBNET (" SQL_UID_FIELD ", name, value) VALUES('%u', 'username', '%s')", tab_prefix, uid, user);
			if (sql->query(query))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "user insert failed");
				goto err_info;
			}

			snprintf(query, sizeof(query), "DELETE FROM %sprofile WHERE " SQL_UID_FIELD " = '%u'", tab_prefix, uid);
			sql->query(query);

			snprintf(query, sizeof(query), "DELETE FROM %sRecord WHERE " SQL_UID_FIELD " = '%u'", tab_prefix, uid);
			sql->query(query);

			snprintf(query, sizeof(query), "DELETE FROM %sfriend WHERE " SQL_UID_FIELD " = '%u'", tab_prefix, uid);
			sql->query(query);

			xfree(user);
			return info;

		err_info:
			xfree((void *)info);

		err_dup:
			xfree(user);

			return NULL;
		}

		static int sql2_read_attrs(t_storage_info * info, t_read_attr_func cb, void *data)
		{
#ifndef SQL_ON_DEMAND
			t_sql_res *result = NULL;
			t_sql_row *row;
			char **tab;
			unsigned int uid;

			if (!sql)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql layer not initilized");
				return -1;
			}

			if (info == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL storage info");
				return -1;
			}

			if (cb == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL callback");
				return -1;
			}

			uid = *((unsigned int *)info);

			for (tab = sql_tables; *tab; tab++)
			{
				snprintf(query, sizeof(query), "SELECT (name, value) FROM %s%s WHERE " SQL_UID_FIELD " = '%u'", tab_prefix, *tab, uid);

				//      eventlog(eventlog_level_trace, __FUNCTION__, "query: \"%s\"",query);

				result = sql->query_res(query);
				if (!result) continue;

				if (sql->num_fields != 2) {
					sql->free_result(result);
					continue;
				}

				for (row = sql->fetch_row(result); row; row = sql->fetch_row(result))
				{
					if (cb(_db_add_tab(*tab, row[0]), row[1], data))
						ERROR1("got error from callback on UID: %u", uid);
				}
				sql->free_result(result);
			}
#endif				/* SQL_ON_DEMAND */
			return 0;
		}

		static t_attr *sql2_read_attr(t_storage_info * info, const char *key)
		{
#ifdef SQL_ON_DEMAND
			t_sql_res *result = NULL;
			t_sql_row *row;
			char *tab, *col;
			unsigned int uid;
			t_attr    *attr;
			char esckey[DB_MAX_ATTRKEY * 2 + 1];

			if (!sql)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql layer not initilized");
				return NULL;
			}

			if (info == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL storage info");
				return NULL;
			}

			if (key == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key");
				return NULL;
			}

			uid = *((unsigned int *)info);

			if (_db_get_tab(key, &tab, &col) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "error from db_get_tab");
				return NULL;
			}
			sql->escape_string(esckey, col, std::strlen(col));

			snprintf(query, sizeof(query), "SELECT value FROM %s%s WHERE " SQL_UID_FIELD " = '%u' and name= '%s'", tab_prefix, tab, uid, esckey);
			if ((result = sql->query_res(query)) == NULL)
				return NULL;

			if (sql->num_rows(result) != 1)
			{
				//      eventlog(eventlog_level_debug, __FUNCTION__, "wrong numer of rows from query (%s)", query);
				sql->free_result(result);
				return NULL;
			}

			if (!(row = sql->fetch_row(result)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not fetch row");
				sql->free_result(result);
				return NULL;
			}

			if (row[0] == NULL)
			{
				//      eventlog(eventlog_level_debug, __FUNCTION__, "NULL value from query (%s)", query);
				sql->free_result(result);
				return NULL;
			}

			attr = attr_create(key, row[0]);
			sql->free_result(result);

			return attr;
#else
			return NULL;
#endif				/* SQL_ON_DEMAND */
		}

		/* write ONLY dirty attributes */
		static int sql2_write_attrs(t_storage_info * info, const t_hlist *attrs)
		{
			char escval[DB_MAX_ATTRVAL * 2 + 1];	/* sql docs say the escape can take a maximum of double original size + 1 */
			char safeval[DB_MAX_ATTRVAL];
			char esckey[DB_MAX_ATTRKEY * 2 + 1];
			char *p, *tab, *col;
			t_attr *attr;
			t_hlist *curr;
			unsigned int uid;

			if (!sql)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql layer not initilized");
				return -1;
			}

			if (info == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL sql info");
				return -1;
			}

			if (attrs == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL attributes list");
				return -1;
			}

			uid = *((unsigned int *)info);

			hlist_for_each(curr, (t_hlist*)attrs) {
				attr = hlist_entry(curr, t_attr, link);

				if (!attr_get_dirty(attr))
					continue;		/* save ONLY dirty attributes */

				if (attr_get_key(attr) == NULL) {
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL key in attributes list");
					continue;
				}

				if (attr_get_val(attr) == NULL)	{
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL value in attributes list");
					continue;
				}

				if (_db_get_tab(attr_get_key(attr), &tab, &col) < 0) {
					eventlog(eventlog_level_error, __FUNCTION__, "error from db_get_tab");
					continue;
				}

				sql->escape_string(esckey, col, std::strlen(col));

				std::strncpy(safeval, attr_get_val(attr), DB_MAX_ATTRVAL - 1);
				safeval[DB_MAX_ATTRVAL - 1] = 0;
				sql->escape_string(escval, safeval, std::strlen(safeval));

				snprintf(query, sizeof(query), "UPDATE %s%s SET value = '%s' WHERE " SQL_UID_FIELD " = '%u' AND name = '%s'", tab_prefix, tab, escval, uid, esckey);

				//	eventlog(eventlog_level_trace, "db_set", "update query: %s", query);

				if (sql->query(query) || !sql->affected_rows()) {
					snprintf(query, sizeof(query), "INSERT INTO %s%s (" SQL_UID_FIELD ", name, value) VALUES ('%u', '%s', '%s')", tab_prefix, tab, uid, esckey, escval);
					//		eventlog(eventlog_level_trace, "db_set", "retry insert query: %s", query);
					if (sql->query(query) || !sql->affected_rows()) {
						eventlog(eventlog_level_error, __FUNCTION__, "could not INSERT attribute '%s'->'%s'", attr_get_key(attr), attr_get_val(attr));
						continue;
					}
				}

				attr_clear_dirty(attr);
			}

			return 0;
		}

		static t_storage_info * sql2_read_account(const char *name, unsigned uid)
		{
			t_sql_res *result = NULL;
			t_sql_row *row;
			t_storage_info *info;

			if (!sql)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql layer not initilized");
				return NULL;
			}

			/* SELECT uid from BNET WHERE uid=x sounds stupid, I agree but its a clean
			 * way to check for account existence by an uid */
			if (name) {
				char *user = xstrdup(name);
				strlower(user);

				snprintf(query, sizeof(query), "SELECT " SQL_UID_FIELD " FROM %sBNET WHERE name = 'username' AND value ='%s'", tab_prefix, user);
				xfree(user);
			}
			else
				snprintf(query, sizeof(query), "SELECT " SQL_UID_FIELD " FROM %sBNET WHERE " SQL_UID_FIELD "= '%u'", tab_prefix, uid);
			result = sql->query_res(query);
			if (!result) {
				eventlog(eventlog_level_error, __FUNCTION__, "error query db (query:\"%s\")", query);
				return NULL;
			}

			if (sql->num_rows(result) < 1)
			{
				sql->free_result(result);
				return NULL;	/* empty user list */
			}

			row = sql->fetch_row(result);
			if (!row) {
				/* could not fetch row, this should not happen */
				sql->free_result(result);
				return NULL;
			}

			if (row[0] == NULL)
				/* empty UID field */
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL uid from db");
			else if ((unsigned int)std::atoi(row[0]) == sql_defacct);
			/* skip default account */
			else {
				info = xmalloc(sizeof(t_sql_info));
				*((unsigned int *)info) = std::atoi(row[0]);
				sql->free_result(result);
				return info;
			}

			sql->free_result(result);
			return NULL;
		}

		static const char *sql2_escape_key(const char *key)
		{
			return key;
		}

	}

}

#endif				/* WITH_SQL */
