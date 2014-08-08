/*
 * Copyright (C) 1998,1999,2001  Ross Combs (rocombs@cs.nmsu.edu)
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
#include "common/bn_type.h"

#include <cstring>

#include "common/eventlog.h"
#include "common/setup_after.h"


/************************************************************/

namespace pvpgn
{

	extern int bn_byte_tag_get(bn_byte const * src, char * dst, unsigned int len)
	{
		unsigned int i;

		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}
		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return -1;
		}
		if (len < 1)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got zero len");
			return -1;
		}

		for (i = 0; i < len - 1 && i < 1; i++)
			dst[i] = (char)(*src)[-i];
		dst[i] = '\0';

		return 0;
	}


	extern int bn_short_tag_get(bn_short const * src, char * dst, unsigned int len)
	{
		unsigned int i;

		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}
		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return -1;
		}
		if (len < 1)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got zero len");
			return -1;
		}

		for (i = 0; i < len - 1 && i < 2; i++)
			dst[i] = (char)(*src)[1 - i];
		dst[i] = '\0';

		return 0;
	}


	extern int bn_int_tag_get(bn_int const * src, char * dst, unsigned int len)
	{
		unsigned int i;

		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}
		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return -1;
		}
		if (len < 1)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got zero len");
			return -1;
		}

		for (i = 0; i < len - 1 && i < 4; i++)
			dst[i] = (char)(*src)[3 - i];
		dst[i] = '\0';

		return 0;
	}


	extern int bn_long_tag_get(bn_long const * src, char * dst, unsigned int len)
	{
		unsigned int i;

		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}
		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return -1;
		}
		if (len < 1)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got zero len");
			return -1;
		}

		for (i = 0; i < len - 1 && i < 8; i++)
			dst[i] = (char)(*src)[7 - i];
		dst[i] = '\0';

		return 0;
	}


	/************************************************************/


	extern int bn_byte_tag_set(bn_byte * dst, char const * tag)
	{
		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}
		if (!tag)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL tag");
			return -1;
		}

		(*dst)[0] = (unsigned char)tag[0];
		return 0;
	}


	extern int bn_short_tag_set(bn_short * dst, char const * tag)
	{
		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}
		if (!tag)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL tag");
			return -1;
		}

		(*dst)[0] = (unsigned char)tag[3];
		(*dst)[1] = (unsigned char)tag[2];
		return 0;
	}


	extern int bn_int_tag_set(bn_int * dst, char const * tag)
	{
		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}
		if (!tag)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL tag");
			return -1;
		}

		(*dst)[0] = (unsigned char)tag[3];
		(*dst)[1] = (unsigned char)tag[2];
		(*dst)[2] = (unsigned char)tag[1];
		(*dst)[3] = (unsigned char)tag[0];
		return 0;
	}


	extern int bn_long_tag_set(bn_long * dst, char const * tag)
	{
		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}
		if (!tag)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL tag");
			return -1;
		}

		(*dst)[0] = (unsigned char)tag[7];
		(*dst)[1] = (unsigned char)tag[6];
		(*dst)[2] = (unsigned char)tag[5];
		(*dst)[3] = (unsigned char)tag[4];
		(*dst)[4] = (unsigned char)tag[3];
		(*dst)[5] = (unsigned char)tag[2];
		(*dst)[6] = (unsigned char)tag[1];
		(*dst)[7] = (unsigned char)tag[0];
		return 0;
	}


	/************************************************************/


	extern t_uint8 bn_byte_get(bn_byte const src)
	{
		t_uint8 temp;

		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return 0;
		}

		temp = ((t_uint8)src[0]);
		return temp;
	}


	extern t_uint16 bn_short_get(bn_short const src)
	{
		t_uint16 temp;

		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return 0;
		}

		temp = ((t_uint16)src[0]);
		temp |= ((t_uint16)src[1]) << 8;
		return temp;
	}


	extern t_uint16 bn_short_nget(bn_short const src)
	{
		t_uint16 temp;

		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return 0;
		}

		temp = ((t_uint16)src[1]);
		temp |= ((t_uint16)src[0]) << 8;
		return temp;
	}


	extern t_uint32 bn_int_get(bn_int const src)
	{
		t_uint32 temp;

		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return 0;
		}

		temp = ((t_uint32)src[0]);
		temp |= ((t_uint32)src[1]) << 8;
		temp |= ((t_uint32)src[2]) << 16;
		temp |= ((t_uint32)src[3]) << 24;
		return temp;
	}


	extern t_uint32 bn_int_nget(bn_int const src)
	{
		t_uint32 temp;

		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return 0;
		}

		temp = ((t_uint32)src[3]);
		temp |= ((t_uint32)src[2]) << 8;
		temp |= ((t_uint32)src[1]) << 16;
		temp |= ((t_uint32)src[0]) << 24;
		return temp;
	}


#ifdef HAVE_T_LONG
	extern t_uint64 bn_long_get(bn_long const src)
	{
		t_uint64 temp;

		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return 0;
		}

		temp = ((t_uint64)src[0]);
		temp |= ((t_uint64)src[1]) << 8;
		temp |= ((t_uint64)src[2]) << 16;
		temp |= ((t_uint64)src[3]) << 24;
		temp |= ((t_uint64)src[4]) << 32;
		temp |= ((t_uint64)src[5]) << 40;
		temp |= ((t_uint64)src[6]) << 48;
		temp |= ((t_uint64)src[7]) << 56;
		return temp;
	}
#endif


	extern t_uint32 bn_long_get_a(bn_long const src)
	{
		t_uint32 temp;

		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return 0;
		}

		temp = ((t_uint32)src[4]);
		temp |= ((t_uint32)src[5]) << 8;
		temp |= ((t_uint32)src[6]) << 16;
		temp |= ((t_uint32)src[7]) << 24;
		return temp;
	}


	extern t_uint32 bn_long_get_b(bn_long const src)
	{
		t_uint32 temp;

		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return 0;
		}

		temp = ((t_uint32)src[0]);
		temp |= ((t_uint32)src[1]) << 8;
		temp |= ((t_uint32)src[2]) << 16;
		temp |= ((t_uint32)src[3]) << 24;
		return temp;
	}


	/************************************************************/


	extern int bn_byte_set(bn_byte * dst, t_uint8 src)
	{
		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}

		(*dst)[0] = (unsigned char)((src));
		return 0;
	}


	extern int bn_short_set(bn_short * dst, t_uint16 src)
	{
		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}

		(*dst)[0] = (unsigned char)((src)& 0xff);
		(*dst)[1] = (unsigned char)((src >> 8));
		return 0;
	}


	extern int bn_short_nset(bn_short * dst, t_uint16 src)
	{
		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}

		(*dst)[0] = (unsigned char)((src >> 8));
		(*dst)[1] = (unsigned char)((src)& 0xff);
		return 0;
	}


	extern int bn_int_set(bn_int * dst, t_uint32 src)
	{
		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}

		(*dst)[0] = (unsigned char)((src)& 0xff);
		(*dst)[1] = (unsigned char)((src >> 8) & 0xff);
		(*dst)[2] = (unsigned char)((src >> 16) & 0xff);
		(*dst)[3] = (unsigned char)((src >> 24));
		return 0;
	}


	extern int bn_int_nset(bn_int * dst, t_uint32 src)
	{
		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}

		(*dst)[0] = (unsigned char)((src >> 24));
		(*dst)[1] = (unsigned char)((src >> 16) & 0xff);
		(*dst)[2] = (unsigned char)((src >> 8) & 0xff);
		(*dst)[3] = (unsigned char)((src)& 0xff);
		return 0;
	}


#ifdef HAVE_T_LONG
	extern int bn_long_set(bn_long * dst, t_uint64 src)
	{
		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}

		(*dst)[0] = (unsigned char)((src)& 0xff);
		(*dst)[1] = (unsigned char)((src >> 8) & 0xff);
		(*dst)[2] = (unsigned char)((src >> 16) & 0xff);
		(*dst)[3] = (unsigned char)((src >> 24) & 0xff);
		(*dst)[4] = (unsigned char)((src >> 32) & 0xff);
		(*dst)[5] = (unsigned char)((src >> 40) & 0xff);
		(*dst)[6] = (unsigned char)((src >> 48) & 0xff);
		(*dst)[7] = (unsigned char)((src >> 56));
		return 0;
	}


	extern int bn_long_nset(bn_long * dst, t_uint64 src)
	{
		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}

		(*dst)[0] = (unsigned char)((src >> 56));
		(*dst)[1] = (unsigned char)((src >> 48) & 0xff);
		(*dst)[2] = (unsigned char)((src >> 40) & 0xff);
		(*dst)[3] = (unsigned char)((src >> 32) & 0xff);
		(*dst)[4] = (unsigned char)((src >> 24) & 0xff);
		(*dst)[5] = (unsigned char)((src >> 16) & 0xff);
		(*dst)[6] = (unsigned char)((src >> 8) & 0xff);
		(*dst)[7] = (unsigned char)((src)& 0xff);
		return 0;
	}
#endif


	extern int bn_long_set_a_b(bn_long * dst, t_uint32 srca, t_uint32 srcb)
	{
		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}

		(*dst)[0] = (unsigned char)((srcb)& 0xff);
		(*dst)[1] = (unsigned char)((srcb >> 8) & 0xff);
		(*dst)[2] = (unsigned char)((srcb >> 16) & 0xff);
		(*dst)[3] = (unsigned char)((srcb >> 24) & 0xff);
		(*dst)[4] = (unsigned char)((srca)& 0xff);
		(*dst)[5] = (unsigned char)((srca >> 8) & 0xff);
		(*dst)[6] = (unsigned char)((srca >> 16) & 0xff);
		(*dst)[7] = (unsigned char)((srca >> 24));
		return 0;
	}


	extern int bn_long_nset_a_b(bn_long * dst, t_uint32 srca, t_uint32 srcb)
	{
		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}

		(*dst)[0] = (unsigned char)((srca >> 24));
		(*dst)[1] = (unsigned char)((srca >> 16) & 0xff);
		(*dst)[2] = (unsigned char)((srca >> 8) & 0xff);
		(*dst)[3] = (unsigned char)((srca)& 0xff);
		(*dst)[4] = (unsigned char)((srcb >> 24) & 0xff);
		(*dst)[5] = (unsigned char)((srcb >> 16) & 0xff);
		(*dst)[6] = (unsigned char)((srcb >> 8) & 0xff);
		(*dst)[7] = (unsigned char)((srcb)& 0xff);
		return 0;
	}


	/************************************************************/


	extern int bn_raw_set(void * dst, void const * src, unsigned int len)
	{
		if (!dst)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst");
			return -1;
		}
		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return -1;
		}

		std::memcpy(dst, src, len);
		return 0;
	}


	/************************************************************/


	extern int bn_byte_tag_eq(bn_byte const src, char const * tag)
	{
		bn_byte temp;

		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return -1;
		}
		if (!tag)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL tag");
			return -1;
		}

		if (bn_byte_tag_set(&temp, tag) < 0)
			return -1;
		if (bn_byte_get(src) == bn_byte_get(temp))
			return 0;

		return -1;
	}


	extern int bn_short_tag_eq(bn_short const src, char const * tag)
	{
		bn_short temp;

		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return -1;
		}
		if (!tag)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL tag");
			return -1;
		}

		if (bn_short_tag_set(&temp, tag) < 0)
			return -1;
		if (bn_short_get(src) == bn_short_get(temp))
			return 0;

		return -1;
	}


	extern int bn_int_tag_eq(bn_int const src, char const * tag)
	{
		bn_int temp;

		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return -1;
		}
		if (!tag)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL tag");
			return -1;
		}

		if (bn_int_tag_set(&temp, tag) < 0)
			return -1;
		if (bn_int_get(src) == bn_int_get(temp))
			return 0;

		return -1;
	}


	extern int bn_long_tag_eq(bn_long const src, char const * tag)
	{
		bn_long temp;

		if (!src)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL src");
			return -1;
		}
		if (!tag)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL tag");
			return -1;
		}

		if (bn_long_tag_set(&temp, tag) < 0)
			return -1;
		if (bn_long_get_a(src) == bn_long_get_a(temp) &&
			bn_long_get_b(src) == bn_long_get_b(temp))
			return 0;

		return -1;
	}


	/************************************************************/


	extern int uint32_to_int(t_uint32 num)
	{
		if (num < (1UL << 30))
			return (int)num;
		return (-(int)((~(num)) + 1));
	}

}
