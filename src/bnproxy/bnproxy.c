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
#include <stdio.h>
#ifdef HAVE_STDDEF_H
# include <stddef.h>
#else
# ifndef NULL
#  define NULL ((void *)0)
# endif
#endif
#ifdef STDC_HEADERS
# include <stdlib.h>
#endif
#include "compat/exitstatus.h"
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#include "compat/stdfileno.h"
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#else
# ifdef HAVE_SYS_FILE_H
#  include <sys/file.h>
# endif
#endif
#ifdef HAVE_STRING_H
# include <string.h>
#else
# ifdef HAVE_STRINGS_H
#  include <strings.h>
# endif
#endif
#ifdef HAVE_MEMORY_H
# include <memory.h>
#endif
#include "compat/memset.h"
#include "compat/memcpy.h"
#include <ctype.h>
#include <errno.h>
#include "compat/strerror.h"
#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_SELECT_H
# include <sys/select.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#include "compat/socket.h"
#include "compat/recv.h"
#ifdef HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#include "compat/netinet_in.h"
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#include "compat/inet_ntoa.h"
#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif
#include "compat/psock.h"
#include "common/packet.h"
#include "common/init_protocol.h"
#include "common/hexdump.h"
#include "common/eventlog.h"
#include "common/bn_type.h"
#include "common/queue.h"
#include "common/network.h"
#include "common/list.h"
#include "common/util.h"
#include "virtconn.h"
#include "common/version.h"
#include "common/setup_after.h"

FILE * hexstrm = NULL;

/* FIXME: This code is horribly unreadable. The UDP stuff is a hack for now. */

#define PROXY_FLAG_UDP 1


static int init_virtconn(t_virtconn * vc, struct sockaddr_in servaddr);
static int proxy_process(unsigned short server_listen_port, struct sockaddr_in servaddr);
static void usage(char const * progname);


static int init_virtconn(t_virtconn * vc, struct sockaddr_in servaddr)
{
	int  addlen;
	char connect_type;

	/* determine connection type by first character sent by client */
	addlen = psock_recv(virtconn_get_client_socket(vc), &connect_type, sizeof(char), 0);

	if (addlen < 0 && (psock_errno() == PSOCK_EINTR || psock_errno() == PSOCK_EAGAIN || psock_errno() == PSOCK_EWOULDBLOCK))
		return 0;

	/* error occurred or connection lost */
	if (addlen < 1)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not get virtconn class (closing connection) (psock_recv: %s)", virtconn_get_client_socket(vc), pstrerror(psock_errno()));
		return -1;
	}

	switch (connect_type)
	{
	case CLIENT_INITCONN_CLASS_BNET:
		eventlog(eventlog_level_info, __FUNCTION__, "[%d] client initiated normal connection", virtconn_get_client_socket(vc));
		virtconn_set_class(vc, virtconn_class_bnet);

		break;

	case CLIENT_INITCONN_CLASS_FILE:
		eventlog(eventlog_level_info, __FUNCTION__, "[%d] client initiated file download connection", virtconn_get_client_socket(vc));
		virtconn_set_class(vc, virtconn_class_file);

		break;

	case CLIENT_INITCONN_CLASS_BOT:
		eventlog(eventlog_level_info, __FUNCTION__, "[%d] client initiated chat bot connection", virtconn_get_client_socket(vc));
		virtconn_set_class(vc, virtconn_class_bot);

		break;

	default:
		eventlog(eventlog_level_error, __FUNCTION__, "[%d] client initiated unknown connection type 0x%02hx (length %d) (closing connection)", virtconn_get_client_socket(vc), (unsigned short)connect_type, addlen);
		return -1;
	}

	/* now connect to the real server */
	if (psock_connect(virtconn_get_server_socket(vc), (struct sockaddr *)&servaddr, (psock_t_socklen)sizeof(servaddr)) < 0)
	{
		if (psock_errno() != PSOCK_EINPROGRESS)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not connect to server (psock_connect: %s)\n", virtconn_get_client_socket(vc), pstrerror(psock_errno()));
			return -1;
		}
		virtconn_set_state(vc, virtconn_state_connecting);
	}
	else
		virtconn_set_state(vc, virtconn_state_connected);

	{
		t_packet * packet;

		if (!(packet = packet_create(packet_class_raw)))
		{
			eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not create packet", virtconn_get_client_socket(vc));
			return -1;
		}
		packet_append_data(packet, &connect_type, 1);
		queue_push_packet(virtconn_get_serverout_queue(vc), packet);
		packet_del_ref(packet);
	}

	return 0;
}


static int proxy_process(unsigned short server_listen_port, struct sockaddr_in servaddr)
{
	int                lsock;
	struct sockaddr_in laddr;
	t_psock_fd_set     rfds, wfds;
	int                highest_fd;
	int                udpsock;
	t_virtconn *       vc;
	t_elem const *     curr;
	int                csocket;
	int                ssocket;

	if ((udpsock = psock_socket(PSOCK_PF_INET, PSOCK_SOCK_DGRAM, PSOCK_IPPROTO_UDP)) < 0)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "could not create UDP socket (psock_socket: %s)", pstrerror(psock_errno()));
		return -1;
	}
	if (psock_ctl(udpsock, PSOCK_NONBLOCK) < 0)
		eventlog(eventlog_level_error, __FUNCTION__, "could not set UDP listen socket to non-blocking mode (psock_ctl: %s)", pstrerror(psock_errno()));

	if ((lsock = psock_socket(PSOCK_PF_INET, PSOCK_SOCK_STREAM, PSOCK_IPPROTO_TCP)) < 0)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "could not create listening socket (psock_socket: %s)", pstrerror(psock_errno()));
		psock_close(udpsock);
		return -1;
	}

	{
		int val = 1;

		if (psock_setsockopt(lsock, PSOCK_SOL_SOCKET, PSOCK_SO_REUSEADDR, &val, (psock_t_socklen)sizeof(int)) < 0)
			eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not set socket option SO_REUSEADDR (psock_setsockopt: %s)", lsock, pstrerror(psock_errno()));
		/* not a fatal error... */
	}

	memset(&laddr, 0, sizeof(laddr));
	laddr.sin_family = PSOCK_AF_INET;
	laddr.sin_port = htons(server_listen_port);
	laddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (psock_bind(lsock, (struct sockaddr *)&laddr, (psock_t_socklen)sizeof(laddr)) < 0)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "could not bind socket to address 0.0.0.0:%hu TCP (psock_bind: %s)", server_listen_port, pstrerror(psock_errno()));
		psock_close(udpsock);
		psock_close(lsock);
		return -1;
	}

	memset(&laddr, 0, sizeof(laddr));
	laddr.sin_family = PSOCK_AF_INET;
	laddr.sin_port = htons(server_listen_port);
	laddr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (psock_bind(udpsock, (struct sockaddr *)&laddr, (psock_t_socklen)sizeof(laddr)) < 0)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "could not bind socket to address 0.0.0.0:%hu UDP (psock_bind: %s)", server_listen_port, pstrerror(psock_errno()));
		psock_close(udpsock);
		psock_close(lsock);
		return -1;
	}
	eventlog(eventlog_level_info, __FUNCTION__, "bound to UDP port %hu", server_listen_port);

	/* tell socket to listen for connections */
	if (psock_listen(lsock, LISTEN_QUEUE) < 0)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "could not listen (psock_listen: %s)", pstrerror(psock_errno()));
		psock_close(udpsock);
		psock_close(lsock);
		return -1;
	}
	if (psock_ctl(lsock, PSOCK_NONBLOCK) < 0)
		eventlog(eventlog_level_error, __FUNCTION__, "could not set TCP listen socket to non-blocking mode (psock_ctl: %s)", pstrerror(psock_errno()));

	eventlog(eventlog_level_info, __FUNCTION__, "listening on TCP port %hu", server_listen_port);

	for (;;)
	{
		/* loop over all connections to create the sets for select() */
		PSOCK_FD_ZERO(&rfds);
		PSOCK_FD_ZERO(&wfds);
		highest_fd = lsock;
		PSOCK_FD_SET(lsock, &rfds);
		if (udpsock > highest_fd)
			highest_fd = udpsock;
		PSOCK_FD_SET(udpsock, &rfds);

		LIST_TRAVERSE_CONST(virtconnlist(), curr)
		{
			vc = elem_get_data(curr);
			csocket = virtconn_get_client_socket(vc);
			if (queue_get_length((t_queue const * const *)virtconn_get_clientout_queue(vc)) > 0)
				PSOCK_FD_SET(csocket, &wfds); /* pending output, also check for writeability */
			PSOCK_FD_SET(csocket, &rfds);

			if (csocket > highest_fd)
				highest_fd = csocket;

			switch (virtconn_get_state(vc))
			{
			case virtconn_state_connecting:
				eventlog(eventlog_level_debug, __FUNCTION__, "waiting for %d to finish connecting", ssocket);
				ssocket = virtconn_get_server_socket(vc);
				PSOCK_FD_SET(ssocket, &wfds); /* wait for connect to complete */

				if (ssocket > highest_fd)
					highest_fd = ssocket;
				break;
			case virtconn_state_connected:
				eventlog(eventlog_level_debug, __FUNCTION__, "checking for reading on connected socket %d", ssocket);
				ssocket = virtconn_get_server_socket(vc);
				if (queue_get_length((t_queue const * const *)virtconn_get_serverout_queue(vc)) > 0)
					PSOCK_FD_SET(ssocket, &wfds); /* pending output, also check for writeability */
				PSOCK_FD_SET(ssocket, &rfds);

				if (ssocket > highest_fd)
					highest_fd = ssocket;
				break;
			default: /* avoid warning */
				break;
			}
		}

		/* find which sockets need servicing */
		if (psock_select(highest_fd + 1, &rfds, &wfds, NULL, NULL) < 0)
		{
			if (errno != PSOCK_EINTR)
				eventlog(eventlog_level_error, __FUNCTION__, "select failed (select: %s)", pstrerror(errno));
			continue;
		}

		/* check for incoming connection */
		if (PSOCK_FD_ISSET(lsock, &rfds))
		{
			int                asock;
			struct sockaddr_in caddr;
			psock_t_socklen    caddr_len;

			/* accept the connection */
			caddr_len = sizeof(caddr);
			if ((asock = psock_accept(lsock, (struct sockaddr *)&caddr, &caddr_len)) < 0)
			{
				if (psock_errno() == PSOCK_EWOULDBLOCK || psock_errno() == PSOCK_ECONNABORTED) /* BSD, POSIX error for aborted connections, SYSV often uses EAGAIN */
					eventlog(eventlog_level_error, __FUNCTION__, "client aborted connection (psock_accept: %s)", pstrerror(psock_errno()));
				else /* EAGAIN can mean out of resources _or_ connection aborted */
				if (psock_errno() != PSOCK_EINTR)
					eventlog(eventlog_level_error, __FUNCTION__, "could not accept new connection (psock_accept: %s)", pstrerror(psock_errno()));
			}
			else
			{
				int ssd;
				int val = 1;

				eventlog(eventlog_level_info, __FUNCTION__, "[%d] accepted connection from %s:%hu", asock, inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));

				if (psock_setsockopt(asock, PSOCK_SOL_SOCKET, PSOCK_SO_KEEPALIVE, &val, (psock_t_socklen)sizeof(val)) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not set socket option SO_KEEPALIVE (psock_setsockopt: %s)", asock, pstrerror(psock_errno()));
				/* not a fatal error */

				if (psock_ctl(asock, PSOCK_NONBLOCK) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not set TCP socket to non-blocking mode (closing connection) (psock_ctl: %s)", asock, pstrerror(psock_errno()));
					psock_close(asock);
				}
				else
				if ((ssd = psock_socket(PSOCK_PF_INET, PSOCK_SOCK_STREAM, PSOCK_IPPROTO_TCP)) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] could create TCP socket (closing connection) (psock_socket: %s)", asock, pstrerror(psock_errno()));
					psock_close(asock);
				}
				else
				if (psock_ctl(ssd, PSOCK_NONBLOCK) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not set TCP socket to non-blocking mode (closing connection) (psock_ctl: %s)", asock, pstrerror(psock_errno()));
					psock_close(ssd);
					psock_close(asock);
				}
				else
				if (!(vc = virtconn_create(asock, ssd, ntohl(caddr.sin_addr.s_addr), BNETD_MIN_TEST_PORT)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] unable to create new connection (closing connection)", asock);
					psock_close(ssd);
					psock_close(asock);
				}
				else
				{
					memset(&caddr, 0, sizeof(caddr));
					caddr.sin_family = PSOCK_AF_INET;
					caddr.sin_port = htons(virtconn_get_udpport(vc));
					caddr.sin_addr.s_addr = htonl(virtconn_get_udpaddr(vc));
					eventlog(eventlog_level_info, __FUNCTION__, "[%d] addr now %s:%hu", asock, inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
				}
			}
		}

		eventlog(eventlog_level_debug, __FUNCTION__, "checking for incoming UDP");
		if (PSOCK_FD_ISSET(udpsock, &rfds))
		{
			t_packet *         upacket;
			struct sockaddr_in toaddr;
			struct sockaddr_in fromaddr;
			psock_t_socklen    fromlen;
			int                len;

			if (!(upacket = packet_create(packet_class_raw)))
				eventlog(eventlog_level_error, __FUNCTION__, "could not allocate raw packet for input");
			else
			{
				/* packet_set_flags(upacket,PROXY_FLAG_UDP);*/

				fromlen = sizeof(fromaddr);
				if ((len = psock_recvfrom(udpsock, packet_get_raw_data_build(upacket, 0), MAX_PACKET_SIZE, 0, (struct sockaddr *)&fromaddr, &fromlen)) < 0)
				{
					if (psock_errno() != PSOCK_EINTR && psock_errno() != PSOCK_EAGAIN && psock_errno() != PSOCK_EWOULDBLOCK)
						eventlog(eventlog_level_error, __FUNCTION__, "could not recv UDP datagram (psock_recvfrom: %s)", pstrerror(psock_errno()));
				}
				else
				{
					if (fromaddr.sin_family != PSOCK_AF_INET)
						eventlog(eventlog_level_error, __FUNCTION__, "got UDP datagram with bad address family %d", fromaddr.sin_family);
					else
					{
						char tempa[32];
						char tempb[32];

						packet_set_size(upacket, len);

						if (fromaddr.sin_addr.s_addr == servaddr.sin_addr.s_addr) /* from server */
						{
							if ((curr = list_get_first_const(virtconnlist()))) /* hack.. find proper client */
							{
								vc = elem_get_data(curr);
								memset(&toaddr, 0, sizeof(toaddr));
								toaddr.sin_family = PSOCK_AF_INET;
								toaddr.sin_port = htons(virtconn_get_udpport(vc));
								toaddr.sin_addr.s_addr = htonl(virtconn_get_udpaddr(vc));
								eventlog(eventlog_level_info, __FUNCTION__, "[%d] addr by UDP send is %s:%hu", virtconn_get_client_socket(vc), inet_ntoa(toaddr.sin_addr), ntohs(toaddr.sin_port));

								if (hexstrm)
								{
									strcpy(tempa, inet_ntoa(fromaddr.sin_addr));
									strcpy(tempb, inet_ntoa(toaddr.sin_addr));
									fprintf(hexstrm, "%d: srv prot=UDP from=%s:%hu to=%s:%hu length=%d\n",
										udpsock,
										tempa,
										ntohs(fromaddr.sin_port),
										tempb,
										ntohs(toaddr.sin_port),
										len);
									hexdump(hexstrm, packet_get_raw_data(upacket, 0), len);
								}

								/*queue_push_packet(virtconn_get_clientout_queue(__));*/ /* where to queue ... */
								for (;;) /* hack.. just block for now */
								{
									if (psock_sendto(udpsock, packet_get_raw_data_const(upacket, 0), len, 0,
										(struct sockaddr *)&toaddr, (psock_t_socklen)sizeof(toaddr)) < len)
									{
										if (psock_errno() == PSOCK_EINTR || psock_errno() == PSOCK_EAGAIN || psock_errno() == PSOCK_EWOULDBLOCK)
											continue;
										eventlog(eventlog_level_error, __FUNCTION__, "could not send UDP datagram to client (psock_sendto: %s)", pstrerror(psock_errno()));
									}
									break;
								}
							}
						}
						else /* from client */
						{
							if (hexstrm)
							{
								strcpy(tempa, inet_ntoa(fromaddr.sin_addr));
								strcpy(tempb, inet_ntoa(servaddr.sin_addr));

								fprintf(hexstrm, "%d: clt prot=UDP from=%s:%hu to=%s:%hu length=%d\n",
									udpsock,
									tempa,
									ntohs(fromaddr.sin_port),
									tempb,
									ntohs(servaddr.sin_port),
									len);
								hexdump(hexstrm, packet_get_raw_data(upacket, 0), len);
							}
							/*queue_push_packet(virtconn_get_serverout_queue(vc));*/
							for (;;) /* hack.. just block for now */
							{
								if (psock_sendto(udpsock, packet_get_raw_data_const(upacket, 0), len, 0,
									(struct sockaddr *)&servaddr, (psock_t_socklen)sizeof(servaddr)) < len)
								{
									if (psock_errno() == PSOCK_EINTR || psock_errno() == PSOCK_EAGAIN || psock_errno() == PSOCK_EWOULDBLOCK)
										continue;
									eventlog(eventlog_level_error, __FUNCTION__, "could not send UDP datagram to server (psock_sendto: %s)", pstrerror(psock_errno()));
								}
								break;
							}
						}
					}
				}
				packet_del_ref(upacket);
			}
		}

		/* search connections for sockets that need service */
		eventlog(eventlog_level_debug, __FUNCTION__, "checking for sockets that need service");
		LIST_TRAVERSE_CONST(virtconnlist(), curr)
		{
			unsigned int currsize;
			t_packet *   packet;

			vc = elem_get_data(curr);

			csocket = virtconn_get_client_socket(vc);
			if (virtconn_get_state(vc) == virtconn_state_connected ||
				virtconn_get_state(vc) == virtconn_state_connecting)
				ssocket = virtconn_get_server_socket(vc);
			else
				ssocket = -1;

			eventlog(eventlog_level_debug, __FUNCTION__, "checking %d for client readability", csocket);
			if (PSOCK_FD_ISSET(csocket, &rfds))
			{
				if (virtconn_get_state(vc) == virtconn_state_initial)
				{
					if (init_virtconn(vc, servaddr) < 0)
					{
						virtconn_destroy(vc);
						continue;
					}
				}
				else
				{
					currsize = virtconn_get_clientin_size(vc);

					if (!queue_get_length(virtconn_get_clientin_queue(vc)))
					{
						switch (virtconn_get_class(vc))
						{
						case virtconn_class_bnet:
							if (!(packet = packet_create(packet_class_bnet)))
							{
								eventlog(eventlog_level_error, __FUNCTION__, "could not allocate normal packet for input");
								continue;
							}
							break;
						case virtconn_class_file:
							if (!(packet = packet_create(packet_class_file)))
							{
								eventlog(eventlog_level_error, __FUNCTION__, "could not allocate file packet for input");
								continue;
							}
							break;
						case virtconn_class_bot:
							if (!(packet = packet_create(packet_class_raw)))
							{
								eventlog(eventlog_level_error, __FUNCTION__, "could not allocate raw packet for input");
								continue;
							}
							packet_set_size(packet, 1); /* start by only reading one char */
							break;
						default:
							eventlog(eventlog_level_error, __FUNCTION__, "[%d] connection has bad type (closing connection)", virtconn_get_client_socket(vc));
							virtconn_destroy(vc);
							continue;
						}
						queue_push_packet(virtconn_get_clientin_queue(vc), packet);
						packet_del_ref(packet);
						if (!queue_get_length(virtconn_get_clientin_queue(vc)))
							continue; /* push failed */
						currsize = 0;
					}

					packet = queue_peek_packet((t_queue const * const *)virtconn_get_clientin_queue(vc)); /* avoid warning */
					switch (net_recv_packet(csocket, packet, &currsize))
					{
					case -1:
						virtconn_destroy(vc);
						continue;

					case 0: /* still working on it */
						virtconn_set_clientin_size(vc, currsize);
						break;

					case 1: /* done reading */
						if (virtconn_get_class(vc) == virtconn_class_bot &&
							currsize < MAX_PACKET_SIZE)
						{
							char const * const temp = packet_get_raw_data_const(packet, 0);

							if (temp[currsize - 1] != '\r' && temp[currsize - 1] != '\n')
							{
								virtconn_set_clientin_size(vc, currsize);
								packet_set_size(packet, currsize + 1);
								break; /* no end of line, get another char */
							}
							/* got a complete line... fall through */
						}

						packet = queue_pull_packet(virtconn_get_clientin_queue(vc));

						if (hexstrm)
						{
							fprintf(hexstrm, "%d: cli class=%s[0x%04hx] type=%s[0x%04hx] length=%hu\n",
								csocket,
								packet_get_class_str(packet), packet_get_class(packet),
								packet_get_type_str(packet, packet_dir_from_client), packet_get_type(packet),
								packet_get_size(packet));
							hexdump(hexstrm, packet_get_raw_data_const(packet, 0), packet_get_size(packet));
						}

						queue_push_packet(virtconn_get_serverout_queue(vc), packet);
						packet_del_ref(packet);
						virtconn_set_clientin_size(vc, 0);
					}
				}
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "checking %d for server readability", ssocket);
			if (ssocket != -1 && PSOCK_FD_ISSET(ssocket, &rfds))
			{
				currsize = virtconn_get_serverin_size(vc);

				if (!queue_get_length(virtconn_get_serverin_queue(vc)))
				{
					switch (virtconn_get_class(vc))
					{
					case virtconn_class_bnet:
						if (!(packet = packet_create(packet_class_bnet)))
						{
							eventlog(eventlog_level_error, __FUNCTION__, "could not allocate normal packet for input");
							continue;
						}
						break;
					case virtconn_class_file:
					{
												unsigned int fileleft;

												if ((fileleft = virtconn_get_fileleft(vc)) > 0)
												{
													if (!(packet = packet_create(packet_class_raw)))
													{
														eventlog(eventlog_level_error, __FUNCTION__, "could not allocate raw file packet for input");
														continue;
													}
													if (fileleft > MAX_PACKET_SIZE)
														packet_set_size(packet, MAX_PACKET_SIZE);
													else
														packet_set_size(packet, fileleft);
												}
												else
												{
													if (!(packet = packet_create(packet_class_file)))
													{
														eventlog(eventlog_level_error, __FUNCTION__, "could not allocate file packet for input");
														continue;
													}
												}
					}
						break;
					case virtconn_class_bot:
						if (!(packet = packet_create(packet_class_raw)))
						{
							eventlog(eventlog_level_error, __FUNCTION__, "could not allocate raw packet for input");
							continue;
						}
						packet_set_size(packet, MAX_PACKET_SIZE); /* read as much as possible */
						break;
					default:
						eventlog(eventlog_level_error, __FUNCTION__, "[%d] connection has bad type (closing connection)", virtconn_get_client_socket(vc));
						virtconn_destroy(vc);
						continue;
					}
					queue_push_packet(virtconn_get_serverin_queue(vc), packet);
					packet_del_ref(packet);
					if (!queue_get_length(virtconn_get_serverin_queue(vc)))
						continue; /* push failed */
					currsize = 0;
				}

				packet = queue_peek_packet((t_queue const * const *)virtconn_get_serverin_queue(vc)); /* avoid warning */
				switch (net_recv_packet(ssocket, packet, &currsize))
				{
				case -1:
					virtconn_destroy(vc);
					continue;

				case 0: /* still working on it */
					virtconn_set_serverin_size(vc, currsize);
					if (virtconn_get_class(vc) != virtconn_class_bot || currsize < 1)
						break;
					else
						packet_set_size(packet, currsize);
					/* fallthough... we take what we can get with the bot data */

				case 1: /* done reading */
					packet = queue_pull_packet(virtconn_get_serverin_queue(vc));
					if (virtconn_get_class(vc) == virtconn_class_file)
					{
						unsigned int len = virtconn_get_fileleft(vc);

						if (len)
							virtconn_set_fileleft(vc, len - currsize);
						else if (packet_get_type(packet) == SERVER_FILE_REPLY &&
							packet_get_size(packet) >= sizeof(t_server_file_reply))
							virtconn_set_fileleft(vc, bn_int_get(packet->u.server_file_reply.filelen));
					}
					queue_push_packet(virtconn_get_clientout_queue(vc), packet);
					packet_del_ref(packet);
					virtconn_set_serverin_size(vc, 0);
				}
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "checking %d for client writeability", csocket);
			if (PSOCK_FD_ISSET(csocket, &wfds))
			{
				currsize = virtconn_get_clientout_size(vc);
				switch (net_send_packet(csocket, queue_peek_packet((t_queue const * const *)virtconn_get_clientout_queue(vc)), &currsize)) /* avoid warning */
				{
				case -1:
					virtconn_destroy(vc);
					continue;

				case 0: /* still working on it */
					virtconn_set_clientout_size(vc, currsize);
					break;

				case 1: /* done sending */
					packet = queue_pull_packet(virtconn_get_clientout_queue(vc));

					if (hexstrm)
					{
						fprintf(hexstrm, "%d: srv class=%s[0x%04hx] type=%s[0x%04hx] length=%hu\n",
							csocket,
							packet_get_class_str(packet), packet_get_class(packet),
							packet_get_type_str(packet, packet_dir_from_server), packet_get_type(packet),
							packet_get_size(packet));
						hexdump(hexstrm, packet_get_raw_data(packet, 0), packet_get_size(packet));
					}

					packet_del_ref(packet);
					virtconn_set_clientout_size(vc, 0);
				}
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "checking %d for server writeability", ssocket);
			if (ssocket != -1 && PSOCK_FD_ISSET(ssocket, &wfds))
			{
				if (virtconn_get_state(vc) == virtconn_state_connecting)
				{
					int             err;
					psock_t_socklen errlen;

					err = 0;
					errlen = sizeof(err);
					if (psock_getsockopt(ssocket, PSOCK_SOL_SOCKET, PSOCK_SO_ERROR, &err, &errlen) < 0)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "[%d] unable to read socket error (psock_getsockopt[psock_connect]: %s)", virtconn_get_client_socket(vc), pstrerror(psock_errno()));
						virtconn_destroy(vc);
						continue;
					}
					if (errlen == 0 || err == 0)
						virtconn_set_state(vc, virtconn_state_connected);
					else
					{
						eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not connect to server (psock_getsockopt[psock_connect]: %s)", virtconn_get_client_socket(vc), pstrerror(err));
						virtconn_destroy(vc);
						continue;
					}
				}
				else
				{
					currsize = virtconn_get_serverout_size(vc);
					switch (net_send_packet(ssocket, queue_peek_packet((t_queue const * const *)virtconn_get_serverout_queue(vc)), &currsize)) /* avoid warning */
					{
					case -1:
						virtconn_destroy(vc);
						continue;

					case 0: /* still working on it */
						virtconn_set_serverout_size(vc, currsize);
						break;

					case 1: /* done sending */
						packet = queue_pull_packet(virtconn_get_serverout_queue(vc));
						packet_del_ref(packet);
						virtconn_set_serverout_size(vc, 0);
					}
				}
			}
		}
		eventlog(eventlog_level_debug, __FUNCTION__, "done checking");
	}

	return 0;
}


static void usage(char const * progname)
{
	fprintf(stderr,
		"usage: %s [<options>] <servername> [<TCP portnumber>]\n"
		"    -d FILE, --hexdump=FILE  do hex dump of packets into FILE\n"
		"    -l FILE, --logfile=FILE  save eventlog lines into FILE\n"
		"    -p PORT, --port=PORT     listen for connections on port PORT\n"
#ifdef DO_DAEMONIZE
		"    -f, --foreground         don't daemonize\n"
#else
		"    -f, --foreground         don't daemonize (default)\n"
#endif
		"    -h, --help, --usage      show this information and exit\n"
		"    -v, --version            print version number and exit\n",
		progname);
	exit(STATUS_FAILURE);
}


extern int main(int argc, char * argv[])
{
	int                a;
	char const *       logfile = NULL;
	char const *       hexfile = NULL;
	int                foreground = 0;
	unsigned short     port = 0;
	char const *       servname = NULL;
	unsigned short     servport = 0;
	struct hostent *   host;
	struct sockaddr_in servaddr;

	if (argc < 1 || !argv || !argv[0])
	{
		fprintf(stderr, "bad arguments\n");
		return STATUS_FAILURE;
	}

	for (a = 1; a < argc; a++)
	if (servname && isdigit((int)argv[a][0]) && a + 1 >= argc)
	{
		if (str_to_ushort(argv[a], &servport) < 0)
		{
			fprintf(stderr, "%s: \"%s\" should be a positive integer\n", argv[0], argv[a]);
			usage(argv[0]);
		}
	}
	else if (!servname && argv[a][0] != '-' && a + 2 >= argc)
		servname = argv[a];
	else if (strncmp(argv[a], "--hexdump=", 10) == 0)
	{
		if (hexfile)
		{
			fprintf(stderr, "%s: hexdump file was already specified as \"%s\"\n", argv[0], hexfile);
			usage(argv[0]);
		}
		hexfile = &argv[a][10];
	}
	else if (strcmp(argv[a], "-d") == 0)
	{
		if (a + 1 >= argc)
		{
			fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
			usage(argv[0]);
		}
		if (hexfile)
		{
			fprintf(stderr, "%s: hexdump file was already specified as \"%s\"\n", argv[0], hexfile);
			usage(argv[0]);
		}
		a++;
		hexfile = argv[a];
	}
	else if (strncmp(argv[a], "--logfile=", 10) == 0)
	{
		if (logfile)
		{
			fprintf(stderr, "%s: eventlog file was already specified as \"%s\"\n", argv[0], logfile);
			usage(argv[0]);
		}
		logfile = &argv[a][10];
	}
	else if (strcmp(argv[a], "-l") == 0)
	{
		if (a + 1 >= argc)
		{
			fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
			usage(argv[0]);
		}
		if (logfile)
		{
			fprintf(stderr, "%s: eventlog file was already specified as \"%s\"\n", argv[0], logfile);
			usage(argv[0]);
		}
		a++;
		logfile = argv[a];
	}
	else if (strncmp(argv[a], "--port=", 7) == 0)
	{
		if (port > 0)
		{
			fprintf(stderr, "%s: listen port was already specified as \"%hu\"\n", argv[0], port);
			usage(argv[0]);
		}
		if (str_to_ushort(&argv[a][7], &port) < 0)
		{
			fprintf(stderr, "%s: \"%s\" should be a positive integer\n", argv[0], argv[a]);
			usage(argv[0]);
		}
	}
	else if (strcmp(argv[a], "-p") == 0)
	{
		if (a + 1 >= argc)
		{
			fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
			usage(argv[0]);
		}
		if (port > 0)
		{
			fprintf(stderr, "%s: eventlog file was already specified as \"%hu\"\n", argv[0], port);
			usage(argv[0]);
		}
		a++;
		if (str_to_ushort(argv[a], &port) < 0)
		{
			fprintf(stderr, "%s: \"%s\" should be a positive integer\n", argv[0], argv[a]);
			usage(argv[0]);
		}
	}
	else if (strcmp(argv[a], "-f") == 0 || strcmp(argv[a], "--foreground") == 0)
		foreground = 1;
	else if (strcmp(argv[a], "-h") == 0 || strcmp(argv[a], "--help") == 0 || strcmp(argv[a], "--usage") == 0)
		usage(argv[0]);
	else if (strcmp(argv[a], "-v") == 0 || strcmp(argv[a], "--version") == 0)
	{
		printf("version "PVPGN_VERSION"\n");
		return STATUS_SUCCESS;
	}
	else if (strcmp(argv[a], "--hexdump") == 0 || strcmp(argv[a], "--logfile") == 0 || strcmp(argv[a], "--port") == 0)
	{
		fprintf(stderr, "%s: option \"%s\" requires and argument.\n", argv[0], argv[a]);
		usage(argv[0]);
	}
	else
	{
		fprintf(stderr, "%s: bad option \"%s\"\n", argv[0], argv[a]);
		usage(argv[0]);
	}

	if (port == 0)
		port = BNETD_SERV_PORT;
	if (servport == 0)
		servport = BNETD_SERV_PORT;
	if (!servname)
		servname = BNETD_DEFAULT_HOST;

	if (psock_init() < 0)
	{
		fprintf(stderr, "%s: could not initialize socket functions\n", argv[0]);
		return STATUS_FAILURE;
	}

	if (!(host = gethostbyname(servname)))
	{
		fprintf(stderr, "%s: unknown host \"%s\"\n", argv[0], servname);
		return STATUS_FAILURE;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = PSOCK_AF_INET;
	servaddr.sin_port = htons(servport);
	memcpy(&servaddr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

	eventlog_set(stderr);
	/* errors to eventlog from here on... */

	if (logfile)
	{
		eventlog_clear_level();
		eventlog_add_level("error");
		eventlog_add_level("warn");
		eventlog_add_level("info");
		eventlog_add_level("debug");

		if (eventlog_open(logfile) < 0)
		{
			eventlog(eventlog_level_fatal, __FUNCTION__, "could not use file \"%s\" for the eventlog (exiting)", logfile);
			return STATUS_FAILURE;
		}
	}

#ifdef DO_DAEMONIZE
	if (!foreground)
	{
		if (chdir("/") < 0)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not change working directory to / (chdir: %s)", pstrerror(errno));
			return STATUS_FAILURE;
		}

		switch (fork())
		{
		case -1:
			eventlog(eventlog_level_error, __FUNCTION__, "could not fork (fork: %s)", pstrerror(errno));
			return STATUS_FAILURE;
		case 0: /* child */
			break;
		default: /* parent */
			return STATUS_SUCCESS;
		}

		close(STDINFD);
		close(STDOUTFD);
		close(STDERRFD);

# ifdef HAVE_SETPGID
		if (setpgid(0, 0) < 0)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not create new process group (setpgid: %s)", pstrerror(errno));
			return STATUS_FAILURE;
		}
# else
#  ifdef HAVE_SETPGRP
#   ifdef SETPGRP_VOID
		if (setpgrp() < 0)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not create new process group (setpgrp: %s)", pstrerror(errno));
			return STATUS_FAILURE;
		}
#   else
		if (setpgrp(0, 0) < 0)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not create new process group (setpgrp: %s)", pstrerror(errno));
			return STATUS_FAILURE;
		}
#   endif
#  else
#   ifdef HAVE_SETSID
		if (setsid() < 0)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not create new process group (setsid: %s)", pstrerror(errno));
			return STATUS_FAILURE;
		}
#   else
#    error "One of setpgid(), setpgrp(), or setsid() is required"
#   endif
#  endif
# endif
	}
#endif

	if (hexfile)
	if (!(hexstrm = fopen(hexfile, "w")))
		eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for writing the hexdump (fopen: %s)", hexfile, pstrerror(errno));

	if (proxy_process(port, servaddr) < 0)
	{
		eventlog(eventlog_level_fatal, __FUNCTION__, "failed to initialize network (exiting)");
		return STATUS_FAILURE;
	}

	if (hexstrm)
	if (fclose(hexstrm) < 0)
		eventlog(eventlog_level_error, __FUNCTION__, "could not close hexdump file \"%s\" after writing (fclose: %s)", hexfile, pstrerror(errno));

	return STATUS_SUCCESS;
}
