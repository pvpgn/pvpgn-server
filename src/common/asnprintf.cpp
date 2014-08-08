/*
 * Copyright (C) 2005 Olaf Freyer (aaron@cs.tu-berlin.de)
 *
 * Based upon lib/vsnprintf.c from the linux kernel sources
 *
 *  Copyright (C) 1991, 1992  Linus Torvalds
 * vsprintf.c -- Lars Wirzenius & Linus Torvalds. *
 *
 * Wirzenius wrote this portably, Torvalds fucked it up :-)
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
#include "asnprintf.h"

#include <cctype>
#include <cstring>

#include "compat/uint.h"
#include "common/setup_after.h"

namespace pvpgn
{

	static int skip_atoi(const char **s)
	{
		int i = 0;

		while (std::isdigit(**s))
			i = i * 10 + *((*s)++) - '0';
		return i;
	}

	t_uint32 do_div(t_uint64 *n, t_uint32 base)
	{
		t_uint32 remainder = *n % base;
		*n = *n / base;
		return remainder;
	}

	static char * number(char * buf, char * end, t_uint64 num, int base, int size, int precision, int type)
	{
		char c, sign, tmp[66];
		const char *digits;
		static const char small_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
		static const char large_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		int i;

		digits = (type & LARGE) ? large_digits : small_digits;
		if (type & LEFT)
			type &= ~ZEROPAD;
		if (base < 2 || base > 36)
			return NULL;
		c = (type & ZEROPAD) ? '0' : ' ';
		sign = 0;
		if (type & SIGN) {
			if ((t_int64)num < 0) {
				sign = '-';
				num = -(t_int64)num;
				size--;
			}
			else if (type & PLUS) {
				sign = '+';
				size--;
			}
			else if (type & SPACE) {
				sign = ' ';
				size--;
			}
		}
		if (type & SPECIAL) {
			if (base == 16)
				size -= 2;
			else if (base == 8)
				size--;
		}
		i = 0;
		if (num == 0)
			tmp[i++] = '0';
		else while (num != 0)
			tmp[i++] = digits[do_div(&num, base)];
		if (i > precision)
			precision = i;
		size -= precision;
		if (!(type&(ZEROPAD + LEFT))) {
			while (size-- > 0) {
				if (buf <= end)
					*buf = ' ';
				++buf;
			}
		}
		if (sign) {
			if (buf <= end)
				*buf = sign;
			++buf;
		}
		if (type & SPECIAL) {
			if (base == 8) {
				if (buf <= end)
					*buf = '0';
				++buf;
			}
			else if (base == 16) {
				if (buf <= end)
					*buf = '0';
				++buf;
				if (buf <= end)
					*buf = digits[33];
				++buf;
			}
		}
		if (!(type & LEFT)) {
			while (size-- > 0) {
				if (buf <= end)
					*buf = c;
				++buf;
			}
		}
		while (i < precision--) {
			if (buf <= end)
				*buf = '0';
			++buf;
		}
		while (i-- > 0) {
			if (buf <= end)
				*buf = tmp[i];
			++buf;
		}
		while (size-- > 0) {
			if (buf <= end)
				*buf = ' ';
			++buf;
		}
		return buf;
	}

	int vasnprintf(char *buf, std::size_t size, t_fmtentry *entries, unsigned entlen, const char *fmt, std::va_list args)
	{
		int len;
		t_uint64 num;
		int i, base;
		char *str, *end, c;
		const char *s;

		int flags;		/* flags to number() */

		int field_width;	/* width of output field */
		int precision;		/* min. # of digits for integers; max
					   number of chars for from string */
		int qualifier;		/* 'h', 'l', or 'L' for integer fields */
		/* 'z' support added 23/7/1999 S.H.    */
		/* 'z' changed to 'Z' --davidm 1/25/99 */
		unsigned fmtno = 0;


		/* Reject out-of-range values early */
		if ((int)size < 0)
			return 0;

		str = buf;
		end = buf + size - 1;
		/*
			if (end < buf - 1) {
			end = ((void *) -1);
			size = end - buf + 1;
			}
			*/
		for (; *fmt; ++fmt) {
			t_fmtentry* const fmtent = entries + fmtno;

			if (*fmt != '%')
				continue;

			if (fmtno >= entlen) return -1;
			fmtent->trans = str;
			fmtent->fmt = fmt;
			fmtno++;

			/* process flags */
			flags = 0;
		repeat:
			++fmt;		/* this also skips first '%' */
			switch (*fmt) {
			case '-': flags |= LEFT; goto repeat;
			case '+': flags |= PLUS; goto repeat;
			case ' ': flags |= SPACE; goto repeat;
			case '#': flags |= SPECIAL; goto repeat;
			case '0': flags |= ZEROPAD; goto repeat;
			}

			/* get field width */
			field_width = -1;
			if (std::isdigit(*fmt))
				field_width = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++fmt;
				/* it's the next argument */
				field_width = va_arg(args, int);
				if (field_width < 0) {
					field_width = -field_width;
					flags |= LEFT;
				}
			}

			/* get the precision */
			precision = -1;
			if (*fmt == '.') {
				++fmt;
				if (std::isdigit(*fmt))
					precision = skip_atoi(&fmt);
				else if (*fmt == '*') {
					++fmt;
					/* it's the next argument */
					precision = va_arg(args, int);
				}
				if (precision < 0)
					precision = 0;
			}

			/* get the conversion qualifier */
			qualifier = -1;
			if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' ||
				*fmt == 'Z' || *fmt == 'z') {
				qualifier = *fmt;
				++fmt;
				if (qualifier == 'l' && *fmt == 'l') {
					qualifier = 'L';
					++fmt;
				}
			}

			/* default base */
			base = 10;

			switch (*fmt) {
			case 'c':
				if (!(flags & LEFT)) {
					while (--field_width > 0) {
						if (str > end)
							return -1;
						*str = ' ';
						++str;
					}
				}
				c = (unsigned char)va_arg(args, int);
				if (str > end)
					return -1;
				*str = c;
				++str;
				while (--field_width > 0) {
					if (str > end)
						return -1;
					*str = ' ';
					++str;
				}
				fmtent->fmtlen = fmt - fmtent->fmt + 1;
				fmtent->translen = str - fmtent->trans;
				continue;

			case 's':
				s = va_arg(args, char *);
				if (!s)
					s = "<NULL>";

				len = std::strlen(s);
				if (len > precision)
					len = precision;

				if (!(flags & LEFT)) {
					while (len < field_width--) {
						if (str > end)
							return -1;
						*str = ' ';
						++str;
					}
				}
				for (i = 0; i < len; ++i) {
					if (str > end)
						return -1;
					*str = *s;
					++str; ++s;
				}
				while (len < field_width--) {
					if (str > end)
						return -1;
					*str = ' ';
					++str;
				}
				fmtent->fmtlen = fmt - fmtent->fmt + 1;
				fmtent->translen = str - fmtent->trans;
				continue;

			case 'p':
				if (field_width == -1) {
					field_width = 2 * sizeof(void *);
					flags |= ZEROPAD;
				}
				str = number(str, end,
					(unsigned long)va_arg(args, void *),
					16, field_width, precision, flags);
				if (str > end) return -1;
				fmtent->fmtlen = fmt - fmtent->fmt + 1;
				fmtent->translen = str - fmtent->trans;
				continue;


			case 'n':
				/* FIXME:
				* What does C99 say about the overflow case here? */
				if (qualifier == 'l') {
					long * ip = va_arg(args, long *);
					*ip = (str - buf);
				}
				else if (qualifier == 'Z' || qualifier == 'z') {
					std::size_t * ip = va_arg(args, std::size_t *);
					*ip = (str - buf);
				}
				else {
					int * ip = va_arg(args, int *);
					*ip = (str - buf);
				}
				fmtent->fmtlen = fmt - fmtent->fmt + 1;
				fmtent->translen = str - fmtent->trans;
				continue;

			case '%':
				if (str > end)
					return -1;
				*str = '%';
				++str;
				fmtent->fmtlen = fmt - fmtent->fmt + 1;
				fmtent->translen = str - fmtent->trans;
				continue;

				/* integer number formats - set up the flags and "break" */
			case 'o':
				base = 8;
				break;

			case 'X':
				flags |= LARGE;
			case 'x':
				base = 16;
				break;

			case 'd':
			case 'i':
				flags |= SIGN;
			case 'u':
				break;

			default:
				if (str > end)
					return -1;
				*str = '%';
				++str;
				if (*fmt) {
					if (str > end)
						return -1;
					*str = *fmt;
					++str;
				}
				else {
					--fmt;
				}
				fmtent->fmtlen = fmt - fmtent->fmt + 1;
				fmtent->translen = str - fmtent->trans;
				continue;
			}
			/*
					if (qualifier == 'L')
					num = va_arg(args, long long);
					else*/ if (qualifier == 'l') {
						num = va_arg(args, unsigned long);
						if (flags & SIGN)
							num = (signed long)num;
					}
					else if (qualifier == 'Z' || qualifier == 'z') {
						num = va_arg(args, std::size_t);
					}
					else if (qualifier == 'h') {
						num = (unsigned short)va_arg(args, int);
						if (flags & SIGN)
							num = (signed short)num;
					}
					else {
						num = va_arg(args, unsigned int);
						if (flags & SIGN)
							num = (signed int)num;
					}
					str = number(str, end, num, base,
						field_width, precision, flags);
					if (str > end) return -1;
					fmtent->fmtlen = fmt - fmtent->fmt + 1;
					fmtent->translen = str - fmtent->trans;
		}
		if (str <= end)
			*str = '\0';
		/* the trailing null byte doesn't count towards the total
		* ++str;
		*/
		return str - buf;
	}

	int asnprintf(char * buf, std::size_t size, t_fmtentry *entries, unsigned entlen, const char *fmt, ...)
	{
		std::va_list args;
		int i;

		va_start(args, fmt);
		i = vasnprintf(buf, size, entries, entlen, fmt, args);
		va_end(args);
		return i;
	}

}
