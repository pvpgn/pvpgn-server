/*
 * Copyright (C) 1998,1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
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


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_UTIL_PROTOS
#define INCLUDED_UTIL_PROTOS

#include <cstdio>
#include <ctime>

namespace pvpgn
{

	extern int strstart(char const * full, char const * part);
	extern char * file_get_line(std::FILE * fp);
	extern char * strreverse(char * str);
	extern int str_to_uint(char const * str, unsigned int * num);
	extern int str_to_ushort(char const * str, unsigned short * num);
	extern int str_print_term(std::FILE * fp, char const * str, unsigned int len, int allow_nl);
	extern int str_get_bool(char const * str);
	extern char const * seconds_to_timestr(unsigned int totsecs); /* FIXME: can this be marked pure? */
	extern int clockstr_to_seconds(char const * clockstr, unsigned int * totsecs);
	extern char * escape_fs_chars(char const * in, unsigned int len);
	extern char * escape_chars(char const * in, unsigned int len);
	extern char * unescape_chars(char const * in);
	extern void str_to_hex(char * target, char const * data, int datalen);
	extern int hex_to_str(char const * source, char * data, int datalen);
	extern char * buildpath(char const *root, const char *suffix);
	extern int timestr_to_time(char const * timestr, std::time_t* ptime);
	extern void strlower(char* str);

	static inline char * str_skip_space(char *str)
	{
		for (; *str == ' ' || *str == '\t'; str++);
		return str;
	}

	static inline char * str_skip_word(char *str)
	{
		for (; *str != ' ' && *str != '\t' && *str; str++);
		return str;
	}

}

#endif
#endif
