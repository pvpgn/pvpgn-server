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

#include "common/setup_before.h"
#include "fdwatch.h"

#include <cstring>

#include "common/eventlog.h"
#include "common/rlimit.h"
#include "fdwatch_select.h"
#include "fdwatch_poll.h"
#include "fdwatch_kqueue.h"
#include "fdwatch_epoll.h"
#include "fdwbackend.h"
#include "common/setup_after.h"

namespace pvpgn
{

	unsigned fdw_maxcons;
	t_fdwatch_fd *fdw_fds = NULL;

	namespace
	{

		FDWBackend * fdw = NULL;
		typedef elist<t_fdwatch_fd> FDWList;
		FDWList freelist(&t_fdwatch_fd::freelist);
		FDWList uselist(&t_fdwatch_fd::uselist);

	}

	extern int fdwatch_init(int maxcons)
	{
		unsigned i;
		int maxsys;

		maxsys = get_socket_limit();
		if (maxsys > 0) maxcons = (maxcons < maxsys) ? maxcons : maxsys;
		if (maxcons < 32) {
			eventlog(eventlog_level_fatal, __FUNCTION__, "too few sockets available (%d)", maxcons);
			return -1;
		}
		fdw_maxcons = maxcons;

		fdw_fds = new t_fdwatch_fd[fdw_maxcons];
		/* add all slots to the freelist */
		for (i = 0; i < fdw_maxcons; i++)
			freelist.push_back(fdw_fds[i]);

#ifdef HAVE_EPOLL
		try {
			fdw = new FDWEpollBackend(fdw_maxcons);
			return 0;
		}
		catch (const FDWBackend::InitError&) {
		}
#endif
#ifdef HAVE_KQUEUE
		try {
			fdw = new FDWKqueueBackend(fdw_maxcons);
			return 0;
		}
		catch (const FDWBackend::InitError&) {
		}
#endif
#ifdef HAVE_POLL
		try {
			fdw = new FDWPollBackend(fdw_maxcons);
			return 0;
		}
		catch (const FDWBackend::InitError&) {
		}
#endif
#ifdef HAVE_SELECT
		try {
			fdw = new FDWSelectBackend(fdw_maxcons);
			return 0;
		}
		catch (const FDWBackend::InitError&) {
		}
#endif

		FATAL0("Found no working fdwatch layer");
		fdw = NULL;
		fdwatch_close();
		return -1;
	}

	extern int fdwatch_close(void)
	{
		if (fdw) { delete fdw; fdw = NULL; }
		if (fdw_fds) { delete[] fdw_fds; fdw_fds = NULL; }
		freelist.clear();
		uselist.clear();

		return 0;
	}

	extern int fdwatch_add_fd(int fd, unsigned rw, fdwatch_handler h, void *data)
	{
		/* max sockets reached */
		if (freelist.empty()) return -1;

		t_fdwatch_fd *cfd = &freelist.front();
		fdw_fd(cfd) = fd;

		if (fdw->add(fdw_idx(cfd), rw)) return -1;

		/* add it to used sockets list, remove it from free list */
		uselist.push_back(*cfd);
		freelist.remove(*cfd);

		fdw_rw(cfd) = rw;
		fdw_data(cfd) = data;
		fdw_hnd(cfd) = h;

		return fdw_idx(cfd);
	}

	extern int fdwatch_update_fd(int idx, unsigned rw)
	{
		if (idx < 0 || idx >= fdw_maxcons) {
			ERROR2("out of bounds idx [%d] (max: %d)", idx, fdw_maxcons);
			return -1;
		}
		/* do not allow completly reset the access because then backend codes
		 * can get confused */
		if (!rw) {
			ERROR0("tried to reset rw, not allowed");
			return -1;
		}

		if (!fdw_rw(fdw_fds + idx)) {
			ERROR0("found reseted rw");
			return -1;
		}

		if (fdw->add(idx, rw)) return -1;
		fdw_rw(&fdw_fds[idx]) = rw;

		return 0;
	}

	extern int fdwatch_del_fd(int idx)
	{
		if (idx < 0 || idx >= fdw_maxcons) {
			ERROR2("out of bounds idx [%d] (max: %d)", idx, fdw_maxcons);
			return -1;
		}

		t_fdwatch_fd *cfd = fdw_fds + idx;
		if (!fdw_rw(cfd)) {
			ERROR0("found reseted rw");
			return -1;
		}

		fdw->del(idx);

		/* remove it from uselist, add it to freelist */
		uselist.remove(*cfd);
		freelist.push_back(*cfd);

		fdw_fd(cfd) = 0;
		fdw_rw(cfd) = 0;
		fdw_data(cfd) = NULL;
		fdw_hnd(cfd) = NULL;

		return 0;
	}

	extern int fdwatch(long timeout_msec)
	{
		return fdw->watch(timeout_msec);
	}

	extern void fdwatch_handle(void)
	{
		fdw->handle();
	}

	extern void fdwatch_traverse(t_fdw_cb cb, void *data)
	{
		for (FDWList::iterator it(uselist.begin()); it != uselist.end(); ++it) {
			if (cb(&*it, data)) break;
		}
	}

}
