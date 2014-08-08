/*
  * Abstraction API/layer for the various ways PvPGN can inspect sockets state
  * 2003 (C)
  *
  * Code is based on the ideas found in thttpd project.
  *
  * *BSD kqueue(2) based backend
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
#ifdef HAVE_KQUEUE
#include "fdwatch_kqueue.h"

#include <cstring>
#include <stdint.h>

#include "common/eventlog.h"
#include "fdwatch.h"
#include "common/setup_after.h"

namespace pvpgn
{

	FDWKqueueBackend::FDWKqueueBackend(int nfds_)
		:FDWBackend(nfds_), sr(0), nochanges(0)
	{
		if ((kq = kqueue()) == -1)
			throw InitError("error from kqueue()");

		kqevents.reset(new struct kevent[nfds]);
		kqchanges.reset(new struct kevent[nfds * 2]);
		rridx.reset(new int[nfds]);
		wridx.reset(new int[nfds]);

		std::memset(kqchanges.get(), 0, sizeof(struct kevent) * nfds);
		for (int i = 0; i < nfds; i++)
		{
			rridx[i] = -1;
			wridx[i] = -1;
		}

		INFO1("fdwatch kqueue() based layer initialized (max %d sockets)", nfds);
	}

	FDWKqueueBackend::~FDWKqueueBackend() throw()
	{}

	int
		FDWKqueueBackend::add(int idx, unsigned rw)
	{
			int idxr;
			t_fdwatch_fd *cfd = fdw_fds + idx;

			/*    eventlog(eventlog_level_trace, __FUNCTION__, "called fd: %d rw: %d", fd, rw); */
			/* adding read event filter */
			if (!(fdw_rw(cfd) & fdwatch_type_read) && rw & fdwatch_type_read)
			{
				if (rridx[idx] >= 0 && rridx[idx] < nochanges && kqchanges[rridx[idx]].ident == fdw_fd(cfd))
				{
					idxr = rridx[idx];
					/*	    eventlog(eventlog_level_trace, __FUNCTION__, "updating change event (read) fd on %d", ridx); */
				}
				else {
					idxr = nochanges++;
					rridx[idx] = idxr;
					/*	    eventlog(eventlog_level_trace, __FUNCTION__, "adding new change event (read) fd on %d", ridx); */
				}
				EV_SET(kqchanges.get() + idxr, fdw_fd(cfd), EVFILT_READ, EV_ADD, 0, 0, (void*)idx);
			}
			else if (fdw_rw(cfd) & fdwatch_type_read && !(rw & fdwatch_type_read))
			{
				if (rridx[idx] >= 0 && rridx[idx] < nochanges && kqchanges[rridx[idx]].ident == fdw_fd(cfd))
				{
					idxr = rridx[idx];
					/*	    eventlog(eventlog_level_trace, __FUNCTION__, "updating change event (read) fd on %d", ridx); */
				}
				else {
					idxr = nochanges++;
					rridx[idx] = idxr;
					/*	    eventlog(eventlog_level_trace, __FUNCTION__, "adding new change event (read) fd on %d", ridx); */
				}
				EV_SET(kqchanges.get() + idxr, fdw_fd(cfd), EVFILT_READ, EV_DELETE, 0, 0, (void*)idx);
			}

			/* adding write event filter */
			if (!(fdw_rw(cfd) & fdwatch_type_write) && rw & fdwatch_type_write)
			{
				if (wridx[idx] >= 0 && wridx[idx] < nochanges && kqchanges[wridx[idx]].ident == fdw_fd(cfd))
				{
					idxr = wridx[idx];
					/*	    eventlog(eventlog_level_trace, __FUNCTION__, "updating change event (write) fd on %d", ridx); */
				}
				else {
					idxr = nochanges++;
					wridx[idx] = idxr;
					/*	    eventlog(eventlog_level_trace, __FUNCTION__, "adding new change event (write) fd on %d", ridx); */
				}
				EV_SET(kqchanges.get() + idxr, fdw_fd(cfd), EVFILT_WRITE, EV_ADD, 0, 0, (void*)idx);
			}
			else if (fdw_rw(cfd) & fdwatch_type_write && !(rw & fdwatch_type_write))
			{
				if (wridx[idx] >= 0 && wridx[idx] < nochanges && kqchanges[wridx[idx]].ident == fdw_fd(cfd))
				{
					idxr = wridx[idx];
					/*	    eventlog(eventlog_level_trace, __FUNCTION__, "updating change event (write) fd on %d", ridx); */
				}
				else {
					idxr = nochanges++;
					wridx[idx] = idxr;
					/*	    eventlog(eventlog_level_trace, __FUNCTION__, "adding new change event (write) fd on %d", ridx); */
				}
				EV_SET(kqchanges.get() + idxr, fdw_fd(cfd), EVFILT_WRITE, EV_DELETE, 0, 0, (void*)idx);
			}

			return 0;
		}

	int
		FDWKqueueBackend::del(int idx)
	{
			/*    eventlog(eventlog_level_trace, __FUNCTION__, "called fd: %d", fd); */
			if (sr > 0)
				eventlog(eventlog_level_error, __FUNCTION__, "BUG: called while still handling sockets");

			t_fdwatch_fd *cfd = fdw_fds + idx;
			/* the last event changes about this fd has not yet been sent to kernel */
			if (fdw_rw(cfd) & fdwatch_type_read &&
				nochanges && rridx[idx] >= 0 && rridx[idx] < nochanges &&
				kqchanges[rridx[idx]].ident == fdw_fd(cfd))
			{
				nochanges--;
				if (rridx[idx] < nochanges)
				{
					intptr_t oidx;

					oidx = (intptr_t)(kqchanges[nochanges].udata);
					if (kqchanges[nochanges].filter == EVFILT_READ &&
						rridx[oidx] == nochanges)
					{
						/*	    eventlog(eventlog_level_trace, __FUNCTION__, "not last, moving %d", kqchanges[rnfds].ident); */
						rridx[oidx] = rridx[idx];
						std::memcpy(kqchanges.get() + rridx[idx], kqchanges.get() + nochanges, sizeof(struct kevent));
					}

					if (kqchanges[nochanges].filter == EVFILT_WRITE &&
						wridx[oidx] == nochanges)
					{
						/*	    eventlog(eventlog_level_trace, __FUNCTION__, "not last, moving %d", kqchanges[rnfds].ident); */
						wridx[oidx] = rridx[idx];
						std::memcpy(kqchanges.get() + rridx[idx], kqchanges.get() + nochanges, sizeof(struct kevent));
					}
				}
				rridx[idx] = -1;
			}

			if (fdw_rw(cfd) & fdwatch_type_write &&
				nochanges && wridx[idx] >= 0 && wridx[idx] < nochanges &&
				kqchanges[wridx[idx]].ident == fdw_fd(cfd))
			{
				nochanges--;
				if (wridx[idx] < nochanges)
				{
					intptr_t oidx;

					oidx = (intptr_t)(kqchanges[nochanges].udata);
					if (kqchanges[nochanges].filter == EVFILT_READ &&
						rridx[oidx] == nochanges)
					{
						/*	    eventlog(eventlog_level_trace, __FUNCTION__, "not last, moving %d", kqchanges[rnfds].ident); */
						rridx[oidx] = wridx[idx];
						std::memcpy(kqchanges.get() + wridx[idx], kqchanges.get() + nochanges, sizeof(struct kevent));
					}

					if (kqchanges[nochanges].filter == EVFILT_WRITE &&
						wridx[oidx] == nochanges)
					{
						/*	    eventlog(eventlog_level_trace, __FUNCTION__, "not last, moving %d", kqchanges[rnfds].ident); */
						wridx[oidx] = wridx[idx];
						std::memcpy(kqchanges.get() + wridx[idx], kqchanges.get() + nochanges, sizeof(struct kevent));
					}
				}
				wridx[idx] = -1;
			}

			/* here we presume the calling code does close() on the socket and if so
			 * it is automatically removed from any kernel kqueues */

			return 0;
		}

	int
		FDWKqueueBackend::watch(long timeout_msec)
	{
			static struct timespec ts;

			ts.tv_sec = timeout_msec / 1000L;
			ts.tv_nsec = (timeout_msec % 1000L) * 1000000L;
			sr = kevent(kq, nochanges > 0 ? kqchanges.get() : NULL, nochanges, kqevents.get(), fdw_maxcons, &ts);
			nochanges = 0;
			return sr;
		}

	void
		FDWKqueueBackend::handle()
	{
			/*    eventlog(eventlog_level_trace, __FUNCTION__, "called"); */
			for (unsigned i = 0; i < sr; i++)
			{
				/*      eventlog(eventlog_level_trace, __FUNCTION__, "checking %d ident: %d read: %d write: %d", i, kqevents[i].ident, kqevents[i].filter & EVFILT_READ, kqevents[i].filter & EVFILT_WRITE); */
				t_fdwatch_fd *cfd = fdw_fds + (intptr_t)kqevents[i].udata;
				if (fdw_rw(cfd) & fdwatch_type_read && kqevents[i].filter == EVFILT_READ)
				if (fdw_hnd(cfd) (fdw_data(cfd), fdwatch_type_read) == -2)
					continue;

				if (fdw_rw(cfd) & fdwatch_type_write && kqevents[i].filter == EVFILT_WRITE)
					fdw_hnd(cfd) (fdw_data(cfd), fdwatch_type_write);
			}
			sr = 0;
		}

}

#endif				/* HAVE_KQUEUE */
