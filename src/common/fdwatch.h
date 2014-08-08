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

#ifndef __FDWATCH_INCLUDED__
#define __FDWATCH_INCLUDED__

#include "common/elist.h"

namespace pvpgn
{

	typedef enum {
		fdwatch_type_none = 0,
		fdwatch_type_read = 1,
		fdwatch_type_write = 2
	} t_fdwatch_type;

	typedef int(*fdwatch_handler)(void *data, t_fdwatch_type);

	struct t_fdwatch_fd {
		int fd;
		int rw;
		fdwatch_handler hnd;
		void *data;

		elist_node<t_fdwatch_fd> uselist;
		elist_node<t_fdwatch_fd> freelist;

		t_fdwatch_fd() :fd(0), rw(0), hnd(0), data(0), uselist(), freelist() {}
	};

	typedef int(*t_fdw_cb)(t_fdwatch_fd *cfd, void *data);

	extern unsigned fdw_maxcons;
	extern t_fdwatch_fd *fdw_fds;

#define fdw_idx(ptr) ((ptr) - fdw_fds)
#define fdw_fd(ptr) ((ptr)->fd)
#define fdw_rw(ptr) ((ptr)->rw)
#define fdw_data(ptr) ((ptr)->data)
#define fdw_hnd(ptr) ((ptr)->hnd)
	extern int fdwatch_init(int maxcons);
	extern int fdwatch_close(void);
	extern int fdwatch_add_fd(int fd, unsigned rw, fdwatch_handler h, void *data);
	extern int fdwatch_update_fd(int idx, unsigned rw);
	extern int fdwatch_del_fd(int idx);
	extern int fdwatch(long timeout_msecs);
	extern void fdwatch_handle(void);
	extern void fdwatch_traverse(t_fdw_cb cb, void *data);

}

#endif /* __FDWATCH_INCLUDED__ */
