/*
 * Copyright (C) 2001		sousou	(liupeng.cs@263.net)
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
#include "setup.h"
#include "dbserver.h"

#include <cstring>
#include <ctime>

#ifdef WIN32
# include <conio.h>
#endif

#include "compat/psock.h"
#include "compat/strerror.h"
#include "common/eventlog.h"
#include "common/addr.h"
#include "common/xalloc.h"
#include "common/network.h"
#include "d2ladder.h"
#include "prefs.h"
#include "charlock.h"
#include "dbspacket.h"
#include "handle_signal.h"
#include "common/setup_after.h"

#ifdef WIN32
extern int g_ServiceStatus;
#endif

namespace pvpgn
{

	namespace d2dbs
	{

		static int		dbs_packet_gs_id = 0;
		static t_preset_d2gsid	*preset_d2gsid_head = NULL;
		t_list * dbs_server_connection_list = NULL;
		int dbs_server_listen_socket = -1;

		/* dbs_server_main
		 * The module's driver function -- we just call other functions and
		 * interpret their results.
		 */

		static int dbs_handle_timed_events(void);
		static void dbs_on_exit(void);

		int dbs_server_init(void);
		void dbs_server_loop(int ListeningSocket);
		int dbs_server_setup_fdsets(fd_set * pReadFDs, fd_set * pWriteFDs,
			fd_set * pExceptFDs, int ListeningSocket);
		bool dbs_server_read_data(t_d2dbs_connection* conn);
		bool dbs_server_write_data(t_d2dbs_connection* conn);
		int dbs_server_list_add_socket(int sd, unsigned int ipaddr);
		static int setsockopt_keepalive(int sock);
		static unsigned int get_preset_d2gsid(unsigned int ipaddr);


		int dbs_server_main(void)
		{
			eventlog(eventlog_level_info, __FUNCTION__, "establishing the listener...");
			dbs_server_listen_socket = dbs_server_init();
			if (dbs_server_listen_socket < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "dbs_server_init error ");
				return 3;
			}
			eventlog(eventlog_level_info, __FUNCTION__, "waiting for connections...");
			dbs_server_loop(dbs_server_listen_socket);
			dbs_on_exit();
			return 0;
		}

		/* dbs_server_init
		 * Sets up a listener on the given interface and port, returning the
		 * listening socket if successful; if not, returns -1.
		 */
		/* FIXME: No it doesn't!  pcAddress is not ever referenced in this
		 * function.
		 * CreepLord: Fixed much better way (will accept dns hostnames)
		 */
		int dbs_server_init(void)
		{
			int sd;
			struct sockaddr_in sinInterface;
			int val;
			t_addr	* servaddr;

			dbs_server_connection_list = list_create();

			if (d2dbs_d2ladder_init() == -1)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "d2ladder_init() failed");
				return -1;
			}

			if (cl_init(DEFAULT_HASHTBL_LEN, DEFAULT_GS_MAX) == -1)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "cl_init() failed");
				return -1;
			}

			if (psock_init() < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "psock_init() failed");
				return -1;
			}

			sd = psock_socket(PSOCK_PF_INET, PSOCK_SOCK_STREAM, PSOCK_IPPROTO_TCP);
			if (sd == -1)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "psock_socket() failed : %s", pstrerror(psock_errno()));
				return -1;
			}

			val = 1;
			if (psock_setsockopt(sd, PSOCK_SOL_SOCKET, PSOCK_SO_REUSEADDR, &val, sizeof(val)) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "psock_setsockopt() failed : %s", pstrerror(psock_errno()));
			}

			if (!(servaddr = addr_create_str(d2dbs_prefs_get_servaddrs(), INADDR_ANY, DEFAULT_LISTEN_PORT)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not get servaddr");
				return -1;
			}

			sinInterface.sin_family = PSOCK_AF_INET;
			sinInterface.sin_addr.s_addr = htonl(addr_get_ip(servaddr));
			sinInterface.sin_port = htons(addr_get_port(servaddr));
			if (psock_bind(sd, (struct sockaddr*)&sinInterface, (psock_t_socklen)sizeof(struct sockaddr_in)) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "psock_bind() failed : %s", pstrerror(psock_errno()));
				return -1;
			}
			if (psock_listen(sd, LISTEN_QUEUE) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "psock_listen() failed : %s", pstrerror(psock_errno()));
				return -1;
			}
			addr_destroy(servaddr);
			return sd;
		}


		/* dbs_server_setup_fdsets
		 * Set up the three FD sets used with select() with the sockets in the
		 * connection list.  Also add one for the listener socket, if we have
		 * one.
		 */

		int dbs_server_setup_fdsets(t_psock_fd_set * pReadFDs, t_psock_fd_set * pWriteFDs, t_psock_fd_set * pExceptFDs, int lsocket)
		{
			t_elem const * elem;
			t_d2dbs_connection* it;
			int highest_fd;

			PSOCK_FD_ZERO(pReadFDs);
			PSOCK_FD_ZERO(pWriteFDs);
			PSOCK_FD_ZERO(pExceptFDs); /* FIXME: don't check these... remove this code */
			/* Add the listener socket to the read and except FD sets, if there is one. */
			if (lsocket >= 0) {
				PSOCK_FD_SET(lsocket, pReadFDs);
				PSOCK_FD_SET(lsocket, pExceptFDs);
			}
			highest_fd = lsocket;

			LIST_TRAVERSE_CONST(dbs_server_connection_list, elem)
			{
				if (!(it = (t_d2dbs_connection*)elem_get_data(elem))) continue;
				if (it->nCharsInReadBuffer < (kBufferSize - kMaxPacketLength)) {
					/* There's space in the read buffer, so pay attention to incoming data. */
					PSOCK_FD_SET(it->sd, pReadFDs);
				}
				if (it->nCharsInWriteBuffer > 0) {
					PSOCK_FD_SET(it->sd, pWriteFDs);
				}
				PSOCK_FD_SET(it->sd, pExceptFDs);
				if (highest_fd < it->sd) highest_fd = it->sd;
			}
			return highest_fd;
		}

		/* dbs_server_read_data
		 * Data came in on a client socket, so read it into the buffer.  Returns
		 * false on failure, or when the client closes its half of the
		 * connection.  (EAGAIN doesn't count as a failure.)
		 */
		bool dbs_server_read_data(t_d2dbs_connection* conn)
		{
			int nBytes;

			nBytes = net_recv(conn->sd, conn->ReadBuf + conn->nCharsInReadBuffer,
				kBufferSize - conn->nCharsInReadBuffer);

			if (nBytes < 0) return false;
			conn->nCharsInReadBuffer += nBytes;
			return true;
		}


		/* dbs_server_write_data
		 * The connection is writable, so send any pending data.  Returns
		 * false on failure.  (EAGAIN doesn't count as a failure.)
		 */
		bool dbs_server_write_data(t_d2dbs_connection* conn)
		{
			int nBytes;

			nBytes = net_send(conn->sd, conn->WriteBuf,
				conn->nCharsInWriteBuffer > kMaxPacketLength ? kMaxPacketLength : conn->nCharsInWriteBuffer);

			if (nBytes < 0) return false;

			conn->nCharsInWriteBuffer -= nBytes;
			if (conn->nCharsInWriteBuffer)
				std::memmove(conn->WriteBuf, conn->WriteBuf + nBytes, conn->nCharsInWriteBuffer);

			return true;
		}

		int dbs_server_list_add_socket(int sd, unsigned int ipaddr)
		{
			t_d2dbs_connection	*it;
			struct in_addr		in;

			it = (t_d2dbs_connection*)xmalloc(sizeof(t_d2dbs_connection));
			std::memset(it, 0, sizeof(t_d2dbs_connection));
			it->sd = sd;
			it->ipaddr = ipaddr;
			it->major = 0;
			it->minor = 0;
			it->type = 0;
			it->stats = 0;
			it->verified = 0;
			it->serverid = get_preset_d2gsid(ipaddr);
			it->last_active = std::time(NULL);
			it->nCharsInReadBuffer = 0;
			it->nCharsInWriteBuffer = 0;
			list_append_data(dbs_server_connection_list, it);
			in.s_addr = htonl(ipaddr);
			std::strncpy((char*)it->serverip, inet_ntoa(in), sizeof(it->serverip) - 1);

			return 1;
		}

		static int dbs_handle_timed_events(void)
		{
			static	std::time_t		prev_ladder_save_time = 0;
			static	std::time_t		prev_keepalive_save_time = 0;
			static  std::time_t		prev_timeout_checktime = 0;
			std::time_t			now;

			now = std::time(NULL);
			if (now - prev_ladder_save_time > (signed)prefs_get_laddersave_interval()) {
				d2ladder_saveladder();
				prev_ladder_save_time = now;
			}
			if (now - prev_keepalive_save_time > (signed)prefs_get_keepalive_interval()) {
				dbs_keepalive();
				prev_keepalive_save_time = now;
			}
			if (now - prev_timeout_checktime > (signed)d2dbs_prefs_get_timeout_checkinterval()) {
				dbs_check_timeout();
				prev_timeout_checktime = now;
			}
			return 0;
		}

		void dbs_server_loop(int lsocket)
		{
			struct sockaddr_in sinRemote;
			int sd;
			fd_set ReadFDs, WriteFDs, ExceptFDs;
			t_elem * elem;
			t_d2dbs_connection* it;
			bool bOK;
			const char* pcErrorType;
			struct timeval         tv;
			int highest_fd;
			psock_t_socklen nAddrSize = sizeof(sinRemote);

			while (1) {

#ifdef WIN32
				if (g_ServiceStatus < 0 && kbhit() && getch() == 'q')
					d2dbs_signal_quit_wrapper();
				if (g_ServiceStatus == 0) d2dbs_signal_quit_wrapper();

				while (g_ServiceStatus == 2) Sleep(1000);
#endif

				if (d2dbs_handle_signal() < 0) break;

				dbs_handle_timed_events();
				highest_fd = dbs_server_setup_fdsets(&ReadFDs, &WriteFDs, &ExceptFDs, lsocket);

				tv.tv_sec = 0;
				tv.tv_usec = SELECT_TIME_OUT;
				switch (psock_select(highest_fd + 1, &ReadFDs, &WriteFDs, &ExceptFDs, &tv)) {
				case -1:
					eventlog(eventlog_level_error, __FUNCTION__, "psock_select() failed : %s", pstrerror(psock_errno()));
					continue;
				case 0:
					continue;
				default:
					break;
				}

				if (PSOCK_FD_ISSET(lsocket, &ReadFDs)) {
					sd = psock_accept(lsocket, (struct sockaddr*)&sinRemote, &nAddrSize);
					if (sd == -1) {
						eventlog(eventlog_level_error, __FUNCTION__, "psock_accept() failed : %s", pstrerror(psock_errno()));
						return;
					}

					eventlog(eventlog_level_info, __FUNCTION__, "accepted connection from %s:%d , socket %d .",
						inet_ntoa(sinRemote.sin_addr), ntohs(sinRemote.sin_port), sd);
					eventlog_step(prefs_get_logfile_gs(), eventlog_level_info, __FUNCTION__, "accepted connection from %s:%d , socket %d .",
						inet_ntoa(sinRemote.sin_addr), ntohs(sinRemote.sin_port), sd);
					setsockopt_keepalive(sd);
					dbs_server_list_add_socket(sd, ntohl(sinRemote.sin_addr.s_addr));
					if (psock_ctl(sd, PSOCK_NONBLOCK) < 0) {
						eventlog(eventlog_level_error, __FUNCTION__, "could not set TCP socket [%d] to non-blocking mode (closing connection) (psock_ctl: %s)", sd, pstrerror(psock_errno()));
						psock_close(sd);
					}
				}
				else if (PSOCK_FD_ISSET(lsocket, &ExceptFDs)) {
					eventlog(eventlog_level_error, __FUNCTION__, "exception on listening socket");
					/* FIXME: exceptions are not errors with TCP, they are out-of-band data */
					return;
				}

				LIST_TRAVERSE(dbs_server_connection_list, elem)
				{
					bOK = true;
					pcErrorType = 0;

					if (!(it = (t_d2dbs_connection*)elem_get_data(elem))) continue;
					if (PSOCK_FD_ISSET(it->sd, &ExceptFDs)) {
						bOK = false;
						pcErrorType = "General socket error"; /* FIXME: no no no no no */
						PSOCK_FD_CLR(it->sd, &ExceptFDs);
					}
					else {

						if (PSOCK_FD_ISSET(it->sd, &ReadFDs)) {
							bOK = dbs_server_read_data(it);
							pcErrorType = "Read error";
							PSOCK_FD_CLR(it->sd, &ReadFDs);
						}

						if (PSOCK_FD_ISSET(it->sd, &WriteFDs)) {
							bOK = dbs_server_write_data(it);
							pcErrorType = "Write error";
							PSOCK_FD_CLR(it->sd, &WriteFDs);
						}
					}

					if (!bOK) {
						int	err, errno2;
						psock_t_socklen	errlen;

						err = 0;
						errlen = sizeof(err);
						errno2 = psock_errno();

						if (psock_getsockopt(it->sd, PSOCK_SOL_SOCKET, PSOCK_SO_ERROR, &err, &errlen) == 0) {
							if (errlen && err != 0) {
								err = err ? err : errno2;
								eventlog(eventlog_level_error, __FUNCTION__, "data socket error : %s(%d)", pstrerror(err), err);
							}
						}
						dbs_server_shutdown_connection(it);
						list_remove_elem(dbs_server_connection_list, &elem);
					}
					else {
						if (dbs_packet_handle(it) == -1) {
							eventlog(eventlog_level_error, __FUNCTION__, "dbs_packet_handle() failed");
							dbs_server_shutdown_connection(it);
							list_remove_elem(dbs_server_connection_list, &elem);
						}
					}
				}
			}
		}

		static void dbs_on_exit(void)
		{
			t_elem * elem;
			t_d2dbs_connection * it;

			if (dbs_server_listen_socket >= 0)
				psock_close(dbs_server_listen_socket);
			dbs_server_listen_socket = -1;

			LIST_TRAVERSE(dbs_server_connection_list, elem)
			{
				if (!(it = (t_d2dbs_connection*)elem_get_data(elem))) continue;
				dbs_server_shutdown_connection(it);
				list_remove_elem(dbs_server_connection_list, &elem);
			}
			cl_destroy();
			d2dbs_d2ladder_destroy();
			list_destroy(dbs_server_connection_list);
			if (preset_d2gsid_head)
			{
				t_preset_d2gsid * curr;
				t_preset_d2gsid * next;

				for (curr = preset_d2gsid_head; curr; curr = next)
				{
					next = curr->next;
					xfree(curr);
				}
			}
			eventlog(eventlog_level_info, __FUNCTION__, "dbserver stopped");
		}

		int dbs_server_shutdown_connection(t_d2dbs_connection* conn)
		{
			psock_shutdown(conn->sd, PSOCK_SHUT_RDWR);
			psock_close(conn->sd);
			if (conn->verified && conn->type == CONNECT_CLASS_D2GS_TO_D2DBS) {
				eventlog(eventlog_level_info, __FUNCTION__, "unlock all characters on gs %s(%d)", conn->serverip, conn->serverid);
				eventlog_step(prefs_get_logfile_gs(), eventlog_level_info, __FUNCTION__, "unlock all characters on gs %s(%d)", conn->serverip, conn->serverid);
				eventlog_step(prefs_get_logfile_gs(), eventlog_level_info, __FUNCTION__, "close connection to gs on socket %d", conn->sd);
				cl_unlock_all_char_by_gsid(conn->serverid);
			}
			xfree(conn);
			return 1;
		}

		static int setsockopt_keepalive(int sock)
		{
			int		optval;
			psock_t_socklen	optlen;

			optval = 1;
			optlen = sizeof(optval);
			if (psock_setsockopt(sock, PSOCK_SOL_SOCKET, PSOCK_SO_KEEPALIVE, &optval, optlen)) {
				eventlog(eventlog_level_info, __FUNCTION__, "failed set KEEPALIVE for socket %d, errno=%d", sock, psock_errno());
				return -1;
			}
			else {
				eventlog(eventlog_level_info, __FUNCTION__, "set KEEPALIVE option for socket %d", sock);
				return 0;
			}
		}

		static unsigned int get_preset_d2gsid(unsigned int ipaddr)
		{
			t_preset_d2gsid		*pgsid;

			pgsid = preset_d2gsid_head;
			while (pgsid)
			{
				if (pgsid->ipaddr == ipaddr)
					return pgsid->d2gsid;
				pgsid = pgsid->next;
			}
			/* not found, build a new item */
			pgsid = (t_preset_d2gsid*)xmalloc(sizeof(t_preset_d2gsid));
			pgsid->ipaddr = ipaddr;
			pgsid->d2gsid = ++dbs_packet_gs_id;
			/* add to list */
			pgsid->next = preset_d2gsid_head;
			preset_d2gsid_head = pgsid;
			return preset_d2gsid_head->d2gsid;
		}

	}

}
