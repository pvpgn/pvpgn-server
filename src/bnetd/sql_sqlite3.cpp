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

#ifdef WITH_SQL_SQLITE3
#include "common/setup_before.h"
# include <sqlite3.h>
#include <cstdlib>
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "storage_sql.h"
#include "sql_sqlite3.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		typedef struct {
			char** results;
			int rows;
			int columns;
			int crow;
		} t_sqlite3_res;

		static int sql_sqlite3_init(const char *, const char *, const char *, const char *, const char *, const char *);
		static int sql_sqlite3_close(void);
		static t_sql_res * sql_sqlite3_query_res(const char *);
		static int sql_sqlite3_query(const char *);
		static t_sql_row * sql_sqlite3_fetch_row(t_sql_res *);
		static void sql_sqlite3_free_result(t_sql_res *);
		static unsigned int sql_sqlite3_num_rows(t_sql_res *);
		static unsigned int sql_sqlite3_num_fields(t_sql_res *);
		static unsigned int sql_sqlite3_affected_rows(void);
		static t_sql_field * sql_sqlite3_fetch_fields(t_sql_res *);
		static int sql_sqlite3_free_fields(t_sql_field *);
		static void sql_sqlite3_escape_string(char *, const char *, int);

		t_sql_engine sql_sqlite3 = {
			sql_sqlite3_init,
			sql_sqlite3_close,
			sql_sqlite3_query_res,
			sql_sqlite3_query,
			sql_sqlite3_fetch_row,
			sql_sqlite3_free_result,
			sql_sqlite3_num_rows,
			sql_sqlite3_num_fields,
			sql_sqlite3_affected_rows,
			sql_sqlite3_fetch_fields,
			sql_sqlite3_free_fields,
			sql_sqlite3_escape_string
		};

		static sqlite3 *db = NULL;

#ifndef RUNTIME_LIBS
# define p_sqlite3_changes	sqlite3_changes
# define p_sqlite3_close	sqlite3_close
# define p_sqlite3_errmsg	sqlite3_errmsg
# define p_sqlite3_exec		sqlite3_exec
# define p_sqlite3_free_table	sqlite3_free_table
# define p_sqlite3_get_table	sqlite3_get_table
# define p_sqlite3_open		sqlite3_open
# define p_sqlite3_snprintf	sqlite3_snprintf
#else
		/* RUNTIME_LIBS */
		static int sqlite_load_library(void);

		typedef int(*f_sqlite3_changes)(sqlite3*);
		typedef int(*f_sqlite3_close)(sqlite3*);
		typedef const char*	(*f_sqlite3_errmsg)(sqlite3*);
		typedef int(*f_sqlite3_exec)(sqlite3*, const char*, sqlite3_callback, void*, char**);
		typedef void(*f_sqlite3_free_table)(char **);
		typedef int(*f_sqlite3_get_table)(sqlite3*, const char*, char***, int*, int*, char**);
		typedef int(*f_sqlite3_open)(const char*, sqlite3**);
		typedef char*		(*f_sqlite3_snprintf)(int, char*, const char*, ...);

		static f_sqlite3_changes	p_sqlite3_changes = NULL;
		static f_sqlite3_close		p_sqlite3_close = NULL;
		static f_sqlite3_errmsg		p_sqlite3_errmsg = NULL;
		static f_sqlite3_exec		p_sqlite3_exec = NULL;
		static f_sqlite3_free_table	p_sqlite3_free_table = NULL;
		static f_sqlite3_get_table	p_sqlite3_get_table = NULL;
		static f_sqlite3_open		p_sqlite3_open = NULL;
		static f_sqlite3_snprintf	p_sqlite3_snprintf = NULL;

#include "compat/runtime_libs.h" /* defines OpenLibrary(), GetFunction(), CloseLibrary() & SQLITE3_LIB */

		static void * handle = NULL;

		static int sqlite_load_library(void)
		{
			if ((handle = OpenLibrary(SQLITE3_LIB)) == NULL) return -1;

			if (((p_sqlite3_changes = (f_sqlite3_changes)GetFunction(handle, "sqlite3_changes")) == NULL) ||
				((p_sqlite3_close = (f_sqlite3_close)GetFunction(handle, "sqlite3_close")) == NULL) ||
				((p_sqlite3_errmsg = (f_sqlite3_errmsg)GetFunction(handle, "sqlite3_errmsg")) == NULL) ||
				((p_sqlite3_exec = (f_sqlite3_exec)GetFunction(handle, "sqlite3_exec")) == NULL) ||
				((p_sqlite3_free_table = (f_sqlite3_free_table)GetFunction(handle, "sqlite3_free_table")) == NULL) ||
				((p_sqlite3_get_table = (f_sqlite3_get_table)GetFunction(handle, "sqlite3_get_table")) == NULL) ||
				((p_sqlite3_open = (f_sqlite3_open)GetFunction(handle, "sqlite3_open")) == NULL) ||
				((p_sqlite3_snprintf = (f_sqlite3_snprintf)GetFunction(handle, "sqlite3_snprintf")) == NULL))
			{
				CloseLibrary(handle);
				handle = NULL;
				return -1;
			}

			return 0;
		}
#endif /* RUNTIME_LIBS */

		static int sql_sqlite3_init(const char *host, const char *port, const char *socket, const char *name, const char *user, const char *pass)
		{
#ifdef RUNTIME_LIBS
			if (sqlite_load_library()) {
				eventlog(eventlog_level_error, __FUNCTION__, "error loading library file \"%s\"", SQLITE3_LIB);
				return -1;
			}
#endif
			/* SQLite3 has no host, port, socket, user or password and the database name is the path to the db file */
			if (p_sqlite3_open(name, &db) != SQLITE_OK) {
				eventlog(eventlog_level_error, __FUNCTION__, "got error from sqlite3_open (%s)", p_sqlite3_errmsg(db));
				p_sqlite3_close(db);
				return -1;
			}

			return 0;
		}

		static int sql_sqlite3_close(void)
		{
			if (db) {
				if (p_sqlite3_close(db) != SQLITE_OK) {
					eventlog(eventlog_level_error, __FUNCTION__, "got error from sqlite3_close (%s)", p_sqlite3_errmsg(db));
					return -1;
				}
				db = NULL;
			}
#ifdef RUNTIME_LIBS
			if (handle) {
				CloseLibrary(handle);
				handle = NULL;
			}
#endif
			return 0;
		}

		static t_sql_res * sql_sqlite3_query_res(const char* query)
		{
			t_sqlite3_res *res;

			if (db == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "sqlite3 driver not initilized");
				return NULL;
			}

			if (query == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL query");
				return NULL;
			}

			res = (t_sqlite3_res *)xmalloc(sizeof(t_sqlite3_res));

			if (p_sqlite3_get_table(db, query, &res->results, &res->rows, &res->columns, NULL) != SQLITE_OK) {
				/*        eventlog(eventlog_level_debug, __FUNCTION__, "got error (%s) from query (%s)", p_sqlite3_errmsg(db), query); */
				xfree((void*)res);
				return NULL;
			}

			res->crow = 0;

			return res;
		}

		static int sql_sqlite3_query(const char* query)
		{
			if (db == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "sqlite3 driver not initilized");
				return -1;
			}

			if (query == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL query");
				return -1;
			}

			return p_sqlite3_exec(db, query, NULL, NULL, NULL) == SQLITE_OK ? 0 : -1;
		}

		static t_sql_row * sql_sqlite3_fetch_row(t_sql_res *result)
		{
			t_sqlite3_res *res = (t_sqlite3_res*)result;

			if (result == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL result");
				return NULL;
			}

			return res->crow < res->rows ? (res->results + res->columns * (++res->crow)) : NULL;
		}

		static void sql_sqlite3_free_result(t_sql_res *result)
		{
			if (result == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL result");
				return;
			}

			p_sqlite3_free_table(((t_sqlite3_res *)result)->results);
			xfree(result);
		}

		static unsigned int sql_sqlite3_num_rows(t_sql_res *result)
		{
			if (result == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL result");
				return 0;
			}

			return ((t_sqlite3_res *)result)->rows;
		}

		static unsigned int sql_sqlite3_num_fields(t_sql_res *result)
		{
			if (result == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL result");
				return 0;
			}

			return ((t_sqlite3_res *)result)->columns;
		}

		static unsigned int sql_sqlite3_affected_rows(void)
		{
			return p_sqlite3_changes(db);
		}

		static t_sql_field * sql_sqlite3_fetch_fields(t_sql_res *result)
		{
			if (result == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL result");
				return NULL;
			}

			/* the first "row" is actually the field names */
			return ((t_sqlite3_res*)result)->results;
		}

		static int sql_sqlite3_free_fields(t_sql_field *fields)
		{
			if (fields == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL fields");
				return -1;
			}

			return 0; /* sqlite3_free_table() should free it properly */
		}

		static void sql_sqlite3_escape_string(char *escape, const char *from, int len)
		{
			if (db == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "sqlite3 driver not initilized");
				return;
			}
			p_sqlite3_snprintf(len * 2 + 1, escape, "%q", from);
		}

	}

}

#endif /* WITH_SQL_SQLITE3 */
