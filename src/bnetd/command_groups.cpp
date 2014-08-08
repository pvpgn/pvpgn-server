/*
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
#define COMMAND_GROUPS_INTERNAL_ACCESS
#include "command_groups.h"

#include <cstdio>
#include <cerrno>
#include <cstring>

#include "common/eventlog.h"
#include "common/list.h"
#include "common/util.h"
#include "common/xalloc.h"

#include "common/setup_after.h"

//#define COMMANDGROUPSDEBUG 1

namespace pvpgn
{

	namespace bnetd
	{

		static t_list * command_groups_head = NULL;
		static std::FILE * fp = NULL;

		extern int command_groups_load(char const * filename)
		{
			unsigned int	line;
			unsigned int	pos;
			char *		buff;
			char *		temp;
			char const *	command;
			unsigned int	group;
			t_command_groups *	entry;

			if (!filename) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
				return -1;
			}
			if (!(fp = std::fopen(filename, "r"))) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for reading (std::fopen: %s)", filename, std::strerror(errno));
				return -1;
			}

			command_groups_head = list_create();

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
				if (!(temp = std::strtok(buff, " \t"))) { /* std::strtok modifies the string it is passed */
					eventlog(eventlog_level_error, __FUNCTION__, "missing group on line %u of file \"%s\"", line, filename);
					continue;
				}
				if (str_to_uint(temp, &group)<0) {
					eventlog(eventlog_level_error, __FUNCTION__, "group '%s' not a valid group (1-8)", temp);
					continue;
				}
				if (group == 0 || group > 8) {
					eventlog(eventlog_level_error, __FUNCTION__, "group '%u' not within groups limits (1-8)", group);
					continue;
				}
				while ((command = std::strtok(NULL, " \t"))) {
					entry = (t_command_groups*)xmalloc(sizeof(t_command_groups));
					entry->group = 1 << (group - 1);
					entry->command = xstrdup(command);
					list_append_data(command_groups_head, entry);
#ifdef COMMANDGROUPSDEBUG
					eventlog(eventlog_level_info, __FUNCTION__, "Added command: %s - with group %u", entry->command, entry->group);
#endif
				}
			}
			file_get_line(NULL); // clear file_get_line buffer
			std::fclose(fp);
			return 0;
		}

		extern int command_groups_unload(void)
		{
			t_elem *		curr;
			t_command_groups *	entry;

			if (command_groups_head) {
				LIST_TRAVERSE(command_groups_head, curr) {
					if (!(entry = (t_command_groups*)elem_get_data(curr)))
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
					else {
						xfree(entry->command);
						xfree(entry);
					}
					list_remove_elem(command_groups_head, &curr);
				}
				list_destroy(command_groups_head);
				command_groups_head = NULL;
			}
			return 0;
		}

		extern unsigned int command_get_group(char const * command)
		{
			t_elem const *	curr;
			t_command_groups *	entry;

			if (command_groups_head) {
				LIST_TRAVERSE(command_groups_head, curr) {
					if (!(entry = (t_command_groups*)elem_get_data(curr)))
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
					else if (!(std::strcmp(entry->command, command)))
						return entry->group;
				}
			}
			return 0;
		}

		extern int command_groups_reload(char const * filename)
		{
			command_groups_unload();
			return command_groups_load(filename);
		}

	}

}
