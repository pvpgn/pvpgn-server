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
#ifdef WITH_SQL_PGSQL

#include "common/setup_before.h"
#include <libpq-fe.h>
#include <cstdlib>
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "storage_sql.h"
#include "sql_pgsql.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static int sql_pgsql_init(const char *, const char *, const char *, const char *, const char *, const char *);
		static int sql_pgsql_close(void);
		static t_sql_res * sql_pgsql_query_res(const char *);
		static int sql_pgsql_query(const char *);
		static t_sql_row * sql_pgsql_fetch_row(t_sql_res *);
		static void sql_pgsql_free_result(t_sql_res *);
		static unsigned int sql_pgsql_num_rows(t_sql_res *);
		static unsigned int sql_pgsql_num_fields(t_sql_res *);
		static unsigned int sql_pgsql_affected_rows(void);
		static t_sql_field * sql_pgsql_fetch_fields(t_sql_res *);
		static int sql_pgsql_free_fields(t_sql_field *);
		static void sql_pgsql_escape_string(char *, const char *, int);

		t_sql_engine sql_pgsql = {
			sql_pgsql_init,
			sql_pgsql_close,
			sql_pgsql_query_res,
			sql_pgsql_query,
			sql_pgsql_fetch_row,
			sql_pgsql_free_result,
			sql_pgsql_num_rows,
			sql_pgsql_num_fields,
			sql_pgsql_affected_rows,
			sql_pgsql_fetch_fields,
			sql_pgsql_free_fields,
			sql_pgsql_escape_string
		};

		static PGconn *pgsql = NULL;
		static unsigned int lastarows = 0;

		typedef struct {
			int crow;
			char ** rowbuf;
			PGresult *pgres;
		} t_pgsql_res;

#ifndef RUNTIME_LIBS
#define p_PQclear		PQclear
#define p_PQcmdTuples		PQcmdTuples
#define p_PQerrorMessage	PQerrorMessage
#define p_PQescapeString	PQescapeString
#define p_PQexec		PQexec
#define p_PQfinish		PQfinish
#define p_PQfname		PQfname
#define p_PQgetvalue		PQgetvalue
#define p_PQnfields		PQnfields
#define p_PQntuples		PQntuples
#define p_PQresultStatus	PQresultStatus
#define p_PQsetdbLogin		PQsetdbLogin
#define p_PQstatus		PQstatus
#else
		/* RUNTIME_LIBS */
		static int pgsql_load_dll(void);

		typedef void(*f_PQclear)(PGresult*);
		typedef char*		(*f_PQcmdTuples)(PGresult*);
		typedef char*		(*f_PQerrorMessage)(const PGconn*);
		typedef size_t(*f_PQescapeString)(char*, const char*, size_t);
		typedef PGresult*	(*f_PQexec)(PGconn*, const char*);
		typedef void(*f_PQfinish)(PGconn*);
		typedef char*		(*f_PQfname)(const PGresult*, int);
		typedef char*		(*f_PQgetvalue)(const PGresult*, int, int);
		typedef int(*f_PQnfields)(const PGresult*);
		typedef int(*f_PQntuples)(const PGresult*);
		typedef ExecStatusType(*f_PQresultStatus)(const PGresult*);
		typedef PGconn*		(*f_PQsetdbLogin)(const char*, const char*, const char*, const char*, const char*, const char*, const char*);
		typedef ConnStatusType(*f_PQstatus)(const PGconn*);

		static f_PQclear	p_PQclear;
		static f_PQcmdTuples	p_PQcmdTuples;
		static f_PQerrorMessage	p_PQerrorMessage;
		static f_PQescapeString	p_PQescapeString;
		static f_PQexec		p_PQexec;
		static f_PQfinish	p_PQfinish;
		static f_PQfname	p_PQfname;
		static f_PQgetvalue	p_PQgetvalue;
		static f_PQnfields	p_PQnfields;
		static f_PQntuples	p_PQntuples;
		static f_PQresultStatus	p_PQresultStatus;
		static f_PQsetdbLogin	p_PQsetdbLogin;
		static f_PQstatus	p_PQstatus;

#include "compat/runtime_libs.h" /* defines OpenLibrary(), GetFunction(), CloseLibrary() & PGSQL_LIB */

		static void * handle = NULL;

		static int pgsql_load_dll(void)
		{
			if ((handle = OpenLibrary(PGSQL_LIB)) == NULL) return -1;

			if (((p_PQclear = (f_PQclear)GetFunction(handle, "PQclear")) == NULL) ||
				((p_PQcmdTuples = (f_PQcmdTuples)GetFunction(handle, "PQcmdTuples")) == NULL) ||
				((p_PQerrorMessage = (f_PQerrorMessage)GetFunction(handle, "PQerrorMessage")) == NULL) ||
				((p_PQescapeString = (f_PQescapeString)GetFunction(handle, "PQescapeString")) == NULL) ||
				((p_PQexec = (f_PQexec)GetFunction(handle, "PQexec")) == NULL) ||
				((p_PQfinish = (f_PQfinish)GetFunction(handle, "PQfinish")) == NULL) ||
				((p_PQfname = (f_PQfname)GetFunction(handle, "PQfname")) == NULL) ||
				((p_PQgetvalue = (f_PQgetvalue)GetFunction(handle, "PQgetvalue")) == NULL) ||
				((p_PQnfields = (f_PQnfields)GetFunction(handle, "PQnfields")) == NULL) ||
				((p_PQntuples = (f_PQntuples)GetFunction(handle, "PQntuples")) == NULL) ||
				((p_PQresultStatus = (f_PQresultStatus)GetFunction(handle, "PQresultStatus")) == NULL) ||
				((p_PQsetdbLogin = (f_PQsetdbLogin)GetFunction(handle, "PQsetdbLogin")) == NULL) ||
				((p_PQstatus = (f_PQstatus)GetFunction(handle, "PQstatus")) == NULL))
			{
				CloseLibrary(handle);
				handle = NULL;
				return -1;
			}

			return 0;
		}
#endif

		static int sql_pgsql_init(const char *host, const char *port, const char *socket, const char *name, const char *user, const char *pass)
		{
			const char *tmphost;

			if (name == NULL || user == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL parameter");
				return -1;
			}

			tmphost = host != NULL ? host : socket;
#ifdef RUNTIME_LIBS
			if (pgsql_load_dll()) {
				eventlog(eventlog_level_error, __FUNCTION__, "error loading library file \"%s\"", PGSQL_LIB);
				return -1;
			}
#endif
			if ((pgsql = p_PQsetdbLogin(host, port, NULL, NULL, name, user, pass)) == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "not enougn memory for new pgsql connection");
				return -1;
			}

			if (p_PQstatus(pgsql) != CONNECTION_OK) {
				eventlog(eventlog_level_error, __FUNCTION__, "error connecting to database (db said: '%s')", p_PQerrorMessage(pgsql));
				p_PQfinish(pgsql);
				pgsql = NULL;
				return -1;
			}

			return 0;
		}

		static int sql_pgsql_close(void)
		{
			if (pgsql) {
				p_PQfinish(pgsql);
				pgsql = NULL;
			}
#ifdef RUNTIME_LIBS
			if (handle) {
				CloseLibrary(handle);
				handle = NULL;
			}
#endif
			return 0;
		}

		static t_sql_res * sql_pgsql_query_res(const char * query)
		{
			t_pgsql_res *res;
			PGresult *pgres;

			if (pgsql == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "pgsql driver not initilized");
				return NULL;
			}

			if (query == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL query");
				return NULL;
			}

			if ((pgres = p_PQexec(pgsql, query)) == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "not enough memory for query (%s)", query);
				return NULL;
			}

			if (p_PQresultStatus(pgres) != PGRES_TUPLES_OK) {
				/*        eventlog(eventlog_level_debug, __FUNCTION__, "got error from query (%s)", query); */
				p_PQclear(pgres);
				return NULL;
			}

			res = (t_pgsql_res *)xmalloc(sizeof(t_pgsql_res));
			res->rowbuf = (char **)xmalloc(sizeof(char *)* p_PQnfields(pgres));
			res->pgres = pgres;
			res->crow = 0;

			/*    eventlog(eventlog_level_debug, __FUNCTION__, "res: %p res->rowbuf: %p res->crow: %d res->pgres: %p", res, res->rowbuf, res->crow, res->pgres); */
			return res;
		}

		static void _pgsql_update_arows(const char *str)
		{
			if (!str || str[0] == '\0') lastarows = 0;
			lastarows = (unsigned int)std::atoi(str);
		}

		static int sql_pgsql_query(const char * query)
		{
			PGresult *pgres;
			int res;

			if (pgsql == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "pgsql driver not initilized");
				return -1;
			}

			if (query == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL query");
				return -1;
			}

			if ((pgres = p_PQexec(pgsql, query)) == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "not enough memory for result");
				return -1;
			}

			res = p_PQresultStatus(pgres) == PGRES_COMMAND_OK ? 0 : -1;
			/* Dizzy: HACK ALERT! cache affected rows here before destroying result */
			if (!res) _pgsql_update_arows(p_PQcmdTuples(pgres));
			p_PQclear(pgres);

			return res;
		}

		static t_sql_row * sql_pgsql_fetch_row(t_sql_res *result)
		{
			int nofields, i;
			t_pgsql_res *res = (t_pgsql_res *)result;

			if (res == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL result");
				return NULL;
			}

			if (res->crow < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got called without a proper res query");
				return NULL;
			}

			if (res->crow >= p_PQntuples(res->pgres)) return NULL; /* end of result */

			nofields = p_PQnfields(res->pgres);
			for (i = 0; i < nofields; i++) {
				res->rowbuf[i] = p_PQgetvalue(res->pgres, res->crow, i);
				/* the next line emulates the mysql way where NULL containing fields return NULL */
				if (res->rowbuf[i] && res->rowbuf[i][0] == '\0') res->rowbuf[i] = NULL;
			}

			res->crow++;

			/*    eventlog(eventlog_level_debug, __FUNCTION__, "res: %p res->rowbuf: %p res->crow: %d res->pgres: %p", res, res->rowbuf, res->crow, res->pgres); */
			return res->rowbuf;
		}

		static void sql_pgsql_free_result(t_sql_res *result)
		{
			t_pgsql_res *res = (t_pgsql_res *)result;

			if (res == NULL) return;
			/*    eventlog(eventlog_level_debug, __FUNCTION__, "res: %p res->rowbuf: %p res->crow: %d res->pgres: %p", res, res->rowbuf, res->crow, res->pgres); */

			if (res->pgres) p_PQclear(res->pgres);
			if (res->rowbuf) xfree((void*)res->rowbuf);
			xfree((void*)res);
		}

		static unsigned int sql_pgsql_num_rows(t_sql_res *result)
		{
			if (result == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL result");
				return 0;
			}

			return p_PQntuples(((t_pgsql_res *)result)->pgres);
		}

		static unsigned int sql_pgsql_num_fields(t_sql_res *result)
		{
			if (result == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL result");
				return 0;
			}

			return p_PQnfields(((t_pgsql_res *)result)->pgres);
		}

		static unsigned int sql_pgsql_affected_rows(void)
		{
			return lastarows;
		}

		static t_sql_field * sql_pgsql_fetch_fields(t_sql_res *result)
		{
			t_pgsql_res *res = (t_pgsql_res *)result;
			unsigned fieldno, i;
			t_sql_field *rfields;

			if (result == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL result");
				return NULL;
			}

			fieldno = p_PQnfields(res->pgres);

			rfields = (t_sql_field *)xmalloc(sizeof(t_sql_field)* (fieldno + 1));
			for (i = 0; i < fieldno; i++)
				rfields[i] = p_PQfname(res->pgres, i);
			rfields[i] = NULL;

			return rfields;
		}

		static int sql_pgsql_free_fields(t_sql_field *fields)
		{
			if (fields == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL fields");
				return -1;
			}

			xfree((void*)fields);
			return 0; /* PQclear() should free the rest properly */
		}

		static void sql_pgsql_escape_string(char *escape, const char *from, int len)
		{
			p_PQescapeString(escape, from, len);
		}

	}

}

#endif /* WITH_SQL_PGSQL */
