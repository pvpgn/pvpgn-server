/*
  * Abstraction API/layer for the various ways PvPGN can inspect sockets state
  * 2003 (C)
  *
  * Code is based on the ideas found in thttpd project.
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

#ifndef __PVPGN_FDWBACKEND_INCLUDED__
#define __PVPGN_FDWBACKEND_INCLUDED__

#include <stdexcept>

namespace pvpgn
{

	class FDWBackend
	{
	public:
		class InitError :public std::runtime_error
		{
		public:
			explicit InitError(const std::string& str = "")
				:std::runtime_error(str) {}
			~InitError() throw() {}
		};

		explicit FDWBackend(int nfds_);
		virtual ~FDWBackend() throw();

		virtual int add(int idx, unsigned rw) = 0;
		virtual int del(int idx) = 0;
		virtual int watch(long timeout_msecs) = 0;
		virtual void handle() = 0;

	protected:
		int nfds;
	};

}

#endif /* __PVPGN_FDWBACKEND_INCLUDED__ */
