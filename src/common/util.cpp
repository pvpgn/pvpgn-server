/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
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
#include "common/util.h"

#include <cstdlib>
#include <cctype>
#include <cstdio>

#include "compat/strcasecmp.h"
#include "compat/strncasecmp.h"
#include "common/xalloc.h"
#include "common/setup_after.h"


namespace pvpgn
{

	extern int strstart(char const * full, char const * part)
	{
		std::size_t strlen_part;
		int compare_result;

		if (!full || !part)
			return 1;

		strlen_part = std::strlen(part);
		compare_result = strncasecmp(full, part, strlen_part);

		/* If there is more than the command, make sure it is separated */
		if (compare_result != 0)
			return compare_result;
		else if (full[strlen_part] != ' ' && full[strlen_part] != '\0')
			return 1;
		else return compare_result;
	}


#define DEF_LEN 64
#define INC_LEN 16

	extern char * file_get_line(std::FILE * fp)
	{
		static char *       line = NULL;
		static unsigned int	len = 0;
		unsigned int 	pos = 0;
		int          	prev_char, curr_char;

		// use file_get_line with NULL argument to clear the buffer
		if (!(fp))
		{
			len = 0;
			if ((line))
				xfree((void *)line);
			line = NULL;
			return NULL;
		}

		if (!(line))
		{
			line = (char*)xmalloc(DEF_LEN);
			len = DEF_LEN;
		}

		prev_char = '\0';
		while ((curr_char = std::fgetc(fp)) != EOF)
		{
			if (((char)curr_char) == '\r')
				continue; /* make DOS line endings look Unix-like */
			if (((char)curr_char) == '\n')
			{
				if (pos < 1 || ((char)prev_char) != '\\')
					break;
				pos--; /* throw away the backslash */
				prev_char = '\0';
				continue;
			}
			prev_char = curr_char;

			line[pos++] = (char)curr_char;
			if ((pos + 1) >= len)
			{
				len += INC_LEN;
				line = (char*)xrealloc(line, len);
			}
		}

		if (curr_char == EOF && pos < 1) /* not even an empty line */
		{
			return NULL;
		}

		line[pos] = '\0';

		return line;
	}


	extern char * strreverse(char * str)
	{
		unsigned int len;
		char         temp;
		char *start, *end;

		if (!str)
			return NULL;

		len = std::strlen(str);

		for (start = str, end = str + len - 1; start < end; start++, end--)
		{
			temp = *end;
			*end = *start;
			*start = temp;
		}

		return str;
	}


	extern int str_to_uint(char const * str, unsigned int * num)
	{
		unsigned int val;
		unsigned int pval;
		char * pos;

		if (!str || !num)
			return -1;
		for (pos = (char *)str; *pos == ' ' || *pos == '\t'; pos++);
		if (*pos == '+')
			pos++;

		val = 0;
		for (; *pos != '\0'; pos++)
		{
			pval = val;
			val *= 10;
			if (val / 10 != pval) /* check for overflow */
				return -1;

			pval = val;
			if (std::isdigit(*pos))
				val += *pos - '0';
			else
				return -1;

			if (val < pval) /* check for overflow */
				return -1;
		}

		*num = val;
		return 0;
	}


	extern int str_to_ushort(char const * str, unsigned short * num)
	{
		unsigned short val;
		unsigned short pval;
		char * pos;

		if (!str || !num)
			return -1;
		for (pos = (char *)str; *pos == ' ' || *pos == '\t'; pos++);
		if (*pos == '+')
			pos++;

		val = 0;
		for (; *pos != '\0'; pos++)
		{
			pval = val;
			val *= 10;
			if (val / 10 != pval) /* check for overflow */
				return -1;

			pval = val;

			if (std::isdigit(*pos))
				val += *pos - '0';
			else
				return -1;

			if (val < pval) /* check for overflow */
				return -1;
		}

		*num = val;
		return 0;
	}


	/* This routine assumes ASCII like control chars.
	   If len is zero, it will print all characters up to the first NUL,
	   otherwise it will print exactly that many characters. */
	int str_print_term(std::FILE * fp, char const * str, unsigned int len, int allow_nl)
	{
		unsigned int i;

		if (!fp)
			return -1;
		if (!str)
			return -1;

		if (len == 0)
			len = std::strlen(str);
		for (i = 0; i < len; i++)
		if ((str[i] == '\177' || (str[i] >= '\000' && str[i] < '\040')) &&
			(!allow_nl || (str[i] != '\r' && str[i] != '\n')))
			std::fprintf(fp, "^%c", str[i] + 64);
		else
			std::fputc((int)str[i], fp);

		return 0;
	}


	extern int str_get_bool(char const * str)
	{
		if (!str)
			return -1;

		if (strcasecmp(str, "true") == 0 ||
			strcasecmp(str, "yes") == 0 ||
			strcasecmp(str, "on") == 0 ||
			std::strcmp(str, "1") == 0)
			return 1;

		if (strcasecmp(str, "false") == 0 ||
			strcasecmp(str, "no") == 0 ||
			strcasecmp(str, "off") == 0 ||
			std::strcmp(str, "0") == 0)
			return 0;

		return -1;
	}


	extern char const * seconds_to_timestr(unsigned int totsecs)
	{
		static char temp[256];
		int         days;
		int         hours;
		int         minutes;
		int         seconds;

		days = totsecs / (24 * 60 * 60);
		hours = (totsecs / (60 * 60)) % 24;
		minutes = (totsecs / 60) % 60;
		seconds = totsecs % 60;

		if (days > 0)
			std::sprintf(temp, "%d day%s %d hour%s %d minute%s %d second%s",
			days, days == 1 ? "" : "s",
			hours, hours == 1 ? "" : "s",
			minutes, minutes == 1 ? "" : "s",
			seconds, seconds == 1 ? "" : "s");
		else if (hours > 0)
			std::sprintf(temp, "%d hour%s %d minute%s %d second%s",
			hours, hours == 1 ? "" : "s",
			minutes, minutes == 1 ? "" : "s",
			seconds, seconds == 1 ? "" : "s");
		else if (minutes > 0)
			std::sprintf(temp, "%d minute%s %d second%s",
			minutes, minutes == 1 ? "" : "s",
			seconds, seconds == 1 ? "" : "s");
		else
			std::sprintf(temp, "%d second%s.",
			seconds, seconds == 1 ? "" : "s");

		return temp;
	}


	extern int clockstr_to_seconds(char const * clockstr, unsigned int * totsecs)
	{
		unsigned int i, j;
		unsigned int temp;

		if (!clockstr)
			return -1;
		if (!totsecs)
			return -1;

		for (i = j = temp = 0; j < std::strlen(clockstr); j++)
		{
			switch (clockstr[j])
			{
			case ':':
				temp *= 60;
				temp += std::strtoul(&clockstr[i], NULL, 10);
				i = j + 1;
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				break;
			default:
				return -1;
			}
		}
		if (i < j)
		{
			temp *= 60;
			temp += std::strtoul(&clockstr[i], NULL, 10);
		}

		*totsecs = temp;
		return 0;
	}


	extern char * escape_fs_chars(char const * in, unsigned int len)
	{
		char *       out;
		unsigned int inpos;
		unsigned int outpos;

		if (!in)
			return NULL;
		out = (char*)xmalloc(len * 3 + 1); /* if all turn into %XX */

		for (inpos = 0, outpos = 0; inpos < len; inpos++)
		{
			if (in[inpos] == '\0' || in[inpos] == '%' || in[inpos] == '/' ||
				in[inpos] == '\\' || in[inpos] == ':') /* FIXME: what other characters does Windows not allow? */
			{
				out[outpos++] = '%';
				/* always 00 through FF hex */
				std::sprintf(&out[outpos], "%02X", (unsigned int)(unsigned char)in[inpos]);
				outpos += 2;
			}
			else
				out[outpos++] = in[inpos];
		}
		/*  if outpos >= len*3+1 then the buffer was overflowed */
		out[outpos] = '\0';

		return out;
	}


	extern char * escape_chars(char const * in, unsigned int len)
	{
		char *       out;
		unsigned int inpos;
		unsigned int outpos;

		if (!in)
			return NULL;
		out = (char*)xmalloc(len * 4 + 1); /* if all turn into \xxx */

		for (inpos = 0, outpos = 0; inpos < len; inpos++)
		{
			if (in[inpos] == '\\')
			{
				out[outpos++] = '\\';
				out[outpos++] = '\\';
			}
			else if (in[inpos] == '"')
			{
				out[outpos++] = '\\';
				out[outpos++] = '"';
			}
			else if (std::isprint((int)in[inpos]))
				out[outpos++] = in[inpos];
			else if (in[inpos] == '\a')
			{
				out[outpos++] = '\\';
				out[outpos++] = 'a';
			}
			else if (in[inpos] == '\b')
			{
				out[outpos++] = '\\';
				out[outpos++] = 'b';
			}
			else if (in[inpos] == '\t')
			{
				out[outpos++] = '\\';
				out[outpos++] = 't';
			}
			else if (in[inpos] == '\n')
			{
				out[outpos++] = '\\';
				out[outpos++] = 'n';
			}
			else if (in[inpos] == '\v')
			{
				out[outpos++] = '\\';
				out[outpos++] = 'v';
			}
			else if (in[inpos] == '\f')
			{
				out[outpos++] = '\\';
				out[outpos++] = 'f';
			}
			else if (in[inpos] == '\r')
			{
				out[outpos++] = '\\';
				out[outpos++] = 'r';
			}
			else
			{
				out[outpos++] = '\\';
				/* always 001 through 377 octal */
				std::sprintf(&out[outpos], "%03o", (unsigned int)(unsigned char)in[inpos]);
				outpos += 3;
			}
		}
		/*  if outpos >= len*4+1 then the buffer was overflowed */
		out[outpos] = '\0';

		return out;
	}


	extern char * unescape_chars(char const * in)
	{
		char *       out;
		unsigned int inpos;
		unsigned int outpos;
		unsigned int inlen;

		if (!in)
			return NULL;

		inlen = std::strlen(in);
		out = (char*)xmalloc(inlen + 1);

		for (inpos = 0, outpos = 0; inpos < inlen; inpos++)
		{
			if (in[inpos] != '\\')
				out[outpos++] = in[inpos];
			else
				switch (in[++inpos])
			{
				case '\\':
					out[outpos++] = '\\';
					break;
				case '"':
					out[outpos++] = '"';
					break;
				case 'a':
					out[outpos++] = '\a';
					break;
				case 'b':
					out[outpos++] = '\b';
					break;
				case 't':
					out[outpos++] = '\t';
					break;
				case 'n':
					out[outpos++] = '\n';
					break;
				case 'v':
					out[outpos++] = '\v';
					break;
				case 'f':
					out[outpos++] = '\f';
					break;
				case 'r':
					out[outpos++] = '\r';
					break;
				default:
				{
						   char         temp[4];
						   unsigned int i;
						   unsigned int num;

						   for (i = 0; i < 3; i++)
						   {
							   if (in[inpos] != '0' &&
								   in[inpos] != '1' &&
								   in[inpos] != '2' &&
								   in[inpos] != '3' &&
								   in[inpos] != '4' &&
								   in[inpos] != '5' &&
								   in[inpos] != '6' &&
								   in[inpos] != '7')
								   break;
							   temp[i] = in[inpos++];
						   }
						   temp[i] = '\0';
						   inpos--;

						   num = std::strtoul(temp, NULL, 8);
						   if (i < 3 || num<1 || num>255) /* bad escape (including \000), leave it as-is */
						   {
							   out[outpos++] = '\\';
							   std::strcpy(&out[outpos], temp);
							   outpos += std::strlen(temp);
						   }
						   else
							   out[outpos++] = (unsigned char)num;
				}
			}
		}
		out[outpos] = '\0';

		return out;
	}


	extern void str_to_hex(char * target, char const * data, int datalen)
	{
		unsigned char c;
		int  i;
		for (i = 0; i < datalen; i++)
		{
			c = (data[i]) & 0xff;

			std::sprintf(target + i * 3, "%02X ", c);
			target[i * 3 + 3] = '\0';

			/* std::fprintf(stderr, "str_to_hex %d | '%02x' '%s'\n", i, c, target); */
		}
	}


	extern int hex_to_str(char const * source, char * data, int datalen)
	{
		/*
		 * TODO: We really need a more robust function here,
		 *       for now, I'll just use this hack for a quick evaluation
		 */
		char byte;
		char c;
		int  i;

		for (i = 0; i < datalen; i++)
		{
			byte = 0;

			/* std::fprintf(stderr, "hex_to_str %d | '%02x'", i, byte); */

			c = source[i * 3 + 0];
			byte += 16 * (c > '9' ? (c - 'A' + 10) : (c - '0'));

			/* std::fprintf(stderr, " | '%c' '%02x'", c, byte); */

			c = source[i * 3 + 1];
			byte += 1 * (c > '9' ? (c - 'A' + 10) : (c - '0'));

			/* std::fprintf(stderr, " | '%c' '%02x'", c, byte); */

			/* std::fprintf(stderr, "\n"); */

			data[i] = byte;
		}

		/* std::fprintf(stderr, "finished, returning %d from '%s'\n", i, source);  */

		return i;
	}

	extern char * buildpath(char const *root, const char *suffix)
	{
		char *result;

		result = (char*)xmalloc(std::strlen(root) + 1 + std::strlen(suffix) + 1);

		std::strcpy(result, root); std::strcat(result, "/"); std::strcat(result, suffix);
		return result;
	}

	/* convert a time string to time_t
	time string format is:
	yyyy/mm/dd or yyyy-mm-dd or yyyy.mm.dd
	hh:mm:ss
	*/
	extern int timestr_to_time(char const * timestr, std::time_t* ptime)
	{
		char const * p;
		char ch;
		struct std::tm when;
		int day_s, time_s, last;

		if (!timestr) return -1;
		if (!timestr[0]) {
			*ptime = 0;
			return 0;
		}

		p = timestr;
		day_s = time_s = 0;
		last = 0;
		std::memset(&when, 0, sizeof(when));
		when.tm_mday = 1;
		when.tm_isdst = -1;
		while (1) {
			ch = *timestr;
			timestr++;
			switch (ch) {
			case '/':
			case '-':
			case '.':
				if (day_s == 0) {
					when.tm_year = std::atoi(p) - 1900;
				}
				else if (day_s == 1) {
					when.tm_mon = std::atoi(p) - 1;
				}
				else if (day_s == 2) {
					when.tm_mday = std::atoi(p);
				}
				time_s = 0;
				day_s++;
				p = timestr;
				last = 1;
				break;
			case ':':
				if (time_s == 0) {
					when.tm_hour = std::atoi(p);
				}
				else if (time_s == 1) {
					when.tm_min = std::atoi(p);
				}
				else if (time_s == 2) {
					when.tm_sec = std::atoi(p);
				}
				day_s = 0;
				time_s++;
				p = timestr;
				last = 2;
				break;
			case ' ':
			case '\t':
			case '\x0':
				if (last == 1) {
					if (day_s == 0) {
						when.tm_year = std::atoi(p) - 1900;
					}
					else if (day_s == 1) {
						when.tm_mon = std::atoi(p) - 1;
					}
					else if (day_s == 2) {
						when.tm_mday = std::atoi(p);
					}
				}
				else if (last == 2) {
					if (time_s == 0) {
						when.tm_hour = std::atoi(p);
					}
					else if (time_s == 1) {
						when.tm_min = std::atoi(p);
					}
					else if (time_s == 2) {
						when.tm_sec = std::atoi(p);
					}
				}
				time_s = day_s = 0;
				p = timestr;
				last = 0;
				break;
			default:
				break;
			}
			if (!ch) break;
		}

		*ptime = std::mktime(&when);
		return 0;
	}

	extern void strlower(char* str)
	{
		for (; *str; str++) *str = std::tolower(*str);
	}

}
