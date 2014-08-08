/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
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
#ifndef INCLUDED_GAMEQUEUE_H
#define INCLUDED_GAMEQUEUE_H

#include "common/packet.h"
#include "common/list.h"

namespace pvpgn
{

namespace d2cs
{

typedef struct
{
	unsigned int	seqno;
	unsigned int	clientid;
	t_packet	* packet;
	char		gamename[MAX_GAMENAME_LEN];
} t_gq;

extern unsigned int gq_get_clientid(t_gq const * gq);
extern int gq_destroy(t_gq * gq, t_elem ** elem);
extern t_gq * gq_create(unsigned int clientid, t_packet * packet, char const * gamename);
extern int gqlist_destroy(void);
extern int gqlist_create(void);
extern t_list * gqlist(void);
extern unsigned int gqlist_get_gq_position(t_gq * gq);
extern int gqlist_update_all_clients(void);
extern int gqlist_check_creategame(int number);
extern t_gq * gqlist_find_game(char const * gamename);
extern unsigned int gqlist_get_length(void);

}

}

#endif
