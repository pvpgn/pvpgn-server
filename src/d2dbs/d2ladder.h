/*
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

#include <cstdio>
#include <cstddef>
#include "common/list.h"
#include "common/d2cs_d2dbs_ladder.h"
#define JUST_NEED_TYPES
#include "d2cs/d2charfile.h"
#undef JUST_NEED_TYPES

#define LADDER_BACKUP_PREFIX		"ladderbk"
#define DEFAULT_LADDER_DIR		"var/ladders"
#define LADDERFILE_CHECKSUM_OFFSET	offsetof(t_d2ladderfile_header,checksum)

namespace pvpgn
{

	namespace d2dbs
	{

		typedef struct
		{
			unsigned int	experience;
			unsigned short	status;
			unsigned char	level;
			unsigned char	chclass;
			char		charname[MAX_CHARNAME_LEN];
		} t_d2ladder_info;

		typedef struct
		{
			unsigned int			type;
			t_d2ladder_info *		info;
			unsigned int			len;
		} t_d2ladder;

		typedef t_list t_d2ladderlist;

#define D2LADDER_MAXNUM			200
#define D2LADDER_OVERALL_MAXNUM		1000
#define D2LADDER_MAXTYPE		35

		extern int d2dbs_d2ladder_init(void);
		extern int d2dbs_d2ladder_destroy(void);
		extern int d2ladder_rebuild(void);
		extern int d2ladder_update(t_d2ladder_info * pcharladderinfo);
		extern int d2ladder_print(std::FILE * ladderstrm);
		extern int d2ladder_saveladder(void);

	}

}

#endif
