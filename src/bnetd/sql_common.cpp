/*
 * Copyright (C) 2002 TheUndying
 * Copyright (C) 2002 zap-zero
 * Copyright (C) 2002,2003,2005 Dizzy
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
#define SQL_INTERNAL
# include "sql_common.h"
#undef SQL_INTERNAL

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include "compat/strcasecmp.h"
#include "compat/snprintf.h"
#include "common/eventlog.h"
#include "common/flags.h"
#include "common/list.h"
#include "common/tag.h"
#define CLAN_INTERNAL_ACCESS
#define TEAM_INTERNAL_ACCESS
#include "team.h"
#include "account.h"
#include "sql_dbcreator.h"
#ifdef WITH_SQL_MYSQL
#include "sql_mysql.h"
#endif
#ifdef WITH_SQL_PGSQL
#include "sql_pgsql.h"
#endif
#ifdef WITH_SQL_SQLITE3
#include "sql_sqlite3.h"
#endif
#ifdef WITH_SQL_ODBC
#include "sql_odbc.h"
#endif
#include "clan.h"
#include "prefs.h"
#undef CLAN_INTERNAL_ACCESS
#undef TEAM_INTERNAL_ACCESS
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		unsigned int sql_defacct;
		t_sql_engine *sql = NULL;

#ifndef SQL_ON_DEMAND
		char *sql_tables[] = { "BNET", "Record", "profile", "friend", "Team", NULL };
#endif	/* SQL_ON_DEMAND */

		const char* tab_prefix = SQL_DEFAULT_PREFIX;

		static char query[1024];

		extern int sql_init(const char *dbpath)
		{
			char *tok, *path, *tmp, *p;
			const char *dbhost = NULL;
			const char *dbname = NULL;
			const char *dbuser = NULL;
			const char *dbpass = NULL;
			const char *driver = NULL;
			const char *dbport = NULL;
			const char *dbsocket = NULL;
			const char *def = NULL;
			const char *pref = NULL;

			path = xstrdup(dbpath);
			tmp = path;
			while ((tok = std::strtok(tmp, ";")) != NULL)
			{
				tmp = NULL;
				if ((p = std::strchr(tok, '=')) == NULL)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "invalid storage_path, no '=' present in token");
					xfree((void *)path);
					return -1;
				}
				*p = '\0';
				if (strcasecmp(tok, "host") == 0)
					dbhost = p + 1;
				else if (strcasecmp(tok, "mode") == 0)
					driver = p + 1;
				else if (strcasecmp(tok, "name") == 0)
					dbname = p + 1;
				else if (strcasecmp(tok, "port") == 0)
					dbport = p + 1;
				else if (strcasecmp(tok, "socket") == 0)
					dbsocket = p + 1;
				else if (strcasecmp(tok, "user") == 0)
					dbuser = p + 1;
				else if (strcasecmp(tok, "pass") == 0)
					dbpass = p + 1;
				else if (strcasecmp(tok, "default") == 0)
					def = p + 1;
				else if (strcasecmp(tok, "prefix") == 0)
					pref = p + 1;
				else
					eventlog(eventlog_level_warn, __FUNCTION__, "unknown token in storage_path : '%s'", tok);
			}

			if (driver == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "no mode specified");
				xfree((void *)path);
				return -1;
			}

			if (def == NULL)
				sql_defacct = STORAGE_SQL_DEFAULT_UID;
			else
				sql_defacct = std::atoi(def);

			if (pref == NULL)
				tab_prefix = SQL_DEFAULT_PREFIX;
			else
				tab_prefix = xstrdup(pref);

			do
			{
#ifdef WITH_SQL_MYSQL
				if (strcasecmp(driver, "mysql") == 0)
				{
					sql = &sql_mysql;
					if (sql->init(dbhost, dbport, dbsocket, dbname, dbuser, dbpass))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got error init db");
						sql = NULL;
						xfree((void *)path);
						return -1;
					}
					break;
				}
#endif				/* WITH_SQL_MYSQL */
#ifdef WITH_SQL_PGSQL
				if (strcasecmp(driver, "pgsql") == 0)
				{
					sql = &sql_pgsql;
					if (sql->init(dbhost, dbport, dbsocket, dbname, dbuser, dbpass))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got error init db");
						sql = NULL;
						xfree((void *)path);
						return -1;
					}
					break;
				}
#endif				/* WITH_SQL_PGSQL */
#ifdef WITH_SQL_SQLITE3
				if (strcasecmp(driver, "sqlite3") == 0)
				{
					sql = &sql_sqlite3;
					if (sql->init(NULL, 0, NULL, dbname, NULL, NULL))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got error init db");
						sql = NULL;
						xfree((void *)path);
						return -1;
					}
					break;
				}
#endif				/* WITH_SQL_SQLITE3 */
#ifdef WITH_SQL_ODBC
				if (strcasecmp(driver, "odbc") == 0)
				{
					sql = &sql_odbc;
					if (sql->init(dbhost, dbport, dbsocket, dbname, dbuser, dbpass))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got error init db");
						sql = NULL;
						free((void *)path);
						return -1;
					}
					break;
				}
#endif				/* WITH_SQL_ODBC */
				eventlog(eventlog_level_error, __FUNCTION__, "no driver found for '%s'", driver);
				xfree((void *)path);
				return -1;
			} while (0);

			xfree((void *)path);

			sql_dbcreator(sql);

			return 0;
		}

		extern int sql_close(void)
		{
			if (sql == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql not initilized");
				return -1;
			}

			sql->close();
			sql = NULL;
			if (tab_prefix != SQL_DEFAULT_PREFIX) {
				xfree((void*)tab_prefix);
				tab_prefix = NULL;
			}

			return 0;
		}

		extern unsigned sql_read_maxuserid(void)
		{
			t_sql_res *result;
			t_sql_row *row;
			long maxuid;

			if (sql == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql not initilized");
				return 0;
			}

			snprintf(query, sizeof(query), "SELECT max("SQL_UID_FIELD") FROM %sBNET", tab_prefix);
			if ((result = sql->query_res(query)) == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "error trying query: \"SELECT max("SQL_UID_FIELD") FROM %sBNET\"", tab_prefix);
				return 0;
			}

			row = sql->fetch_row(result);
			if (row == NULL || row[0] == NULL)
			{
				sql->free_result(result);
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL max");
				return 0;
			}

			maxuid = std::atol(row[0]);
			sql->free_result(result);
			if (maxuid < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid maxuserid");
				return 0;
			}

			return maxuid;
		}

		extern int sql_read_accounts(int flag, t_read_accounts_func cb, void *data)
		{
			t_sql_res *result = NULL;
			t_sql_row *row;
			t_storage_info *info;

			if (!sql)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql layer not initilized");
				return -1;
			}

			if (cb == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "get NULL callback");
				return -1;
			}

			/* don't actually load anything here if ST_FORCE is not set as SQL is indexed */
			if (!FLAG_ISSET(flag, ST_FORCE)) return 1;

			snprintf(query, sizeof(query), "SELECT DISTINCT("SQL_UID_FIELD") FROM %sBNET", tab_prefix);
			if ((result = sql->query_res(query)) != NULL)
			{
				if (sql->num_rows(result) <= 1)
				{
					sql->free_result(result);
					return 0;		/* empty user list */
				}

				while ((row = sql->fetch_row(result)) != NULL)
				{
					if (row[0] == NULL)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL uid from db");
						continue;
					}

					if ((unsigned int)std::atoi(row[0]) == sql_defacct)
						continue;	/* skip default account */

					info = xmalloc(sizeof(t_sql_info));
					*((unsigned int *)info) = std::atoi(row[0]);
					cb(info, data);
				}
				sql->free_result(result);
			}
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "error query db (query:\"%s\")", query);
				return -1;
			}

			return 0;
		}

		extern int sql_cmp_info(t_storage_info * info1, t_storage_info * info2)
		{
			return *((unsigned int *)info1) != *((unsigned int *)info2);
		}

		extern int sql_free_info(t_storage_info * info)
		{
			if (info)
				xfree((void *)info);

			return 0;
		}

		extern t_storage_info *sql_get_defacct(void)
		{
			t_storage_info *info;

			info = xmalloc(sizeof(t_sql_info));
			*((unsigned int *)info) = sql_defacct;

			return info;
		}

		extern int sql_load_clans(t_load_clans_func cb)
		{
			t_sql_res *result;
			t_sql_res *result2;
			t_sql_row *row;
			t_sql_row *row2;
			t_clan *clan;
			int member_uid;
			t_clanmember *member;

			if (!sql)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql layer not initilized");
				return -1;
			}

			if (cb == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "get NULL callback");
				return -1;
			}

			snprintf(query, sizeof(query), "SELECT cid, short, name, motd, creation_time FROM %sclan WHERE cid > 0", tab_prefix);
			if ((result = sql->query_res(query)) != NULL)
			{
				if (sql->num_rows(result) < 1)
				{
					sql->free_result(result);
					return 0;		/* empty clan list */
				}

				while ((row = sql->fetch_row(result)) != NULL)
				{
					if (row[0] == NULL)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL cid from db");
						continue;
					}

					clan = (t_clan *)xmalloc(sizeof(t_clan));

					if (!(clan->clanid = std::atoi(row[0])))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got bad cid");
						sql->free_result(result);
						return -1;
					}

					clan->tag = std::atoi(row[1]);

					clan->clanname = xstrdup(row[2]);
					clan->clan_motd = xstrdup(row[3]);
					clan->creation_time = std::atoi(row[4]);
					clan->created = 1;
					clan->modified = 0;
					clan->channel_type = prefs_get_clan_channel_default_private();
					clan->members = list_create();

					snprintf(query, sizeof(query), "SELECT "SQL_UID_FIELD", status, join_time FROM %sclanmember WHERE cid='%u'", tab_prefix, clan->clanid);

					if ((result2 = sql->query_res(query)) != NULL)
					{
						if (sql->num_rows(result2) >= 1)
						while ((row2 = sql->fetch_row(result2)) != NULL)
						{
							member = (t_clanmember *)xmalloc(sizeof(t_clanmember));
							if (row2[0] == NULL)
							{
								eventlog(eventlog_level_error, __FUNCTION__, "got NULL uid from db");
								continue;
							}
							if (!(member_uid = std::atoi(row2[0])))
								continue;
							if (!(member->memberacc = accountlist_find_account_by_uid(member_uid)))
							{
								eventlog(eventlog_level_error, __FUNCTION__, "cannot find uid %u", member_uid);
								xfree((void *)member);
								continue;
							}
							member->status = std::atoi(row2[1]);
							member->join_time = std::atoi(row2[2]);
							member->clan = clan;
							member->fullmember = 1;

							if ((member->status == CLAN_NEW) && (std::time(NULL) - member->join_time > prefs_get_clan_newer_time() * 3600))
							{
								member->status = CLAN_PEON;
								clan->modified = 1;
								member->modified = 1;
							}

							list_append_data(clan->members, member);

							account_set_clanmember(member->memberacc, member);
							eventlog(eventlog_level_trace, __FUNCTION__, "added member: uid: %i status: %c join_time: %u", member_uid, member->status + '0', (unsigned)member->join_time);
						}
						sql->free_result(result2);
						cb(clan);
					}
					else
						eventlog(eventlog_level_error, __FUNCTION__, "error query db (query:\"%s\")", query);
				}

				sql->free_result(result);
			}
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "error query db (query:\"%s\")", query);
				return -1;
			}
			return 0;
		}

		extern int sql_write_clan(void *data)
		{
			char esc_motd[CLAN_MOTD_MAX * 2 + 1];
			t_sql_res *result;
			t_sql_row *row;
			t_elem *curr;
			t_clanmember *member;
			t_clan *clan = (t_clan *)data;
			int num;

			if (!sql)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql layer not initilized");
				return -1;
			}

			snprintf(query, sizeof(query), "SELECT count(*) FROM %sclan WHERE cid='%u'", tab_prefix, clan->clanid);
			if ((result = sql->query_res(query)) != NULL)
			{
				row = sql->fetch_row(result);
				if (row == NULL || row[0] == NULL)
				{
					sql->free_result(result);
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL count");
					return -1;
				}
				num = std::atol(row[0]);
				sql->free_result(result);
				sql->escape_string(esc_motd, clan->clan_motd, std::strlen(clan->clan_motd));
				if (num < 1)
					snprintf(query, sizeof(query), "INSERT INTO %sclan (cid, short, name, motd, creation_time) VALUES('%u', '%d', '%s', '%s', '%u')", tab_prefix, clan->clanid, clan->tag, clan->clanname, esc_motd, (unsigned)clan->creation_time);
				else
					snprintf(query, sizeof(query), "UPDATE %sclan SET short='%d', name='%s', motd='%s', creation_time='%u' WHERE cid='%u'", tab_prefix, clan->tag, clan->clanname, esc_motd, (unsigned)clan->creation_time, clan->clanid);
				if (sql->query(query) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "error trying query: \"%s\"", query);
					return -1;
				}
				LIST_TRAVERSE(clan->members, curr)
				{
					unsigned int uid;

					if (!(member = (t_clanmember *)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL elem in list");
						continue;
					}
					if (member->fullmember == 0)
						continue;
					if ((member->status == CLAN_NEW) && (std::time(NULL) - member->join_time > prefs_get_clan_newer_time() * 3600))
					{
						member->status = CLAN_PEON;
						member->modified = 1;
					}
					if (member->modified)
					{
						uid = account_get_uid(member->memberacc);
						snprintf(query, sizeof(query), "SELECT count(*) FROM %sclanmember WHERE "SQL_UID_FIELD"='%u'", tab_prefix, uid);
						if ((result = sql->query_res(query)) != NULL)
						{
							row = sql->fetch_row(result);
							if (row == NULL || row[0] == NULL)
							{
								sql->free_result(result);
								eventlog(eventlog_level_error, __FUNCTION__, "got NULL count");
								return -1;
							}
							num = std::atol(row[0]);
							sql->free_result(result);
							if (num < 1)
								snprintf(query, sizeof(query), "INSERT INTO %sclanmember (cid, "SQL_UID_FIELD", status, join_time) VALUES('%u', '%u', '%d', '%u')", tab_prefix, clan->clanid, uid, member->status, (unsigned)member->join_time);
							else
								snprintf(query, sizeof(query), "UPDATE %sclanmember SET cid='%u', status='%d', join_time='%u' WHERE "SQL_UID_FIELD"='%u'", tab_prefix, clan->clanid, member->status, (unsigned)member->join_time, uid);
							if (sql->query(query) < 0)
							{
								eventlog(eventlog_level_error, __FUNCTION__, "error trying query: \"%s\"", query);
								return -1;
							}
						}
						else
						{
							eventlog(eventlog_level_error, __FUNCTION__, "error trying query: \"%s\"", query);
							return -1;
						}
						member->modified = 0;
					}
				}
			}
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "error trying query: \"%s\"", query);
				return -1;
			}

			return 0;
		}

		extern int sql_remove_clan(int clantag)
		{
			t_sql_res *result;
			t_sql_row *row;

			if (!sql)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql layer not initilized");
				return -1;
			}

			snprintf(query, sizeof(query), "SELECT cid FROM %sclan WHERE short = '%d'", tab_prefix, clantag);
			if (!(result = sql->query_res(query)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "error query db (query:\"%s\")", query);
				return -1;
			}

			if (sql->num_rows(result) != 1)
			{
				sql->free_result(result);
				return -1;		/*clan not found or found more than 1 */
			}

			if ((row = sql->fetch_row(result)))
			{
				unsigned int cid = std::atoi(row[0]);
				snprintf(query, sizeof(query), "DELETE FROM %sclanmember WHERE cid='%u'", tab_prefix, cid);
				if (sql->query(query) != 0)
					return -1;
				snprintf(query, sizeof(query), "DELETE FROM %sclan WHERE cid='%u'", tab_prefix, cid);
				if (sql->query(query) != 0)
					return -1;
			}

			sql->free_result(result);

			return 0;
		}

		extern int sql_remove_clanmember(int uid)
		{
			if (!sql)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql layer not initilized");
				return -1;
			}

			snprintf(query, sizeof(query), "DELETE FROM %sclanmember WHERE "SQL_UID_FIELD"='%u'", tab_prefix, uid);
			if (sql->query(query) != 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "error trying query: \"%s\"", query);
				return -1;
			}

			return 0;
		}

		extern int sql_load_teams(t_load_teams_func cb)
		{
			t_sql_res *result;
			t_sql_row *row;
			t_team *team;
			int i;

			if (!sql)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql layer not initilized");
				return -1;
			}

			if (cb == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "get NULL callback");
				return -1;
			}

			snprintf(query, sizeof(query), "SELECT teamid, size, clienttag, lastgame, member1, member2, member3, member4, wins,losses, xp, level, rank FROM %sarrangedteam WHERE teamid > 0", tab_prefix);
			if ((result = sql->query_res(query)) != NULL)
			{
				if (sql->num_rows(result) < 1)
				{
					sql->free_result(result);
					return 0;		/* empty team list */
				}

				while ((row = sql->fetch_row(result)) != NULL)
				{
					if (row[0] == NULL)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL teamid from db");
						continue;
					}

					team = (t_team *)xmalloc(sizeof(t_team));

					if (!(team->teamid = std::atoi(row[0])))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got bad teamid");
						sql->free_result(result);
						return -1;
					}

					team->size = std::atoi(row[1]);
					team->clienttag = tag_str_to_uint(row[2]);
					team->lastgame = std::strtoul(row[3], NULL, 10);
					team->teammembers[0] = std::strtoul(row[4], NULL, 10);
					team->teammembers[1] = std::strtoul(row[5], NULL, 10);
					team->teammembers[2] = std::strtoul(row[6], NULL, 10);
					team->teammembers[3] = std::strtoul(row[7], NULL, 10);

					for (i = 0; i < MAX_TEAMSIZE; i++)
					{
						if (i < team->size)
						{
							if ((team->teammembers[i] == 0))
							{
								eventlog(eventlog_level_error, __FUNCTION__, "invalid team data: too few members");
								free((void *)team);
								goto load_team_failure;
							}
						}
						else
						{
							if ((team->teammembers[i] != 0))
							{
								eventlog(eventlog_level_error, __FUNCTION__, "invalid team data: too many members");
								free((void *)team);
								goto load_team_failure;
							}

						}
						team->members[i] = NULL;
					}

					team->wins = std::atoi(row[8]);
					team->losses = std::atoi(row[9]);
					team->xp = std::atoi(row[10]);
					team->level = std::atoi(row[11]);
					team->rank = std::atoi(row[12]);

					eventlog(eventlog_level_trace, __FUNCTION__, "succesfully loaded team %u", team->teamid);
					cb(team);
				load_team_failure:
					;
				}

				sql->free_result(result);
			}
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "error query db (query:\"%s\")", query);
				return -1;
			}
			return 0;
		}

		extern int sql_write_team(void *data)
		{
			t_sql_res *result;
			t_sql_row *row;
			t_team *team = (t_team *)data;
			int num;

			if (!sql)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql layer not initilized");
				return -1;
			}

			snprintf(query, sizeof(query), "SELECT count(*) FROM %sarrangedteam WHERE teamid='%u'", tab_prefix, team->teamid);
			if ((result = sql->query_res(query)) != NULL)
			{
				row = sql->fetch_row(result);
				if (row == NULL || row[0] == NULL)
				{
					sql->free_result(result);
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL count");
					return -1;
				}
				num = std::atol(row[0]);
				sql->free_result(result);
				if (num < 1)
					snprintf(query, sizeof(query), "INSERT INTO %sarrangedteam (teamid, size, clienttag, lastgame, member1, member2, member3, member4, wins,losses, xp, level, rank) VALUES('%u', '%c', '%s', '%u', '%u', '%u', '%u', '%u', '%d', '%d', '%d', '%d', '%d')", tab_prefix, team->teamid, team->size + '0', clienttag_uint_to_str(team->clienttag), (unsigned int)team->lastgame, team->teammembers[0], team->teammembers[1], team->teammembers[2], team->teammembers[3], team->wins, team->losses, team->xp, team->level, team->rank);
				else
					snprintf(query, sizeof(query), "UPDATE %sarrangedteam SET size='%c', clienttag='%s', lastgame='%u', member1='%u', member2='%u', member3='%u', member4='%u', wins='%d', losses='%d', xp='%d', level='%d', rank='%d' WHERE teamid='%u'", tab_prefix, team->size + '0', clienttag_uint_to_str(team->clienttag), (unsigned int)team->lastgame, team->teammembers[0], team->teammembers[1], team->teammembers[2], team->teammembers[3], team->wins, team->losses, team->xp, team->level, team->rank, team->teamid);
				if (sql->query(query) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "error trying query: \"%s\"", query);
					return -1;
				}
			}
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "error trying query: \"%s\"", query);
				return -1;
			}

			return 0;
		}

		extern int sql_remove_team(unsigned int teamid)
		{
			if (!sql)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "sql layer not initilized");
				return -1;
			}

			snprintf(query, sizeof(query), "DELETE FROM %sarrangedteam WHERE teamid='%u'", tab_prefix, teamid);
			if (sql->query(query) != 0)
				return -1;

			return 0;
		}

	}

}

#endif				/* WITH_SQL */
