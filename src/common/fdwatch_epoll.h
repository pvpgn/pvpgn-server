/*
  * Abstraction API/layer for the various ways PvPGN can inspect sockets state
  * 2003 (C)
  *
  * Code is based on the ideas found in thttpd project.
  *
  * Linux epoll(4) based backend
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
#ifndef __INCLUDED_FDWATCH_EPOLL__
#define __INCLUDED_FDWATCH_EPOLL__

#ifdef HAVE_EPOLL

#ifdef HAVE_SYS_EPOLL_H
# include "compat/uint.h"
# include <sys/epoll.h>
#endif

#include "scoped_array.h"
#include "fdwbackend.h"

namespace pvpgn
{

	class FDWEpollBackend : public FDWBackend
	{
	public:
		explicit FDWEpollBackend(int nfds_);
		~FDWEpollBackend() throw();

		int add(int idx, unsigned rw);
		int del(int idx);
		int watch(long timeout_msecs);
		void handle();

	private:
		int sr;
		int epfd;
		/* events to investigate */
		scoped_array<struct epoll_event> epevents;
	};

}

#endif /* HAVE_EPOLL */

#endif /* __INCLUDED_FDWATCH_EPOLL__ */
