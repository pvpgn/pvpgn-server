/*
 * Copyright (C) 2004      ls_sons  (ls@gamelife.org)
 * Copyright (C) 2004      Olaf Freyer (aaron@cs.tu-berlin.de)
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

#include "common/elist.h"
#include "common/d2cs_d2gs_character.h"

namespace pvpgn
{

namespace d2cs
{

typedef struct  	d2charlist{
    t_d2charinfo_file	*charinfo;
    int			expiration_time;
    t_elist		list;
} t_d2charlist;

extern int d2charlist_add_char(t_elist *, t_d2charinfo_file *i, unsigned int);

}

}
