/*
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
#ifndef INCLUDED_QUEUE_TYPES
#define INCLUDED_QUEUE_TYPES

#ifdef QUEUE_INTERNAL_ACCESS
#ifdef JUST_NEED_TYPES
# include "common/packet.h"
#else
# define JUST_NEED_TYPES
# include "common/packet.h"
# undef JUST_NEED_TYPES
#endif
#endif

namespace pvpgn
{

	typedef struct queue
#ifdef QUEUE_INTERNAL_ACCESS
	{
		unsigned ulen, alen;
		t_packet ** ring;
		unsigned head, tail;
	}
#endif
	t_queue;

}

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_QUEUE_PROTOS
#define INCLUDED_QUEUE_PROTOS

#define JUST_NEED_TYPES
#include "common/packet.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	extern t_packet * queue_pull_packet(t_queue * * queue);
	extern t_packet * queue_peek_packet(t_queue const * const * queue);
	extern void queue_push_packet(t_queue * * queue, t_packet * packet);
	extern int queue_get_length(t_queue const * const * queue);
	extern void queue_clear(t_queue * * queue);

}
#endif
#endif
