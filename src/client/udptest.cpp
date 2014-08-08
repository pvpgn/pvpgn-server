/*
 * Copyright (C) 1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#include "udptest.h"

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <ctime>

#include "compat/psock.h"
#include "common/packet.h"
#include "common/bn_type.h"
#include "common/tag.h"
#include "common/setup_after.h"


#ifdef CLIENTDEBUG
#define dprintf printf
#else
#define dprintf if (0) printf
#endif


namespace pvpgn
{

	namespace client
	{

		extern int client_udptest_setup(char const * progname, unsigned short * lsock_port_ret)
		{
			int                lsock;
			struct sockaddr_in laddr;
			unsigned short     lsock_port;

			if (!progname)
			{
				std::fprintf(stderr, "got NULL progname\n");
				return -1;
			}

			if ((lsock = psock_socket(PF_INET, SOCK_DGRAM, PSOCK_IPPROTO_UDP)) < 0)
			{
				std::fprintf(stderr, "%s: could not create UDP socket (psock_socket: %s)\n", progname, std::strerror(psock_errno()));
				return -1;
			}

			if (psock_ctl(lsock, PSOCK_NONBLOCK) < 0)
				std::fprintf(stderr, "%s: could not set UDP socket to non-blocking mode (psock_ctl: %s)\n", progname, std::strerror(psock_errno()));

			for (lsock_port = BNETD_MIN_TEST_PORT; lsock_port <= BNETD_MAX_TEST_PORT; lsock_port++)
			{
				std::memset(&laddr, 0, sizeof(laddr));
				laddr.sin_family = PSOCK_AF_INET;
				laddr.sin_port = htons(lsock_port);
				laddr.sin_addr.s_addr = htonl(INADDR_ANY);
				if (psock_bind(lsock, (struct sockaddr *)&laddr, (psock_t_socklen)sizeof(laddr)) == 0)
					break;

				if (lsock_port == BNETD_MIN_TEST_PORT)
					dprintf("Could not bind to standard UDP port %hu, trying others. (psock_bind: %s)\n", BNETD_MIN_TEST_PORT, std::strerror(psock_errno()));
			}
			if (lsock_port > BNETD_MAX_TEST_PORT)
			{
				std::fprintf(stderr, "%s: could not bind to any UDP port %hu through %hu (psock_bind: %s)\n", progname, BNETD_MIN_TEST_PORT, BNETD_MAX_TEST_PORT, std::strerror(psock_errno()));
				psock_close(lsock);
				return -1;
			}

			if (lsock_port_ret)
				*lsock_port_ret = lsock_port;

			return lsock;
		}


		extern int client_udptest_recv(char const * progname, int lsock, unsigned short lsock_port, unsigned int timeout)
		{
			int          len;
			unsigned int count;
			t_packet *   rpacket;
			std::time_t       start;

			if (!progname)
			{
				std::fprintf(stderr, "%s: got NULL progname\n", progname);
				return -1;
			}

			if (!(rpacket = packet_create(packet_class_bnet)))
			{
				std::fprintf(stderr, "%s: could not create packet\n", progname);
				return -1;
			}

			start = std::time(NULL);
			count = 0;
			while (start + (std::time_t)timeout >= std::time(NULL))  /* timeout after a few seconds from last packet */
			{
				if ((len = psock_recv(lsock, packet_get_raw_data_build(rpacket, 0), MAX_PACKET_SIZE, 0)) < 0)
				{
					if (psock_errno() != PSOCK_EAGAIN && psock_errno() != PSOCK_EWOULDBLOCK)
						std::fprintf(stderr, "%s: failed to receive UDPTEST on port %hu (psock_recv: %s)\n", progname, lsock_port, std::strerror(psock_errno()));
					continue;
				}
				packet_set_size(rpacket, len);

				if (packet_get_type(rpacket) != SERVER_UDPTEST)
				{
					dprintf("Got unexpected UDP packet type %u on port %hu\n", packet_get_type(rpacket), lsock_port);
					continue;
				}

				if (bn_int_tag_eq(rpacket->u.server_udptest.bnettag, BNETTAG) < 0)
				{
					std::fprintf(stderr, "%s: got bad UDPTEST packet on port %hu\n", progname, lsock_port);
					continue;
				}

				count++;
				if (count >= 2)
					break;

				start = std::time(NULL);
			}

			packet_destroy(rpacket);

			if (count < 2)
			{
				std::printf("Only received %d UDP packets on port %hu. Connection may be slow or firewalled.\n", count, lsock_port);
				return -1;
			}

			return 0;
		}

	}

}
