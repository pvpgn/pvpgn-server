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

#define FDWATCH_BACKEND
#include "common/setup_before.h"
#include "fdwatch_select.h"

#include <cstring>

#include "common/eventlog.h"
#include "common/setup_after.h"

#ifdef HAVE_SELECT

namespace pvpgn
{

	FDWSelectBackend::FDWSelectBackend(int nfds_)
		:FDWBackend(nfds_), sr(0), smaxfd(0)
	{
		/* this should not happen */
		if (nfds > FD_SETSIZE)
			throw InitError("nfds over FD_SETSIZE");

		rfds.reset(new t_psock_fd_set);
		wfds.reset(new t_psock_fd_set);
		trfds.reset(new t_psock_fd_set);
		twfds.reset(new t_psock_fd_set);

		PSOCK_FD_ZERO(trfds.get()); PSOCK_FD_ZERO(twfds.get());

		INFO1("fdwatch select() based layer initialized (max %d sockets)", nfds);
	}

	FDWSelectBackend::~FDWSelectBackend() throw()
	{}

	int
		FDWSelectBackend::add(int idx, unsigned rw)
	{
			//    eventlog(eventlog_level_trace, __FUNCTION__, "called fd: %d rw: %d", fd, rw);
			int fd = fdw_fd(fdw_fds + idx);

			/* select() interface is limited by FD_SETSIZE max socket value */
			if (fd >= FD_SETSIZE) return -1;

			if (rw & fdwatch_type_read) PSOCK_FD_SET(fd, trfds.get());
			else PSOCK_FD_CLR(fd, trfds.get());
			if (rw & fdwatch_type_write) PSOCK_FD_SET(fd, twfds.get());
			else PSOCK_FD_CLR(fd, twfds.get());
			if (smaxfd < fd) smaxfd = fd;

			return 0;
		}

	int
		FDWSelectBackend::del(int idx)
	{
			int fd = fdw_fd(fdw_fds + idx);
			//    eventlog(eventlog_level_trace, __FUNCTION__, "called fd: %d", fd);
			if (sr > 0)
				ERROR0("BUG: called while still handling sockets");
			PSOCK_FD_CLR(fd, trfds.get());
			PSOCK_FD_CLR(fd, twfds.get());

			return 0;
		}

	int
		FDWSelectBackend::watch(long timeout_msec)
	{
			static struct timeval tv;

			tv.tv_sec = timeout_msec / 1000;
			tv.tv_usec = timeout_msec % 1000;

			/* set the working sets based on the templates */
			std::memcpy(rfds.get(), trfds.get(), sizeof(t_psock_fd_set));
			std::memcpy(wfds.get(), twfds.get(), sizeof(t_psock_fd_set));

			return (sr = psock_select(smaxfd + 1, rfds.get(), wfds.get(), NULL, &tv));
		}

	namespace
	{

		int fdw_select_cb(t_fdwatch_fd *cfd, void *data)
		{
			FDWSelectBackend *obj = static_cast<FDWSelectBackend*>(data);

			return obj->cb(cfd);
		}

	}

	int
		FDWSelectBackend::cb(t_fdwatch_fd* cfd)
	{
			//    eventlog(eventlog_level_trace, __FUNCTION__, "idx: %d fd: %d", idx, fdw_fd->fd);
			if (fdw_rw(cfd) & fdwatch_type_read && PSOCK_FD_ISSET(fdw_fd(cfd), rfds.get())
				&& fdw_hnd(cfd)(fdw_data(cfd), fdwatch_type_read) == -2) return 0;
			if (fdw_rw(cfd) & fdwatch_type_write && PSOCK_FD_ISSET(fdw_fd(cfd), wfds.get()))
				fdw_hnd(cfd)(fdw_data(cfd), fdwatch_type_write);

			return 0;
		}

	void
		FDWSelectBackend::handle()
	{
			//    eventlog(eventlog_level_trace, __FUNCTION__, "called nofds: %d", fdw_nofds);
			fdwatch_traverse(fdw_select_cb, this);
			sr = 0;
		}

}

#endif /* HAVE_SELECT */
