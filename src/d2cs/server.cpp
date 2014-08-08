/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
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
#include "server.h"

#include <cstring>
#include <ctime>

#ifdef WIN32
# include <conio.h>
#endif

#include "compat/psock.h"
#include "compat/strerror.h"
#include "common/addr.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "prefs.h"
#include "net.h"
#include "connection.h"
#include "serverqueue.h"
#include "gamequeue.h"
#include "d2gs.h"
#include "s2s.h"
#include "handle_signal.h"
#include "game.h"
#include "d2ladder.h"
#include "common/setup_after.h"


#ifdef WIN32
extern int g_ServiceStatus;
#endif

namespace pvpgn
{

namespace d2cs
{

static int server_listen(void);
static int server_accept(int sock);
static int server_handle_timed_event(void);
static int server_handle_socket(void);
static int server_loop(void);
static int server_cleanup(void);

t_addrlist		* server_listen_addrs;

static int server_listen(void)
{
	t_addr		* curr_laddr;
	t_addr_data	laddr_data;
	int		sock;

	if (!(server_listen_addrs=addrlist_create(prefs_get_servaddrs(),INADDR_ANY,D2CS_SERVER_PORT))) {
		eventlog(eventlog_level_error,__FUNCTION__,"error create listening address list");
		return -1;
	}
	BEGIN_LIST_TRAVERSE_DATA(server_listen_addrs,curr_laddr,t_addr)
	{
		sock=net_listen(addr_get_ip(curr_laddr),addr_get_port(curr_laddr),PSOCK_SOCK_STREAM);
		if (sock<0) {
			eventlog(eventlog_level_error,__FUNCTION__,"error listen socket");
			return -1;
		}

		if (psock_ctl(sock,PSOCK_NONBLOCK)<0) {
			eventlog(eventlog_level_error,__FUNCTION__,"error set listen socket in non-blocking mode");
		}

		laddr_data.i = sock;
		addr_set_data(curr_laddr,laddr_data);

		if (fdwatch_add_fd(sock, fdwatch_type_read, d2cs_server_handle_accept, curr_laddr)<0) {
		    eventlog(eventlog_level_error,__FUNCTION__,"error adding socket %d to fdwatch pool (max sockets?)",sock);
		    psock_close(sock);
		    return -1;
		}

		eventlog(eventlog_level_info,__FUNCTION__,"listen on %s", addr_num_to_addr_str(addr_get_ip(curr_laddr),addr_get_port(curr_laddr)));
	}
	END_LIST_TRAVERSE_DATA()
	return 0;
}

static int server_accept(int sock)
{
	int			csock;
	struct sockaddr_in	caddr, raddr;
	psock_t_socklen		caddr_len, raddr_len;
	int			val;
	unsigned int		ip;
	unsigned short		port;
	t_connection	       *cc;

	caddr_len=sizeof(caddr);
	std::memset(&caddr,0,sizeof(caddr));
	csock=psock_accept(sock,(struct sockaddr *)&caddr,&caddr_len);
	if (csock<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error accept new connection");
		return -1;
	} else eventlog(eventlog_level_info,__FUNCTION__,"accept connection from %s",inet_ntoa(caddr.sin_addr));

	val=1;
	if (psock_setsockopt(csock, PSOCK_SOL_SOCKET, PSOCK_SO_KEEPALIVE, &val,sizeof(val))<0) {
		eventlog(eventlog_level_warn,__FUNCTION__,"error set sock option keep alive");
	}
	if (psock_ctl(csock, PSOCK_NONBLOCK)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error set socket to non-blocking mode");
		psock_close(csock);
		return -1;
	}

	raddr_len=sizeof(raddr);
	std::memset(&raddr,0,sizeof(raddr));
	ip=port=0;
	if (psock_getsockname(csock,(struct sockaddr *)&raddr,&raddr_len)<0) {
		eventlog(eventlog_level_warn,__FUNCTION__,"unable to get local socket info");
	} else {
		if (raddr.sin_family!=PSOCK_AF_INET) {
			eventlog(eventlog_level_warn,__FUNCTION__,"got bad socket family %d",raddr.sin_family);
		} else {
			ip=ntohl(raddr.sin_addr.s_addr);
			port=ntohs(raddr.sin_port);
		}
	}
	if (!(cc = d2cs_conn_create(csock,ip,port,ntohl(caddr.sin_addr.s_addr),ntohs(caddr.sin_port)))) {
		eventlog(eventlog_level_error,__FUNCTION__,"error create new connection");
		psock_close(csock);
		return -1;
	}
	if (conn_add_fd(cc, fdwatch_type_read, d2cs_server_handle_tcp)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error adding socket %d to fdwatch pool (max sockets?)",csock);
		d2cs_conn_set_state(cc,conn_state_destroy);
		return -1;
	}

	return 0;
}

static int server_handle_timed_event(void)
{
	static  std::time_t	prev_list_purgetime=0;
	static  std::time_t	prev_gamequeue_checktime=0;
	static	std::time_t	prev_s2s_checktime=0;
	static	std::time_t	prev_sq_checktime=0;
	static  std::time_t	prev_d2ladder_refresh_time=0;
	static  std::time_t	prev_s2s_keepalive_time=0;
	static  std::time_t	prev_timeout_checktime;
	std::time_t		now;

	now=std::time(NULL);
	if (now-prev_list_purgetime>(signed)prefs_get_list_purgeinterval()) {
		hashtable_purge(d2cs_connlist());
		d2cs_gamelist_check_voidgame();
		prev_list_purgetime=now;
	}
	if (now-prev_gamequeue_checktime>(signed)prefs_get_gamequeue_checkinterval()) {
		gqlist_update_all_clients();
		prev_gamequeue_checktime=now;
	}
	if (now-prev_s2s_checktime>(signed)prefs_get_s2s_retryinterval()) {
		s2s_check();
		prev_s2s_checktime=now;
	}
	if (now-prev_sq_checktime>(signed)prefs_get_sq_checkinterval()) {
		sqlist_check_timeout();
		prev_sq_checktime=now;
	}
	if (now-prev_d2ladder_refresh_time>(signed)prefs_get_d2ladder_refresh_interval()) {
		d2ladder_refresh();
		prev_d2ladder_refresh_time=now;
	}
	if (now-prev_s2s_keepalive_time>(signed)prefs_get_s2s_keepalive_interval()) {
		d2gs_keepalive();
		prev_s2s_keepalive_time=now;
	}
	if (now-prev_timeout_checktime>(signed)prefs_get_timeout_checkinterval()) {
		connlist_check_timeout();
		prev_timeout_checktime=now;
	}
	return 0;
}

extern int d2cs_server_handle_accept(void *data, t_fdwatch_type rw)
{
    int sock;

    sock = addr_get_data((t_addr *)data).i;
    server_accept(sock);
    return 0;
}

extern int d2cs_server_handle_tcp(void *data, t_fdwatch_type rw)
{
    t_connection *c = (t_connection *)data;

    if (rw & fdwatch_type_read) conn_add_socket_flag(c,SOCKET_FLAG_READ);
    if (rw & fdwatch_type_write) conn_add_socket_flag(c,SOCKET_FLAG_WRITE);
    if (conn_handle_socket(c)<0)
        d2cs_conn_set_state(c, conn_state_destroy);

    return 0;
}

static int server_handle_socket(void)
{
	switch (fdwatch(BNETD_POLL_INTERVAL)) {
		case -1:
			if (
#ifdef PSOCK_EINTR
			    psock_errno()!=PSOCK_EINTR &&
#endif
			    1) {
				eventlog(eventlog_level_error,__FUNCTION__,"select failed (select: %s)",pstrerror(psock_errno()));
				return -1;
			}
			/* fall through */
		case 0:
			return 0;
		default:
			break;
	}

	fdwatch_handle();

	return 0;
}

static int server_loop(void)
{
	unsigned int count;

	count=0;
	while (1) {

#ifdef WIN32
	if (g_ServiceStatus<0 && kbhit() && getch()=='q')
	    signal_quit_wrapper();
	if (g_ServiceStatus == 0) signal_quit_wrapper();

	while (g_ServiceStatus == 2) Sleep(1000);
#endif

		if (handle_signal()<0) break;
		if (++count>=(1000/BNETD_POLL_INTERVAL)) {
			server_handle_timed_event();
			count=0;
		}
		server_handle_socket();
		d2cs_connlist_reap();
	}
	return 0;
}

static int server_cleanup(void)
{
	t_addr		* curr_laddr;
	int		sock;

	BEGIN_LIST_TRAVERSE_DATA(server_listen_addrs,curr_laddr,t_addr)
	{
		sock=addr_get_data(curr_laddr).i;
		psock_close(sock);
	}
	END_LIST_TRAVERSE_DATA()
	addrlist_destroy(server_listen_addrs);
	return 0;
}

extern int d2cs_server_process(void)
{
#ifndef WIN32
	handle_signal_init();
#endif
	if (psock_init()<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"failed to init network");
		return -1;
	}
	eventlog(eventlog_level_info,__FUNCTION__,"network initialized");
	if (s2s_init()<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"failed to init s2s connection");
		return -1;
	}
	if (server_listen()<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"failed to setup listen socket");
		return -1;
	}
	eventlog(eventlog_level_info,__FUNCTION__,"entering server loop");
	server_loop();
	eventlog(eventlog_level_info,__FUNCTION__,"std::exit from server loop,cleanup");
	server_cleanup();
	return 0;
}

}

}
