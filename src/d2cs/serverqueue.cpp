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
#include "serverqueue.h"

#include <ctime>

#include "common/eventlog.h"
#include "common/xalloc.h"
/*
#include "common/packet.h"
#include "common/list.h"
*/
#include "prefs.h"
#include "common/setup_after.h"

namespace pvpgn
{

namespace d2cs
{

static t_list		* sqlist_head=NULL;
static unsigned int	sqlist_seqno=0;

extern t_list * sqlist(void)
{
	return sqlist_head;
}

extern int sqlist_create(void)
{
	sqlist_head=list_create();
  	return 0;
}

extern int sqlist_destroy(void)
{
	t_sq	 * sq;

	BEGIN_LIST_TRAVERSE_DATA_CONST(sqlist_head,sq,t_sq)
	{
		sq_destroy(sq,(t_elem **)curr_elem_);
	}
	END_LIST_TRAVERSE_DATA_CONST()

	if (list_destroy(sqlist_head)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error destroy server queue list");
		return -1;
	}
	sqlist_head=NULL;
	return 0;
}

extern int sqlist_check_timeout(void)
{
	t_sq	* sq;
	std::time_t	now;

	now=std::time(NULL);
	BEGIN_LIST_TRAVERSE_DATA(sqlist_head, sq, t_sq)
	{
		if (now - sq->ctime > prefs_get_sq_timeout()) {
			eventlog(eventlog_level_info,__FUNCTION__,"destroying expired server queue %d",sq->seqno);
			sq_destroy(sq,&curr_elem_);
		}
	}
	END_LIST_TRAVERSE_DATA()
	return 0;
}

extern t_sq * sqlist_find_sq(unsigned int seqno)
{
	t_sq	* sq;

	BEGIN_LIST_TRAVERSE_DATA_CONST(sqlist_head,sq,t_sq)
	{
		if (sq->seqno==seqno) return sq;
	}
	END_LIST_TRAVERSE_DATA_CONST()
	return NULL;
}

extern t_sq * sq_create(unsigned int clientid, t_packet * packet,unsigned int gameid )
{
	t_sq	* sq;

	sq=(t_sq*)xmalloc(sizeof(t_sq));
	sq->seqno=++sqlist_seqno;
	sq->ctime=std::time(NULL);
	sq->clientid=clientid;
	sq->gameid=gameid;
	sq->packet=packet;
	sq->gametoken=0;
	if (packet) packet_add_ref(packet);
	list_append_data(sqlist_head,sq);
	return sq;
}

extern int sq_destroy(t_sq * sq,t_elem ** curr)
{
	ASSERT(sq,-1);
	if (list_remove_data(sqlist_head,sq,curr)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error remove server queue from list");
		return -1;
	}
	if (sq->packet) packet_del_ref(sq->packet);
	xfree(sq);
	return 0;
}

extern unsigned int sq_get_clientid(t_sq const * sq)
{
	ASSERT(sq,0);
	return sq->clientid;
}

extern t_packet * sq_get_packet(t_sq const * sq)
{
	ASSERT(sq,NULL);
	return sq->packet;
}

extern unsigned int sq_get_gameid(t_sq const * sq)
{
	ASSERT(sq,0);
	return sq->gameid;
}

extern unsigned int sq_get_seqno(t_sq const * sq)
{
	ASSERT(sq,0);
	return sq->seqno;
}

extern int sq_set_gametoken(t_sq * sq, unsigned int gametoken)
{
	ASSERT(sq,-1);
	sq->gametoken=gametoken;
	return 0;
}

extern unsigned int sq_get_gametoken(t_sq const * sq)
{
	ASSERT(sq,0);
	return sq->gametoken;
}

}

}
