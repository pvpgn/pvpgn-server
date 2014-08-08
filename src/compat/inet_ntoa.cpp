/*
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
#ifndef HAVE_INET_NTOA
#include "inet_ntoa.h"

#include <cstdio>
#include <cstddef>

#ifdef HAVE_SYS_TYPES
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_WINSOCK2_H
# include <winsock2.h>
#endif
#include "common/setup_after.h"


namespace pvpgn
{

	extern char const * inet_ntoa(struct in_addr const * addr)
	{
		static char   buff[16];
		unsigned long val;

		if (!addr)
			return NULL;

		val = ntohl(addr->s_addr);
		std::sprintf(buff, "%u.%u.%u.%u",
			(val >> 24) & 0xff,
			(val >> 16) & 0xff,
			(val >> 8) & 0xff,
			(val)& 0xff);
		return buff;
	}

}

#else
typedef int filenotempty; /* make ISO standard happy */
#endif
