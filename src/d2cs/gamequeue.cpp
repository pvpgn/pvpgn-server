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
#include "common/setup_before.h"
#include "setup.h"
#include "gamequeue.h"

#include <cstring>

#include "compat/strcasecmp.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "connection.h"
#include "handle_d2cs.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace d2cs
	{

		static t_list	* gqlist_head = NULL;
		static unsigned int gqlist_seqno = 0;

		extern t_list * gqlist(void)
		{
			return gqlist_head;
		}

		extern int gqlist_create(void)
		{
			gqlist_head = list_create();
			return 0;
		}

		extern int gqlist_destroy(void)
		{
			t_gq	* gq;

			BEGIN_LIST_TRAVERSE_DATA(gqlist_head, gq, t_gq)
			{
				gq_destroy(gq, (t_elem **)curr_elem_);
			}
			END_LIST_TRAVERSE_DATA()

			if (list_destroy(gqlist_head) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error destroy game queue list");
				return -1;
			}
			gqlist_head = NULL;
			return 0;
		}

		extern t_gq * gq_create(unsigned int clientid, t_packet * packet, char const * gamename)
		{
			t_gq	* gq;

			gq = (t_gq*)xmalloc(sizeof(t_gq));
			gq->seqno = ++gqlist_seqno;
			gq->clientid = clientid;
			gq->packet = packet;
			std::strncpy(gq->gamename, gamename, MAX_GAMENAME_LEN);
			if (packet) packet_add_ref(packet);
			list_append_data(gqlist_head, gq);
			return gq;
		}

		extern int gq_destroy(t_gq * gq, t_elem ** elem)
		{
			ASSERT(gq, -1);
			if (list_remove_data(gqlist_head, gq, elem) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error remove game queue from list");
				return -1;
			}
			if (gq->packet) packet_del_ref(gq->packet);
			xfree(gq);
			return 0;
		}

		extern unsigned int gq_get_clientid(t_gq const * gq)
		{
			ASSERT(gq, 0);
			return gq->clientid;
		}

		extern int gqlist_check_creategame(int number)
		{
			t_connection	* c;
			t_gq		* gq;
			int		i = 0;

			if (number <= 0) return -1;

			BEGIN_LIST_TRAVERSE_DATA(gqlist_head, gq, t_gq)
			{
				c = d2cs_connlist_find_connection_by_sessionnum(gq->clientid);
				if (!c) {
					eventlog(eventlog_level_error, __FUNCTION__, "client %d not found (gamename: %s)", gq->clientid, gq->gamename);
					gq_destroy(gq, &curr_elem_);
					continue;
				}
				else if (!conn_get_gamequeue(c)) {
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL game queue for client %s", d2cs_conn_get_account(c));
					gq_destroy(gq, &curr_elem_);
					continue;
				}
				else {
					eventlog(eventlog_level_info, __FUNCTION__, "try create game %s for account %s", gq->gamename, d2cs_conn_get_account(c));
					d2cs_handle_client_creategame(c, gq->packet);
					conn_set_gamequeue(c, NULL);
					gq_destroy(gq, &curr_elem_);
					i++;
					if (i >= number) break;
				}
			}
			END_LIST_TRAVERSE_DATA()
				return 0;
		}

		extern int gqlist_update_all_clients(void)
		{
			t_connection	* c;
			t_gq		* gq;
			unsigned int	n;

			n = 0;
			BEGIN_LIST_TRAVERSE_DATA(gqlist_head, gq, t_gq)
			{
				c = d2cs_connlist_find_connection_by_sessionnum(gq->clientid);
				if (!c) {
					eventlog(eventlog_level_error, __FUNCTION__, "client %d not found (gamename: %s)", gq->clientid, gq->gamename);
					gq_destroy(gq, &curr_elem_);
					continue;
				}
				else {
					n++;
					eventlog(eventlog_level_debug, __FUNCTION__, "update client %s position to %d", d2cs_conn_get_account(c), n);
					d2cs_send_client_creategamewait(c, n);
				}
			}
			END_LIST_TRAVERSE_DATA()
			if (n) eventlog(eventlog_level_info, __FUNCTION__, "total %d game queues", n);
			return 0;
		}

		extern unsigned int gqlist_get_gq_position(t_gq * gq)
		{
			t_gq		* tmp;
			unsigned int	pos;

			pos = 0;
			BEGIN_LIST_TRAVERSE_DATA(gqlist_head, tmp, t_gq)
			{
				pos++;
				if (tmp == gq) return pos;
			}
			END_LIST_TRAVERSE_DATA()
				return 0;
		}

		extern unsigned int gqlist_get_length(void)
		{
			return list_get_length(gqlist_head);
		}

		extern t_gq * gqlist_find_game(char const * gamename)
		{
			t_gq		* gq;

			BEGIN_LIST_TRAVERSE_DATA(gqlist_head, gq, t_gq)
			{
				if (!strcasecmp(gq->gamename, gamename)) return gq;
			}
			END_LIST_TRAVERSE_DATA()
				return NULL;
		}

	}

}
