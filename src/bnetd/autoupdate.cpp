/*
 * Copyright (C) 2000 Rob Crittenden (rcrit@greyoak.com)
 * Copyright (C) 2002 Gianluigi Tiesi (sherpya@netfarm.it)
 * Copyright (C) 2004 CreepLord (creeplord@pvpgn.org)
 * Copyright (C) 2008 Pelish (pelish@gmail.com)
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
#define AUTOUPDATE_INTERNAL_ACCESS
#include "autoupdate.h"

#include <cerrno>
#include <cstring>

#include "common/eventlog.h"
#include "common/list.h"
#include "common/util.h"
#include "common/proginfo.h"
#include "common/tag.h"
#include "common/xalloc.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static t_list * autoupdate_head = NULL;
		static std::FILE * fp = NULL;


		/*
		 * Open the autoupdate configuration file, create a linked list of the
		 * clienttag and the update file for it.  The format of the file is:
		 * archtag<tab>clienttag<tab>versiontag<tab>update file
		 *
		 * Comments begin with # and are ignored.
		 *
		 * The server assumes that the update file is in the "files" directory
		 * so do not include "/" in the filename - it won't be sent
		 * (because it is a security risk).
		 */

		extern int autoupdate_load(char const * filename)
		{
			unsigned int   line;
			unsigned int   pos;
			char *         buff;
			char *         temp;
			char const *   archtag;
			char const *   clienttag;
			char const *   updatefile;
			char const *   versiontag;
			char const *   path;
			t_autoupdate * entry;

			if (!filename) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
				return -1;
			}

			if (!(fp = std::fopen(filename, "r"))) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for reading (std::fopen: %s)", filename, std::strerror(errno));
				return -1;
			}

			autoupdate_head = list_create();

			for (line = 1; (buff = file_get_line(fp)); line++) {
				for (pos = 0; buff[pos] == '\t' || buff[pos] == ' '; pos++);

				if (buff[pos] == '\0' || buff[pos] == '#') {
					continue;
				}

				if ((temp = std::strrchr(buff, '#'))) {
					unsigned int len;
					unsigned int endpos;

					*temp = '\0';
					len = std::strlen(buff) + 1;
					for (endpos = len - 1; buff[endpos] == '\t' || buff[endpos] == ' '; endpos--);
					buff[endpos + 1] = '\0';
				}

				/* FIXME: use next_token instead of std::strtok */
				if (!(archtag = std::strtok(buff, " \t"))) { /* std::strtok modifies the string it is passed */
					eventlog(eventlog_level_error, __FUNCTION__, "missing archtag on line %u of file \"%s\"", line, filename);
					continue;
				}
				if (!(clienttag = std::strtok(NULL, " \t"))) {
					eventlog(eventlog_level_error, __FUNCTION__, "missing clienttag on line %u of file \"%s\"", line, filename);
					continue;
				}
				if (!(versiontag = std::strtok(NULL, " \t"))) {
					eventlog(eventlog_level_error, __FUNCTION__, "missing versiontag on line %u of file \"%s\"", line, filename);
					continue;
				}
				if (!(updatefile = std::strtok(NULL, " \t"))) {
					eventlog(eventlog_level_error, __FUNCTION__, "missing updatefile on line %u of file \"%s\"", line, filename);
					continue;
				}
				if ((!(path = std::strtok(NULL, " \t"))) && tag_check_wolv1(tag_str_to_uint(clienttag)) && tag_check_wolv2(tag_str_to_uint(clienttag))) { /* Only in WOL is needed to have path */
					eventlog(eventlog_level_error, __FUNCTION__, "missing path on line %u of file \"%s\"", line, filename);
				}

				entry = (t_autoupdate*)xmalloc(sizeof(t_autoupdate));

				if (!tag_check_arch((entry->archtag = tag_str_to_uint(archtag)))) {
					eventlog(eventlog_level_error, __FUNCTION__, "got unknown archtag");
					xfree(entry);
					continue;
				}
				if (!tag_check_client((entry->clienttag = tag_str_to_uint(clienttag)))) {
					eventlog(eventlog_level_error, __FUNCTION__, "got unknown clienttag");
					xfree(entry);
					continue;
				}
				entry->versiontag = xstrdup(versiontag);
				entry->updatefile = xstrdup(updatefile);
				if (path)
					entry->path = xstrdup(path);
				else
					entry->path = NULL;

				eventlog(eventlog_level_debug, __FUNCTION__, "update '%s' version '%s' with file %s", clienttag, versiontag, updatefile);

				list_append_data(autoupdate_head, entry);
			}
			file_get_line(NULL); // clear file_get_line buffer
			std::fclose(fp);
			return 0;
		}

		/*
		 * Free up all of the elements in the linked list
		 */

		extern int autoupdate_unload(void)
		{
			if (autoupdate_head) {
				t_elem *       curr;
				t_autoupdate * entry;
				LIST_TRAVERSE(autoupdate_head, curr)
				{
					if (!(entry = (t_autoupdate*)elem_get_data(curr)))
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
					else {
						xfree((void *)entry->versiontag);	/* avoid warning */
						xfree((void *)entry->updatefile);	/* avoid warning */
						if (entry->path)
							xfree((void *)entry->path);		/* avoid warning */
						xfree(entry);
					}
					list_remove_elem(autoupdate_head, &curr);
				}

				if (list_destroy(autoupdate_head) < 0) return -1;
				autoupdate_head = NULL;
			}
			return 0;
		}

		/*
		 *  Check to see if an update exists for the clients version
		 *  return file name if there is one
		 *  retrun NULL if no update exists
		 */

		extern char * autoupdate_check(t_tag archtag, t_tag clienttag, t_tag gamelang, char const * versiontag, char const * sku)
		{
			if (autoupdate_head) {
				t_elem const * curr;
				t_autoupdate * entry;
				char * temp;

				LIST_TRAVERSE_CONST(autoupdate_head, curr)
				{
					if (!(entry = (t_autoupdate*)elem_get_data(curr))) {
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
						continue;
					}

					if (entry->archtag != archtag)
						continue;
					if (entry->clienttag != clienttag)
						continue;
					if (std::strcmp(entry->versiontag, versiontag) != 0)
						continue;

					/* if we have a gamelang or SKU then add it to the update file name */
					// so far only WAR3 uses gamelang specific MPQs!
					if (((gamelang) && ((clienttag == CLIENTTAG_WARCRAFT3_UINT) || (clienttag == CLIENTTAG_WAR3XP_UINT)))
						|| ((sku) && (tag_check_wolv2(clienttag)))) {
						char gltag[5];
						char * tempmpq;
						char * extention;
						char const * path = entry->path;

						tempmpq = xstrdup(entry->updatefile);

						extention = std::strrchr(tempmpq, '.');
						*extention = '\0';
						extention++;

						if ((clienttag == CLIENTTAG_WARCRAFT3_UINT) || (clienttag == CLIENTTAG_WAR3XP_UINT)) {
							tag_uint_to_str(gltag, gamelang);

							temp = (char*)xmalloc(std::strlen(entry->updatefile) + 6);
							std::sprintf(temp, "%s_%s.%s", tempmpq, gltag, extention);
						}
						else {
							temp = (char*)xmalloc(std::strlen(path) + std::strlen(entry->updatefile) + std::strlen(sku) + 3);
							std::sprintf(temp, "%s %s_%s.%s", path, tempmpq, sku, extention);
						}

						xfree((void *)tempmpq);
						return temp;
					}
					temp = xstrdup(entry->updatefile);
					return temp;
				}
			}
			return NULL;
		}

	}

}
