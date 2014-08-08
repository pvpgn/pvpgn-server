/*
 * Copyright (C) 2004,2005  Dizzy
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
#include "common/conf.h"

#include <cassert>
#include <cstring>

#include "compat/snprintf.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/util.h"
#include "common/setup_after.h"

namespace pvpgn
{

	extern int conf_set_int(unsigned *pint, const char *valstr, unsigned def)
	{
		if (!valstr) *pint = def;
		else {
			unsigned temp;

			if (str_to_uint(valstr, &temp) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "invalid integer value '%s'", valstr);
				return -1;
			}
			else *pint = temp;
		}

		return 0;
	}

	extern int conf_set_bool(unsigned *pbool, const char *valstr, unsigned def)
	{
		if (!valstr) *pbool = def;
		else {
			switch (str_get_bool(valstr)) {
			case 1:
				*pbool = 1;
				break;
			case 0:
				*pbool = 0;
				break;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "invalid boolean value '%s'", valstr);
				return -1;
			}
		}

		return 0;
	}

	extern int conf_set_str(const char **pstr, const char *valstr, const char *def)
	{
		if (*pstr) xfree((void*)*pstr);
		if (!valstr && !def) *pstr = NULL;
		else *pstr = xstrdup(valstr ? valstr : def);

		return 0;
	}

	extern int conf_set_timestr(std::time_t* ptime, const char *valstr, std::time_t def)
	{
		if (!valstr) *ptime = def;
		else if (timestr_to_time(valstr, ptime) < 0) {
			eventlog(eventlog_level_error, __FUNCTION__, "invalid timestr value '%s'", valstr);
			return -1;
		}

		return 0;
	}

	extern const char* conf_get_int(unsigned ival)
	{
		static char tmpbuf[128];
		snprintf(tmpbuf, sizeof(tmpbuf), "%u", ival);
		return tmpbuf;
	}

	extern const char* conf_get_bool(unsigned ival)
	{
		return ival ? "true" : "false";
	}

	static void _conf_reset_defaults(t_conf_entry *conftab)
	{
		t_conf_entry *curr;

		assert(conftab);
		for (curr = conftab; curr->name; curr++)
			curr->setdef();
	}

	static char * _get_value(char *str, unsigned lineno)
	{
		char *cp, prev;

		if (*str == '"') {
			for (cp = ++str, prev = '\0'; *cp; cp++) {
				switch (*cp) {
				case '"':
					if (prev != '\\') break;
					prev = '"';
					continue;
				case '\\':
					if (prev == '\\') prev = '\0';
					else prev = '\\';
					continue;
				default:
					prev = *cp;
					continue;
				}
				break;
			}

			if (*cp != '"') {
				eventlog(eventlog_level_error, __FUNCTION__, "missing end quota at line %u", lineno);
				return NULL;
			}

			*cp = '\0';
			cp = str_skip_space(cp + 1);
			if (*cp) {
				eventlog(eventlog_level_error, __FUNCTION__, "extra characters in value after ending quote at line %u", lineno);
				return NULL;
			}
		}
		else {
			cp = str_skip_word(str);
			if (*cp) {
				*cp = '\0';
				cp = str_skip_space(cp + 1);
				if (*cp) {
					eventlog(eventlog_level_error, __FUNCTION__, "extra characters after the value at line %u", lineno);
					return NULL;
				}
			}
		}

		return str;
	}

	static void _process_option(const char *key, const char *val, t_conf_entry *conftab)
	{
		t_conf_entry *curr;

		for (curr = conftab; curr->name; curr++)
		if (!std::strcmp(key, curr->name)) {
			curr->set(val);
			return;
		}

		eventlog(eventlog_level_error, __FUNCTION__, "option '%s' unknown", key);
	}

	extern int conf_load_file(std::FILE *fd, t_conf_entry *conftab)
	{
		char *buff, *directive, *value, *cp;
		unsigned cflag, lineno;

		if (!fd) {
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL file");
			return -1;
		}

		if (!conftab) {
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL conftab");
			return -1;
		}

		/* restore defaults */
		_conf_reset_defaults(conftab);

		for (lineno = 1; (buff = file_get_line(fd)); lineno++) {
			cflag = 1;
			for (cp = buff; *cp; cp++) {
				switch (*cp) {
				case '\\':
					if (!*(++cp)) cflag = 0;
					break;
				case '"':
					switch (cflag) {
					case 1: cflag = 2; break;
					case 2: cflag = 1; break;
					}
					break;
				case '#':
					if (cflag == 1) cflag = 0;
					break;
				}
				if (!cflag) break;
			}

			if (*cp == '#') *cp = '\0';
			cp = str_skip_space(buff);
			if (!*cp) continue;

			directive = cp;
			cp = str_skip_word(cp + 1);
			if (*cp) *(cp++) = '\0';

			cp = str_skip_space(cp);
			if (*cp != '=') {
				eventlog(eventlog_level_error, __FUNCTION__, "missing = on line %u", lineno);
				continue;
			}

			cp = str_skip_space(cp + 1);
			if (!*cp) {
				eventlog(eventlog_level_error, __FUNCTION__, "missing value at line %u", lineno);
				continue;
			}

			value = _get_value(cp, lineno);
			if (!value) continue;

			_process_option(directive, value, conftab);
		}

		return 0;
	}

	extern void conf_unload(t_conf_entry *conftab)
	{
		t_conf_entry *curr;

		if (!conftab) {
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL conftab");
			return;
		}

		for (curr = conftab; curr->name; curr++)
			curr->set(NULL);
	}

	extern int conf_load_cmdline(int argc, char **argv, t_conf_entry *conftab)
	{
		int i;
		char *key, *val, *newkey;

		if (!conftab) {
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL conftab");
			return -1;
		}

		/* restore defaults */
		_conf_reset_defaults(conftab);

		for (i = 1; i < argc; ++i) {
			newkey = NULL;
			key = argv[i];

			if (*(key++) != '-') /* skip non options */
				continue;

			if (*key == '-') key++;	/* allow both - and -- options */

			if ((val = std::strchr(key, '='))) {	/* we got option=value format */
				newkey = xstrdup(key);
				key = newkey;
				val = std::strchr(key, '=');
				*(val++) = '\0';
			}
			else if (i + 1 < argc && argv[i + 1][0] != '-') {
				val = argv[++i];
			}

			if (!val)	/* option without argument, so it's like boolean */
				val = (char *)("true");

			_process_option(key, val, conftab);

			if (newkey) xfree(newkey);
		}

		return 0;
	}

}
