/*
 * Copyright (C) 2004,2006  Dizzy
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
#ifndef INCLUDED_ELIST_TYPES
#define INCLUDED_ELIST_TYPES

namespace pvpgn
{

	template<typename T>
	class elist_node
	{
	public:
		elist_node() : info_(0), prev_(this), next_(this) {}
		~elist_node() throw() {
			remove();
		}

		void link_front(elist_node& where, T& info) {
			info_ = &info;
			where.next_->prev_ = this;
			next_ = where.next_;
			prev_ = &where;
			where.next_ = this;
		}

		void link_back(elist_node& where, T& info) {
			info_ = &info;
			where.prev_->next_ = this;
			next_ = &where;
			prev_ = where.prev_;
			where.prev_ = this;
		}

		void remove() {
			/* remove ourselves from any link */
			prev_->next_ = next_;
			next_->prev_ = prev_;
			prev_ = next_ = this;
			/* poison this to find out bugs a lot earlier */
			info_ = 0;
		}

		T& info() const { return *info_; }

		elist_node* next() const { return next_; }
		elist_node* prev() const { return prev_; }

	private:
		T* info_;
		elist_node *prev_, *next_;

		elist_node(const elist_node&);
		elist_node& operator=(const elist_node&);
	};

	template<typename T>
	class elist
	{
	public:
		class iterator
		{
		public:
			iterator() : ptr(0) {}
			iterator(const iterator& orig) :ptr(orig.ptr) {}
			~iterator() throw() {}

			bool operator==(const iterator& op) const {
				return ptr == op.ptr;
			}
			bool operator!=(const iterator& op) const {
				return ptr != op.ptr;
			}

			iterator& operator++() {
				ptr = ptr->next();
				return *this;
			}
			iterator operator++(int) {
				elist_node<T>* save = ptr;
				ptr = ptr->next();
				return iterator(save);
			}

			T& operator*() const {
				return ptr->info();
			}

			T* operator->() const {
				return &ptr->info();
			}

		private:
			elist_node<T> *ptr;

			explicit iterator(elist_node<T>* ptr_) : ptr(ptr_) {}
			friend class elist;
		};

		explicit elist(elist_node<T> T::* node_) : node(node_) {}
		~elist() throw() {
			clear();
		}

		void push_back(T& obj) {
			(obj.*node).link_back(head, obj);
		}

		void push_front(T& obj) {
			(obj.*node).link_front(head, obj);
		}

		void remove(T& obj) {
			(obj.*node).remove();
		}

		void remove(iterator& it) {
			it.ptr->remove();
		}

		void clear() {
			while (!empty())
				head.prev()->remove();
		}

		bool empty() {
			return &head == head.next();
		}

		T& front() const { return head.next()->info(); }
		T& back() const { return head.prev()->info(); }

		iterator begin() {
			return iterator(head.next());
		}
		iterator end() {
			return iterator(&head);
		}

	private:
		/* the head of the list */
		elist_node<T> head;

		/* used to access the elist_node<T> of a given T */
		elist_node<T> T::* node;

		elist(const elist&);
		elist& operator=(const elist&);
	};

	typedef struct elist_struct {
		struct elist_struct *next, *prev;
	} t_elist;

	/* hlist are single linked elists */
	typedef struct hlist_struct {
		struct hlist_struct *next;
	} t_hlist;

}

#endif /* INCLUDED_ELIST_TYPES */

#ifndef INCLUDED_ELIST_PROTOS
#define INCLUDED_ELIST_PROTOS

#include <cstddef>

/* access to it's members */
#define elist_next(ptr) ((ptr)->next)
#define elist_prev(ptr) ((ptr)->prev)

#define __elist_init(elist,val) { (elist)->next = (elist)->prev = (val); }
#define elist_init(elist) __elist_init(elist,elist)
#define DECLARE_ELIST_INIT(var) \
	t_elist var = { &var, &var }

namespace pvpgn
{

	/* link an new node just after "where" */
	static inline void elist_add(t_elist *where, t_elist *what)
	{
		what->next = where->next;
		where->next->prev = what;
		what->prev = where;
		where->next = what;
	}

	/* link a new node just before "where" (usefull in creating queues) */
	static inline void elist_add_tail(t_elist *where, t_elist *what)
	{
		what->prev = where->prev;
		where->prev->next = what;
		what->next = where;
		where->prev = what;
	}

	/* unlink "what" from it's list */
	static inline void elist_del(t_elist *what)
	{
		what->next->prev = what->prev;
		what->prev->next = what->next;
	}

	/* finds out the container address by computing it from the list node
	 * address substracting the offset inside the container of the list node
	 * member */
#define elist_entry(ptr,type,member) ((type*)(((char*)ptr)-offsetof(type,member)))

	/* DONT remove while traversing with this ! */
#define elist_for_each(pos,head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define elist_for_each_rev(pos,head) \
	for (pos = (head)->prev; pos != (head); pos = pos->prev)

	/* safe for removals while traversing */
#define elist_for_each_safe(pos,head,save) \
	for (pos = (head)->next, save = pos->next; pos != (head); \
	pos = save, save = pos->next)

#define elist_for_each_safe_rev(pos,head,save) \
	for (pos = (head)->prev, save = pos->prev; pos != (head); \
	pos = save, save = pos->prev)

#define elist_empty(ptr) ((ptr)->next == (ptr))

#define hlist_next(ptr) elist_next(ptr)

#define __hlist_init(hlist,val) { (hlist)->next = (val); }
#define hlist_init(hlist) __hlist_init(hlist,hlist)
#define DECLARE_HLIST_INIT(var) \
	t_hlist var = { &var };

	/* link an new node just after "where" */
	static inline void hlist_add(t_hlist *where, t_hlist *what)
	{
		what->next = where->next;
		where->next = what;
	}

	/* unlink "what" from it's list, prev->next = what */
	static inline void hlist_del(t_hlist *what, t_hlist *prev)
	{
		prev->next = what->next;
	}

	/* move "what" back in the list with one position, never NULL
	 * prev: the previous element in the list, never NULL
	 * prev2: the previous element of "prev", if NULL means "what" is first
	 */
	static inline void hlist_promote(t_hlist *what, t_hlist *prev, t_hlist *prev2)
	{
		if (prev2) {
			prev->next = what->next;
			prev2->next = what;
			what->next = prev;
		}
	}

	/* finds out the container address by computing it from the list node
	 * address substracting the offset inside the container of the list node
	 * member */
#define hlist_entry(ptr,type,member) elist_entry(ptr,type,member)

	/* DONT remove while traversing with this ! */
#define hlist_for_each(pos,head) elist_for_each(pos,head)

	/* safe for removals while traversing */
#define hlist_for_each_safe(pos,head,save) elist_for_each_safe(pos,head,save)

#define hlist_empty(ptr) elist_empty(ptr)

}

#endif /* INCLUDED_ELIST_PROTOS */
