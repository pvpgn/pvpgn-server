/*
 * Copyright (C) 1998,1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2000,2001  Marco Ziech (mmz@gmx.net)
 * Copyright (C) 2002,2003,2004 Dizzy
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
#include "storage_file.h"

#include <sstream>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <ctime>
#include <cstdlib>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "compat/strcasecmp.h"
#include "compat/pdir.h"
#include "compat/rename.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "common/tag.h"
#include "common/util.h"

#define CLAN_INTERNAL_ACCESS
#define TEAM_INTERNAL_ACCESS
#include "team.h"
#include "account.h"
#include "file_plain.h"
#include "file_cdb.h"
#include "prefs.h"
#include "clan.h"
#undef CLAN_INTERNAL_ACCESS
#undef TEAM_INTERNAL_ACCESS
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		/* file storage API functions */

		static int file_init(const char *);
		static int file_close(void);
		static unsigned file_read_maxuserid(void);
		static t_storage_info *file_create_account(char const *);
		static t_storage_info *file_get_defacct(void);
		static int file_free_info(t_storage_info *);
		static int file_read_attrs(t_storage_info *, t_read_attr_func, void *);
		static t_attr *file_read_attr(t_storage_info *, const char *);
		static int file_write_attrs(t_storage_info *, const t_hlist *);
		static int file_read_accounts(int, t_read_accounts_func, void *);
		static t_storage_info *file_read_account(const char *, unsigned);
		static int file_cmp_info(t_storage_info *, t_storage_info *);
		static const char *file_escape_key(const char *);
		static int file_load_clans(t_load_clans_func);
		static int file_write_clan(void *);
		static int file_remove_clan(int);
		static int file_remove_clanmember(int);
		static int file_load_teams(t_load_teams_func);
		static int file_write_team(void *);
		static int file_remove_team(unsigned int);

		/* storage struct populated with the functions above */

		t_storage storage_file = {
			file_init,
			file_close,
			file_read_maxuserid,
			file_create_account,
			file_get_defacct,
			file_free_info,
			file_read_attrs,
			file_write_attrs,
			file_read_attr,
			file_read_accounts,
			file_read_account,
			file_cmp_info,
			file_escape_key,
			file_load_clans,
			file_write_clan,
			file_remove_clan,
			file_remove_clanmember,
			file_load_teams,
			file_write_team,
			file_remove_team
		};

		/* start of actual file storage code */

		static const char *accountsdir = NULL;
		static const char *clansdir = NULL;
		static const char *teamsdir = NULL;
		static const char *defacct = NULL;
		static t_file_engine *file = NULL;

		static unsigned file_read_maxuserid(void)
		{
			return maxuserid;
		}

		static int file_init(const char *path)
		{
			char *tok, *copy, *tmp, *p;
			const char *dir = NULL;
			const char *clan = NULL;
			const char *team = NULL;
			const char *def = NULL;
			const char *driver = NULL;

			if (path == NULL || path[0] == '\0')
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL or empty path");
				return -1;
			}

			copy = xstrdup(path);
			tmp = copy;
			while ((tok = std::strtok(tmp, ";")) != NULL)
			{
				tmp = NULL;
				if ((p = std::strchr(tok, '=')) == NULL)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "invalid storage_path, no '=' present in token");
					xfree((void *)copy);
					return -1;
				}
				*p = '\0';
				if (strcasecmp(tok, "dir") == 0)
					dir = p + 1;
				else if (strcasecmp(tok, "clan") == 0)
					clan = p + 1;
				else if (strcasecmp(tok, "team") == 0)
					team = p + 1;
				else if (strcasecmp(tok, "default") == 0)
					def = p + 1;
				else if (strcasecmp(tok, "mode") == 0)
					driver = p + 1;
				else
					eventlog(eventlog_level_warn, __FUNCTION__, "unknown token in storage_path : '%s'", tok);
			}

			if (def == NULL || clan == NULL || team == NULL || dir == NULL || driver == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "invalid storage_path line for file module (doesnt have a 'dir', a 'clan', a 'team', a 'default' token and a 'mode' token)");
				xfree((void *)copy);
				return -1;
			}

			if (!strcasecmp(driver, "plain"))
				file = &file_plain;
			else if (!strcasecmp(driver, "cdb"))
				file = &file_cdb;
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "unknown mode '%s' must be either plain or cdb", driver);
				xfree((void *)copy);
				return -1;
			}

			if (accountsdir)
				file_close();

			accountsdir = xstrdup(dir);
			clansdir = xstrdup(clan);
			teamsdir = xstrdup(team);
			defacct = xstrdup(def);

			xfree((void *)copy);

			return 0;
		}

		static int file_close(void)
		{
			if (accountsdir)
				xfree((void *)accountsdir);
			accountsdir = NULL;

			if (clansdir)
				xfree((void *)clansdir);
			clansdir = NULL;

			if (teamsdir)
				xfree((void *)teamsdir);
			teamsdir = NULL;

			if (defacct)
				xfree((void *)defacct);
			defacct = NULL;

			file = NULL;

			return 0;
		}

		static t_storage_info *file_create_account(const char *username)
		{
			char *temp;

			if (accountsdir == NULL || file == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "file storage not initilized");
				return NULL;
			}

			if (prefs_get_savebyname())
			{
				char const *safename;

				if (!std::strcmp(username, defacct))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "username as defacct not allowed");
					return NULL;
				}

				if (!(safename = escape_fs_chars(username, std::strlen(username))))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not escape username");
					return NULL;
				}
				temp = (char*)xmalloc(std::strlen(accountsdir) + 1 + std::strlen(safename) + 1);	/* dir + / + name + NUL */
				std::sprintf(temp, "%s/%s", accountsdir, safename);
				xfree((void *)safename);	/* avoid warning */
			}
			else
			{
				temp = (char*)xmalloc(std::strlen(accountsdir) + 1 + 8 + 1);	/* dir + / + uid + NUL */
				std::sprintf(temp, "%s/%06u", accountsdir, maxuserid + 1);	/* FIXME: hmm, maybe up the %06 to %08... */
			}

			return temp;
		}

		static int file_write_attrs(t_storage_info * info, const t_hlist *attributes)
		{
			char *tempname;

			if (accountsdir == NULL || file == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "file storage not initilized");
				return -1;
			}

			if (info == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL info storage");
				return -1;
			}

			if (attributes == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL attributes");
				return -1;
			}

			tempname = (char*)xmalloc(std::strlen(accountsdir) + 1 + std::strlen(BNETD_ACCOUNT_TMP) + 1);
			std::sprintf(tempname, "%s/%s", accountsdir, BNETD_ACCOUNT_TMP);

			if (file->write_attrs(tempname, attributes))
			{
				/* no eventlog here, it should be reported from the file layer */
				xfree(tempname);
				return -1;
			}

			if (p_rename(tempname, (const char *)info) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not std::rename account file to \"%s\" (std::rename: %s)", (char *)info, std::strerror(errno));
				xfree(tempname);
				return -1;
			}

			xfree(tempname);

			return 0;
		}

		static int file_read_attrs(t_storage_info * info, t_read_attr_func cb, void *data)
		{
			if (accountsdir == NULL || file == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "file storage not initilized");
				return -1;
			}

			if (info == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL info storage");
				return -1;
			}

			if (cb == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL callback");
				return -1;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "loading \"%s\"", reinterpret_cast<const char*>(info));

			if (file->read_attrs((const char *)info, cb, data))
			{
				/* no eventlog, error reported earlier */
				return -1;
			}

			return 0;
		}

		static t_attr *file_read_attr(t_storage_info * info, const char *key)
		{
			if (accountsdir == NULL || file == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "file storage not initilized");
				return NULL;
			}

			if (info == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL info storage");
				return NULL;
			}

			if (key == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL key");
				return NULL;
			}

			return file->read_attr((const char *)info, key);
		}

		static int file_free_info(t_storage_info * info)
		{
			if (info)
				xfree((void *)info);
			return 0;
		}

		static t_storage_info *file_get_defacct(void)
		{
			t_storage_info *info;

			if (defacct == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "file storage not initilized");
				return NULL;
			}

			info = xstrdup(defacct);

			return info;
		}

		static int file_read_accounts(int flag, t_read_accounts_func cb, void *data)
		{
			if (!accountsdir) {
				ERROR0("file storage not initilized");
				return -1;
			}

			if (!cb) {
				ERROR0("got NULL callback");
				return -1;
			}

			try {
				Directory accdir(accountsdir);

				char const *dentry;
				while ((dentry = accdir.read())) {
					std::ostringstream ostr;
					ostr << accountsdir << '/' << dentry;

					cb(xstrdup(ostr.str().c_str()), data);
				}
			}
			catch (const Directory::OpenError& ex) {
				ERROR2("unable to open user directory \"%s\" for reading (error: %s)", accountsdir, ex.what());
				return -1;
			}

			return 0;
		}

		static t_storage_info *file_read_account(const char *accname, unsigned uid)
		{
			char *pathname;

			if (accountsdir == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "file storage not initilized");
				return NULL;
			}

			/* ONLY if requesting for a username and if savebyname() is true
			 * PS: yes its kind of a hack, we will make a proper index file
			 */
			if (accname && prefs_get_savebyname()) {
				pathname = (char*)xmalloc(std::strlen(accountsdir) + 1 + std::strlen(accname) + 1);	/* dir + / + file + NUL */
				std::sprintf(pathname, "%s/%s", accountsdir, accname);
				if (access(pathname, F_OK))	/* if it doesn't exist */
				{
					xfree((void *)pathname);
					return NULL;
				}
				return pathname;
			}

			return NULL;
		}

		static int file_cmp_info(t_storage_info * info1, t_storage_info * info2)
		{
			return std::strcmp((const char *)info1, (const char *)info2);
		}

		static const char *file_escape_key(const char *key)
		{
			return key;
		}

		static int file_load_clans(t_load_clans_func cb)
		{
			char const *dentry;
			char *pathname;
			int clantag;
			t_clan *clan;
			std::FILE *fp;
			char *clanname, *motd, *p;
			char line[1024];
			int cid, creation_time;
			int member_uid, member_join_time;
			char member_status;
			t_clanmember *member;

			if (cb == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "get NULL callback");
				return -1;
			}

			try {
				Directory clandir(clansdir);

				eventlog(eventlog_level_trace, __FUNCTION__, "start reading clans");

				pathname = (char*)xmalloc(std::strlen(clansdir) + 1 + 4 + 1);
				while ((dentry = clandir.read()))
				{
					if (std::strlen(dentry) > 4)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found too long clan filename in clandir \"%s\"", dentry);
						continue;
					}

					std::sprintf(pathname, "%s/%s", clansdir, dentry);

					clantag = str_to_clantag(dentry);

					if ((fp = std::fopen(pathname, "r")) == NULL)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "can't open clanfile \"%s\"", pathname);
						continue;
					}

					clan = (t_clan*)xmalloc(sizeof(t_clan));
					clan->tag = clantag;

					if (!std::fgets(line, 1024, fp))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid clan file: no first line");
						xfree((void*)clan);
						continue;
					}

					clanname = line;
					if (*clanname != '"')
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid clan file: invalid first line");
						xfree((void*)clan);
						continue;
					}
					clanname++;
					p = std::strchr(clanname, '"');
					if (!p)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid clan file: invalid first line");
						xfree((void*)clan);
						continue;
					}
					*p = '\0';
					if (std::strlen(clanname) >= CLAN_NAME_MAX)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid clan file: invalid first line");
						xfree((void*)clan);
						continue;
					}

					p++;
					if (*p != ',')
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid clan file: invalid first line");
						xfree((void*)clan);
						continue;
					}
					p++;
					if (*p != '"')
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid clan file: invalid first line");
						xfree((void*)clan);
						continue;
					}
					motd = p + 1;
					p = std::strchr(motd, '"');
					if (!p)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid clan file: invalid first line");
						xfree((void*)clan);
						continue;
					}
					*p = '\0';

					if (std::sscanf(p + 1, ",%d,%d\n", &cid, &creation_time) != 2)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid first line in clanfile");
						xfree((void*)clan);
						continue;
					}
					clan->clanname = xstrdup(clanname);
					clan->clan_motd = xstrdup(motd);
					clan->clanid = cid;
					clan->creation_time = (std::time_t) creation_time;
					clan->created = 1;
					clan->modified = 0;
					clan->channel_type = prefs_get_clan_channel_default_private();

					eventlog(eventlog_level_trace, __FUNCTION__, "name: %s motd: %s clanid: %i time: %i", clanname, motd, cid, creation_time);

					clan->members = list_create();

					while (std::fscanf(fp, "%i,%c,%i\n", &member_uid, &member_status, &member_join_time) == 3)
					{
						member = (t_clanmember*)xmalloc(sizeof(t_clanmember));
						if (!(member->memberacc = accountlist_find_account_by_uid(member_uid)))
						{
							eventlog(eventlog_level_error, __FUNCTION__, "cannot find uid %u", member_uid);
							xfree((void *)member);
							continue;
						}
						member->status = member_status - '0';
						member->join_time = member_join_time;
						member->clan = clan;
						member->fullmember = 1; /* In files we have only fullmembers */

						if ((member->status == CLAN_NEW) && (std::time(NULL) - member->join_time > prefs_get_clan_newer_time() * 3600))
						{
							member->status = CLAN_PEON;
							clan->modified = 1;
						}

						list_append_data(clan->members, member);

						account_set_clanmember((t_account*)member->memberacc, member);
						eventlog(eventlog_level_trace, __FUNCTION__, "added member: uid: %i status: %c join_time: %i", member_uid, member_status + '0', member_join_time);
					}

					std::fclose(fp);

					cb(clan);

				}

				xfree((void *)pathname);

			}
			catch (const Directory::OpenError& ex) {
				ERROR2("unable to open clan directory \"%s\" for reading (error: %s)", clansdir, ex.what());
				return -1;
			}

			eventlog(eventlog_level_trace, __FUNCTION__, "finished reading clans");

			return 0;
		}

		static int file_write_clan(void *data)
		{
			std::FILE *fp;
			t_elem *curr;
			t_clanmember *member;
			char *clanfile;
			t_clan *clan = (t_clan *)data;

			clanfile = (char*)xmalloc(std::strlen(clansdir) + 1 + 4 + 1);
			std::sprintf(clanfile, "%s/%s", clansdir, clantag_to_str(clan->tag));

			if ((fp = std::fopen(clanfile, "w")) == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "can't open clanfile \"%s\"", clanfile);
				xfree((void *)clanfile);
				return -1;
			}

			std::fprintf(fp, "\"%s\",\"%s\",%i,%i\n", clan->clanname, clan->clan_motd, clan->clanid, (int)clan->creation_time);

			LIST_TRAVERSE(clan->members, curr)
			{
				if (!(member = (t_clanmember*)elem_get_data(curr)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL elem in list");
					continue;
				}
				if ((member->status == CLAN_NEW) && (std::time(NULL) - member->join_time > prefs_get_clan_newer_time() * 3600))
					member->status = CLAN_PEON;
				if (member->fullmember == 1) /* only fullmembers are stored */
					std::fprintf(fp, "%i,%c,%u\n", account_get_uid((t_account*)member->memberacc), member->status + '0', (unsigned)member->join_time);
			}

			std::fclose(fp);
			xfree((void *)clanfile);
			return 0;
		}

		static int file_remove_clan(int clantag)
		{
			char *tempname;

			tempname = (char*)xmalloc(std::strlen(clansdir) + 1 + 4 + 1);
			std::sprintf(tempname, "%s/%s", clansdir, clantag_to_str(clantag));
			if (std::remove((const char *)tempname) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not delete clan file \"%s\" (std::remove: %s)", (char *)tempname, std::strerror(errno));
				xfree(tempname);
				return -1;
			}
			xfree(tempname);
			return 0;
		}

		static int file_remove_clanmember(int uid)
		{
			return 0;
		}

		static int file_load_teams(t_load_teams_func cb)
		{
			char const *dentry;
			char *pathname;
			unsigned int teamid;
			t_team *team;
			std::FILE *fp;
			char * line;
			unsigned int fteamid, lastgame;
			unsigned char size;
			char clienttag[5];
			int i;

			if (cb == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "get NULL callback");
				return -1;
			}

			try {
				Directory teamdir(teamsdir);

				eventlog(eventlog_level_trace, __FUNCTION__, "start reading teams");

				pathname = (char*)xmalloc(std::strlen(teamsdir) + 1 + 8 + 1);
				while ((dentry = teamdir.read()))
				{
					if (std::strlen(dentry) != 8)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found invalid team filename in teamdir \"%s\"", dentry);
						continue;
					}

					std::sprintf(pathname, "%s/%s", teamsdir, dentry);

					teamid = (unsigned int)std::strtoul(dentry, NULL, 16); // we use hexadecimal teamid as filename

					if ((fp = std::fopen(pathname, "r")) == NULL)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "can't open teamfile \"%s\"", pathname);
						continue;
					}

					team = (t_team*)xmalloc(sizeof(t_team));
					team->teamid = teamid;

					if (!(line = file_get_line(fp)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid team file: file is empty");
						goto load_team_failure;
					}

					if (std::sscanf(line, "%u,%c,%4s,%u", &fteamid, &size, clienttag, &lastgame) != 4)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid team file: invalid number of arguments on first line");
						goto load_team_failure;
					}

					if (fteamid != teamid)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid team file: filename and stored teamid don't match");
						goto load_team_failure;
					}

					size -= '0';
					if ((size<2) || (size>4))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid team file: invalid team size");
						goto load_team_failure;
					}
					team->size = size;

					if (!(tag_check_client(team->clienttag = tag_str_to_uint(clienttag))))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid team file: invalid clienttag");
						goto load_team_failure;
					}
					team->lastgame = (std::time_t)lastgame;

					if (!(line = file_get_line(fp)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid team file: missing 2nd line");
						goto load_team_failure;
					}

					if (std::sscanf(line, "%u,%u,%u,%u", &team->teammembers[0], &team->teammembers[1], &team->teammembers[2], &team->teammembers[3]) != 4)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid team file: invalid number of arguments on 2nd line");
						goto load_team_failure;
					}

					for (i = 0; i < MAX_TEAMSIZE; i++)
					{
						if (i < size)
						{
							if ((team->teammembers[i] == 0))
							{
								eventlog(eventlog_level_error, __FUNCTION__, "invalid team file: too few members");
								goto load_team_failure;
							}
						}
						else
						{
							if ((team->teammembers[i] != 0))
							{
								eventlog(eventlog_level_error, __FUNCTION__, "invalid team file: too many members");
								goto load_team_failure;
							}

						}
						team->members[i] = NULL;
					}

					if (!(line = file_get_line(fp)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid team file: missing 3rd line");
						goto load_team_failure;
					}

					if (std::sscanf(line, "%d,%d,%d,%d,%d", &team->wins, &team->losses, &team->xp, &team->level, &team->rank) != 5)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "invalid team file: invalid number of arguments on 3rd line");
						goto load_team_failure;
					}

					eventlog(eventlog_level_trace, __FUNCTION__, "succesfully loaded team %s", dentry);
					cb(team);

					goto load_team_success;
				load_team_failure:
					xfree((void*)team);
					eventlog(eventlog_level_error, __FUNCTION__, "error while reading file \"%s\"", dentry);

				load_team_success:

					file_get_line(NULL); // clear file_get_line buffer
					std::fclose(fp);


				}

				xfree((void *)pathname);

			}
			catch (const Directory::OpenError& ex) {
				ERROR2("unable to open team directory \"%s\" for reading (error: %s)", teamsdir, ex.what());
				return -1;
			}

			eventlog(eventlog_level_trace, __FUNCTION__, "finished reading teams");

			return 0;
		}

		static int file_write_team(void *data)
		{
			std::FILE *fp;
			char *teamfile;
			t_team *team = (t_team *)data;

			teamfile = (char*)xmalloc(std::strlen(teamsdir) + 1 + 8 + 1);
			std::sprintf(teamfile, "%s/%08x", teamsdir, team->teamid);

			if ((fp = std::fopen(teamfile, "w")) == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "can't open teamfile \"%s\"", teamfile);
				xfree((void *)teamfile);
				return -1;
			}

			std::fprintf(fp, "%u,%c,%s,%u\n", team->teamid, team->size + '0', clienttag_uint_to_str(team->clienttag), (unsigned int)team->lastgame);
			std::fprintf(fp, "%u,%u,%u,%u\n", team->teammembers[0], team->teammembers[1], team->teammembers[2], team->teammembers[3]);
			std::fprintf(fp, "%d,%d,%d,%d,%d\n", team->wins, team->losses, team->xp, team->level, team->rank);

			std::fclose(fp);
			xfree((void *)teamfile);

			return 0;
		}

		static int file_remove_team(unsigned int teamid)
		{

			char *tempname;

			tempname = (char*)xmalloc(std::strlen(clansdir) + 1 + 8 + 1);
			std::sprintf(tempname, "%s/%08x", clansdir, teamid);
			if (std::remove((const char *)tempname) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not delete team file \"%s\" (std::remove: %s)", (char *)tempname, std::strerror(errno));
				xfree(tempname);
				return -1;
			}
			xfree(tempname);

			return 0;
		}

	}

}
