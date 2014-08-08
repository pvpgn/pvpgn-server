/*
 * Copyright (C) 2000 Onlyer (onlyer@263.net)
 * Copyright (C) 2001 Ross Combs (ross@bnetd.org)
 * Copyright (C) 2002 Gianluigi Tiesi (sherpya@netfarm.it)
 * Copyright (C) 2004 CreepLord (creeplord@pvpgn.org)
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
#define VERSIONCHECK_INTERNAL_ACCESS
#include "versioncheck.h"

#include <ctime>
#include <cstring>
#include <cstdio>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <cstdlib>

#include "compat/strcasecmp.h"
#include "common/list.h"
#include "common/xalloc.h"
#include "common/eventlog.h"
#include "common/util.h"
#include "common/field_sizes.h"
#include "common/token.h"
#include "common/proginfo.h"
#include "common/xstring.h"

#include "prefs.h"
#include "common/setup_after.h"


namespace pvpgn
{

	namespace bnetd
	{

		static t_list * versioninfo_head = NULL;
		static t_versioncheck dummyvc = { "A=42 B=42 C=42 4 A=A^S B=B^B C=C^C A=A^S", "IX86ver1.mpq", "NoVC" };

		static int versioncheck_compare_exeinfo(t_parsed_exeinfo * pattern, t_parsed_exeinfo * match);

		extern t_versioncheck * versioncheck_create(t_tag archtag, t_tag clienttag)
		{
			t_elem const *   curr;
			t_versioninfo *  vi;
			t_versioncheck * vc;
			char             archtag_str[5];
			char             clienttag_str[5];

			LIST_TRAVERSE_CONST(versioninfo_head, curr)
			{
				if (!(vi = (t_versioninfo*)elem_get_data(curr))) /* should not happen */
				{
					eventlog(eventlog_level_error, __FUNCTION__, "version list contains NULL item");
					continue;
				}

				eventlog(eventlog_level_debug, __FUNCTION__, "version check entry archtag=%s, clienttag=%s",
					tag_uint_to_str(archtag_str, vi->archtag),
					tag_uint_to_str(clienttag_str, vi->clienttag));

				if (vi->archtag != archtag)
					continue;
				if (vi->clienttag != clienttag)
					continue;

				/* FIXME: randomize the selection if more than one match */
				vc = (t_versioncheck*)xmalloc(sizeof(t_versioncheck));
				vc->eqn = xstrdup(vi->eqn);
				vc->mpqfile = xstrdup(vi->mpqfile);
				vc->versiontag = xstrdup(tag_uint_to_str(clienttag_str, clienttag));

				return vc;
			}

			/*
			 * No entries in the file that match, return the dummy because we have to send
			 * some equation and auth mpq to the client.  The client is not going to pass the
			 * validation later unless skip_versioncheck or allow_unknown_version is enabled.
			 */
			return &dummyvc;
		}


		extern int versioncheck_destroy(t_versioncheck * vc)
		{
			if (!vc)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL vc");
				return -1;
			}

			if (vc == &dummyvc)
				return 0;

			xfree((void *)vc->versiontag);
			xfree((void *)vc->mpqfile);
			xfree((void *)vc->eqn);
			xfree(vc);

			return 0;
		}

		extern int versioncheck_set_versiontag(t_versioncheck * vc, char const * versiontag)
		{
			if (!vc) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL vc");
				return -1;
			}
			if (!versiontag) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL versiontag");
				return -1;
			}

			if (vc->versiontag != NULL) xfree((void *)vc->versiontag);
			vc->versiontag = xstrdup(versiontag);
			return 0;
		}


		extern char const * versioncheck_get_versiontag(t_versioncheck const * vc)
		{
			if (!vc) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL vc");
				return NULL;
			}

			return vc->versiontag;
		}


		extern char const * versioncheck_get_mpqfile(t_versioncheck const * vc)
		{
			if (!vc)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL vc");
				return NULL;
			}

			return vc->mpqfile;
		}


		extern char const * versioncheck_get_eqn(t_versioncheck const * vc)
		{
			if (!vc)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL vc");
				return NULL;
			}

			return vc->eqn;
		}

		t_parsed_exeinfo * parse_exeinfo(char const * exeinfo)
		{
			t_parsed_exeinfo * parsed_exeinfo;

			if (!exeinfo) {
				return NULL;
			}

			parsed_exeinfo = (t_parsed_exeinfo*)xmalloc(sizeof(t_parsed_exeinfo));
			parsed_exeinfo->exe = xstrdup(exeinfo);
			parsed_exeinfo->time = 0;
			parsed_exeinfo->size = 0;

			if (std::strcmp(prefs_get_version_exeinfo_match(), "parse") == 0)
			{
#ifdef HAVE_MKTIME
				struct std::tm t1;
				char *exe;
				char mask[MAX_EXEINFO_STR + 1];
				char * marker;
				unsigned size;
				char time_invalid = 0;

				if ((exeinfo[0] == '\0') ||	   //happens when using war3-noCD and having deleted war3.org
					(std::strcmp(exeinfo, "badexe") == 0)) //happens when AUTHREQ had no owner/exeinfo entry
				{
					xfree((void *)parsed_exeinfo->exe);
					xfree((void *)parsed_exeinfo);
					eventlog(eventlog_level_error, __FUNCTION__, "found empty exeinfo");
					return NULL;
				}

				std::memset(&t1, 0, sizeof(t1));
				t1.tm_isdst = -1;

				exeinfo = strreverse((char *)exeinfo);
				if (!(marker = std::strchr((char *)exeinfo, ' ')))
				{
					xfree((void *)parsed_exeinfo->exe);
					xfree((void *)parsed_exeinfo);
					return NULL;
				}
				for (; marker[0] == ' '; marker++);

				if (!(marker = std::strchr(marker, ' ')))
				{
					xfree((void *)parsed_exeinfo->exe);
					xfree((void *)parsed_exeinfo);
					return NULL;
				}
				for (; marker[0] == ' '; marker++);

				if (!(marker = std::strchr(marker, ' ')))
				{
					xfree((void *)parsed_exeinfo->exe);
					xfree((void *)parsed_exeinfo);
					return NULL;
				}
				for (; marker[0] == ' '; marker++);
				marker--;
				marker[0] = '\0';
				marker++;

				exe = xstrdup(marker);
				xfree((void *)parsed_exeinfo->exe);
				parsed_exeinfo->exe = strreverse((char *)exe);

				exeinfo = strreverse((char *)exeinfo);

				std::sprintf(mask, "%%02u/%%02u/%%u %%02u:%%02u:%%02u %%u");

				if (std::sscanf(exeinfo, mask, &t1.tm_mon, &t1.tm_mday, &t1.tm_year, &t1.tm_hour, &t1.tm_min, &t1.tm_sec, &size) != 7) {
					if (std::sscanf(exeinfo, "%*s %*s %u", &size) != 1)
					{

						eventlog(eventlog_level_warn, __FUNCTION__, "parser error while parsing pattern \"%s\"", exeinfo);
						xfree((void *)parsed_exeinfo->exe);
						xfree((void *)parsed_exeinfo);
						return NULL; /* neq */
					}
					time_invalid = 1;
				}

				/* Now we have a Y2K problem :)  Thanks for using a 2 digit decimal years, Blizzard. */
				/* 00-79 -> 2000-2079
			 *             * 80-99 -> 1980-1999
			 *             * 100+ unchanged */
				if (t1.tm_year < 80)
					t1.tm_year = t1.tm_year + 100;

				if (time_invalid)
					parsed_exeinfo->time = -1;
				else
					parsed_exeinfo->time = std::mktime(&t1);
				parsed_exeinfo->size = size;

#else
				eventlog(eventlog_level_error, __FUNCTION__, "Your std::system does not support std::mktime(). Please select another exeinfo matching method.");
				return NULL;
#endif
			}

			return parsed_exeinfo;
		}


		/* This implements some dumb kind of pattern matching. Any '?'
		 * signs in the pattern are treated as "don't care" signs. This
		 * means that it doesn't matter what's on this place in the match.
		 */
		//static int versioncheck_compare_exeinfo(char const * pattern, char const * match)
		static int versioncheck_compare_exeinfo(t_parsed_exeinfo * pattern, t_parsed_exeinfo * match)
		{
			assert(pattern);
			assert(match);

			if (!strcasecmp(prefs_get_version_exeinfo_match(), "none"))
				return 0;	/* ignore exeinfo */

			if (std::strlen(pattern->exe) != std::strlen(match->exe))
				return 1; /* neq */

			if (std::strcmp(prefs_get_version_exeinfo_match(), "exact") == 0) {
				return strcasecmp(pattern->exe, match->exe);
			}
			else if (std::strcmp(prefs_get_version_exeinfo_match(), "exactcase") == 0) {
				return std::strcmp(pattern->exe, match->exe);
			}
			else if (std::strcmp(prefs_get_version_exeinfo_match(), "wildcard") == 0) {
				unsigned int i;

				for (i = 0; i < std::strlen(pattern->exe); i++)
				if ((pattern->exe[i] != '?') && /* out "don't care" sign */
					(safe_toupper(pattern->exe[i]) != safe_toupper(match->exe[i])))
					return 1; /* neq */
				return 0; /* ok */
			}
			else if (std::strcmp(prefs_get_version_exeinfo_match(), "parse") == 0) {

				if (strcasecmp(pattern->exe, match->exe) != 0)
				{
					eventlog(eventlog_level_trace, __FUNCTION__, "filename differs");
					return 1; /* neq */
				}
				if (pattern->size != match->size)
				{
					eventlog(eventlog_level_trace, __FUNCTION__, "size differs");
					return 1; /* neq */
				}
				if ((pattern->time != -1) && prefs_get_version_exeinfo_maxdiff() && (abs(pattern->time - match->time) > (signed)prefs_get_version_exeinfo_maxdiff()))
				{
					eventlog(eventlog_level_trace, __FUNCTION__, "time differs by %i", abs(pattern->time - match->time));
					return 1;
				}
				return 0; /* ok */
			}
			else {
				eventlog(eventlog_level_error, __FUNCTION__, "unknown version exeinfo match method \"%s\"", prefs_get_version_exeinfo_match());
				return -1; /* neq/fail */
			}
		}

		void free_parsed_exeinfo(t_parsed_exeinfo * parsed_exeinfo)
		{
			if (parsed_exeinfo)
			{
				if (parsed_exeinfo->exe)
					xfree((void *)parsed_exeinfo->exe);
				xfree((void *)parsed_exeinfo);
			}
		}

		extern int versioncheck_validate(t_versioncheck * vc, t_tag archtag, t_tag clienttag, char const * exeinfo, unsigned long versionid, unsigned long gameversion, unsigned long checksum)
		{
			t_elem const     * curr;
			t_versioninfo    * vi;
			int                badexe, badcs;
			t_parsed_exeinfo * parsed_exeinfo;

			if (!vc)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL vc");
				return -1;
			}

			badexe = badcs = 0;
			parsed_exeinfo = parse_exeinfo(exeinfo);
			LIST_TRAVERSE_CONST(versioninfo_head, curr)
			{
				if (!(vi = (t_versioninfo*)elem_get_data(curr))) /* should not happen */
				{
					eventlog(eventlog_level_error, __FUNCTION__, "version list contains NULL item");
					continue;
				}

				if (vi->archtag != archtag)
					continue;
				if (vi->clienttag != clienttag)
					continue;
				if (std::strcmp(vi->eqn, vc->eqn) != 0)
					continue;
				if (std::strcmp(vi->mpqfile, vc->mpqfile) != 0)
					continue;

				if (vi->versionid && vi->versionid != versionid)
					continue;

				if (vi->gameversion && vi->gameversion != gameversion)
					continue;


				if ((!(parsed_exeinfo)) || (vi->parsed_exeinfo && (versioncheck_compare_exeinfo(vi->parsed_exeinfo, parsed_exeinfo) != 0)))
				{
					/*
					 * Found an entry matching but the exeinfo doesn't match.
					 * We need to rember this because if no other matching versions are found
					 * we will return badversion.
					 */
					badexe = 1;
				}
				else
					badexe = 0;

				if (vi->checksum && vi->checksum != checksum)
				{
					/*
					 * Found an entry matching but the checksum doesn't match.
					 * We need to rember this because if no other matching versions are found
					 * we will return badversion.
					 */
					badcs = 1;
				}
				else
					badcs = 0;

				if (vc->versiontag)
					xfree((void *)vc->versiontag);
				vc->versiontag = xstrdup(vi->versiontag);

				if (badexe || badcs)
					continue;

				/* Ok, version and checksum matches or exeinfo/checksum are disabled
				 * anyway we have found a complete match */
				eventlog(eventlog_level_info, __FUNCTION__, "got a matching entry: %s", vc->versiontag);
				free_parsed_exeinfo(parsed_exeinfo);
				return 1;
			}

			if (badcs) /* A match was found but the checksum was different */
			{
				eventlog(eventlog_level_info, __FUNCTION__, "bad checksum, closest match is: %s", vc->versiontag);
				free_parsed_exeinfo(parsed_exeinfo);
				return -1;
			}
			if (badexe) /* A match was found but the exeinfo string was different */
			{
				eventlog(eventlog_level_info, __FUNCTION__, "bad exeinfo, closest match is: %s", vc->versiontag);
				free_parsed_exeinfo(parsed_exeinfo);
				return -1;
			}

			/* No match in list */
			eventlog(eventlog_level_info, __FUNCTION__, "no match in list, setting to: %s", vc->versiontag);
			free_parsed_exeinfo(parsed_exeinfo);
			return 0;
		}

		extern int versioncheck_load(char const * filename)
		{
			std::FILE *	    fp;
			unsigned int    line;
			unsigned int    pos;
			char *	    buff;
			char *	    temp;
			char const *    eqn;
			char const *    mpqfile;
			char const *    archtag;
			char const *    clienttag;
			char const *    exeinfo;
			char const *    versionid;
			char const *    gameversion;
			char const *    checksum;
			char const *    versiontag;
			t_versioninfo * vi;

			if (!filename)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
				return -1;
			}

			if (!(versioninfo_head = list_create()))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could create list");
				return -1;
			}
			if (!(fp = std::fopen(filename, "r")))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for reading (std::fopen: %s)", filename, std::strerror(errno));
				list_destroy(versioninfo_head);
				versioninfo_head = NULL;
				return -1;
			}

			line = 1;
			for (; (buff = file_get_line(fp)); line++)
			{
				for (pos = 0; buff[pos] == '\t' || buff[pos] == ' '; pos++);
				if (buff[pos] == '\0' || buff[pos] == '#')
				{
					continue;
				}
				if ((temp = std::strrchr(buff, '#')))
				{
					unsigned int len;
					unsigned int endpos;

					*temp = '\0';
					len = std::strlen(buff) + 1;
					for (endpos = len - 1; buff[endpos] == '\t' || buff[endpos] == ' '; endpos--);
					buff[endpos + 1] = '\0';
				}

				if (!(eqn = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing eqn near line %u of file \"%s\"", line, filename);
					continue;
				}
				line++;
				if (!(mpqfile = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing mpqfile near line %u of file \"%s\"", line, filename);
					continue;
				}
				line++;
				if (!(archtag = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing archtag near line %u of file \"%s\"", line, filename);
					continue;
				}
				line++;
				if (!(clienttag = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing clienttag near line %u of file \"%s\"", line, filename);
					continue;
				}
				line++;
				if (!(exeinfo = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing exeinfo near line %u of file \"%s\"", line, filename);
					continue;
				}
				line++;
				if (!(versionid = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing versionid near line %u of file \"%s\"", line, filename);
					continue;
				}
				line++;
				if (!(gameversion = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing gameversion near line %u of file \"%s\"", line, filename);
					continue;
				}
				line++;
				if (!(checksum = next_token(buff, &pos)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "missing checksum near line %u of file \"%s\"", line, filename);
					continue;
				}
				line++;
				if (!(versiontag = next_token(buff, &pos)))
				{
					versiontag = NULL;
				}

				vi = (t_versioninfo*)xmalloc(sizeof(t_versioninfo));
				vi->eqn = xstrdup(eqn);
				vi->mpqfile = xstrdup(mpqfile);
				if (std::strlen(archtag) != 4)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "invalid arch tag on line %u of file \"%s\"", line, filename);
					xfree((void *)vi->mpqfile); /* avoid warning */
					xfree((void *)vi->eqn); /* avoid warning */
					xfree(vi);
					continue;
				}
				if (!tag_check_arch((vi->archtag = tag_str_to_uint(archtag))))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got unknown archtag \"%s\"", archtag);
					xfree((void *)vi->mpqfile); /* avoid warning */
					xfree((void *)vi->eqn); /* avoid warning */
					xfree(vi);
					continue;
				}
				if (std::strlen(clienttag) != 4)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "invalid client tag on line %u of file \"%s\"", line, filename);
					xfree((void *)vi->mpqfile); /* avoid warning */
					xfree((void *)vi->eqn); /* avoid warning */
					xfree(vi);
					continue;
				}
				if (!tag_check_client((vi->clienttag = tag_str_to_uint(clienttag))))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got unknown clienttag\"%s\"", clienttag);
					xfree((void *)vi->mpqfile); /* avoid warning */
					xfree((void *)vi->eqn); /* avoid warning */
					xfree(vi);
					continue;
				}
				if (std::strcmp(exeinfo, "NULL") == 0)
					vi->parsed_exeinfo = NULL;
				else
				{
					if (!(vi->parsed_exeinfo = parse_exeinfo(exeinfo)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "encountered an error while parsing exeinfo");
						xfree((void *)vi->mpqfile); /* avoid warning */
						xfree((void *)vi->eqn); /* avoid warning */
						xfree(vi);
						continue;
					}
				}

				vi->versionid = std::strtoul(versionid, NULL, 0);
				if (verstr_to_vernum(gameversion, &vi->gameversion) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "malformed version on line %u of file \"%s\"", line, filename);
					xfree((void *)vi->parsed_exeinfo); /* avoid warning */
					xfree((void *)vi->mpqfile); /* avoid warning */
					xfree((void *)vi->eqn); /* avoid warning */
					xfree(vi);
					continue;
				}

				vi->checksum = std::strtoul(checksum, NULL, 0);
				if (versiontag)
					vi->versiontag = xstrdup(versiontag);
				else
					vi->versiontag = NULL;


				list_append_data(versioninfo_head, vi);
			}

			file_get_line(NULL); // clear file_get_line buffer
			if (std::fclose(fp) < 0)
				eventlog(eventlog_level_error, __FUNCTION__, "could not close versioncheck file \"%s\" after reading (std::fclose: %s)", filename, std::strerror(errno));

			return 0;
		}


		extern int versioncheck_unload(void)
		{
			t_elem *	    curr;
			t_versioninfo * vi;

			if (versioninfo_head)
			{
				LIST_TRAVERSE(versioninfo_head, curr)
				{
					if (!(vi = (t_versioninfo*)elem_get_data(curr))) /* should not happen */
					{
						eventlog(eventlog_level_error, __FUNCTION__, "version list contains NULL item");
						continue;
					}

					if (list_remove_elem(versioninfo_head, &curr) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "could not remove item from list");

					if (vi->parsed_exeinfo)
					{
						if (vi->parsed_exeinfo->exe)
							xfree((void *)vi->parsed_exeinfo->exe);
						xfree((void *)vi->parsed_exeinfo); /* avoid warning */
					}
					xfree((void *)vi->mpqfile); /* avoid warning */
					xfree((void *)vi->eqn); /* avoid warning */
					if (vi->versiontag)
						xfree((void *)vi->versiontag); /* avoid warning */
					xfree(vi);
				}

				if (list_destroy(versioninfo_head) < 0)
					return -1;
				versioninfo_head = NULL;
			}

			return 0;
		}

	}

}
