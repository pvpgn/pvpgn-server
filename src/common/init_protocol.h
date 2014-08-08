/*
 * Copyright (C) 1998,1999  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_INIT_PROTOCOL_TYPES
#define INCLUDED_INIT_PROTOCOL_TYPES

#ifdef JUST_NEED_TYPES
# include "common/bn_type.h"
#else
# define JUST_NEED_TYPES
# include "common/bn_type.h"
# undef JUST_NEED_TYPES
#endif

namespace pvpgn
{

	/* There is no header for this packet class and there is only
	 * one packet type.
	 */

	/*
	 * There is a single byte sent upon initial connections that
	 * reports the connection type.
	 */

	/******************************************************/
#define CLIENT_INITCONN 0x1
	typedef struct
	{
		bn_byte cclass;
	} PACKED_ATTR() t_client_initconn;
	/******************************************************/

}

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_INIT_PROTOCOL_PROTOS
#define INCLUDED_INIT_PROTOCOL_PROTOS

#define CLIENT_INITCONN_CLASS_BNET		0x01 /* standard bnet protocol */
#define CLIENT_INITCONN_CLASS_FILE		0x02 /* BNFTP */
#define CLIENT_INITCONN_CLASS_BOT		0x03
#define CLIENT_INITCONN_CLASS_ENC		0x04 /* encrypted connection */
#define CLIENT_INITCONN_CLASS_TELNET		0x0d /* Hack alert: look for user to hit \r when they connect */
#define CLIENT_INITCONN_CLASS_D2CS		0x01
#define CLIENT_INITCONN_CLASS_D2GS		0x64
#define CLIENT_INITCONN_CLASS_D2CS_BNETD	0x65
#define CLIENT_INITCONN_CLASS_LOCALMACHINE	0x98 /* local computer connecting via 127.0.0.1 */

#endif
#endif
