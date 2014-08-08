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
#ifndef INCLUDED_LIST_TYPES
#define INCLUDED_LIST_TYPES

namespace pvpgn
{

	typedef struct elem
#ifdef LIST_INTERNAL_ACCESS
	{
		void *        data;
		struct elem * prev;
		struct elem * next;
	}
#endif
	t_elem;

	typedef struct list
#ifdef LIST_INTERNAL_ACCESS
	{
		unsigned int  len;
		t_elem *      head;
		t_elem *      tail;
	}
#endif
	t_list;

}

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_LIST_PROTOS
#define INCLUDED_LIST_PROTOS

namespace pvpgn
{

	extern t_list * list_create(void);
	extern int list_destroy(t_list * list);
	extern unsigned int list_get_length(t_list const * list);
	extern int list_prepend_data(t_list * list, void * data);
	extern int list_append_data(t_list * list, void * data);
	extern t_elem * list_get_elem_by_data(t_list const * list, void const * data);
	extern t_elem const * list_get_elem_by_data_const(t_list const * list, void const * data);

	/* note changed API for those commands:
		 due to direct removal of elements from list, you need to take special care during list traversal.
		 a pointer to the traversal variable needs to be passed to the list_remove functions, so they
		 can properly modify it to point to the "previous" element (the one before the element to be deleted)
		 so the next elem_get_next call will address the "next" element (the one after the element to be deleted) */

	extern int list_remove_data(t_list * list, void const * data, t_elem ** elem); /* delete matching item */
	extern int list_remove_elem(t_list * list, t_elem ** elem);

	extern void * list_get_data_by_pos(t_list const * list, unsigned int pos);
#ifdef LIST_DEBUG
	extern t_elem * list_get_first_real(t_list const * list, char const * fn, unsigned int ln);
# define list_get_first(L) list_get_first_real(L,__FILE__,__LINE__)
#else
	extern t_elem * list_get_first(t_list const * list);
#endif
#ifdef LIST_DEBUG
	extern t_elem const * list_get_first_const_real(t_list const * list, char const * fn, unsigned int ln);
# define list_get_first_const(L) list_get_first_const_real(L,__FILE__,__LINE__)
#else
	extern t_elem const * list_get_first_const(t_list const * list);
#endif

	extern void * elem_get_data(t_elem const * elem);
	extern int elem_set_data(t_elem * elem, void * data);
#define elem_get_next(list,elem) elem_get_next_real(list,elem,__FILE__,__LINE__)
	extern t_elem * elem_get_next_real(t_list const * list, t_elem const * elem, char const * fn, unsigned int ln);
	extern t_elem const * elem_get_next_const(t_list const * list, t_elem const * elem);

#define LIST_TRAVERSE(list,curr) for (curr=(list)?list_get_first(list):(NULL); curr; curr=elem_get_next(list,curr))
#define LIST_TRAVERSE_CONST(list,curr) for (curr=(list)?list_get_first_const(list):(NULL); curr; curr=elem_get_next_const(list,curr))

}

#endif
#endif
