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
#ifndef INCLUDED_SERVERQUEUE_H
#define INCLUDED_SERVERQUEUE_H

#include "common/list.h"
#include "common/packet.h"

namespace pvpgn
{

	namespace d2cs
	{

		typedef struct serverqueue
		{
			unsigned int		seqno;
			unsigned int		ctime;
			unsigned int		clientid;
			t_packet		* packet;
			unsigned int		gameid;
			unsigned int		gametoken;
		} t_sq;

		extern t_list * sqlist(void);
		extern int sqlist_create(void);
		extern int sqlist_destroy(void);

		extern int sq_destroy(t_sq * sq, t_elem ** curr);
		extern int sqlist_check_timeout(void);
		extern t_sq * sqlist_find_sq(unsigned int seqno);
		extern t_sq * sq_create(unsigned int clientid, t_packet * packet, unsigned int gameid);

		extern unsigned int sq_get_clientid(t_sq const * sq);
		extern unsigned int sq_get_gameid(t_sq const * sq);
		extern unsigned int sq_get_seqno(t_sq const * sq);
		extern int sq_set_gametoken(t_sq * sq, unsigned int gametoken);
		extern unsigned int sq_get_gametoken(t_sq const * sq);
		extern t_packet * sq_get_packet(t_sq const * sq);

	}

}

#endif
