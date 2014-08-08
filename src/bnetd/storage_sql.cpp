/*
* Copyright (C) 2002 TheUndying
* Copyright (C) 2002 zap-zero
* Copyright (C) 2002,2003 Dizzy
* Copyright (C) 2002 Zzzoom
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
#include "storage_sql.h"

#include <cstring>
#include <cstdlib>

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

		static t_storage_info *sql_create_account(char const *);
		static int sql_read_attrs(t_storage_info *, t_read_attr_func, void *);
		static t_attr *sql_read_attr(t_storage_info *, const char *);
		static int sql_write_attrs(t_storage_info *, const t_hlist *);
		static t_storage_info * sql_read_account(const char *, unsigned);
		static const char *sql_escape_key(const char *);

		t_storage storage_sql = {
			sql_init,
			sql_close,
			sql_read_maxuserid,
			sql_create_account,
			sql_get_defacct,
			sql_free_info,
			sql_read_attrs,
			sql_write_attrs,
			sql_read_attr,
			sql_read_accounts,
			sql_read_account,
			sql_cmp_info,
			sql_escape_key,
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

#endif	/* SQL_ON_DEMAND */

		static int _db_get_tab(const char *key, char **ptab, char **pcol)
		{
			static char tab[DB_MAX_ATTRKEY];
			static char col[DB_MAX_ATTRKEY];

			std::strncpy(tab, key, DB_MAX_TAB - 1);
			tab[DB_MAX_TAB - 1] = 0;

			if (!std::strchr(tab, '_'))
				return -1;


			*(std::strchr(tab, '_')) = 0;
			std::strncpy(col, key + std::strlen(tab) + 1, DB_MAX_TAB - 1);
			col[DB_MAX_TAB - 1] = 0;
			/* return tab and col as 2 static buffers */
			*ptab = tab;
			*pcol = col;
			return 0;
		}

		static t_storage_info *sql_create_account(char const *username)
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
			snprintf(query, sizeof(query), "SELECT count(*) FROM %sBNET WHERE username='%s'", tab_prefix, user);

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
			snprintf(query, sizeof(query), "DELETE FROM %sBNET WHERE "SQL_UID_FIELD" = '%u'", tab_prefix, uid);
			sql->query(query);
			snprintf(query, sizeof(query), "INSERT INTO %sBNET ("SQL_UID_FIELD",username) VALUES('%u','%s')", tab_prefix, uid, user);
			if (sql->query(query))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "user insert failed (query: '%s')", query);
				goto err_info;
			}

			snprintf(query, sizeof(query), "DELETE FROM %sprofile WHERE "SQL_UID_FIELD" = '%u'", tab_prefix, uid);
			sql->query(query);
			snprintf(query, sizeof(query), "INSERT INTO %sprofile ("SQL_UID_FIELD") VALUES('%u')", tab_prefix, uid);
			if (sql->query(query))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "user insert failed (query: '%s')", query);
				goto err_info;
			}

			snprintf(query, sizeof(query), "DELETE FROM %sRecord WHERE "SQL_UID_FIELD" = '%u'", tab_prefix, uid);
			sql->query(query);
			snprintf(query, sizeof(query), "INSERT INTO %sRecord ("SQL_UID_FIELD") VALUES('%u')", tab_prefix, uid);
			if (sql->query(query))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "user insert failed (query: '%s')", query);
				goto err_info;
			}

			snprintf(query, sizeof(query), "DELETE FROM %sfriend WHERE "SQL_UID_FIELD" = '%u'", tab_prefix, uid);
			sql->query(query);
			snprintf(query, sizeof(query), "INSERT INTO %sfriend ("SQL_UID_FIELD") VALUES('%u')", tab_prefix, uid);
			if (sql->query(query))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "user insert failed (query: '%s')", query);
				goto err_info;
			}

			xfree(user);
			return info;

		err_info:
			xfree((void *)info);

		err_dup:
			xfree(user);

			return NULL;
		}

		static int sql_read_attrs(t_storage_info * info, t_read_attr_func cb, void *data)
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
				snprintf(query, sizeof(query), "SELECT * FROM %s%s WHERE "SQL_UID_FIELD"='%u'", tab_prefix, *tab, uid);

				//      eventlog(eventlog_level_trace, __FUNCTION__, "query: \"%s\"",query);

				if ((result = sql->query_res(query)) != NULL && sql->num_rows(result) == 1 && sql->num_fields(result) > 1)
				{
					unsigned int i;
					t_sql_field *fields, *fentry;

					if ((fields = sql->fetch_fields(result)) == NULL)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not fetch the fields");
						sql->free_result(result);
						return -1;
					}

					if (!(row = sql->fetch_row(result)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not fetch row");
						sql->free_fields(fields);
						sql->free_result(result);
						return -1;
					}

					for (i = 0, fentry = fields; *fentry; fentry++, i++)
					{			/* we have to skip "uid" */
						char *output;
						/* we ignore the field used internally by sql */
						if (std::strcmp(*fentry, SQL_UID_FIELD) == 0)
							continue;

						//              eventlog(eventlog_level_trace, __FUNCTION__, "read key (step1): '%s' val: '%s'", _db_add_tab(*tab, *fentry), unescape_chars(row[i]));
						if (row[i] == NULL)
							continue;	/* its an NULL value sql field */

						//              eventlog(eventlog_level_trace, __FUNCTION__, "read key (step2): '%s' val: '%s'", _db_add_tab(*tab, *fentry), unescape_chars(row[i]));
						if (cb(_db_add_tab(*tab, *fentry), (output = unescape_chars(row[i])), data))
							eventlog(eventlog_level_error, __FUNCTION__, "got error from callback on UID: %u", uid);
						if (output)
							xfree((void *)output);
						//              eventlog(eventlog_level_trace, __FUNCTION__, "read key (final): '%s' val: '%s'", _db_add_tab(*tab, *fentry), unescape_chars(row[i]));
					}

					sql->free_fields(fields);
				}
				if (result)
					sql->free_result(result);
			}
#endif				/* SQL_ON_DEMAND */
			return 0;
		}

		static t_attr *sql_read_attr(t_storage_info * info, const char *key)
		{
#ifdef SQL_ON_DEMAND
			t_sql_res *result = NULL;
			t_sql_row *row;
			char *tab, *col;
			unsigned int uid;
			t_attr    *attr;

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
				eventlog(eventlog_level_error, __FUNCTION__, "error from _db_get_tab");
				return NULL;
			}

			snprintf(query, sizeof(query), "SELECT `%s` FROM %s%s WHERE " SQL_UID_FIELD " = %u", col, tab_prefix, tab, uid);
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

			/* Because we are double quoting the column name
			sqlite3 treats it as a string when the column does not exist
			and returns the column name as the result.
			We need to return NULL under this condition */
			if (std::strcmp(row[0], col) == 0)
			{
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
		int sql_write_attrs(t_storage_info * info, const t_hlist *attrs)
		{
			char escape[DB_MAX_ATTRVAL * 2 + 1];	/* sql docs say the escape can take a maximum of double original size + 1 */
			char safeval[DB_MAX_ATTRVAL];
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
					eventlog(eventlog_level_error, __FUNCTION__, "error from _db_get_tab");
					continue;
				}

				std::strncpy(safeval, attr_get_val(attr), DB_MAX_ATTRVAL - 1);
				safeval[DB_MAX_ATTRVAL - 1] = 0;
				for (p = safeval; *p; p++)
				if (*p == '\'')	/* value shouldn't contain ' */
					*p = '"';

				sql->escape_string(escape, safeval, std::strlen(safeval));

				snprintf(query, sizeof(query), "UPDATE %s%s SET `%s` = '%s' WHERE "SQL_UID_FIELD" = '%u'", tab_prefix, tab, col, escape, uid);
				//      eventlog(eventlog_level_trace, "db_set", "update query: %s", query);

				if (sql->query(query) || !sql->affected_rows()) {
					char query2[512];

					//	    eventlog(eventlog_level_debug, __FUNCTION__, "trying to insert new column %s", col);
					snprintf(query2, sizeof(query2), "ALTER TABLE %s%s ADD COLUMN `%s` VARCHAR(128)", tab_prefix, tab, col);

					//          eventlog(eventlog_level_trace, __FUNCTION__, "alter query: %s", query2);
					sql->query(query2);

					/* try query again */
					//          eventlog(eventlog_level_trace, "db_set", "retry insert query: %s", query);
					if (sql->query(query) || !sql->affected_rows()) {
						// Tried everything, now trying to insert that user to the table for the first time
						snprintf(query2, sizeof(query2), "INSERT INTO %s%s ("SQL_UID_FIELD",`%s`) VALUES ('%u','%s')", tab_prefix, tab, col, uid, escape);
						//              eventlog(eventlog_level_error, __FUNCTION__, "update failed so tried INSERT for the last chance");
						if (sql->query(query2))
						{
							eventlog(eventlog_level_error, __FUNCTION__, "could not INSERT attribute '%s'->'%s'", attr_get_key(attr), attr_get_val(attr));
							continue;
						}
					}
				}

				attr_clear_dirty(attr);
			}

			return 0;
		}

		static t_storage_info * sql_read_account(const char *name, unsigned uid)
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

				snprintf(query, sizeof(query), "SELECT "SQL_UID_FIELD" FROM %sBNET WHERE username='%s'", tab_prefix, user);
				xfree(user);
			}
			else
				snprintf(query, sizeof(query), "SELECT "SQL_UID_FIELD" FROM %sBNET WHERE "SQL_UID_FIELD" = '%u'", tab_prefix, uid);
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

		static const char *sql_escape_key(const char *key)
		{
			const char *newkey = key;
			char *p;
			int idx;

			for (idx = 0, p = (char *)newkey; *p; p++, idx++)
			if (*p == '\\' || *p == '`' || *p == '"' || *p == '\'') {
				newkey = xstrdup(key);
				newkey = xstrdup(key);
				p = (char *)(newkey + idx);
				*(p++) = '_';
				for (; *p; p++)
				if (*p == '\\' || *p == '`' || *p == '"' || *p == '\'')
					*p = '_';
				break;
			}

			return newkey;
		}

	}

}

#endif				/* WITH_SQL */
