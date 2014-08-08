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

#ifdef WITH_SQL_MYSQL
#include "common/setup_before.h"
#define NO_CLIENT_LONG_LONG
#include "sql_mysql.h"

#ifdef HAVE_WINDOWS_H
#include <windows.h>
#endif
#include <mysql.h>
#include <cstdlib>

#include "common/eventlog.h"
#include "common/xalloc.h"
#include "storage_sql.h"

#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static int sql_mysql_init(const char *, const char *, const char *, const char *, const char *, const char *);
		static int sql_mysql_close(void);
		static t_sql_res * sql_mysql_query_res(const char *);
		static int sql_mysql_query(const char *);
		static t_sql_row * sql_mysql_fetch_row(t_sql_res *);
		static void sql_mysql_free_result(t_sql_res *);
		static unsigned int sql_mysql_num_rows(t_sql_res *);
		static unsigned int sql_mysql_num_fields(t_sql_res *);
		static unsigned int sql_mysql_affected_rows(void);
		static t_sql_field * sql_mysql_fetch_fields(t_sql_res *);
		static int sql_mysql_free_fields(t_sql_field *);
		static void sql_mysql_escape_string(char *, const char *, int);

		t_sql_engine sql_mysql = {
			sql_mysql_init,
			sql_mysql_close,
			sql_mysql_query_res,
			sql_mysql_query,
			sql_mysql_fetch_row,
			sql_mysql_free_result,
			sql_mysql_num_rows,
			sql_mysql_num_fields,
			sql_mysql_affected_rows,
			sql_mysql_fetch_fields,
			sql_mysql_free_fields,
			sql_mysql_escape_string
		};

		static MYSQL *mysql = NULL;

#ifndef RUNTIME_LIBS
#define p_mysql_affected_rows		mysql_affected_rows
#define p_mysql_close			mysql_close
#define p_mysql_error			mysql_error
#define p_mysql_fetch_fields		mysql_fetch_fields
#define p_mysql_fetch_row		mysql_fetch_row
#define p_mysql_free_result		mysql_free_result
#define p_mysql_init			mysql_init
#define p_mysql_num_fields		mysql_num_fields
#define p_mysql_num_rows		mysql_num_rows
#define p_mysql_query			mysql_query
#define p_mysql_real_connect		mysql_real_connect
#define p_mysql_real_escape_string	mysql_real_escape_string
#define p_mysql_store_result		mysql_store_result
#else
		/* RUNTIME_LIBS */
		static int mysql_load_dll(void);

		typedef my_ulonglong(STDCALL *f_mysql_affected_rows)(MYSQL*);
		typedef void		(STDCALL *f_mysql_close)(MYSQL*);
		typedef const char*	(STDCALL *f_mysql_error)(MYSQL*);
		typedef MYSQL_FIELD*	(STDCALL *f_mysql_fetch_fields)(MYSQL_RES*);
		typedef MYSQL_ROW(STDCALL *f_mysql_fetch_row)(MYSQL_RES*);
		typedef my_bool(STDCALL *f_mysql_free_result)(MYSQL_RES*);
		typedef MYSQL*		(STDCALL *f_mysql_init)(MYSQL*);
		typedef unsigned int	(STDCALL *f_mysql_num_fields)(MYSQL_RES*);
		typedef my_ulonglong(STDCALL *f_mysql_num_rows)(MYSQL_RES*);
		typedef int		(STDCALL *f_mysql_query)(MYSQL*, const char*);
		typedef MYSQL*		(STDCALL *f_mysql_real_connect)(MYSQL*, const char*, const char*, const char*, const char*, unsigned int, const char*, unsigned long);
		typedef unsigned long	(STDCALL *f_mysql_real_escape_string)(MYSQL*, char*, const char*, unsigned long);
		typedef MYSQL_RES*	(STDCALL *f_mysql_store_result)(MYSQL*);

		static f_mysql_affected_rows		p_mysql_affected_rows;
		static f_mysql_close			p_mysql_close;
		static f_mysql_error			p_mysql_error;
		static f_mysql_fetch_fields		p_mysql_fetch_fields;
		static f_mysql_fetch_row		p_mysql_fetch_row;
		static f_mysql_free_result		p_mysql_free_result;
		static f_mysql_init			p_mysql_init;
		static f_mysql_num_fields		p_mysql_num_fields;
		static f_mysql_num_rows			p_mysql_num_rows;
		static f_mysql_query			p_mysql_query;
		static f_mysql_real_connect		p_mysql_real_connect;
		static f_mysql_real_escape_string	p_mysql_real_escape_string;
		static f_mysql_store_result		p_mysql_store_result;

#include "compat/runtime_libs.h" /* defines OpenLibrary(), GetFunction(), CloseLibrary() & MYSQL_LIB */

		static void * handle = NULL;

		static int mysql_load_dll(void)
		{
			if ((handle = OpenLibrary(MYSQL_LIB)) == NULL) return -1;

			if (((p_mysql_affected_rows = (f_mysql_affected_rows)GetFunction(handle, "mysql_affected_rows")) == NULL) ||
				((p_mysql_close = (f_mysql_close)GetFunction(handle, "mysql_close")) == NULL) ||
				((p_mysql_error = (f_mysql_error)GetFunction(handle, "mysql_error")) == NULL) ||
				((p_mysql_fetch_fields = (f_mysql_fetch_fields)GetFunction(handle, "mysql_fetch_fields")) == NULL) ||
				((p_mysql_fetch_row = (f_mysql_fetch_row)GetFunction(handle, "mysql_fetch_row")) == NULL) ||
				((p_mysql_free_result = (f_mysql_free_result)GetFunction(handle, "mysql_free_result")) == NULL) ||
				((p_mysql_init = (f_mysql_init)GetFunction(handle, "mysql_init")) == NULL) ||
				((p_mysql_num_fields = (f_mysql_num_fields)GetFunction(handle, "mysql_num_fields")) == NULL) ||
				((p_mysql_num_rows = (f_mysql_num_rows)GetFunction(handle, "mysql_num_rows")) == NULL) ||
				((p_mysql_query = (f_mysql_query)GetFunction(handle, "mysql_query")) == NULL) ||
				((p_mysql_real_connect = (f_mysql_real_connect)GetFunction(handle, "mysql_real_connect")) == NULL) ||
				((p_mysql_real_escape_string = (f_mysql_real_escape_string)GetFunction(handle, "mysql_real_escape_string")) == NULL) ||
				((p_mysql_store_result = (f_mysql_store_result)GetFunction(handle, "mysql_store_result")) == NULL))
			{
				CloseLibrary(handle);
				handle = NULL;
				return -1;
			}

			return 0;
		}
#endif /* RUNTIME_LIBS */

		static int sql_mysql_init(const char *host, const char *port, const char *socket, const char *name, const char *user, const char *pass)
		{
			if (name == NULL || user == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL parameter");
				return -1;
			}
#ifdef RUNTIME_LIBS
			if (mysql_load_dll()) {
				eventlog(eventlog_level_error, __FUNCTION__, "error loading library file \"%s\"", MYSQL_LIB);
				return -1;
			}
#endif
			if ((mysql = p_mysql_init(NULL)) == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got error from mysql_init");
				return -1;
			}

#if MYSQL_VERSION_ID >= 50003
#if MYSQL_VERSION_ID < 50013
			eventlog(eventlog_level_warn, __FUNCTION__, "Your mySQL client lib version does not support reconnecting after a timeout.");
			eventlog(eventlog_level_warn, __FUNCTION__, "If this causes you any trouble you are advices to upgrade");
			eventlog(eventlog_level_warn, __FUNCTION__, "your mySQL client libs to at least mySQL 5.0.13 to resolve this problem.");
			// we might try a dirty hack like the following, but I'm not sure if it will work
			// mysql->reconnect = 1;
#endif
#if MYSQL_VERSION_ID >= 50019
			my_bool  my_true = true;
			if (mysql_options(mysql, MYSQL_OPT_RECONNECT, &my_true)){
				eventlog(eventlog_level_warn, __FUNCTION__, "Failed to turn on MYSQL_OPT_RECONNECT.");
			}
			else{
				eventlog(eventlog_level_info, __FUNCTION__, "Successfully turned on MYSQL_OPT_RECONNECT.");
			}
#endif
#endif

			if (p_mysql_real_connect(mysql, host, user, pass, name, port ? atoi(port) : 0, socket, CLIENT_FOUND_ROWS) == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "error connecting to database (db said: '%s')", p_mysql_error(mysql));
				p_mysql_close(mysql);
				return -1;
			}

#if MYSQL_VERSION_ID >= 50013
#if MYSQL_VERSION_ID < 50019
			my_bool  my_true = true;
			if (mysql_options(mysql, MYSQL_OPT_RECONNECT, &my_true)){
				eventlog(eventlog_level_warn, __FUNCTION__, "Failed to turn on MYSQL_OPT_RECONNECT.");
			}
			else{
				eventlog(eventlog_level_info, __FUNCTION__, "Successfully turned on MYSQL_OPT_RECONNECT.");
			}
#endif
#endif

			/* allows identifers (specificly column names) to be quoted using double quotes (") in addition to ticks (`) */
			sql_mysql_query("SET sql_mode='ANSI_QUOTES'");

			return 0;
		}

		static int sql_mysql_close(void)
		{
			if (mysql) {
				p_mysql_close(mysql);
				mysql = NULL;
			}
#ifdef RUNTIME_LIBS
			if (handle) {
				CloseLibrary(handle);
				handle = NULL;
			}
#endif
			return 0;
		}

		static t_sql_res * sql_mysql_query_res(const char * query)
		{
			t_sql_res *res;

			if (mysql == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "mysql driver not initilized");
				return NULL;
			}

			if (query == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL query");
				return NULL;
			}

			if (p_mysql_query(mysql, query)) {
				//        eventlog(eventlog_level_debug, __FUNCTION__, "got error from query (%s)", query);
				return NULL;
			}

			res = p_mysql_store_result(mysql);
			if (res == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got error from store result");
				return NULL;
			}

			return res;
		}

		static int sql_mysql_query(const char * query)
		{
			if (mysql == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "mysql driver not initilized");
				return -1;
			}

			if (query == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL query");
				return -1;
			}

			return p_mysql_query(mysql, query) == 0 ? 0 : -1;
		}

		static t_sql_row * sql_mysql_fetch_row(t_sql_res *result)
		{
			if (result == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL result");
				return NULL;
			}

			return p_mysql_fetch_row((MYSQL_RES *)result);
		}

		static void sql_mysql_free_result(t_sql_res *result)
		{
			if (result == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL result");
				return;
			}

			p_mysql_free_result((MYSQL_RES *)result);
		}

		static unsigned int sql_mysql_num_rows(t_sql_res *result)
		{
			if (result == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL result");
				return 0;
			}

			return p_mysql_num_rows((MYSQL_RES *)result);
		}

		static unsigned int sql_mysql_num_fields(t_sql_res *result)
		{
			if (result == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL result");
				return 0;
			}

			return p_mysql_num_fields((MYSQL_RES *)result);
		}

		static unsigned int sql_mysql_affected_rows(void)
		{
			return p_mysql_affected_rows(mysql);
		}

		static t_sql_field * sql_mysql_fetch_fields(t_sql_res *result)
		{
			MYSQL_FIELD *fields;
			unsigned fieldno, i;
			t_sql_field *rfields;

			if (result == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL result");
				return NULL;
			}

			fieldno = p_mysql_num_fields((MYSQL_RES *)result);
			fields = p_mysql_fetch_fields((MYSQL_RES *)result);

			rfields = (t_sql_field *)xmalloc(sizeof(t_sql_field)* (fieldno + 1));
			for (i = 0; i < fieldno; i++)
				rfields[i] = fields[i].name;
			rfields[i] = NULL;

			return rfields;
		}

		static int sql_mysql_free_fields(t_sql_field *fields)
		{
			if (fields == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL fields");
				return -1;
			}

			xfree((void*)fields);
			return 0; /* mysql_free_result() should free the rest properly */
		}

		static void sql_mysql_escape_string(char *escape, const char *from, int len)
		{
			if (mysql == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "mysql driver not initilized");
				return;
			}
			p_mysql_real_escape_string(mysql, escape, from, len);
		}

	}

}

#endif /* WITH_SQL_MYSQL */
