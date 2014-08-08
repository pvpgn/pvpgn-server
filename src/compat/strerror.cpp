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

#ifdef WIN32
#include <cstring>
#include <winsock2.h>

namespace pvpgn
{

	extern char const * pstrerror(int errornum)
	{
		switch (errornum)
		{
		case WSAEINTR:		return "Interrupted function call";
		case WSAEACCES:		return "Permission denied";
		case WSAEFAULT:		return "Bad address";
		case WSAEINVAL:		return "Inavlid argument";
		case WSAEMFILE:		return "Too many open files";
		case WSAEWOULDBLOCK:	return "Resource temorarily unavailable";
		case WSAEINPROGRESS:	return "Operation now in progress";
		case WSAEALREADY:		return "Operation already in progress";
		case WSAENOTSOCK:		return "Socket operation on nonsocket";
		case WSAEDESTADDRREQ:	return "Destination address required";
		case WSAEMSGSIZE:		return "Message too long";
		case WSAEPROTOTYPE:		return "Protocol wrong type fpr socket";
		case WSAENOPROTOOPT:	return "Bad protocol option";
		case WSAEPROTONOSUPPORT:	return "Protocol not supported";
		case WSAESOCKTNOSUPPORT:	return "Socket type not supported";
		case WSAEOPNOTSUPP:		return "Operation not supported";
		case WSAEPFNOSUPPORT:	return "Protocol family not supported";
		case WSAEAFNOSUPPORT:	return "Address family not supported by protocol family";
		case WSAEADDRINUSE:		return "Address already in use";
		case WSAEADDRNOTAVAIL:	return "Cannot assign requested address";
		case WSAENETDOWN:		return "Network is down";
		case WSAENETUNREACH:	return "Network is unreachable";
		case WSAENETRESET:		return "Network dropped connection on reset";
		case WSAECONNABORTED:	return "Software caused connection abort";
		case WSAECONNRESET:		return "Connection reset by peer";
		case WSAENOBUFS:		return "No buffer space available";
		case WSAEISCONN:		return "Socket is already connected";
		case WSAENOTCONN:		return "Socket is not connected";
		case WSAESHUTDOWN:		return "Cannot send after socket shutdown";
		case WSAETIMEDOUT:		return "Connection timed out";
		case WSAECONNREFUSED:	return "Connection refused";
		case WSAEHOSTDOWN:		return "Host is down";
		case WSAEHOSTUNREACH:	return "No route to host";
		case WSAEPROCLIM:		return "Too many processes";
		case WSASYSNOTREADY:	return "Network subsystem is unavailable";
		case WSAVERNOTSUPPORTED:	return "Winsock.dll version out of range";
		case WSANOTINITIALISED:	return "Successful WSAStartup not yet performed";
		case WSAEDISCON:		return "Graceful shutdown in progress";
		case WSATYPE_NOT_FOUND:	return "Class type not found";
		case WSAHOST_NOT_FOUND:	return "Host not found";
		case WSATRY_AGAIN:		return "Nonauthoriative host not found";
		case WSANO_RECOVERY:	return "This is a nonrecoverable error";
		case WSANO_DATA:		return "Valid name, no data record of requested type";
		case WSA_INVALID_HANDLE:	return "Specified event object handle is invalid";
		case WSA_INVALID_PARAMETER:	return "One or more parameters are invalid";
		case WSA_IO_INCOMPLETE:	return "Overlapped I/O event object not in signaled state";
		case WSA_IO_PENDING:	return "Overlapped operation will complete later";
		case WSA_NOT_ENOUGH_MEMORY:	return "Insufficient memory available";
		case WSA_OPERATION_ABORTED:	return "Overlapped operation aborted";
		case WSASYSCALLFAILURE:	return "System call failure";
		default: return std::strerror(errornum);
		}
	}

}

#endif
