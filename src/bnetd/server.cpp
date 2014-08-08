/*
 * Copyright (C) 1998,1999  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2000,2001  Marco Ziech (mmz@gmx.net)
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
#define SERVER_INTERNAL_ACCESS
#include "server.h"

#include <cctype>
#include <cerrno>
#include <cstring>
#include <cstdio>

#ifdef DO_POSIXSIG
# include <signal.h>
#endif
#ifdef HAVE_SYS_TIME_H
# include <sys/time.h>
#endif
#include "compat/strerror.h"
#include "compat/psock.h"
#include "common/fdwatch.h"
#include "common/addr.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/packet.h"
#include "common/hexdump.h"
#include "common/network.h"
#include "common/list.h"
#include "common/trans.h"
#ifdef WIN32
# include "win32/service.h"
# include "windows.h"
#endif
#include "common/util.h"

#include "prefs.h"
#include "connection.h"
#include "ipban.h"
#include "timer.h"
#include "handle_bnet.h"
#include "handle_bot.h"
#include "handle_telnet.h"
#include "handle_file.h"
#include "handle_init.h"
#include "handle_d2cs.h"
#include "handle_irc_common.h"
#include "handle_udp.h"
#include "handle_apireg.h"
#include "handle_wol_gameres.h"
#include "anongame.h"
#include "clan.h"
#include "attrlayer.h"
#include "account.h"
#include "message.h"
#include "game.h"
#include "cmdline.h"
#include "tracker.h"
#include "ladder.h"
#include "output.h"
#include "channel.h"
#include "realm.h"
#include "autoupdate.h"
#include "news.h"
#include "versioncheck.h"
#include "helpfile.h"
#include "adbanner.h"
#include "command_groups.h"
#include "alias_command.h"
#include "tournament.h"
#include "icons.h"
#include "anongame_infos.h"
#include "topic.h"
#include "i18n.h"
#include "common/setup_after.h"

#ifdef WITH_LUA
#include "luainterface.h"
#endif

extern std::FILE * hexstrm; /* from main.c */
extern int g_ServiceStatus;

namespace pvpgn
{

	namespace bnetd
	{

#ifdef DO_POSIXSIG
		static void quit_sig_handle(int unused);
		static void restart_sig_handle(int unused);
		static void save_sig_handle(int unused);
#ifdef HAVE_SETITIMER
		static void timer_sig_handle(int unused);
#endif
#endif


		time_t now;
		static std::time_t starttime;
		static std::time_t curr_exittime;
		static volatile std::time_t sigexittime = 0;
		static volatile int do_restart = 0;
		static volatile int do_save = 0;
		static volatile int got_epipe = 0;
		static char const * server_hostname = NULL;

		extern void server_quit_delay(int delay)
		{
			/* getting negative delay is ok too because it will force immediate
			 * shutdown */
			if (delay)
				sigexittime = std::time(NULL) + delay;
			else
				sigexittime = 0;
		}


		extern void server_quit_wraper(void)
		{
			if (sigexittime)
				sigexittime -= prefs_get_shutdown_decr();
			else
				sigexittime = std::time(NULL) + (std::time_t)prefs_get_shutdown_delay();
		}

		extern void server_restart_wraper(int mode){
			do_restart = mode;
		}

		extern void server_save_wraper(void){
			do_save = 1;
		}

#ifdef DO_POSIXSIG
		static void quit_sig_handle(int unused)
		{
			server_quit_wraper();
		}


		static void restart_sig_handle(int unused)
		{
			do_restart = 1;
		}


		static void save_sig_handle(int unused)
		{
			do_save = 1;
		}

		static void pipe_sig_handle(int unused)
		{
			got_epipe = 1;
		}

#ifdef HAVE_SETITIMER
		static void timer_sig_handle(int unused)
		{
			std::time(&now);
		}
#endif

		static void forced_quit_sig_handle(int unused)
		{
			server_quit_delay(-1);	/* programs shutdown 1 second before now */
		}
#endif


		extern unsigned int server_get_uptime(void)
		{
			return (unsigned int)std::difftime(std::time(NULL), starttime);
		}


		extern unsigned int server_get_starttime(void)
		{
			return (unsigned int)starttime;
		}


		static char const * laddr_type_get_str(t_laddr_type laddr_type)
		{
			switch (laddr_type)
			{
			case laddr_type_bnet:
				return "bnet";
			case laddr_type_w3route:
				return "w3route";
			case laddr_type_irc:
				return "irc";
			case laddr_type_wolv1:
				return "wolv1";
			case laddr_type_wolv2:
				return "wolv2";
			case laddr_type_apireg:
				return "apireg";
			case laddr_type_wgameres:
				return "wgameres";
			case laddr_type_telnet:
				return "telnet";
			default:
				return "UNKNOWN";
			}
		}


		static int handle_accept(void *data, t_fdwatch_type rw);
		static int handle_tcp(void *data, t_fdwatch_type rw);
		static int handle_udp(void *data, t_fdwatch_type rw);


		static int sd_accept(t_addr const * curr_laddr, t_laddr_info const * laddr_info, int ssocket, int usocket)
		{
			char               tempa[32];
			int                csocket;
			struct sockaddr_in caddr;
			psock_t_socklen    caddr_len;
			unsigned int       raddr;
			unsigned short     rport;

			if (!addr_get_addr_str(curr_laddr, tempa, sizeof(tempa)))
				std::strcpy(tempa, "x.x.x.x:x");

			/* accept the connection */
			std::memset(&caddr, 0, sizeof(caddr)); /* not sure if this is needed... modern systems are ok anyway */
			caddr_len = sizeof(caddr);
			if ((csocket = psock_accept(ssocket, (struct sockaddr *)&caddr, &caddr_len)) < 0)
			{
				/* BSD, POSIX error for aborted connections, SYSV often uses EAGAIN or EPROTO */
				if (
#ifdef PSOCK_EWOULDBLOCK
					psock_errno() == PSOCK_EWOULDBLOCK ||
#endif
#ifdef PSOCK_ECONNABORTED
					psock_errno() == PSOCK_ECONNABORTED ||
#endif
#ifdef PSOCK_EPROTO
					psock_errno() == PSOCK_EPROTO ||
#endif
					0)
					eventlog(eventlog_level_error, __FUNCTION__, "client aborted connection on %s (psock_accept: %s)", tempa, pstrerror(psock_errno()));
				else /* EAGAIN can mean out of resources _or_ connection aborted :( */
				if (
#ifdef PSOCK_EINTR
					psock_errno() != PSOCK_EINTR &&
#endif
					1)
					eventlog(eventlog_level_error, __FUNCTION__, "could not accept new connection on %s (psock_accept: %s)", tempa, pstrerror(psock_errno()));
				return -1;
			}

			/* dont accept new connections while shutting down */
			if (curr_exittime) {
				psock_shutdown(csocket, PSOCK_SHUT_RDWR);
				psock_close(csocket);
				return 0;
			}

			if (ipbanlist_check(inet_ntoa(caddr.sin_addr)) != 0)
			{
				eventlog(eventlog_level_info, __FUNCTION__, "[%d] connection from banned address %s denied (closing connection)", csocket, inet_ntoa(caddr.sin_addr));
				psock_close(csocket);
				return -1;
			}

			eventlog(eventlog_level_info, __FUNCTION__, "[%d] accepted connection from %s on %s", csocket, addr_num_to_addr_str(ntohl(caddr.sin_addr.s_addr), ntohs(caddr.sin_port)), tempa);

			if (prefs_get_use_keepalive())
			{
				int val = 1;

				if (psock_setsockopt(csocket, PSOCK_SOL_SOCKET, PSOCK_SO_KEEPALIVE, &val, (psock_t_socklen)sizeof(val)) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not set socket option SO_KEEPALIVE (psock_setsockopt: %s)", csocket, pstrerror(psock_errno()));
				/* not a fatal error */
			}

			{
				struct sockaddr_in rsaddr;
				psock_t_socklen    rlen;

				std::memset(&rsaddr, 0, sizeof(rsaddr)); /* not sure if this is needed... modern systems are ok anyway */
				rlen = sizeof(rsaddr);
				if (psock_getsockname(csocket, (struct sockaddr *)&rsaddr, &rlen) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] unable to determine real local port (psock_getsockname: %s)", csocket, pstrerror(psock_errno()));
					/* not a fatal error */
					raddr = addr_get_ip(curr_laddr);
					rport = addr_get_port(curr_laddr);
				}
				else
				{
					if (rsaddr.sin_family != PSOCK_AF_INET)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "local address returned with bad address family %d", (int)rsaddr.sin_family);
						/* not a fatal error */
						raddr = addr_get_ip(curr_laddr);
						rport = addr_get_port(curr_laddr);
					}
					else
					{
						raddr = ntohl(rsaddr.sin_addr.s_addr);
						rport = ntohs(rsaddr.sin_port);
					}
				}
			}

			if (psock_ctl(csocket, PSOCK_NONBLOCK) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not set TCP socket to non-blocking mode (closing connection) (psock_ctl: %s)", csocket, pstrerror(psock_errno()));
				psock_close(csocket);
				return -1;
			}

			{
				t_connection * c;

				if (!(c = conn_create(csocket, usocket, raddr, rport, addr_get_ip(curr_laddr), addr_get_port(curr_laddr), ntohl(caddr.sin_addr.s_addr), ntohs(caddr.sin_port))))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] unable to create new connection (closing connection)", csocket);
					psock_close(csocket);
					return -1;
				}

				if (conn_add_fdwatch(c, handle_tcp) < 0) {
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] unable to add socket to fdwatch pool (max connections?)", csocket);
					conn_set_state(c, conn_state_destroy);
					return -1;
				}

				eventlog(eventlog_level_debug, __FUNCTION__, "[%d] client connected to a %s listening address", csocket, laddr_type_get_str(laddr_info->type));
				switch (laddr_info->type)
				{
				case laddr_type_irc:
				case laddr_type_wolv1:
				case laddr_type_wolv2:
					conn_set_class(c, conn_class_ircinit);
					conn_set_state(c, conn_state_connected);
					break;
				case laddr_type_apireg:
					conn_set_class(c, conn_class_apireg);
					conn_set_state(c, conn_state_connected);
					break;
				case laddr_type_wgameres:
					conn_set_class(c, conn_class_wgameres);
					conn_set_state(c, conn_state_connected);
					break;
				case laddr_type_w3route:
					conn_set_class(c, conn_class_w3route);
					conn_set_state(c, conn_state_connected);
					break;
				case laddr_type_telnet:
					conn_set_class(c, conn_class_telnet);
					conn_set_state(c, conn_state_connected);
					break;
				case laddr_type_bnet:
				{	/* add a timer to close stale connections */
										int delay;
										t_timer_data data;

										data.p = NULL;
										delay = prefs_get_initkill_timer();
										if (delay) timerlist_add_timer(c, std::time(NULL) + delay, conn_shutdown, data);
				}
				default:
					/* We have to wait for an initial "magic" byte on bnet connections to
						 * tell us exactly what connection class we are dealing with.
						 */
					break;
				}
			}

			return 0;
		}


		static int sd_udpinput(t_addr * const curr_laddr, t_laddr_info const * laddr_info, int ssocket, int usocket)
		{
			int             err;
			psock_t_socklen errlen;
			t_packet *      upacket;

			err = 0;
			errlen = sizeof(err);
			if (psock_getsockopt(usocket, PSOCK_SOL_SOCKET, PSOCK_SO_ERROR, &err, &errlen) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] unable to read socket error (psock_getsockopt: %s)", usocket, pstrerror(psock_errno()));
				return -1;
			}
			if (errlen && err) /* if it was an error, there is no packet to read */
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] async UDP socket error notification (psock_getsockopt: %s)", usocket, pstrerror(err));
				return -1;
			}
			if (!(upacket = packet_create(packet_class_udp)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not allocate raw packet for input");
				return -1;
			}

			{
				struct sockaddr_in fromaddr;
				psock_t_socklen    fromlen;
				int                len;

				fromlen = sizeof(fromaddr);
				if ((len = psock_recvfrom(usocket, packet_get_raw_data_build(upacket, 0), MAX_PACKET_SIZE, 0, (struct sockaddr *)&fromaddr, &fromlen)) < 0)
				{
					if (
#ifdef PSOCK_EINTR
						psock_errno() != PSOCK_EINTR &&
#endif
#ifdef PSOCK_EAGAIN
						psock_errno() != PSOCK_EAGAIN &&
#endif
#ifdef PSOCK_EWOULDBLOCK
						psock_errno() != PSOCK_EWOULDBLOCK &&
#endif
#ifdef PSOCK_ECONNRESET
						psock_errno() != PSOCK_ECONNRESET &&	/* this is a win2k/winxp issue
											 * their socket implementation returns this value
											 * although it shouldn't
											 */
#endif
											 1)
											 eventlog(eventlog_level_error, __FUNCTION__, "could not recv UDP datagram (psock_recvfrom: %s)", pstrerror(psock_errno()));
					packet_del_ref(upacket);
					return -1;
				}

				if (fromaddr.sin_family != PSOCK_AF_INET)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got UDP datagram with bad address family %d", (int)fromaddr.sin_family);
					packet_del_ref(upacket);
					return -1;
				}

				packet_set_size(upacket, len);

				if (hexstrm)
				{
					char tempa[32];

					if (!addr_get_addr_str(curr_laddr, tempa, sizeof(tempa)))
						std::strcpy(tempa, "x.x.x.x:x");
					std::fprintf(hexstrm, "%d: recv class=%s[0x%02x] type=%s[0x%04x] from=%s to=%s length=%u\n",
						usocket,
						packet_get_class_str(upacket), (unsigned int)packet_get_class(upacket),
						packet_get_type_str(upacket, packet_dir_from_client), packet_get_type(upacket),
						addr_num_to_addr_str(ntohl(fromaddr.sin_addr.s_addr), ntohs(fromaddr.sin_port)),
						tempa,
						packet_get_size(upacket));
					hexdump(hexstrm, packet_get_raw_data(upacket, 0), packet_get_size(upacket));
				}

				handle_udp_packet(usocket, ntohl(fromaddr.sin_addr.s_addr), ntohs(fromaddr.sin_port), upacket);
				packet_del_ref(upacket);
			}

			return 0;
		}


		static int sd_tcpinput(t_connection * c)
		{
			unsigned int currsize;
			t_packet *   packet;
			int		 csocket = conn_get_socket(c);
			bool	 skip;

			currsize = conn_get_in_size(c);

			if (!conn_get_in_queue(c))
			{
				switch (conn_get_class(c))
				{
				case conn_class_init:
					if (!(packet = packet_create(packet_class_init)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not allocate init packet for input");
						return -1;
					}
					break;
				case conn_class_d2cs_bnetd:
					if (!(packet = packet_create(packet_class_d2cs_bnetd)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not allocate d2cs_bnetd packet");
						return -1;
					}
					break;
				case conn_class_bnet:
					if (!(packet = packet_create(packet_class_bnet)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not allocate bnet packet for input");
						return -1;
					}
					break;
				case conn_class_file:
					switch (conn_get_state(c)) {
					case conn_state_pending_raw:
						if (!(packet = packet_create(packet_class_raw)))
						{
							eventlog(eventlog_level_error, __FUNCTION__, "could not allocate raw packet for input");
							return -1;
						}
						packet_set_size(packet, sizeof(t_client_file_req3));
						break;
					default:
						if (!(packet = packet_create(packet_class_file)))
						{
							eventlog(eventlog_level_error, __FUNCTION__, "could not allocate file packet for input");
							return -1;
						}
						break;
					}
					break;
				case conn_class_bot:
				case conn_class_ircinit:
				case conn_class_irc:
				case conn_class_wol:
				case conn_class_wserv:
				case conn_class_apireg:
				case conn_class_wladder:
				case conn_class_telnet:
					if (!(packet = packet_create(packet_class_raw)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not allocate raw packet for input");
						return -1;
					}
					packet_set_size(packet, 1); /* start by only reading one char */
					break;
				case conn_class_w3route:
					if (!(packet = packet_create(packet_class_w3route)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not allocate w3route packet for input");
						return -1;
					}
					break;
				case conn_class_wgameres:
					if (!(packet = packet_create(packet_class_wolgameres)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not allocate wolgameres packet for input");
						return -1;
					}
					break;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] connection has bad class (closing connection)", conn_get_socket(c));
					conn_close_read(c);
					return -2;
				}
				conn_put_in_queue(c, packet);
				currsize = 0;
			}

			packet = conn_get_in_queue(c);
			switch (net_recv_packet(csocket, packet, &currsize))
			{
			case -1:
				eventlog(eventlog_level_debug, __FUNCTION__, "[%d] read returned -1 (closing connection)", conn_get_socket(c));
				conn_close_read(c);
				return -2;

			case 0: /* still working on it */
				/* eventlog(eventlog_level_debug,__FUNCTION__,"[%d] still reading \"%s\" packet (%u of %u bytes so far)",conn_get_socket(c),packet_get_class_str(packet),conn_get_in_size(c),packet_get_size(packet)); */
				conn_set_in_size(c, currsize);
				break;

			case 1: /* done reading */
				switch (conn_get_class(c))
				{
				case conn_class_bot:
				case conn_class_telnet:
					if (currsize < MAX_PACKET_SIZE) /* if we overflow, we can't wait for the end of the line.
									 handle_*_packet() should take care of it */
					{
						char const * const temp = (char const * const)packet_get_raw_data_const(packet, 0);

						if ((temp[currsize - 1] == '\003') || (temp[currsize - 1] == '\004')) {
							/* we have to ignore these special characters, since
							 * some bots even send them after login (eg. UltimateBot)
							 */
							currsize--;
							skip = true;
							break;
						}

						if (temp[currsize - 1] != '\r' && temp[currsize - 1] != '\n')
						{
							conn_set_in_size(c, currsize);
							packet_set_size(packet, currsize + 1);
							skip = true;
							break; /* no end of line, get another char */
						}
					}
					skip = false;
					break;

				case conn_class_ircinit:
				case conn_class_irc:
				case conn_class_wol:
				case conn_class_wserv:
				case conn_class_wladder:
					if (currsize < MAX_PACKET_SIZE)
						/* if we overflow, we can't wait for the end of the line.
								 handle_*_packet() should take care of it */
					{
						char const * const temp = (char const * const)packet_get_raw_data_const(packet, 0);

						if ((temp[currsize - 1] == '\r') || (temp[currsize - 1] == '\0')) {
							/* kindly ignore \r and NUL ... */
							currsize--;
							skip = true;
							break;
						}

						if (temp[currsize - 1] != '\n')
						{
							conn_set_in_size(c, currsize);
							packet_set_size(packet, currsize + 1);
							skip = true;
							break; /* no end of line, get another char */
						}
					}
					skip = false;
					break;

				default:
					skip = false;
				}

				if (!skip) {
					conn_put_in_queue(c, NULL);

					if (hexstrm)
					{
						std::fprintf(hexstrm, "%d: recv class=%s[0x%02x] type=%s[0x%04x] length=%u\n",
							csocket,
							packet_get_class_str(packet), (unsigned int)packet_get_class(packet),
							packet_get_type_str(packet, packet_dir_from_client), packet_get_type(packet),
							packet_get_size(packet));
						hexdump(hexstrm, packet_get_raw_data_const(packet, 0), packet_get_size(packet));
					}

					if (conn_get_class(c) == conn_class_bot ||
						conn_get_class(c) == conn_class_telnet) /* NUL terminate the line to make life easier */
					{
						char * const temp = (char*)packet_get_raw_data(packet, 0);

						if (temp[currsize - 1] == '\r' || temp[currsize - 1] == '\n')
							temp[currsize - 1] = '\0'; /* have to do it here instead of above so everything
										is intact for the hexdump */
					}

					{
						int ret;

						switch (conn_get_class(c))
						{
						case conn_class_init:
							ret = handle_init_packet(c, packet);
							break;
						case conn_class_bnet:
							ret = handle_bnet_packet(c, packet);
							break;
						case conn_class_d2cs_bnetd:
							ret = handle_d2cs_packet(c, packet);
							break;
						case conn_class_bot:
							ret = handle_bot_packet(c, packet);
							break;
						case conn_class_telnet:
							ret = handle_telnet_packet(c, packet);
							break;
						case conn_class_file:
							ret = handle_file_packet(c, packet);
							break;
						case conn_class_ircinit:
						case conn_class_irc:
						case conn_class_wol:
						case conn_class_wserv:
						case conn_class_wladder:
							ret = handle_irc_common_packet(c, packet);
							break;
						case conn_class_apireg:
							ret = handle_apireg_packet(c, packet);
							break;
						case conn_class_w3route:
							ret = handle_w3route_packet(c, packet);
							break;
						case conn_class_wgameres:
							ret = handle_wol_gameres_packet(c, packet);
							break;
						default:
							eventlog(eventlog_level_error, __FUNCTION__, "[%d] bad packet class %d (closing connection)", conn_get_socket(c), (int)packet_get_class(packet));
							ret = -1;
						}
						packet_del_ref(packet);
						if (ret < 0)
						{
							conn_close_read(c);
							return -2;
						}
					}

					conn_set_in_size(c, 0);
				}
			}

			return 0;
		}


		static int sd_tcpoutput(t_connection * c)
		{
			unsigned int currsize;
			unsigned int totsize;
			t_packet *   packet;
			int		 csocket = conn_get_socket(c);

			totsize = 0;
			for (;;)
			{
				currsize = conn_get_out_size(c);

				if ((packet = conn_peek_outqueue(c)) == NULL)
					return -2;

				switch (net_send_packet(csocket, packet, &currsize)) /* avoid warning */
				{
				case -1:
					/* marking connection as "destroyed", memory will be freed later */
					conn_clear_outqueue(c);
					conn_set_state(c, conn_state_destroy);
					return -2;

				case 0: /* still working on it */
					conn_set_out_size(c, currsize);
					return 0; /* bail out */

				case 1: /* done sending */
					if (hexstrm)
					{
						std::fprintf(hexstrm, "%d: send class=%s[0x%02x] type=%s[0x%04x] length=%u\n",
							csocket,
							packet_get_class_str(packet), (unsigned int)packet_get_class(packet),
							packet_get_type_str(packet, packet_dir_from_server), packet_get_type(packet),
							packet_get_size(packet));
						hexdump(hexstrm, packet_get_raw_data(packet, 0), packet_get_size(packet));
					}

					packet = conn_pull_outqueue(c);
					packet_del_ref(packet);
					conn_set_out_size(c, 0);

					/* stop at about BNETD_MAX_OUTBURST (or until out of packets or EWOULDBLOCK) */
					if (totsize > BNETD_MAX_OUTBURST || !conn_peek_outqueue(c))
						return 0;
					totsize += currsize;
					break;
				}
			}

			/* not reached */
		}

		void server_check_and_fix_hostname(char const * sname)
		{
			int ok = 1;
			char * tn = (char *)sname;
			char * sn = (char *)sname;

			if (!std::isalnum((int)*sn))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "hostname contains invalid first symbol (must be alphanumeric)");
				*tn = 'a';
			}
			tn++;
			sn++;

			for (; *sn != '\0'; sn++)
			{
				if (std::isalnum((int)*sn) || *sn == '-' || *sn == '.')
				{
					*tn = *sn;
					tn++;
				}
				else
					ok = 0;
			}
			*tn = '\0';

			if (!ok)
				eventlog(eventlog_level_error, __FUNCTION__, "hostname contains invalid symbol(s) (must be alphanumeric, '-' or '.') - skipped those symbols");
		}

		extern void server_set_hostname(void)
		{
			char temp[250];
			char const * hn;
#ifdef HAVE_GETHOSTBYNAME
			struct hostent *   hp;
#endif

			if (server_hostname) {
				xfree((void *)server_hostname); /* avoid warning */
				server_hostname = NULL;
			}

			hn = prefs_get_hostname();
			if ((!hn) || (hn[0] == '\0')) {
				if (gethostname(temp, sizeof(temp)) < 0) {
#ifdef WIN32
					std::sprintf(temp, "localhost");
#else
					eventlog(eventlog_level_error, __FUNCTION__, "could not get hostname: %s", pstrerror(errno));
					return;
#endif
				}
#ifdef HAVE_GETHOSTBYNAME
				hp = gethostbyname(temp);
				if (!hp || !hp->h_name) {
#endif
					server_hostname = xstrdup(temp);
#ifdef HAVE_GETHOSTBYNAME
				}
				else {
					int i = 0;

					if (std::strchr(hp->h_name, '.'))
						/* Default name is already a FQDN */
						server_hostname = xstrdup(hp->h_name);
					/* ... if not we have to examine the aliases */
					while (!server_hostname && hp->h_aliases && hp->h_aliases[i]) {
						if (std::strchr(hp->h_aliases[i], '.'))
							server_hostname = xstrdup(hp->h_aliases[i]);
						i++;
					}
					if (!server_hostname)
						/* Fall back to default name which might not be a FQDN */
						server_hostname = xstrdup(hp->h_name);
				}
#endif
			}
			else {
				server_hostname = xstrdup(hn);
			}
			server_check_and_fix_hostname(server_hostname);
			eventlog(eventlog_level_info, __FUNCTION__, "set hostname to \"%s\"", server_hostname);
		}

		extern char const * server_get_hostname(void)
		{
			if (!server_hostname)
				return "(null)"; /* avoid crashes */
			else
				return server_hostname;
		}


		extern void server_clear_hostname(void)
		{
			if (server_hostname) {
				xfree((void *)server_hostname); /* avoid warning */
				server_hostname = NULL;
			}
		}


		static int handle_accept(void *data, t_fdwatch_type rw)
		{
			t_laddr_info *laddr_info = (t_laddr_info*)addr_get_data((t_addr *)data).p;

			return sd_accept((t_addr *)data, laddr_info, laddr_info->ssocket, laddr_info->usocket);
		}


		static int handle_udp(void *data, t_fdwatch_type rw)
		{
			t_laddr_info *laddr_info = (t_laddr_info*)addr_get_data((t_addr *)data).p;

			return sd_udpinput((t_addr *)data, laddr_info, laddr_info->ssocket, laddr_info->usocket);
		}


		static int handle_tcp(void *data, t_fdwatch_type rw)
		{
			switch (rw) {
			case fdwatch_type_read: return sd_tcpinput((t_connection *)data);
			case fdwatch_type_write: return sd_tcpoutput((t_connection *)data);
			default:
				return -1;
			}
		}


		static int _setup_add_addrs(t_addrlist **pladdrs, const char *str, unsigned int defaddr, unsigned short defport, t_laddr_type type)
		{
			t_addr *        curr_laddr;
			t_addr_data     laddr_data;
			t_laddr_info *  laddr_info;
			t_elem const *  acurr;

			if (*pladdrs == NULL) {
				*pladdrs = addrlist_create(str, defaddr, defport);
				if (*pladdrs == NULL) return -1;
			}
			else if (addrlist_append(*pladdrs, str, defaddr, defport)) return -1;

			/* Mark all these laddrs for being classic Battle.net service */
			LIST_TRAVERSE_CONST(*pladdrs, acurr)
			{
				curr_laddr = (t_addr*)elem_get_data(acurr);
				if (addr_get_data(curr_laddr).p)
					continue;
				laddr_info = (t_laddr_info*)xmalloc(sizeof(t_laddr_info));
				laddr_info->usocket = -1;
				laddr_info->ssocket = -1;
				laddr_info->type = type;
				laddr_data.p = laddr_info;
				if (addr_set_data(curr_laddr, laddr_data) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not set address data");
					if (laddr_info->usocket != -1)
					{
						psock_close(laddr_info->usocket);
						laddr_info->usocket = -1;
					}
					if (laddr_info->ssocket != -1)
					{
						psock_close(laddr_info->ssocket);
						laddr_info->ssocket = -1;
					}
					return -1;
				}
			}

			return 0;
		}

		static int _set_reuseaddr(int sock)
		{
			int val = 1;

			return psock_setsockopt(sock, PSOCK_SOL_SOCKET, PSOCK_SO_REUSEADDR, &val, (psock_t_socklen)sizeof(int));
		}

		static int _bind_socket(int sock, unsigned addr, short port)
		{
			struct sockaddr_in saddr;

			std::memset(&saddr, 0, sizeof(saddr));
			saddr.sin_family = PSOCK_AF_INET;
			saddr.sin_port = htons(port);
			saddr.sin_addr.s_addr = htonl(addr);
			return psock_bind(sock, (struct sockaddr *)&saddr, (psock_t_socklen)sizeof(saddr));
		}

		static int _setup_listensock(t_addrlist *laddrs)
		{
			t_addr *        curr_laddr;
			t_laddr_info *  laddr_info;
			t_elem const *  acurr;
			char            tempa[32];
			int		    fidx;

			LIST_TRAVERSE_CONST(laddrs, acurr)
			{
				curr_laddr = (t_addr*)elem_get_data(acurr);
				if (!(laddr_info = (t_laddr_info*)addr_get_data(curr_laddr).p))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "NULL address info");
					goto err;
				}

				if (!addr_get_addr_str(curr_laddr, tempa, sizeof(tempa)))
					std::strcpy(tempa, "x.x.x.x:x");

				laddr_info->ssocket = psock_socket(PSOCK_PF_INET, PSOCK_SOCK_STREAM, PSOCK_IPPROTO_TCP);
				if (laddr_info->ssocket < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not create a %s listening socket (psock_socket: %s)", laddr_type_get_str(laddr_info->type), pstrerror(psock_errno()));
					goto err;
				}

				if (_set_reuseaddr(laddr_info->ssocket) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not set option SO_REUSEADDR on %s socket %d (psock_setsockopt: %s)", laddr_type_get_str(laddr_info->type), laddr_info->ssocket, pstrerror(psock_errno()));
				/* not a fatal error... */

				if (_bind_socket(laddr_info->ssocket, addr_get_ip(curr_laddr), addr_get_port(curr_laddr)) < 0) {
					eventlog(eventlog_level_error, __FUNCTION__, "could not bind %s socket to address %s TCP (psock_bind: %s)", laddr_type_get_str(laddr_info->type), tempa, pstrerror(psock_errno()));
					goto errsock;
				}

				/* tell socket to listen for connections */
				if (psock_listen(laddr_info->ssocket, LISTEN_QUEUE) < 0) {
					eventlog(eventlog_level_error, __FUNCTION__, "could not set %s socket %d to listen (psock_listen: %s)", laddr_type_get_str(laddr_info->type), laddr_info->ssocket, pstrerror(psock_errno()));
					goto errsock;
				}

				if (psock_ctl(laddr_info->ssocket, PSOCK_NONBLOCK) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not set %s TCP listen socket to non-blocking mode (psock_ctl: %s)", laddr_type_get_str(laddr_info->type), pstrerror(psock_errno()));

				/* index not stored persisently because we dont need to refer to it later */
				fidx = fdwatch_add_fd(laddr_info->ssocket, fdwatch_type_read, handle_accept, curr_laddr);
				if (fidx < 0) {
					eventlog(eventlog_level_error, __FUNCTION__, "could not add listening socket %d to fdwatch pool (max sockets?)", laddr_info->ssocket);
					goto errsock;
				}

				eventlog(eventlog_level_info, __FUNCTION__, "listening for %s connections on %s TCP", laddr_type_get_str(laddr_info->type), tempa);

				if (laddr_info->type == laddr_type_bnet)
				{
					laddr_info->usocket = psock_socket(PSOCK_PF_INET, PSOCK_SOCK_DGRAM, PSOCK_IPPROTO_UDP);
					if (laddr_info->usocket < 0)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not create UDP socket (psock_socket: %s)", pstrerror(psock_errno()));
						goto errfdw;
					}

					if (_set_reuseaddr(laddr_info->usocket) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "could not set option SO_REUSEADDR on %s socket %d (psock_setsockopt: %s)", laddr_type_get_str(laddr_info->type), laddr_info->usocket, pstrerror(psock_errno()));
					/* not a fatal error... */

					if (_bind_socket(laddr_info->usocket, addr_get_ip(curr_laddr), addr_get_port(curr_laddr)) < 0) {
						eventlog(eventlog_level_error, __FUNCTION__, "could not bind %s socket to address %s UDP (psock_bind: %s)", laddr_type_get_str(laddr_info->type), tempa, pstrerror(psock_errno()));
						goto errusock;
					}

					if (psock_ctl(laddr_info->usocket, PSOCK_NONBLOCK) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "could not set %s UDP socket to non-blocking mode (psock_ctl: %s)", laddr_type_get_str(laddr_info->type), pstrerror(psock_errno()));

					/* index ignored because we never need it after this */
					if (fdwatch_add_fd(laddr_info->usocket, fdwatch_type_read, handle_udp, curr_laddr) < 0) {
						eventlog(eventlog_level_error, __FUNCTION__, "could not add listening socket %d to fdwatch pool (max sockets?)", laddr_info->usocket);
						goto errusock;
					}
				}
			}

			return 0;

		errusock:
			psock_close(laddr_info->usocket);
			laddr_info->usocket = -1;

		errfdw:
			fdwatch_del_fd(fidx);

		errsock:
			psock_close(laddr_info->ssocket);
			laddr_info->ssocket = -1;

		err:
			return -1;
		}

#ifdef DO_POSIXSIG
		static sigset_t        block_set;
		static sigset_t        save_set;

		static int _setup_posixsig(void)
		{
			if (sigemptyset(&save_set) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not initialize std::signal set (sigemptyset: %s)", pstrerror(errno));
				return -1;
			}
			if (sigemptyset(&block_set) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not initialize std::signal set (sigemptyset: %s)", pstrerror(errno));
				return -1;
			}
			if (sigaddset(&block_set, SIGINT) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not add std::signal to set (sigemptyset: %s)", pstrerror(errno));
				return -1;
			}
			if (sigaddset(&block_set, SIGHUP) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not add std::signal to set (sigemptyset: %s)", pstrerror(errno));
				return -1;
			}
			if (sigaddset(&block_set, SIGTERM) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not add std::signal to set (sigemptyset: %s)", pstrerror(errno));
				return -1;
			}
			if (sigaddset(&block_set, SIGUSR1) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not add std::signal to set (sigemptyset: %s)", pstrerror(errno));
				return -1;
			}
			if (sigaddset(&block_set, SIGUSR2) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not add std::signal to set (sigemptyset: %s)", pstrerror(errno));
				return -1;
			}
			if (sigaddset(&block_set, SIGALRM) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not add std::signal to set (sigemptyset: %s)", pstrerror(errno));
				return -1;
			}

			{
				struct sigaction quit_action;
				struct sigaction restart_action;
				struct sigaction save_action;
				struct sigaction pipe_action;
#ifdef	HAVE_SETITIMER
				struct sigaction timer_action;
#endif
				struct sigaction forced_quit_action;

				quit_action.sa_handler = quit_sig_handle;
				if (sigemptyset(&quit_action.sa_mask) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not initialize std::signal set (sigemptyset: %s)", pstrerror(errno));
				quit_action.sa_flags = SA_RESTART;

				restart_action.sa_handler = restart_sig_handle;
				if (sigemptyset(&restart_action.sa_mask) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not initialize std::signal set (sigemptyset: %s)", pstrerror(errno));
				restart_action.sa_flags = SA_RESTART;

				save_action.sa_handler = save_sig_handle;
				if (sigemptyset(&save_action.sa_mask) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not initialize std::signal set (sigemptyset: %s)", pstrerror(errno));
				save_action.sa_flags = SA_RESTART;

				pipe_action.sa_handler = pipe_sig_handle;
				if (sigemptyset(&pipe_action.sa_mask) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not initialize std::signal set (sigemptyset: %s)", pstrerror(errno));
				pipe_action.sa_flags = SA_RESTART;
#ifdef HAVE_SETITIMER
				timer_action.sa_handler = timer_sig_handle;
				if (sigemptyset(&timer_action.sa_mask) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not initialize std::signal set (sigemptyset: %s)", pstrerror(errno));
				timer_action.sa_flags = SA_RESTART;
#endif /* HAVE_SETITIMER */
				forced_quit_action.sa_handler = forced_quit_sig_handle;
				if (sigemptyset(&forced_quit_action.sa_mask) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not initialize std::signal set (sigemptyset: %s)", pstrerror(errno));
				forced_quit_action.sa_flags = SA_RESTART;

				if (sigaction(SIGINT, &quit_action, NULL) < 0) /* control-c */
					eventlog(eventlog_level_error, __FUNCTION__, "could not set SIGINT std::signal handler (sigaction: %s)", pstrerror(errno));
				if (sigaction(SIGHUP, &restart_action, NULL) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not set SIGHUP std::signal handler (sigaction: %s)", pstrerror(errno));
				if (sigaction(SIGTERM, &quit_action, NULL) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not set SIGTERM std::signal handler (sigaction: %s)", pstrerror(errno));
				if (sigaction(SIGUSR1, &save_action, NULL) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not set SIGUSR1 std::signal handler (sigaction: %s)", pstrerror(errno));
				if (sigaction(SIGPIPE, &pipe_action, NULL) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not set SIGPIPE std::signal handler (sigaction: %s)", pstrerror(errno));
#ifdef HAVE_SETITIMER
				/* setup asynchronus timestamp update timer */
				if (sigaction(SIGALRM, &timer_action, NULL) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not set SIGALRM std::signal handler (sigaction: %s)", pstrerror(errno));

				{
					struct itimerval it;

					it.it_interval.tv_sec = BNETD_JIFFIES / 1000;
					it.it_interval.tv_usec = BNETD_JIFFIES % 1000;
					it.it_value.tv_sec = BNETD_JIFFIES / 1000;
					it.it_value.tv_usec = BNETD_JIFFIES % 1000;

					if (setitimer(ITIMER_REAL, &it, NULL)) {
						eventlog(eventlog_level_fatal, __FUNCTION__, "failed to set timers (setitimer(): %s)", pstrerror(errno));
						return -1;
					}
				}
#endif
				if (sigaction(SIGQUIT, &forced_quit_action, NULL) < 0) /* immediate shutdown */
					eventlog(eventlog_level_error, __FUNCTION__, "could not set SIGQUIT std::signal handler (sigaction: %s)", pstrerror(errno));
			}

			return 0;
		}
#endif

#ifdef WIN32
# ifndef WIN32_GUI
		BOOL CtrlHandler(DWORD fdwCtrlType)
		{
			switch (fdwCtrlType)
			{
				// Handle the CTRL-C signal.
			case CTRL_C_EVENT:
				server_quit_wraper();
				return(TRUE);

				// CTRL-CLOSE: confirm that the user wants to exit.
				server_quit_wraper();
				return(TRUE);

				// Pass other signals to the next handler.
			case CTRL_BREAK_EVENT:
				server_quit_wraper();
				return FALSE;

			case CTRL_LOGOFF_EVENT:
				server_quit_wraper();
				return FALSE;

			case CTRL_SHUTDOWN_EVENT:
				server_quit_wraper();
				return FALSE;

			default:
				return FALSE;
			}
		}
# endif
#endif

		static std::time_t prev_exittime;

		static void _server_mainloop(t_addrlist *laddrs)
		{
			std::time_t          next_savetime, track_time;
			std::time_t          war3_ladder_updatetime;
			std::time_t          output_updatetime;
			unsigned int    prev_time = 0;

			starttime = std::time(NULL);
			track_time = starttime - prefs_get_track();
			next_savetime = starttime + prefs_get_user_sync_timer();
			war3_ladder_updatetime = starttime - prefs_get_war3_ladder_update_secs();
			output_updatetime = starttime - prefs_get_output_update_secs();

			for (;;)
			{
#ifdef WIN32
				if (g_ServiceStatus == 0) server_quit_wraper();

				while (g_ServiceStatus == 2) Sleep(1000);
#endif

#ifdef DO_POSIXSIG
				if (sigprocmask(SIG_SETMASK, &save_set, NULL) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not unblock signals");
				/* receive signals here */
				if (sigprocmask(SIG_SETMASK, &block_set, NULL) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not block signals");
#endif
				if (got_epipe)
				{
					got_epipe = 0;
					eventlog(eventlog_level_info, __FUNCTION__, "handled SIGPIPE");
				}

#if !defined(DO_POSIXSIG) || !defined(HAVE_SETITIMER)
				/* if no DO_POSIXSIG or no HAVE_SETITIMER so we must read timestamp each loop */
				std::time(&now);
#endif
				curr_exittime = sigexittime;
				if (curr_exittime && (curr_exittime <= now || connlist_login_get_length() < 1))
				{
					eventlog(eventlog_level_info, __FUNCTION__, "the server is shutting down (%d connections left)", connlist_get_length());
					clanlist_save();
					/* no need for accountlist_save() when using "force" */
					accountlist_save(FS_FORCE | FS_ALL);
					accountlist_flush(FS_FORCE | FS_ALL);
					break;
				}
				if (prev_exittime != curr_exittime)
				{
					t_message *  message;
					char const * tstr;

					if (curr_exittime != 0)
					{
						char text[29 + 256 + 2 + 32 + 24 + 1]; /* "The ... in " + std::time + " (" num + " connection ...)." + NUL */

						tstr = seconds_to_timestr(curr_exittime - now);
						std::sprintf(text, "The server will shut down in %s (%d connections remaining).", tstr, connlist_get_length());
						if ((message = message_create(message_type_error, NULL, text)))
						{
							message_send_all(message);
							message_destroy(message);
						}
						eventlog(eventlog_level_info, __FUNCTION__, "the server will shut down in %s (%d connections remaining)", tstr, connlist_get_length());
					}
					else
					{
						if ((message = message_create(message_type_error, NULL, "Server shutdown has been canceled.")))
						{
							message_send_all(message);
							message_destroy(message);
						}
						eventlog(eventlog_level_info, __FUNCTION__, "server shutdown canceled");
					}
				}
				prev_exittime = curr_exittime;

				if (next_savetime <= now) {
					/* do this stuff in usersync periods */
					clanlist_save();
					gamelist_check_voidgame();
					ladders.save();
					next_savetime += prefs_get_user_sync_timer();
				}
				accountlist_save(FS_NONE);
				accountlist_flush(FS_NONE);

				if (prefs_get_track() && track_time + (std::time_t)prefs_get_track() <= now)
				{
					track_time = now;
					tracker_send_report(laddrs);
				}

				if (prefs_get_war3_ladder_update_secs() && war3_ladder_updatetime + (std::time_t)prefs_get_war3_ladder_update_secs() <= now)
				{
					war3_ladder_updatetime = now;
					ladders.status();
				}

				if (prefs_get_output_update_secs() && output_updatetime + (std::time_t)prefs_get_output_update_secs() <= now)
				{
					output_updatetime = now;
					output_write_to_file();
				}


				if (do_save)
				{
					eventlog(eventlog_level_info, __FUNCTION__, "saving accounts due to std::signal");
					clanlist_save();

					do_save = 0;
				}

				if (do_restart)
				{
					if (do_restart == restart_mode_all)
					{
						eventlog(eventlog_level_info, __FUNCTION__, "reading configuration files");
						if (cmdline_get_preffile())
						{
							if (prefs_load(cmdline_get_preffile()) < 0)
								eventlog(eventlog_level_error, __FUNCTION__, "could not parse configuration file");
						}
						else
						if (prefs_load(BNETD_DEFAULT_CONF_FILE) < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "using default configuration");

						if (eventlog_open(prefs_get_logfile()) < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "could not use the file \"%s\" for the eventlog", prefs_get_logfile());

						/* FIXME: load new network settings */

						/* reload server name */
						server_set_hostname();

						attrlayer_load_default();
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_i18n)
					{
						i18n_reload();
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_channels)
					{
						channellist_reload();
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_realms)
					{
						if (realmlist_reload(prefs_get_realmfile()) < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "could not reload realm list");
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_autoupdate)
					{
						autoupdate_unload();
						if (autoupdate_load(prefs_get_mpqfile()) < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "could not load autoupdate list");
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_news)
					{
						news_unload();
						if (news_load(prefs_get_newsfile()) < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "could not load news list");
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_versioncheck)
					{
						versioncheck_unload();
						if (versioncheck_load(prefs_get_versioncheck_file()) < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "could not load versioncheck list");
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_ipbans)
					{
						if (ipbanlist_destroy() < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "could not unload old IP ban list");
						ipbanlist_create();
						if (ipbanlist_load(prefs_get_ipbanfile()) < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "could not load new IP ban list");
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_helpfile)
					{
						helpfile_unload();
						if (helpfile_init(prefs_get_helpfile()) < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "could not load the helpfile");
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_banners)
					{
						adbannerlist.reset(new AdBannerComponent(prefs_get_adfile()));
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_tracker)
					{
						if (prefs_get_track())
							tracker_set_servers(prefs_get_trackserv_addrs());
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_commandgroups)
					{
						if (command_groups_reload(prefs_get_command_groups_file()) < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "could not load new command_groups list");
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_aliasfile)
					{
						aliasfile_unload();
						aliasfile_load(prefs_get_aliasfile());
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_transfile)
					{
						if (trans_reload(prefs_get_transfile(), TRANS_BNETD) < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "could not reload trans list");
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_tournament)
					{
						tournament_reload(prefs_get_tournament_file());
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_icons)
					{
						customicons_unload();
						customicons_load(prefs_get_customicons_file());
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_anongame)
					{
						anongame_infos_unload();
						anongame_infos_load(prefs_get_anongame_infos_file());
					}

					if (do_restart == restart_mode_all || do_restart == restart_mode_topiclist)
					{
						topiclist_unload();
						if (topiclist_load(prefs_get_topicfile()) < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "could not load new topic list");
					}

#ifdef WITH_LUA
					if (do_restart == restart_mode_all || do_restart == restart_mode_lua)
					{
						lua_handle_server(luaevent_server_rehash);

						lua_unload();
						lua_load(prefs_get_scriptdir());
					}
#endif

					eventlog(eventlog_level_info, __FUNCTION__, "done reconfiguring");

					do_restart = 0;
				}

				/* only check timers once a second */
				if (now > prev_time) 
				{
					prev_time = now;
					timerlist_check_timers(now);
#ifdef WITH_LUA
					lua_handle_server(luaevent_server_mainloop);
#endif
				}

				/* no need to populate the fdwatch structures as they are populated on the fly
				 * by sd_accept, conn_push_outqueue, conn_pull_outqueue, conn_destory */

				/* find which sockets need servicing */
				switch (fdwatch(BNETD_POLL_INTERVAL))
				{
				case -1: /* error */
					if (
#ifdef PSOCK_EINTR
						psock_errno() != PSOCK_EINTR &&
#endif
						1)
						eventlog(eventlog_level_error, __FUNCTION__, "fdwatch() failed (errno: %s)", pstrerror(psock_errno()));
				case 0: /* timeout... no sockets need checking */
					continue;
				}

				/* cycle through the ready sockets and handle them */
				fdwatch_handle();

				/* reap dead connections */
				connlist_reap();

			}
		}


		static void _shutdown_conns(void)
		{
			t_elem *ccurr;
			t_connection *c;

			LIST_TRAVERSE(connlist(), ccurr)
			{
				c = (t_connection *)elem_get_data(ccurr);
				conn_destroy(c, &ccurr, DESTROY_FROM_CONNLIST);
			}
		}


		static void _shutdown_addrs(t_addrlist *laddrs)
		{
			t_addr *        curr_laddr;
			t_laddr_info *  laddr_info;
			t_elem const *  acurr;

			LIST_TRAVERSE_CONST(laddrs, acurr)
			{
				curr_laddr = (t_addr*)elem_get_data(acurr);
				if ((laddr_info = (t_laddr_info*)addr_get_data(curr_laddr).p))
				{
					if (laddr_info->usocket != -1)
						psock_close(laddr_info->usocket);
					if (laddr_info->ssocket != -1)
						psock_close(laddr_info->ssocket);
					xfree(laddr_info);
				}
			}
			addrlist_destroy(laddrs);
		}

		extern int server_process(void)
		{
			t_addrlist *    laddrs;

			laddrs = NULL;
			/* Start with the Battle.net address list */
			if (_setup_add_addrs(&laddrs, prefs_get_bnetdserv_addrs(), INADDR_ANY, BNETD_SERV_PORT, laddr_type_bnet))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not create %s server address list from \"%s\"", laddr_type_get_str(laddr_type_bnet), prefs_get_bnetdserv_addrs());
				return -1;
			}

			/* Append list of addresses to listen for IRC connections */
			if (_setup_add_addrs(&laddrs, prefs_get_irc_addrs(), INADDR_ANY, BNETD_IRC_PORT, laddr_type_irc))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not create %s server address list from \"%s\"", laddr_type_get_str(laddr_type_irc), prefs_get_irc_addrs());
				_shutdown_addrs(laddrs);
				return -1;
			}

			/* Append list of addresses to listen for WOLv1 connections */
			if (_setup_add_addrs(&laddrs, prefs_get_wolv1_addrs(), INADDR_ANY, BNETD_WOLV1_PORT, laddr_type_wolv1))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not create %s server address list from \"%s\"", laddr_type_get_str(laddr_type_wolv1), prefs_get_wolv1_addrs());
				_shutdown_addrs(laddrs);
				return -1;
			}

			/* Append list of addresses to listen for WOLv2 connections */
			if (_setup_add_addrs(&laddrs, prefs_get_wolv2_addrs(), INADDR_ANY, BNETD_WOLV2_PORT, laddr_type_wolv2))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not create %s server address list from \"%s\"", laddr_type_get_str(laddr_type_wolv2), prefs_get_wolv2_addrs());
				_shutdown_addrs(laddrs);
				return -1;
			}

			/* Append list of addresses to listen for APIREGISER connections */
			if (_setup_add_addrs(&laddrs, prefs_get_apireg_addrs(), INADDR_ANY, BNETD_APIREG_PORT, laddr_type_apireg))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not create %s server address list from \"%s\"", laddr_type_get_str(laddr_type_apireg), prefs_get_apireg_addrs());
				_shutdown_addrs(laddrs);
				return -1;
			}

			/* Append list of addresses to listen for WGAMERES connections */
			if (_setup_add_addrs(&laddrs, prefs_get_wgameres_addrs(), INADDR_ANY, BNETD_WGAMERES_PORT, laddr_type_wgameres))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not create %s server address list from \"%s\"", laddr_type_get_str(laddr_type_wgameres), prefs_get_wgameres_addrs());
				_shutdown_addrs(laddrs);
				return -1;
			}

			/* Append list of addresses to listen for W3ROUTE connections */
			if (_setup_add_addrs(&laddrs, prefs_get_w3route_addr(), INADDR_ANY, BNETD_W3ROUTE_PORT, laddr_type_w3route))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not create %s server address list from \"%s\"", laddr_type_get_str(laddr_type_w3route), prefs_get_w3route_addr());
				_shutdown_addrs(laddrs);
				return -1;
			}

			/* Append list of addresses to listen for telnet connections */
			if (_setup_add_addrs(&laddrs, prefs_get_telnet_addrs(), INADDR_ANY, BNETD_TELNET_PORT, laddr_type_telnet))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not create %s server address list from \"%s\"", laddr_type_get_str(laddr_type_telnet), prefs_get_telnet_addrs());
				_shutdown_addrs(laddrs);
				return -1;
			}

			if (_setup_listensock(laddrs)) {
				_shutdown_addrs(laddrs);
				return -1;
			}

			/* setup std::signal handlers */
			prev_exittime = sigexittime;
#ifdef DO_POSIXSIG
			_setup_posixsig();
#endif

#ifdef WIN32
# ifndef WIN32_GUI
			SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, TRUE);
# endif
#endif

			_server_mainloop(laddrs);

#ifdef WIN32
# ifndef WIN32_GUI
			SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, FALSE);
# endif
#endif


			/* cleanup for server shutdown */
			_shutdown_conns();
			_shutdown_addrs(laddrs);

			return 0;
		}

	}

}
