/*
 * Copyright (C) 1999  Philippe Dubois (pdubois1@hotmail.com)
 * Copyright (C) 1999  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifdef WIN32

#include "psock.h"
#include "common/setup_after.h"


extern int psock_init(void)
{
	WORD    wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);

	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		/* FIXME: Tell the user that we could not find a usable WinSock DLL. */
		return -1;
	}

	return 0;
}

extern int psock_deinit(void)
{

	if (WSACleanup() != 0)
	{
		return -1;
	}

	return 0;
}

extern int psock_ctl(int sd, int mode)
{
	unsigned long nParam = 1; /* u_long or DWORD or what? */

	return ioctlsocket(sd, mode, &nParam);
}

#else

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#else
# ifdef HAVE_SYS_FILE_H
#  include <sys/file.h>
# endif
#endif
#include "psock.h"
#include "common/setup_after.h"


extern int psock_ctl(int sd, long int mode)
{
	long int oldmode;

	if ((oldmode = fcntl(sd, F_GETFL)) < 0)
		oldmode = 0;
	oldmode |= mode;
	return fcntl(sd, F_SETFL, mode);
}

#endif
