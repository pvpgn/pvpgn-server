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
#ifndef INCLUDED_FILE_PROTOCOL_TYPES
#define INCLUDED_FILE_PROTOCOL_TYPES

#ifdef JUST_NEED_TYPES
# include "common/bn_type.h"
#else
# define JUST_NEED_TYPES
# include "common/bn_type.h"
# undef JUST_NEED_TYPES
#endif

namespace pvpgn
{

	/******************************************************/
	typedef struct
	{
		bn_short size;
		bn_short type;
	} PACKED_ATTR() t_file_header;
	/******************************************************/


	/******************************************************/
	typedef struct
	{
		t_file_header h;
	} PACKED_ATTR() t_file_generic;
	/******************************************************/


	/******************************************************/
	/*
	CLIENT FILE MPQ REQ
	2D 00 00 01 36 38 58 49            -...68XI
	50 58 45 53 00 00 00 00   00 00 00 00 00 00 00 00    PXES............
	00 2C 58 E1 09 28 BC 01   49 58 38 36 76 65 72 31    .,X..(..IX86ver1
	2E 6D 70 71 00                                       .mpq.

	CLIENT FILE TOS REQ
	28 00 00 01 36 38 58 49            (...68XI
	50 58 45 53 00 00 00 00   00 00 00 00 00 00 00 00    PXES............
	30 C3 89 86 09 4F BD 01   74 6F 73 2E 74 78 74 00    0....O..tos.txt.

	CLIENT FILE AD REQ
	2D 00 00 01 36 38 58 49            -...68XI
	50 58 45 53 2B 51 02 00   2E 70 63 78 00 00 00 00    PXES+Q...pcx....
	00 00 00 00 58 01 B2 00   61 64 30 32 35 31 32 62    ....X...ad02512b
	2E 70 63 78 00                                       .pcx.
	*/
#define CLIENT_FILE_REQ 0x0100
	typedef struct
	{
		t_file_header h;
		bn_int        archtag;
		bn_int        clienttag;
		bn_int        adid;
		bn_int        extensiontag; /* unlike other tags, this one is "forward" */
		bn_int        startoffset; /* is this actually used in the original clients? */
		bn_long       timestamp;
		/* filename */
	} PACKED_ATTR() t_client_file_req;
	/******************************************************/

	/*
	the next 2 file requests appeared on w3 first. first it sends filereq2 then
	server replies with some rnadom int then client sends filereq3 (which is a
	raw packet with no valuable header) then filename then server replies with
	normal file_reply and sends the contents

	12: recv class=file[0x03] type=unknown[0x0200] length=20
	0000:   14 00 00 02 36 38 58 49   33 52 41 57 00 00 00 00    ....68XI3RAW....
	0010:   00 00 00 00                                          ....
	*/
#define CLIENT_FILE_REQ2		0x0200
	typedef struct
	{
		t_file_header h;
		bn_int        archtag;
		bn_int        clienttag;
		bn_long       unknown1;
	} PACKED_ATTR() t_client_file_req2;

#define CLIENT_FILE_REQ3		0x0000
	typedef struct
	{
		bn_int	unknown1;
		bn_long	timestamp;
		bn_long	unknown2;
		bn_long	unknown3;
		bn_long	unknown4;
		bn_long	unknown5;
		bn_long	unknown6;
		/* filename */
	} PACKED_ATTR() t_client_file_req3;

	/******************************************************/
	/* SERVER FILE MPQ REPLY
							  25 00 00 00 33 1B 00 00            %...3...
							  00 00 00 00 00 00 00 00   00 2C 58 E1 09 28 BC 01    .........,X..(..
							  49 58 38 36 76 65 72 31   2E 6D 70 71 00             IX86ver1.mpq.

							  SERVER FILE TOS REPLY
							  20 00 00 00 E4 00 00 00    P."..... .......
							  00 00 00 00 00 00 00 00   30 C3 89 86 09 4F BD 01    ........0....O..
							  74 6F 73 2E 74 78 74 00                              tos.txt.

							  SERVER FILE AD REPLY
							  25 00 00 00 30 2E 00 00            %...0...
							  2B 51 02 00 2E 70 63 78   00 00 00 00 58 01 B2 00    +Q...pcx....X...
							  61 64 30 32 35 31 32 62   2E 70 63 78 00             ad02512b.pcx.
							  */
#define SERVER_FILE_REPLY 0x0000
	typedef struct
	{
		t_file_header h;
		bn_int        filelen;
		bn_int        adid;
		bn_int        extensiontag; /* unlike other tags, this one is "forward" */
		bn_long       timestamp;
		/* filename */
	} PACKED_ATTR() t_server_file_reply;
	/******************************************************/

#define SERVER_FILE_UNKNOWN1 0xdeadbeef
	typedef struct
	{
		bn_int	unknown;
	} PACKED_ATTR() t_server_file_unknown1;

}

#endif
