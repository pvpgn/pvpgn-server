/*
 * Copyright (C) 2005 Dizzy
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

#ifndef PVPGN_SCOPED_ARRAY_H
#define PVPGN_SCOPED_ARRAY_H

namespace pvpgn
{

	template<typename T>
	class scoped_array
	{
	public:
		/** initilize the object aquiring ownership of the given parameter (0 for no onwership) */
		explicit scoped_array(T* ptr_ = 0)
			:ptr(ptr_) {}

		/** initilize the object from a blind pointer supporting implicit conversions */
		template<typename V>
		explicit scoped_array(V* ptr_)
			:ptr(ptr_) {}

		/** release memory if aquired ownershipt */
		~scoped_array() throw() {
			cleanup();
		}

		/** get the wrapped array pointer */
		T* get() const { return ptr; }

		/** release ownership of the array */
		T* release() {
			T* tmp = ptr;
			ptr = 0;
			return tmp;
		}

		/** reinitilize object, release owned resource first if any */
		void reset(T* ptr_ = 0) {
			cleanup();
			ptr = ptr_;
		}

		/** reinitilize object, supports implicit conversions */
		template<typename V>
		void reset(V* ptr_) {
			cleanup();
			ptr = ptr_;
		}

		/** allow indexed const dereferencing access to the wrapped array */
		const T& operator[](unsigned idx) const {
			return ptr[idx];
		}

		/** allow indexed dereferencing access to the wrapped array */
		T& operator[](unsigned idx) {
			return ptr[idx];
		}

	private:
		T* ptr;

		/* do not allow to copy a scoped_array */
		scoped_array(const scoped_array&);
		scoped_array& operator=(const scoped_array&);

		void cleanup() {
			if (ptr) delete[] ptr;
		}
	};

}

#endif
