/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_UDP_PROTOCOL_TYPES
#define INCLUDED_UDP_PROTOCOL_TYPES

namespace pvpgn
{

	/*
	 * This file describes the UDP chat packets. The game packets aren't
	 * in here and would probably go in another file.  The first 4 bytes
	 * seem to be a little-endian 32 bit integer id for the packet type.
	 * FIXME: add more UDP packet types
	 */
	/******************************************************/
	typedef struct
	{
		bn_int type;
	} PACKED_ATTR() t_udp_header;
	/******************************************************/


	/******************************************************/
	typedef struct
	{
		t_udp_header h;
	} PACKED_ATTR() t_udp_generic;
	/******************************************************/


	/******************************************************/
	/* client echos back with UDPOK packet */
	/*
	05 00 00 00 74 65 6E 62                              ....tenb
	*/
#define SERVER_UDPTEST 0x00000005
	typedef struct
	{
		t_udp_header h;
		bn_int       bnettag;  /* 74 65 6E 62 */
	} PACKED_ATTR() t_server_udptest;
	/******************************************************/


	/******************************************************/
	/*
	07 00 00 00 49 41 19 00                              ....IA..

	07 00 00 00 11 60 1D 00                              .....`..
	*/
#define CLIENT_UDPPING 0x00000007
	typedef struct
	{
		t_udp_header h;
		bn_int       unknown1;  /* time? */
	} PACKED_ATTR() t_client_udpping;
	/******************************************************/


	/******************************************************/
	/*
	From Brood War 1.04
	08 00 00 00 75 EC A0 28                              ....u..(
	*/
#define CLIENT_SESSIONADDR1 0x00000008
	typedef struct
	{
		t_udp_header h;
		bn_int       sessionkey;
	} PACKED_ATTR() t_client_sessionaddr1;
	/******************************************************/


	/******************************************************/
	/*
	From Brood War 1.07
	09 00 00 00 7A 11 07 ED   9D DF 01 00                ....z.......
	*/
#define CLIENT_SESSIONADDR2 0x00000009
	typedef struct
	{
		t_udp_header h;
		bn_int       sessionkey;
		bn_int       sessionnum;
	} PACKED_ATTR() t_client_sessionaddr2;
	/******************************************************/

}

#endif
