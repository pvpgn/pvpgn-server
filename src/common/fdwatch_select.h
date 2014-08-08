/*
  * Abstraction API/layer for the various ways PvPGN can inspect sockets state
  * 2003 (C)
  *
  * Code is based on the ideas found in thttpd project.
  *
  * select() based backend
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
#ifndef __INCLUDED_FDWATCH_SELECT__
#define __INCLUDED_FDWATCH_SELECT__

#ifdef HAVE_SELECT

#include "compat/psock.h"
#include "common/scoped_ptr.h"
#include "fdwatch.h"
#include "fdwbackend.h"

namespace pvpgn
{

	class FDWSelectBackend : public FDWBackend
	{
	public:
		explicit FDWSelectBackend(int nfds_);
		~FDWSelectBackend() throw();

		int add(int idx, unsigned rw);
		int del(int idx);
		int watch(long timeout_msecs);
		void handle();

		int cb(t_fdwatch_fd* cfd);

	private:
		int sr, smaxfd;
		scoped_ptr<t_psock_fd_set> rfds, wfds, /* working sets (updated often) */
			trfds, twfds; /* templates (updated rare) */
	};

}

#endif /* HAVE_SELECT */

#endif /* __INCLUDED_FDWATCH_SELECT__ */
