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
#ifndef INCLUDED_D2CS_D2DBS_LADDER_H
#define INCLUDED_D2CS_D2DBS_LADDER_H

#include "common/bn_type.h"
#include "common/field_sizes.h"

namespace pvpgn
{

	typedef struct
	{
		bn_int		type;
		bn_int		offset;
		bn_int		number;
	} t_d2ladderfile_ladderindex;

	typedef struct
	{
		bn_int		experience;
		bn_short	status;
		bn_byte		level;
		bn_byte		chclass;
		char		charname[MAX_CHARNAME_LEN];
	} t_d2ladderfile_ladderinfo;

	typedef struct
	{
		bn_int		maxtype;
		bn_int		checksum;
	} t_d2ladderfile_header;

}

#define	LADDER_FILE_PREFIX	"ladder"

#define D2LADDER_HC_OVERALL		0x00
#define D2LADDER_STD_OVERALL		0x09
#define D2LADDER_EXP_HC_OVERALL		0x13
#define D2LADDER_EXP_STD_OVERALL	0x1B
const unsigned D2CHAR_CLASS_MAX = 0x04;
const unsigned D2CHAR_EXP_CLASS_MAX = 0x06;

#endif
