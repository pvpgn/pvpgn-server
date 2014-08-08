/*
 * Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
 * Copyright (C) 1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_LADDER_CALC_TYPES
#define INCLUDED_LADDER_CALC_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		typedef struct
		{
			double       prob;
			unsigned int k;
			int          adj;
			unsigned int oldrating;
			unsigned int oldrank;
		} t_ladder_info;

	}

}

#endif

/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_LADDER_CALC_PROTOS
#define INCLUDED_LADDER_CALC_PROTOS

#define JUST_NEED_TYPES
#include "ladder.h"
#include "account.h"
#include "game.h"
#include "common/tag.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern int ladder_calc_info(t_clienttag clienttag, t_ladder_id id, unsigned int count, t_account * * players, t_game_result * results, t_ladder_info * info);

	}

}

#endif
#endif
