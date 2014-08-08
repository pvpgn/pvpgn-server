/*
 * Copyright (C) 1998,1999,2001  Ross Combs (rocombs@cs.nmsu.edu)
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
#include "common/setup_before.h"
#define QUEUE_INTERNAL_ACCESS
#include "common/queue.h"

#include <cstring>

#include "common/packet.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/setup_after.h"

#define QUEUE_QUANTUM	10 /* allocate ring buffer slots for 10 packets at once */

namespace pvpgn
{

	extern t_packet * queue_pull_packet(t_queue * * queue)
	{
		t_queue *  temp;
		t_packet * packet;

		//    eventlog(eventlog_level_debug, __FUNCTION__, "entered: queue %p", queue);
		if (!queue)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL queue pointer");
			return NULL;
		}

		temp = *queue;

		if (!temp || !temp->ulen)
			return NULL;

		//    eventlog(eventlog_level_debug, __FUNCTION__, "getting element from tail (%d/%d head/tail %d/%d)", temp->alen, temp->ulen, temp->head, temp->tail);
		/* getting entry from tail and updating queue */
		packet = temp->ring[temp->tail];
		temp->tail = (temp->tail + 1) % temp->alen;
		temp->ulen--;
		//    eventlog(eventlog_level_debug, __FUNCTION__, "read %p element from tail (%d/%d head/tail %d/%d)", packet, temp->alen, temp->ulen, temp->head, temp->tail);

		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "NULL packet in queue");
			return NULL;
		}

		return packet;
	}


	extern t_packet * queue_peek_packet(t_queue const * const * queue)
	{
		t_packet * packet;

		//    eventlog(eventlog_level_debug, __FUNCTION__, "entered: queue %p", queue);
		if (!queue)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL queue pointer");
			return NULL;
		}
		if (!*queue || !(*queue)->ulen)
			return NULL;

		packet = (*queue)->ring[(*queue)->tail];

		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "NULL packet in queue");
			return NULL;
		}

		return packet;
	}


	extern void queue_push_packet(t_queue * * queue, t_packet * packet)
	{
		t_queue * temp;
		void *ptr;

		//    eventlog(eventlog_level_debug, __FUNCTION__, "entered: queue %p packet %p", queue, packet);
		if (!queue)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL queue pointer");
			return;
		}
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return;
		}

		temp = *queue;

		if (!temp)
		{
			//	eventlog(eventlog_level_debug, __FUNCTION__, "queue is NULL , initilizing");
			temp = (t_queue*)xmalloc(sizeof(t_queue));
			temp->alen = temp->ulen = 0;
			temp->ring = NULL;
			temp->head = temp->tail = 0;
			*queue = temp;
		}

		if (temp->ulen == temp->alen) { /* ring queue is full, need to allocate some memory */
			/* FIXME: find a solution
				if (temp->alen)
				eventlog(eventlog_level_error, __FUNCTION__, "queue is full (resizing) (oldsize: %u)", temp->alen);
				*/

			ptr = xrealloc(temp->ring, sizeof(t_packet *)* (temp->alen + QUEUE_QUANTUM));
			temp->ring = (t_packet **)ptr;
			temp->alen += QUEUE_QUANTUM;

			//	eventlog(eventlog_level_debug, __FUNCTION__, "queue new size %d/%d head/tail %d/%d", temp->alen, temp->ulen, temp->head, temp->tail);
			if (temp->head) {
				unsigned moved;

				moved = (QUEUE_QUANTUM <= temp->head) ? QUEUE_QUANTUM : temp->head;
				std::memmove(temp->ring + temp->ulen, temp->ring, sizeof(t_packet *)* moved);
				if (temp->head > QUEUE_QUANTUM) {
					std::memmove(temp->ring, temp->ring + moved, sizeof(t_packet *)* (temp->head - moved));
					temp->head -= moved;
				}
				else if (temp->head < QUEUE_QUANTUM)
					temp->head = temp->ulen + moved;
				else temp->head = 0;
			}
			else temp->head = temp->ulen;

		}

		temp->ring[temp->head] = packet_add_ref(packet);

		temp->head = (temp->head + 1) % temp->alen;
		temp->ulen++;
		//    eventlog(eventlog_level_debug, __FUNCTION__, "packet added (%d/%d head/tail %d/%d)", temp->alen, temp->ulen, temp->head, temp->tail);
	}


	extern int queue_get_length(t_queue const * const * queue)
	{
		//    eventlog(eventlog_level_debug, __FUNCTION__, "entered: queue %p", queue);
		if (!queue)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL queue pointer");
			return 0;
		}

		if (*queue == NULL) return 0;

		return (*queue)->ulen;
	}


	extern void queue_clear(t_queue * * queue)
	{
		t_packet * temp;

		//    eventlog(eventlog_level_debug, __FUNCTION__, "entered: queue %p", queue);
		if (!queue)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL queue pointer");
			return;
		}

		if (*queue) {
			while ((temp = queue_pull_packet(queue)))
				packet_del_ref(temp);

			if ((*queue)->ring) xfree((void*)((*queue)->ring));
			xfree((void*)(*queue));
			/* poison the queue, this should make invalid
			 * accessed queues crash earlier */
			*queue = 0;
		}
	}

}
