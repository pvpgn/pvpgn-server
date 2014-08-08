/*
  * Abstraction API/layer for the various ways PvPGN can inspect sockets state
  * 2003 (C)
  *
  * Code is based on the ideas found in thttpd project.
  *
  * *BSD kqueue() based backend
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
#ifndef __INCLUDED_FDWATCH_KQUEUE__
#define __INCLUDED_FDWATCH_KQUEUE__

#ifdef HAVE_KQUEUE

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_EVENT_H
# include <sys/event.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif

#include "scoped_array.h"
#include "fdwbackend.h"

namespace pvpgn
{

	class FDWKqueueBackend : public FDWBackend
	{
	public:
		explicit FDWKqueueBackend(int nfds_);
		~FDWKqueueBackend() throw();

		int add(int idx, unsigned rw);
		int del(int idx);
		int watch(long timeout_msecs);
		void handle();

	private:
		int sr;
		int kq;
		/* changes to make to kqueue */
		scoped_array<struct kevent> kqchanges;
		/* events to investigate */
		scoped_array<struct kevent> kqevents;
		/* r/w indices from idx to the kqchanges index where the change is stored */
		scoped_array<int> rridx, wridx;
		unsigned nochanges;
	};

}

#endif /* HAVE_KQUEUE */

#endif /* __INCLUDED_FDWATCH_KQUEUE__ */
