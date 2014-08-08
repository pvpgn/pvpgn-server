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

#include "common/setup_before.h"
#ifdef HAVE_EPOLL
#include "fdwatch_epoll.h"

#include <cstring>

#include "common/eventlog.h"
#include "fdwatch.h"
#include "common/setup_after.h"

namespace pvpgn
{

	FDWEpollBackend::FDWEpollBackend(int nfds_)
		:FDWBackend(nfds_), sr(0)
	{
		if ((epfd = epoll_create(nfds)) < 0)
			throw InitError("failed to open epoll device");
		epevents.reset(new struct epoll_event[nfds]);

		std::memset(epevents.get(), 0, sizeof(struct epoll_event) * nfds);

		INFO1("fdwatch epoll() based layer initialized (max %d sockets)", nfds);
	}

	FDWEpollBackend::~FDWEpollBackend() throw()
	{}

	int
		FDWEpollBackend::add(int idx, unsigned rw)
	{
			//    eventlog(eventlog_level_trace, __FUNCTION__, "called fd: %d rw: %d", fd, rw);

			struct epoll_event tmpev;
			std::memset(&tmpev, 0, sizeof(tmpev));
			tmpev.events = 0;
			if (rw & fdwatch_type_read)
				tmpev.events |= EPOLLIN;
			if (rw & fdwatch_type_write)
				tmpev.events |= EPOLLOUT;

			int op = fdw_rw(fdw_fds + idx) ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

			tmpev.data.fd = idx;
			if (epoll_ctl(epfd, op, fdw_fd(fdw_fds + idx), &tmpev)) {
				ERROR0("got error from epoll_ctl()");
				return -1;
			}

			return 0;
		}

	int
		FDWEpollBackend::del(int idx)
	{
			//    eventlog(eventlog_level_trace, __FUNCTION__, "called fd: %d", fd);
			if (sr > 0)
				ERROR0("BUG: called while still handling sockets");

			if (fdw_rw(fdw_fds + idx)) {
				struct epoll_event tmpev;
				std::memset(&tmpev, 0, sizeof(tmpev));
				tmpev.events = 0;
				tmpev.data.fd = idx;
				if (epoll_ctl(epfd, EPOLL_CTL_DEL, fdw_fd(fdw_fds + idx), &tmpev)) {
					ERROR0("got error from epoll_ctl()");
					return -1;
				}
			}

			return 0;
		}

	int
		FDWEpollBackend::watch(long timeout_msec)
	{
			return (sr = epoll_wait(epfd, epevents.get(), fdw_maxcons, timeout_msec));
		}

	void
		FDWEpollBackend::handle()
	{
			//    eventlog(eventlog_level_trace, __FUNCTION__, "called");
			for (struct epoll_event *ev = epevents.get(); sr; sr--, ev++)
			{
				//      eventlog(eventlog_level_trace, __FUNCTION__, "checking %d ident: %d read: %d write: %d", i, kqevents[i].ident, kqevents[i].filter & EVFILT_READ, kqevents[i].filter & EVFILT_WRITE);
				t_fdwatch_fd *cfd = fdw_fds + ev->data.fd;

				if (fdw_rw(cfd) & fdwatch_type_read && ev->events & (EPOLLIN | EPOLLERR | EPOLLHUP))
				if (fdw_hnd(cfd) (fdw_data(cfd), fdwatch_type_read) == -2)
					continue;

				if (fdw_rw(cfd) & fdwatch_type_write && ev->events & (EPOLLOUT | EPOLLERR | EPOLLHUP))
					fdw_hnd(cfd) (fdw_data(cfd), fdwatch_type_write);
			}
			sr = 0;
		}

}

#endif				/* HAVE_EPOLL */
