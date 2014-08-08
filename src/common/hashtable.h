/*
 * Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_HASHTABLE_TYPES
#define INCLUDED_HASHTABLE_TYPES

namespace pvpgn
{

#ifdef HASHTABLE_INTERNAL_ACCESS
	struct hashtable; /* forward reference for t_entry */
#endif

#ifdef HASHTABLE_INTERNAL_ACCESS
	typedef struct internentry
	{
		void *               data;
		struct internentry * next;
	}
	t_internentry;
#endif

	typedef struct entry
#ifdef HASHTABLE_INTERNAL_ACCESS
	{
		unsigned int             row;
		t_internentry *          real;
		struct hashtable const * hashtable;
	}
#endif
	t_entry;

	typedef struct hashtable
#ifdef HASHTABLE_INTERNAL_ACCESS
	{
		unsigned int      num_rows;
		unsigned int      len;
		unsigned int      zombies;
		t_internentry * * rows;
	}
#endif
	t_hashtable;

}

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_HASHTABLE_PROTOS
#define INCLUDED_HASHTABLE_PROTOS

namespace pvpgn
{

	extern t_hashtable * hashtable_create(unsigned int num_rows);
	extern int hashtable_destroy(t_hashtable * hashtable);
	extern int hashtable_purge(t_hashtable * hashtable);
	extern int hashtable_check(t_hashtable const * hashtable);
	extern unsigned int hashtable_get_length(t_hashtable const * hashtable);
	extern int hashtable_insert_data(t_hashtable * hashtable, void * data, unsigned int hash);
	extern t_entry * hashtable_get_entry_by_data(t_hashtable const * hashtable, void const * data, unsigned int hash);
	extern t_entry const * hashtable_get_entry_by_data_const(t_hashtable const * hashtable, void const * data, unsigned int hash);
	extern int hashtable_remove_data(t_hashtable * hashtable, void const * data, unsigned int hash); /* delete matching item */
	extern int hashtable_remove_entry(t_hashtable * hashtable, t_entry * entry);
	extern void * hashtable_get_data_by_pos(t_hashtable const * hashtable, unsigned int pos);
	extern void * entry_get_data(t_entry const * entry);
#ifdef HASHTABLE_DEBUG
	extern t_entry * hashtable_match_get_first_real(t_hashtable const * hashtable, unsigned int hash, char const * fn, unsigned int ln);
# define hashtable_match_get_first(L,H) hashtable_match_get_first_real(L,H,__FILE__,__LINE__)
#else
	extern t_entry * hashtable_match_get_first(t_hashtable const * hashtable, unsigned int hash);
#endif
	extern t_entry * entry_match_get_next(t_entry const * entry, unsigned int hash);
#ifdef HASHTABLE_DEBUG
	extern t_entry * hashtable_get_first_real(t_hashtable const * hashtable, char const * fn, unsigned int ln);
# define hashtable_get_first(L) hashtable_get_first_real(L,__FILE__,__LINE__)
#else
	extern t_entry * hashtable_get_first(t_hashtable const * hashtable);
#endif
	extern t_entry * entry_get_next(t_entry * entry);
#ifdef HASHTABLE_DEBUG
	extern t_entry * hashtable_get_first_matching_real(t_hashtable const * hashtable, unsigned int hash, char const * fn, unsigned int ln);
# define hashtable_get_first_matching(L,H) hashtable_get_first_matching_real(L,H,__FILE__,__LINE__)
#else
	extern t_entry * hashtable_get_first_matching(t_hashtable const * hashtable, unsigned int hash);
#endif
	extern t_entry * entry_get_next_matching(t_entry * entry);
	extern int hashtable_entry_release(t_entry * entry);
	extern int hashtable_stats(t_hashtable * hashtable);

#define HASHTABLE_TRAVERSE(hashtable,curr) for (curr=hashtable_get_first(hashtable); curr; curr=entry_get_next(curr))
#define HASHTABLE_TRAVERSE_MATCHING(hashtable,curr,hash) for (curr=hashtable_get_first_matching(hashtable,hash); curr; curr=entry_get_next_matching(curr))

}

#endif
#endif
