/*
  * Abstraction API/layer for the various ways PvPGN can inspect sockets state
  * 2003 (C)
  *
  * Code is based on the ideas found in thttpd project.
  *
  * poll(2) based backend
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
#include "fdwatch_poll.h"

#include <cstring>

#include "common/eventlog.h"
#include "fdwatch.h"
#include "common/setup_after.h"

#ifdef HAVE_POLL
namespace pvpgn
{

	FDWPollBackend::FDWPollBackend(int nfds_)
		:FDWBackend(nfds_), sr(0), fds(new struct pollfd[nfds]), rridx(new int[nfds]), ridx(new int[nfds]), nofds(0)
	{
		std::memset(fds.get(), 0, sizeof(struct pollfd) * nfds);
		std::memset(rridx.get(), 0, sizeof(int)* nfds);
		/* I would use a memset with 255 but that is dirty and doesnt gain us anything */
		for (int i = 0; i < nfds; i++) ridx[i] = -1;

		INFO1("fdwatch poll() based layer initialized (max %d sockets)", nfds);
	}

	FDWPollBackend::~FDWPollBackend() throw()
	{}

	int
		FDWPollBackend::add(int idx, unsigned rw)
	{
			int idxr;

			//    eventlog(eventlog_level_trace, __FUNCTION__, "called fd: %d rw: %d", fd, rw);
			if (ridx[idx] < 0) {
				idxr = nofds++;
				fds[idxr].fd = fdw_fd(fdw_fds + idx);
				ridx[idx] = idxr;
				rridx[idxr] = idx;
				//	eventlog(eventlog_level_trace, __FUNCTION__, "adding new fd on %d", ridx);
			}
			else {
				if (fds[ridx[idx]].fd != fdw_fd(fdw_fds + idx)) {
					ERROR0("BUG: found existent poll_fd entry for same idx with different fd");
					return -1;
				}
				idxr = ridx[idx];
				//	eventlog(eventlog_level_trace, __FUNCTION__, "updating fd on %d", ridx);
			}

			fds[idxr].events = 0;
			if (rw & fdwatch_type_read) fds[idxr].events |= POLLIN;
			if (rw & fdwatch_type_write) fds[idxr].events |= POLLOUT;

			return 0;
		}

	int
		FDWPollBackend::del(int idx)
	{
			//    eventlog(eventlog_level_trace, __FUNCTION__, "called fd: %d", fd);
			if (ridx[idx] < 0 || !nofds) return -1;
			if (sr > 0)
				ERROR0("BUG: called while still handling sockets");

			/* move the last entry to the deleted one and decrement nofds count */
			nofds--;
			if (ridx[idx] < nofds) {
				//	eventlog(eventlog_level_trace, __FUNCTION__, "not last, moving %d", tfds[nofds].fd);
				ridx[rridx[nofds]] = ridx[idx];
				rridx[ridx[idx]] = rridx[nofds];
				std::memcpy(fds.get() + ridx[idx], fds.get() + nofds, sizeof(struct pollfd));
			}
			ridx[idx] = -1;

			return 0;
		}

	int
		FDWPollBackend::watch(long timeout_msec)
	{
			return (sr = poll(fds.get(), nofds, timeout_msec));
		}

	void
		FDWPollBackend::handle()
	{
			for (unsigned i = 0; i < nofds && sr; i++) {
				bool changed = false;
				t_fdwatch_fd *cfd = fdw_fds + rridx[i];

				if (fdw_rw(cfd) & fdwatch_type_read &&
					fds[i].revents & (POLLIN | POLLERR | POLLHUP | POLLNVAL))
				{
					if (fdw_hnd(cfd)(fdw_data(cfd), fdwatch_type_read) == -2) {
						sr--;
						continue;
					}
					changed = true;
				}

				if (fdw_rw(cfd) & fdwatch_type_write &&
					fds[i].revents & (POLLOUT | POLLERR | POLLHUP | POLLNVAL))
				{
					fdw_hnd(cfd)(fdw_data(cfd), fdwatch_type_write);
					changed = true;
				}

				if (changed) sr--;
			}
		}

}

#endif /* HAVE_POLL */
