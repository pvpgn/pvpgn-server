/*
 * Copyright (C) 2005 Olaf Freyer (aaron@cs.tu-berlin.de)
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

#ifndef ASNPRINTF_H
#define ASNPRINTF_H

#include <cstdarg>
#include <cstddef>

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define LARGE	64		/* use 'ABCDEF' instead of 'abcdef' */

namespace pvpgn
{

	typedef struct {
		const char* trans;	/* points to the translation of the format */
		unsigned translen;	/* how many chars in this translation */
		const char* fmt;	/* points to the fmt location of this format spec */
		unsigned fmtlen;	/* how many chars in this fmt spec */
	} t_fmtentry;

	/* "array" snprintf - this function prints all vargs into the prepared buffer,
	 * each of them as a \0 terminated string. The prepared char* [] locations will
	 * contain the starting position of each seperate string afterwards.
	 * The function returns the number of vargs that have been printed.
	 */
	int asnprintf(char * buffer, std::size_t size, t_fmtentry *entries, unsigned entlen, const char *fmt, ...);
	int vasnprintf(char * buffer, std::size_t size, t_fmtentry *entries, unsigned entlen, const char *fmt, std::va_list args);

}

#endif
