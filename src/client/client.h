/*
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
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_CLIENT_PROTOS
#define INCLUDED_CLIENT_PROTOS

#include "common/packet.h"

namespace pvpgn
{

	namespace client
	{

		extern int client_blocksend_packet(int sock, t_packet const * packet);
		extern int client_blockrecv_packet(int sock, t_packet * packet);
		extern int client_blockrecv_raw_packet(int sock, t_packet * packet, unsigned int len);
		extern int client_get_termsize(int fd, unsigned int * w, unsigned int * h);
		extern int client_get_comm(char const * prompt, char * text, unsigned int maxlen, unsigned int * curpos, int visible, int redraw, unsigned int width);

	}

}
#endif
#endif
