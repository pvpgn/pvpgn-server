/*
 * Copyright (C) 1998,1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#define LIST_INTERNAL_ACCESS
#include "common/list.h"

#include <cassert>

#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/setup_after.h"


namespace pvpgn
{

	static t_elem listhead;


	extern t_list * list_create(void)
	{
		t_list * newl;

		newl = (t_list*)xmalloc(sizeof(t_list));
		newl->head = NULL;
		newl->tail = NULL;
		newl->len = 0;

		return newl;
	}


	extern int list_destroy(t_list * list)
	{
		if (!list)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL list");
			return -1;
		}

		if (list->head)
			eventlog(eventlog_level_error, __FUNCTION__, "got non-empty list");

		xfree(list);

		return 0;
	}

	extern unsigned int list_get_length(t_list const * list)
	{
		if (!list)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL list");
			return 0;
		}

		return list->len;
	}


	extern int list_prepend_data(t_list * list, void * data)
	{
		t_elem * elem;

		assert(list != NULL);

		elem = (t_elem*)xmalloc(sizeof(t_elem));
		elem->data = data;

		if (list->head)
			list->head->prev = elem;
		elem->next = list->head;
		elem->prev = NULL;
		list->head = elem;
		if (!list->tail)
			list->tail = elem;
		list->len++;

		return 0;
	}


	extern int list_append_data(t_list * list, void * data)
	{
		t_elem * elem;

		assert(list != NULL);

		elem = (t_elem*)xmalloc(sizeof(t_elem));
		elem->data = data;

		elem->next = NULL;
		if (!list->head)
		{
			list->head = elem;
			elem->prev = NULL;
		}
		if (list->tail)
		{
			elem->prev = list->tail;
			list->tail->next = elem;
		}
		list->tail = elem;
		list->len++;

		return 0;
	}


	extern t_elem * list_get_elem_by_data(t_list const * list, void const * data)
	{
		t_elem * curr;

		if (!list)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL list");
			return NULL;
		}

		LIST_TRAVERSE(list, curr)
		if (curr->data == data)
			return curr;

		return NULL;
	}


	extern t_elem const * list_get_elem_by_data_const(t_list const * list, void const * data)
	{
		t_elem const * curr;

		if (!list)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL list");
			return NULL;
		}

		LIST_TRAVERSE_CONST(list, curr)
		if (curr->data == data)
			return curr;

		return NULL;
	}


	extern int list_remove_elem(t_list * list, t_elem ** elem)
	{
		t_elem * target;

		if (!list)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL list");
			return -1;
		}

		if (!elem)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL *elem");
			return -1;
		}

		target = *elem;

		if (target->prev)
		{
			target->prev->next = target->next;
		}
		if (target->next)
		{
			target->next->prev = target->prev;
		}

		if (target == list->tail)
		{
			list->tail = target->prev;
		}
		if (target == list->head)
		{
			list->head = target->next;
			*elem = &listhead;
		}
		else
			*elem = target->prev;

		target->next = NULL;
		target->prev = NULL;
		xfree(target);

		list->len--;

		return 0;
	}


	extern int list_remove_data(t_list * list, void const * data, t_elem ** elem)
	{
		if (!list)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL list");
			return -1;
		}

		if (!(*elem = list_get_elem_by_data(list, data)))
			return -1;

		return list_remove_elem(list, elem);
	}


	extern int elem_set_data(t_elem * elem, void * data)
	{
		if (!elem)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL elem");
			return -1;
		}

		elem->data = data;

		return 0;
	}


	extern void * elem_get_data(t_elem const * elem)
	{
		if (!elem)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL elem");
			return NULL;
		}

		return elem->data;
	}


	extern void * list_get_data_by_pos(t_list const * list, unsigned int pos)
	{
		t_elem const * curr;
		unsigned int   len;

		len = 0;
		LIST_TRAVERSE_CONST(list, curr)
		if (len++ == pos)
			return curr->data;

		eventlog(eventlog_level_error, __FUNCTION__, "requested position %u but len=%u", pos, len);
		return NULL;
	}


#ifdef LIST_DEBUG
	extern t_elem * list_get_first_real(t_list const * list, char const * fn, unsigned int ln)
#else
	extern t_elem * list_get_first(t_list const * list)
#endif
	{
		if (!list)
		{
#ifdef LIST_DEBUG
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL list from %s:%u", fn, ln);
#else
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL list");
#endif
			return NULL;
		}


		return list->head;
	}


#ifdef LIST_DEBUG
	extern t_elem const * list_get_first_const_real(t_list const * list, char const * fn, unsigned int ln)
#else
	extern t_elem const * list_get_first_const(t_list const * list)
#endif
	{
		if (!list)
		{
#ifdef LIST_DEBUG
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL list from %s:%u", fn, ln);
#else
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL list");
#endif
			return NULL;
		}

		return list->head;
	}


	extern t_elem * elem_get_next_real(t_list const * list, t_elem const * elem, char const * fn, unsigned int ln)
	{
		if (!elem)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL elem from %s:%u", fn, ln);
			return NULL;
		}

		if (elem == &listhead)
			return list->head;
		else
			return elem->next;
	}


	extern t_elem const * elem_get_next_const(t_list const * list, t_elem const * elem)
	{
		if (!elem)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL elem");
			return NULL;
		}

		if (elem == &listhead)
			return list->head;
		else
			return elem->next;
	}

}
