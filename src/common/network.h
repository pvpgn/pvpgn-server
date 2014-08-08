/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_NETWORK_PROTOS
#define INCLUDED_NETWORK_PROTOS

#define JUST_NEED_TYPES
#include "common/packet.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	extern int net_recv(int sock, void *buff, int len);
	extern int net_send(int sock, const void *buff, int len);
	extern int net_recv_packet(int sock, t_packet * packet, unsigned int * currsize);
	extern int net_send_packet(int sock, t_packet const * packet, unsigned int * currsize);

}

#endif
#endif
