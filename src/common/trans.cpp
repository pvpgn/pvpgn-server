/*
 * Copyright (C) 2004  CreepLord (creeplord@pvpgn.org)
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
#define TRANS_INTERNAL_ACCESS
#include "trans.h"

#include <cerrno>
#include <cstring>

#include "common/setup_before.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "common/addr.h"
#include "common/util.h"
#include "common/xalloc.h"
#include "common/setup_after.h"

#define DEBUG_TRANS

namespace pvpgn
{

	static t_list * trans_head = NULL;

	extern int trans_load(char const * filename, int program)
	{
		std::FILE		*fp;
		unsigned int	line;
		unsigned int	pos;
		char		*buff;
		char		*temp;
		char 	*input;
		char const		*output;
		char const		*exclude;
		char const		*include;
		unsigned int	npos;
		char		*network;
		char		*tmp;
		char 		tmp1[32];
		char		tmp2[32];
		char		tmp3[32];
		t_trans		*entry;

		if (!filename) {
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
			return -1;
		}
		if (!(fp = std::fopen(filename, "r"))) {
			eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for reading (std::fopen: %s)", filename, std::strerror(errno));
			return -1;
		}
		trans_head = list_create();
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
			if (!(input = std::strtok(buff, " \t"))) { /* std::strtok modifies the string it is passed */
				eventlog(eventlog_level_error, __FUNCTION__, "missing input line %u of file \"%s\"", line, filename);
				continue;
			}
			/* check for port number - this tells us what programs will use this entry */
			if (!(temp = std::strrchr(input, ':'))) {
				eventlog(eventlog_level_error, __FUNCTION__, "missing port # on input line %u of file \"%s\"", line, filename);
				continue;
			}
			temp++;
			/* bnetd doesn't want the port 4000 entries */
			if (program == TRANS_BNETD  && std::strcmp(temp, "4000") == 0) {
#ifdef DEBUG_TRANS
				eventlog(eventlog_level_debug, __FUNCTION__, "d2gs input (ignoring) \"%s\"", input);
#endif
				continue;
			}
			/* d2cs only wants the port 4000 entries */
			if (program == TRANS_D2CS && std::strcmp(temp, "4000") != 0) {
#ifdef DEBUG_TRANS
				eventlog(eventlog_level_debug, __FUNCTION__, "non d2gs input (ignoring) \"%s\"", input);
#endif
				continue;
			}
			if (!(output = std::strtok(NULL, " \t"))) {
				eventlog(eventlog_level_error, __FUNCTION__, "missing output on line %u of file \"%s\"", line, filename);
				continue;
			}
			if (!(exclude = std::strtok(NULL, " \t"))) {
				eventlog(eventlog_level_error, __FUNCTION__, "missing exclude on line %u of file \"%s\"", line, filename);
				continue;
			}
			if (!(include = std::strtok(NULL, " \t"))) {
				eventlog(eventlog_level_error, __FUNCTION__, "missing include on line %u of file \"%s\"", line, filename);
				continue;
			}
			/* add exlude networks */
			tmp = xstrdup(exclude);
			npos = 0;
			while (tmp[npos]) {
				network = &tmp[npos];
				for (; tmp[npos] != ',' && tmp[npos] != '\0'; npos++);
				if (tmp[npos] == '\0')
					npos--;
				else
					tmp[npos] = '\0';
				if (std::strcmp(network, "NONE") == 0) {
					npos++;
					continue;
				}
				entry = (t_trans*)xmalloc(sizeof(t_trans));
				if (!(entry->input = addr_create_str(input, 0, 0))) {
					eventlog(eventlog_level_error, __FUNCTION__, "could not allocate memory for input address");
					xfree(entry);
					npos++;
					continue;
				}
				if (!(entry->output = addr_create_str(input, 0, 0))) {
					eventlog(eventlog_level_error, __FUNCTION__, "could not allocate memory for output address");
					addr_destroy(entry->input);
					xfree(entry);
					npos++;
					continue;
				}
				if (std::strcmp(network, "ANY") == 0) {
					if (!(entry->network = netaddr_create_str("0.0.0.0/0"))) {
						eventlog(eventlog_level_error, __FUNCTION__, "could not allocate memory for network address");
						addr_destroy(entry->output);
						addr_destroy(entry->input);
						xfree(entry);
						npos++;
						continue;
					}
				}
				else {
					if (!(entry->network = netaddr_create_str(network))) {
						eventlog(eventlog_level_error, __FUNCTION__, "could not allocate memory for network address");
						addr_destroy(entry->output);
						addr_destroy(entry->input);
						xfree(entry);
						npos++;
						continue;
					}
				}
#ifdef DEBUG_TRANS
				eventlog(eventlog_level_debug, __FUNCTION__,
					"Adding Host -> %s, Output -> %s, Network %s - (exclude)",
					addr_get_addr_str(entry->input, tmp1, sizeof(tmp1)),
					addr_get_addr_str(entry->output, tmp2, sizeof(tmp2)),
					netaddr_get_addr_str(entry->network, tmp3, sizeof(tmp3)));
#endif
				list_append_data(trans_head, entry);
				npos++;
			}
			xfree(tmp);
			/* add include networks */
			tmp = xstrdup(include);
			npos = 0;
			while (tmp[npos]) {
				network = &tmp[npos];
				for (; tmp[npos] != ',' && tmp[npos] != '\0'; npos++);
				if (tmp[npos] == '\0')
					npos--;
				else
					tmp[npos] = '\0';
				if (std::strcmp(network, "NONE") == 0) {
					npos++;
					continue;
				}
				entry = (t_trans*)xmalloc(sizeof(t_trans));
				if (!(entry->input = addr_create_str(input, 0, 0))) {
					eventlog(eventlog_level_error, __FUNCTION__, "could not allocate memory for input address");
					xfree(entry);
					npos++;
					continue;
				}
				if (!(entry->output = addr_create_str(output, 0, 0))) {
					eventlog(eventlog_level_error, __FUNCTION__, "could not allocate memory for output address");
					addr_destroy(entry->input);
					xfree(entry);
					npos++;
					continue;
				}
				if (std::strcmp(network, "ANY") == 0) {
					if (!(entry->network = netaddr_create_str("0.0.0.0/0"))) {
						eventlog(eventlog_level_error, __FUNCTION__, "could not allocate memory for network address");
						addr_destroy(entry->output);
						addr_destroy(entry->input);
						xfree(entry);
						npos++;
						continue;
					}
				}
				else {
					if (!(entry->network = netaddr_create_str(network))) {
						eventlog(eventlog_level_error, __FUNCTION__, "could not allocate memory for network address");
						addr_destroy(entry->output);
						addr_destroy(entry->input);
						xfree(entry);
						npos++;
						continue;
					}
				}
#ifdef DEBUG_TRANS
				eventlog(eventlog_level_debug, __FUNCTION__,
					"Adding Host -> %s, Output -> %s, Network %s - (include)",
					addr_get_addr_str(entry->input, tmp1, sizeof(tmp1)),
					addr_get_addr_str(entry->output, tmp2, sizeof(tmp2)),
					netaddr_get_addr_str(entry->network, tmp3, sizeof(tmp3)));
#endif
				list_append_data(trans_head, entry);
				npos++;
			}
			xfree(tmp);
		}
		file_get_line(NULL); // clear file_get_line buffer
		std::fclose(fp);
		eventlog(eventlog_level_info, __FUNCTION__, "trans file loaded");
		return 0;
	}

	extern int trans_unload(void)
	{
		t_elem	*curr;
		t_trans	*entry;

		if (trans_head) {
			LIST_TRAVERSE(trans_head, curr)
			{
				if (!(entry = (t_trans*)elem_get_data(curr))) {
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
				}
				else {
					netaddr_destroy(entry->network);
					addr_destroy(entry->output);
					addr_destroy(entry->input);
					xfree(entry);
				}
				list_remove_elem(trans_head, &curr);
			}
			list_destroy(trans_head);
			trans_head = NULL;
		}
		return 0;
	}

	extern int trans_reload(char const * filename, int program)
	{
		trans_unload();
		if (trans_load(filename, program) < 0) return -1;
		return 0;
	}

	extern int trans_net(unsigned int clientaddr, unsigned int *addr, unsigned short *port)
	{
		t_elem const *curr;
		t_trans	 *entry;
		char	 temp1[32];
		char         temp2[32];
		char         temp3[32];
		char	 temp4[32];

#ifdef DEBUG_TRANS
		eventlog(eventlog_level_debug, __FUNCTION__, "checking %s for client %s ...",
			addr_num_to_addr_str(*addr, *port),
			addr_num_to_ip_str(clientaddr));
#endif

		if (trans_head) {
			LIST_TRAVERSE_CONST(trans_head, curr)
			{
				if (!(entry = (t_trans*)elem_get_data(curr))) {
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
					continue;
				}

#ifdef DEBUG_TRANS
				eventlog(eventlog_level_debug, __FUNCTION__, "against entry -> %s output %s network %s",
					addr_get_addr_str(entry->input, temp1, sizeof(temp1)),
					addr_get_addr_str(entry->output, temp2, sizeof(temp2)),
					netaddr_get_addr_str(entry->network, temp3, sizeof(temp3)));
#endif
				if (addr_get_ip(entry->input) != *addr || addr_get_port(entry->input) != *port) {
#ifdef DEBUG_TRANS
					eventlog(eventlog_level_debug, __FUNCTION__, "entry does match input address");
#endif
					continue;
				}
				if (netaddr_contains_addr_num(entry->network, clientaddr) == 0) {
#ifdef DEBUG_TRANS
					eventlog(eventlog_level_debug, __FUNCTION__, "client is not in the correct network");
#endif
					continue;
				}
#ifdef DEBUG_TRANS
				eventlog(eventlog_level_debug, __FUNCTION__, "%s translated to %s",
					addr_num_to_addr_str(*addr, *port),
					addr_get_addr_str(entry->output, temp4, sizeof(temp4)));
#endif
				*addr = addr_get_ip(entry->output);
				*port = addr_get_port(entry->output);
				return 1; /* match found in list */
			}
		}
#ifdef DEBUG_TRANS
		eventlog(eventlog_level_debug, __FUNCTION__, "no match found for %s (not translated)",
			addr_num_to_addr_str(*addr, *port));
#endif
		return 0; /* no match found in list */
	}

}
