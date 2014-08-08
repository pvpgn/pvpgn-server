/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
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
#include "xstring.h"

#include <cstdio>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

#include "compat/strdup.h"
#include "common/xalloc.h"
#include "common/setup_after.h"

namespace pvpgn
{

	extern char * strtolower(char * str)
	{
		unsigned int	i;
		unsigned char	ch;

		if (!str) return NULL;
		for (i = 0; (ch = str[i]); i++) {
			if ((std::isupper(ch))) str[i] = ch + ('a' - 'A');
		}
		return str;
	}

	extern unsigned char xtoi(unsigned char ch)
	{
		unsigned char retval;

		if (std::isalpha(ch)) retval = std::tolower(ch);
		else retval = ch;
		if (retval < 'A') retval -= ('0' - 0);
		else retval -= ('a' - 0xa);
		return retval;
	}

	extern char * str_strip_affix(char * str, char const * affix)
	{
		unsigned int i, j, n;
		int		match;

		if (!str) return NULL;
		if (!affix) return str;
		for (i = 0; str[i]; i++) {
			match = 0;
			for (n = 0; affix[n]; n++) {
				if (str[i] == affix[n]) {
					match = 1;
					break;
				}
			}
			if (!match) break;
		}
		for (j = std::strlen(str) - 1; j >= i; j--) {
			match = 0;
			for (n = 0; affix[n]; n++) {
				if (str[j] == affix[n]) {
					match = 1;
					break;
				}
			}
			if (!match) break;
		}
		if (i > j) {
			str[0] = '\0';
		}
		else {
			std::memmove(str, str + i, j - i + 1);
			str[j - i + 1] = '\0';
		}
		return str;
	}

	extern char * hexstrdup(unsigned char const * src)
	{
		char	* dest;
		int	len;

		if (!src) return NULL;
		dest = xstrdup((const char*)src);
		len = hexstrtoraw(src, dest, std::strlen(dest) + 1);
		dest[len] = '\0';
		return dest;
	}

	extern unsigned int hexstrtoraw(unsigned char const * src, char * data, unsigned int datalen)
	{
		unsigned char	ch;
		unsigned int	i, j;

		for (i = 0, j = 0; j < datalen; i++) {
			ch = src[i];
			if (!ch) break;
			if (ch == '\\') {
				i++;
				ch = src[i];
				if (!ch) {
					break;
				}
				else if (ch == '\\') {
					data[j++] = ch;
				}
				else if (ch == 'x') {
					if (std::isxdigit(src[i + 1])) {
						if (std::isxdigit(src[i + 2])) {
							data[j++] = xtoi(src[i + 1]) * 0x10 + xtoi(src[i + 2]);
							i += 2;
						}
						else {
							data[j++] = xtoi(src[i + 1]);
							i++;
						}
					}
					else {
						data[j++] = ch;
					}
				}
				else if (ch == 'n') {
					data[j++] = '\n';
				}
				else if (ch == 'r') {
					data[j++] = '\r';
				}
				else if (ch == 'a') {
					data[j++] = '\a';
				}
				else if (ch == 't') {
					data[j++] = '\t';
				}
				else if (ch == 'b') {
					data[j++] = '\b';
				}
				else if (ch == 'f') {
					data[j++] = '\f';
				}
				else if (ch == 'v') {
					data[j++] = '\v';
				}
				else {
					data[j++] = ch;
				}
			}
			else {
				data[j++] = ch;
				continue;
			}
		}
		return j;
	}

#define SPLIT_STRING_INIT_COUNT		32
#define	SPLIT_STRING_INCREASEMENT	32
	extern char * * strtoargv(char const * str, unsigned int * count)
	{
		unsigned int	n, index_size;
		char		* temp;
		unsigned int	i;
		int		j;
		int		* pindex;
		void		** ptrindex;
		char		* result;

		if (!str || !count) return NULL;
		temp = (char*)xmalloc(std::strlen(str) + 1);
		n = SPLIT_STRING_INIT_COUNT;
		pindex = (int*)xmalloc(n * sizeof (int));

		i = j = 0;
		*count = 0;
		while (str[i]) {
			while (str[i] == ' ' || str[i] == '\t') i++;
			if (!str[i]) break;
			if (*count >= n) {
				n += SPLIT_STRING_INCREASEMENT;
				pindex = (int *)xrealloc(pindex, n * sizeof(int));
			}
			pindex[*count] = j;
			(*count)++;
			if (str[i] == '"') {
				i++;
				while (str[i]) {
					if (str[i] == '\\') {
						i++;
						if (!str[i]) break;
					}
					else if (str[i] == '"') {
						i++;
						break;
					}
					temp[j++] = str[i++];
				}
			}
			else {
				while (str[i] && str[i] != ' ' && str[i] != '\t') {
					temp[j++] = str[i++];
				}
			}
			temp[j++] = '\0';
		}
		index_size = *count * sizeof(char *);
		if (!index_size) {
			xfree(temp);
			xfree(pindex);
			return NULL;
		}
		result = (char*)xmalloc(j + index_size);
		std::memcpy(result + index_size, temp, j);

		ptrindex = (void**)xmalloc(*count * sizeof (char*));
		for (i = 0; i < *count; i++) {
			ptrindex[i] = result + index_size + pindex[i];
		}
		std::memcpy(result, ptrindex, index_size);
		xfree(temp);
		xfree(pindex);
		xfree(ptrindex);
		return (char * *)result;
	}

#define COMBINE_STRING_INIT_LEN		1024
#define COMBINE_STRING_INCREASEMENT	1024
	extern char * arraytostr(char * * array, char const * delim, int count)
	{
		int	i;
		unsigned int n;
		char	* result;
		int	need_delim;

		if (!delim || !array) return NULL;

		n = COMBINE_STRING_INIT_LEN;
		result = (char*)xmalloc(n);
		result[0] = '\0';

		need_delim = 0;
		for (i = 0; i < count; i++) {
			if (!array[i]) continue;
			if (std::strlen(result) + std::strlen(array[i]) + std::strlen(delim) >= n) {
				n += COMBINE_STRING_INCREASEMENT;
				result = (char*)xrealloc(result, n);
			}
			if (need_delim) {
				std::strcat(result, delim);
			}
			std::strcat(result, array[i]);
			need_delim = 1;
		}
		result = (char*)xrealloc(result, std::strlen(result) + 1);
		return result;
	}


	// You must free the result if result is non-NULL.
	extern char *str_replace(char *orig, char *rep, char *with)
	{
		char *result; // the return string
		char *ins;    // the next insert point
		char *tmp;    // varies
		int len_rep;  // length of rep
		int len_with; // length of with
		int len_front; // distance between rep and end of last rep
		int count;    // number of replacements

		if (!orig)
			return NULL;
		if (!rep)
			rep = "";
		len_rep = strlen(rep);
		if (!with)
			with = "";
		len_with = strlen(with);

		ins = orig;
		for (count = 0; tmp = strstr(ins, rep); ++count) {
			ins = tmp + len_rep;
		}

		// first time through the loop, all the variable are set correctly
		// from here on,
		//    tmp points to the end of the result string
		//    ins points to the next occurrence of rep in orig
		//    orig points to the remainder of orig after "end of rep"
		tmp = result = (char*)malloc(strlen(orig) + (len_with - len_rep) * count + 1);

		if (!result)
			return NULL;

		while (count--) {
			ins = strstr(orig, rep);
			len_front = ins - orig;
			tmp = strncpy(tmp, orig, len_front) + len_front;
			tmp = strcpy(tmp, with) + len_with;
			orig += len_front + len_rep; // move to next "end of rep"
		}
		strcpy(tmp, orig);
		return result;
	}


	/* Replace "\n" in string to a new line character '\n' */
	extern std::string str_replace_nl(char const * text)
	{
		std::string s(text);
		size_t pos = 0;
		while ((pos = s.find("\\n", pos)) != std::string::npos) {
			s.replace(pos, 2, "\n");
		}
		return s;
	}


}
