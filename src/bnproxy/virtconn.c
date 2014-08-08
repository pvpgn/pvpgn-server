/*
 * Copyright (C) 1999  Ross Combs (rocombs@cs.nmsu.edu)
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
#define VIRTCONN_INTERNAL_ACCESS
#include "common/setup_before.h"
#ifdef HAVE_STDDEF_H
# include <stddef.h>
#else
# ifndef NULL
#  define NULL ((void *)0)
# endif
#endif
#ifdef STDC_HEADERS
# include <stdlib.h>
#else
# ifdef HAVE_MALLOC_H
#  include <malloc.h>
# endif
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include "compat/socket.h"
#include "compat/psock.h"
#include "common/queue.h"
#include "common/list.h"
#include "common/eventlog.h"
#include "virtconn.h"
#include "common/setup_after.h"


static t_list * virtconn_head = NULL;


extern t_virtconn * virtconn_create(int csd, int ssd, unsigned int udpaddr, unsigned short udpport)
{
	t_virtconn * temp = NULL;

	if (csd < 0)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got bad client socket");
		return NULL;
	}
	if (ssd < 0)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got bad server socket");
		return NULL;
	}

	if (!(temp = malloc(sizeof(t_virtconn))))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "could not allocate memory for temp");
		return NULL;
	}

	temp->csd = csd;
	temp->ssd = ssd;
	temp->udpport = udpport;
	temp->udpaddr = udpaddr;
	temp->class = virtconn_class_none;
	temp->state = virtconn_state_initial;
	temp->coutqueue = NULL;
	temp->coutsize = 0;
	temp->cinqueue = NULL;
	temp->cinsize = 0;
	temp->soutqueue = NULL;
	temp->soutsize = 0;
	temp->sinqueue = NULL;
	temp->sinsize = 0;
	temp->fileleft = 0;

	if (list_prepend_data(virtconn_head, temp) < 0)
	{
		free(temp);
		eventlog(eventlog_level_error, __FUNCTION__, "could not prepend temp");
		return NULL;
	}

	return temp;
}


extern void virtconn_destroy(t_virtconn * vc)
{
	t_elem * curr;

	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL virtconn");
		return;
	}
	if (list_remove_data(virtconn_head, vc, &curr) < 0)
		eventlog(eventlog_level_error, __FUNCTION__, "could not remove item from list");

	vc->state = virtconn_state_empty;

	psock_close(vc->ssd);
	psock_close(vc->csd);

	/* clear out the packet queues */
	queue_clear(&vc->sinqueue);
	queue_clear(&vc->soutqueue);
	queue_clear(&vc->cinqueue);
	queue_clear(&vc->coutqueue);

	eventlog(eventlog_level_info, __FUNCTION__, "[%d] closed server connection (%d) class=%d", vc->ssd, vc->csd, (int)vc->class);
	eventlog(eventlog_level_info, __FUNCTION__, "[%d] closed client connection (%d) class=%d", vc->csd, vc->ssd, (int)vc->class);

	free(vc);
}


extern t_virtconn_class virtconn_get_class(t_virtconn const * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL virtconn");
		return virtconn_class_none;
	}

	return vc->class;
}


extern void virtconn_set_class(t_virtconn * vc, t_virtconn_class class)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL virtconn");
		return;
	}

	vc->class = class;
}


extern t_virtconn_state virtconn_get_state(t_virtconn const * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL virtconn");
		return virtconn_state_empty;
	}

	return vc->state;
}


extern void virtconn_set_state(t_virtconn * vc, t_virtconn_state state)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL virtconn");
		return;
	}

	vc->state = state;
}


extern unsigned int virtconn_get_udpaddr(t_virtconn const * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL virtconn");
		return 0;
	}

	return vc->udpaddr;
}


extern unsigned short virtconn_get_udpport(t_virtconn const * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL virtconn");
		return 0;
	}

	return vc->udpport;
}


extern t_queue * * virtconn_get_clientin_queue(t_virtconn * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
		return NULL;
	}

	return &vc->cinqueue;
}


extern int virtconn_get_clientin_size(t_virtconn const * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
		return -1;
	}

	return vc->cinsize;
}


extern void virtconn_set_clientin_size(t_virtconn * vc, unsigned int size)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
		return;
	}

	vc->cinsize = size;
}


extern t_queue * * virtconn_get_clientout_queue(t_virtconn * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
		return NULL;
	}

	return &vc->coutqueue;
}


extern int virtconn_get_clientout_size(t_virtconn const * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
		return -1;
	}

	return vc->coutsize;
}


extern void virtconn_set_clientout_size(t_virtconn * vc, unsigned int size)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
		return;
	}

	vc->coutsize = size;
}


extern int virtconn_get_client_socket(t_virtconn const * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL virtconn");
		return -1;
	}
	return vc->csd;
}


extern t_queue * * virtconn_get_serverin_queue(t_virtconn * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
		return NULL;
	}

	return &vc->sinqueue;
}


extern int virtconn_get_serverin_size(t_virtconn const * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
		return -1;
	}

	return vc->sinsize;
}


extern void virtconn_set_serverin_size(t_virtconn * vc, unsigned int size)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
		return;
	}

	vc->sinsize = size;
}


extern t_queue * * virtconn_get_serverout_queue(t_virtconn * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
		return NULL;
	}

	return &vc->soutqueue;
}


extern int virtconn_get_serverout_size(t_virtconn const * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
		return -1;
	}

	return vc->soutsize;
}


extern void virtconn_set_serverout_size(t_virtconn * vc, unsigned int size)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
		return;
	}

	vc->soutsize = size;
}


extern int virtconn_get_server_socket(t_virtconn const * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL virtconn");
		return -1;
	}
	return vc->ssd;
}


extern void virtconn_set_fileleft(t_virtconn * vc, unsigned int size)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL virtconn");
		return;
	}
	vc->fileleft = size;
}


extern unsigned int virtconn_get_fileleft(t_virtconn const * vc)
{
	if (!vc)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "got NULL virtconn");
		return 0;
	}
	return vc->fileleft;
}


extern int virtconnlist_create(void)
{
	if (!(virtconn_head = list_create()))
		return -1;
	return 0;
}


extern int virtconnlist_destroy(void)
{
	if (list_destroy(virtconn_head) < 0)
		return -1;
	virtconn_head = NULL;
	return 0;
}


extern t_list * virtconnlist(void)
{
	return virtconn_head;
}
