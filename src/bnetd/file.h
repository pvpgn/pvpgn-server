/*
 * Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
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
#ifndef INCLUDED_FILE_PROTOS
#define INCLUDED_FILE_PROTOS

#define JUST_NEED_TYPES
# include "common/bn_type.h"
# include "connection.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern int file_to_mod_time(t_connection * c, char const * rawname, bn_long * modtime);
		extern int file_send(t_connection * c, char const * rawname, unsigned int adid, unsigned int etag, unsigned int startoffset, int need_header);

	}

}

#endif
#endif
