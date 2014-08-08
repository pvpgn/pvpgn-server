/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
 * Copyright (C) 2001		sousou	(liupeng.cs@263.net)
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
#ifndef INCLUDED_D2LADDER_H
#define INCLUDED_D2LADDER_H

#include "common/bn_type.h"
#include "common/d2cs_protocol.h"
#include "common/d2cs_d2dbs_ladder.h"

namespace pvpgn
{

namespace d2cs
{

typedef struct
{
	unsigned int			type;
	unsigned int			len;
	unsigned int			curr_len;
	t_d2cs_client_ladderinfo	* info;
} t_d2ladder;

extern int d2ladder_init(void);
extern int d2ladder_refresh(void);
extern int d2ladder_destroy(void);
extern int d2ladder_get_ladder(unsigned int * from, unsigned int * count, unsigned int type,
				t_d2cs_client_ladderinfo const * * info);
extern int d2ladder_find_character_pos(unsigned int type, char const * charname);

}

}

#endif
