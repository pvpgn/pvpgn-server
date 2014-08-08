/*
 * Copyright (C) 2000  Dizzy
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
#ifndef INCLUDED_QUOTA_TYPES
#define INCLUDED_QUOTA_TYPES

#include <ctime>

#ifdef JUST_NEED_TYPES
# include "common/list.h"
#else
# define JUST_NEED_TYPES
# include "common/list.h"
# undef JUST_NEED_TYPES
#endif

namespace pvpgn
{

	namespace bnetd
	{

		typedef struct
		{
			std::time_t	 inf;
			unsigned int count;
		} t_qline;

		typedef struct
		{
			unsigned int totcount;
			t_list *     list;
		} t_quota;

	}

}

#endif
