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
#include "net.h"

#include <cctype>
#include <cstring>

#include "compat/strerror.h"
#include "common/eventlog.h"
#include "compat/psock.h"
#include "common/setup_after.h"

namespace pvpgn
{

namespace d2cs
{


/* FIXME: use addr.h code to do this and make a separate error
 * return path so that 255.255.255.255 isn't an error.
 */
extern unsigned long int net_inet_addr(char const * host)
{
	struct hostent	* hp;

	if (std::isdigit((int)host[0])) {
		return inet_addr(host);
	} else {
		hp=gethostbyname(host);
		if (!hp || !(hp->h_addr_list)) {
			return ~0UL;
		}
		return *(unsigned long *)hp->h_addr_list[0];
	}
}

extern int net_socket(int type)
{
	int	sock;
	int	val;
	int     ipproto;

	if (type==PSOCK_SOCK_STREAM) {
		ipproto = PSOCK_IPPROTO_TCP;
	} else {
		ipproto = PSOCK_IPPROTO_UDP;
	}
	if ((sock=psock_socket(PSOCK_PF_INET, type, ipproto))<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error creating socket (psock_socket: %s)", pstrerror(psock_errno()));
		return -1;
	}
	val=1;
	if (psock_setsockopt(sock,PSOCK_SOL_SOCKET, PSOCK_SO_KEEPALIVE, &val, sizeof(val))<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error set socket option KEEPALIVE (psock_setsockopt: %s)",pstrerror(psock_errno()));
	}
	if (psock_ctl(sock,PSOCK_NONBLOCK)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error set socket mode to non-block (psock_ctl: %s)",pstrerror(psock_errno()));
		psock_close(sock);
		return -1;
	}
	return sock;
}

extern int net_check_connected(int sock)
{
	int		err;
	psock_t_socklen	errlen;

	err = 0;
	errlen = sizeof(err);
	if (psock_getsockopt(sock,PSOCK_SOL_SOCKET, PSOCK_SO_ERROR, &err, &errlen)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error get socket option SO_ERROR (psock_getsockopt: %s)",pstrerror(psock_errno()));
		return -1;
	}
	if (errlen && err)
	       return -1;
	return 0;
}


extern int net_listen(unsigned int ip, unsigned int port, int type)
{
	int			sock;
	int			val;
	struct  sockaddr_in	addr;
	int    			ipproto;

	if (type==PSOCK_SOCK_STREAM) {
		ipproto = PSOCK_IPPROTO_TCP;
	} else {
		ipproto = PSOCK_IPPROTO_UDP;
	}
	if ((sock=psock_socket(PSOCK_PF_INET, type, ipproto))<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error create listen socket");
		return -1;
	}
	val=1;
	if (psock_setsockopt(sock,PSOCK_SOL_SOCKET,PSOCK_SO_REUSEADDR,&val,sizeof(int))<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error set socket option SO_REUSEADDR");
	}
	std::memset(&addr,0,sizeof(addr));
	addr.sin_family=PSOCK_AF_INET;
	addr.sin_port=htons(port);
	addr.sin_addr.s_addr=htonl(ip);
	if (psock_bind(sock,(struct sockaddr *)&addr, sizeof(addr))<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error bind listen socket");
		psock_close(sock);
		return -1;
	}
	if (psock_listen(sock,LISTEN_QUEUE)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error listen socket");
		psock_close(sock);
		return -1;
	}
	return sock;
}

extern int net_send_data(int sock, char * buff, int buffsize, int * pos, int * currsize)
{
	int	nsend;

	ASSERT(buff,-1);
	ASSERT(pos,-1);
	if (*pos>buffsize) {
		eventlog(eventlog_level_error,__FUNCTION__,"[%d] send buffer overflow pos=%d buffsize=%d",sock,*pos,buffsize);
		return -1;
	}
	if (*currsize>*pos) {
		eventlog(eventlog_level_error,__FUNCTION__,"[%d] send buffer error currsize=%d pos=%d",sock,*currsize,*pos);
		return -1;
	}
	nsend=psock_send(sock,buff+*pos-*currsize,*currsize,0);
	if (nsend==0) {
		eventlog(eventlog_level_error,__FUNCTION__,"[%d] no data sent (close connection)",sock);
		return -1;
	}
	if (nsend<0) {
		if (
#ifdef PSOCK_EINTR
			psock_errno()==PSOCK_EINTR ||
#endif
#ifdef PSOCK_EAGAIN
			psock_errno()==PSOCK_EAGAIN ||
#endif
#ifdef PSOCK_EWOULDBLOCK
			psock_errno()==PSOCK_EWOULDBLOCK ||
#endif
#ifdef PSOCK_NOBUFS
			psock_errno()==PSOCK_ENOBUFS ||
#endif
#ifdef PSOCK_ENOMEM
			psock_errno()==ENOMEM ||
#endif
		0)
			return 0;

		eventlog(eventlog_level_error,__FUNCTION__,"[%d] error sent data (closing connection) (psock_send: %s)",sock,pstrerror(psock_errno()));
		return -1;
	}
	*currsize -= nsend;
	return 1;
}

extern int net_recv_data(int sock, char * buff, int buffsize, int * pos, int * currsize)
{
	int	nrecv;

	ASSERT(buff,-1);
	ASSERT(pos,-1);
	if (*pos>buffsize) {
		eventlog(eventlog_level_error,__FUNCTION__,"[%d] recv buffer overflow pos=%d buffsize=%d",sock,*pos,buffsize);
		return -1;
	}
	if (*currsize>*pos) {
		eventlog(eventlog_level_error,__FUNCTION__,"[%d] recv buffer error currsize=%d pos=%d",sock,*currsize,*pos);
		return -1;
	}
	if (*pos==buffsize) {
		std::memmove(buff,buff+*pos,*currsize);
		*pos=0;
	}
	nrecv=psock_recv(sock,buff+*pos,buffsize-*pos,0);
	if (nrecv==0) {
		eventlog(eventlog_level_info,__FUNCTION__,"[%d] remote host closed connection",sock);
		return -1;
	}
	if (nrecv<0) {
		if (
#ifdef PSOCK_EINTR
			psock_errno()==PSOCK_EINTR ||
#endif
#ifdef PSOCK_EAGAIN
			psock_errno()==PSOCK_EAGAIN ||
#endif
#ifdef PSOCK_EWOULDBLOCK
			psock_errno()==PSOCK_EWOULDBLOCK ||
#endif
#ifdef PSOCK_ENOMEM
			psock_errno()==ENOMEM ||
#endif
		0)
			return 0;

		if (
#ifdef PSOCK_ENOTCONN
			psock_errno()==PSOCK_ENOTCONN ||
#endif
#ifdef PSOCK_ECONNRESET
			psock_errno()==PSOCK_ECONNRESET ||
#endif
		0) {
		eventlog(eventlog_level_error,__FUNCTION__,"[%d] remote host closed connection (psock_recv: %s)",sock,pstrerror(psock_errno()));
		return -1;
		}
		eventlog(eventlog_level_error,__FUNCTION__,"[%d] recv error (closing connection) (psock_recv: %s)",sock,pstrerror(psock_errno()));
		return -1;
	}
	* currsize += nrecv;
	return 1;
}

}

}
