/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_BNET_PROTOCOL_TYPES
#define INCLUDED_BNET_PROTOCOL_TYPES

#ifdef JUST_NEED_TYPES
# include "common/bn_type.h"
#else
# define JUST_NEED_TYPES
# include "common/bn_type.h"
# undef JUST_NEED_TYPES
#endif

/*
 * The "bnet" protocol (previously known as "normal") that is used for
 * chatting, ladder info, game listings, etc.
 * FIXME: put the #define's into the PROTO section
 */


namespace pvpgn
{

	/******************************************************/
	typedef struct
	{
		bn_short type;
		bn_short size;
	} PACKED_ATTR() t_bnet_header;
	/******************************************************/

	/* [zap-zero] 20020529 - added support for war3 anongame routing packets */
	/******************************************************/
	typedef struct
	{
		bn_short type;
		bn_short size;
	} PACKED_ATTR() t_w3route_header;
	/******************************************************/


	/******************************************************/
	typedef struct
	{
		t_bnet_header h;
	} PACKED_ATTR() t_bnet_generic;
	/******************************************************/

	/******************************************************/
	typedef struct
	{
		t_w3route_header h;
	} PACKED_ATTR() t_w3route_generic;
	/******************************************************/

	/* for unhandled pmap packets */
#define CLIENT_NULL 0xfeff


	/******************************************************/
	/*
	14: recv class=w3route[0x0c] type=unknown[0x00f7] length=53
	0000:   F7 1E 35 00 02 02 00 00   68 BF 0B 08 00 E0 17 01    ..5.....h.......
	0010:   00 00 00 42 6C 61 63 6B   72 61 74 00 08 81 3E 0C    ...Blackrat...>.
	0020:   00 20 00 00 00 02 00 17   E1 C0 A8 00 01 00 00 00    . ..............
	0030:   00 00 00 00 00                                       .....
	*/
#define CLIENT_W3ROUTE_REQ 0x1ef7
	typedef struct
	{
		t_w3route_header h;
		bn_int unknown1;
		bn_int id;			/* unique id sent by server */
		bn_byte unknown2;
		bn_short port;
		bn_int handle;		/* handle/descriptor for client <-> client communication */
		/* player name, ... */
	} PACKED_ATTR() t_client_w3route_req;
	/******************************************************/



	/******************************************************/
	/*
	f7 23 04 00
	*/
#define CLIENT_W3ROUTE_LOADINGDONE 0x23f7
	typedef struct
	{
		t_w3route_header h;
	} PACKED_ATTR() t_client_w3route_loadingdone;
	/******************************************************/


	/******************************************************/
	/*
	f7 14 05 00 00
	*/
#define SERVER_W3ROUTE_READY 0x14f7
	typedef struct
	{
		t_w3route_header h;
		bn_byte unknown1;
	} PACKED_ATTR() t_server_w3route_ready;
	/******************************************************/


	/******************************************************/
	/*
	f7 21 08 00 01 00 00 00
	*/

#define CLIENT_W3ROUTE_ABORT 0x21f7

	typedef struct
	{
		t_w3route_header h;
		bn_int unknown1;       /* count? */
	} PACKED_ATTR() t_client_w3route_abort;

	/******************************************************/
	/*
	f7 08 05 00 02
	*/
#define SERVER_W3ROUTE_LOADINGACK 0x08f7
	typedef struct
	{
		t_w3route_header h;
		bn_byte playernum;
	} PACKED_ATTR() t_server_w3route_loadingack;
	/******************************************************/


	/******************************************************/
	/*
	f7 3b 06 00 09 00
	*/
#define CLIENT_W3ROUTE_CONNECTED 0x3bf7
	typedef struct
	{
		t_w3route_header h;
		bn_short unknown1;
	} PACKED_ATTR() t_client_w3route_connected;
	/******************************************************/

	/******************************************************/
	/*
	f7 01 08 00 2e 85 2d 7b
	*/
#define SERVER_W3ROUTE_ECHOREQ 0x01f7
	typedef struct
	{
		t_w3route_header h;
		bn_int ticks;
	} PACKED_ATTR() t_server_w3route_echoreq;
	/******************************************************/


#define CLIENT_W3ROUTE_ECHOREPLY 0x46f7


	/******************************************************/
	/*
	13: recv class=w3route[0x0c] type=unknown[0x2ef7] length=107
	0000:   F7 2E 6B 00 01 01 03 00   00 00 20 00 00 00 5C 01    ..k....... ...\.
	0010:   00 00 00 05 00 00 00 00   00 00 00 00 00 00 00 1A    ................
	0020:   04 00 00 00 00 00 00 00   00 00 00 05 00 00 00 00    ................
	0030:   00 00 00 01 00 00 00 00   00 00 00 05 00 00 00 00    ................
	0040:   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
	0050:   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
	0060:   00 00 00 00 00 00 00 00   00 00 00                   ...........
	*/
#define CLIENT_W3ROUTE_GAMERESULT 0x2ef7
	/*
							   F7 3A-84 00 02 01 04 00 00 00
							   0x0040   20 00 00 00 83 09 00 00-00 00 00 00 02 03 00 00
							   0x0050   00 20 00 00 00 2C 01 00-00 00 00 00 00 5C 04 00
							   0x0060   00 00 03 00 00 00 00 00-00 00 01 00 00 00 CC 0B
							   0x0070   00 00 00 00 00 00 72 0B-00 00 0E 00 00 00 00 00
							   0x0080   00 00 04 00 00 00 01 00-00 00 15 00 00 00 00 00
							   0x0090   00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00
							   0x00A0   00 00 46 0A 00 00 2C 01-00 00 00 00 00 00 00 00
							   0x00B0   00 00 00 00 00 00 00 00-00 00

							   F7 3A-84 00 02 02 03 00 00 00
							   0x0040   20 00 00 00 2C 01 00 00-00 00 00 00 01 04 00 00
							   0x0050   00 20 00 00 00 83 09 00-00 00 00 00 00 5C 04 00
							   0x0060   00 00 01 00 00 00 01 00-00 00 00 00 00 00 39 03
							   0x0070   00 00 00 00 00 00 00 00-00 00 05 00 00 00 00 00
							   0x0080   00 00 01 00 00 00 00 00-00 00 05 00 00 00 00 00
							   0x0090   00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00
							   0x00A0   00 00 00 00 00 00 00 00-00 00 00 00 00 00 00 00
							   0x00B0   00 00 00 00 00 00 00 00-00 00
							   */

#define CLIENT_W3ROUTE_GAMERESULT_W3XP 0x3af7
	typedef struct
	{
		t_w3route_header h;
		bn_byte number_of_results;
		/*
				t_client_w3route_gameresult_player players[];   1-n times
				t_client_w3route_gameresult_part2  part2;
				t_client_w3route_gameresult_hero   heroes[];    0-n times
				t_client_w3route_gameresult_part3  part3; */
	} PACKED_ATTR() t_client_w3route_gameresult;
	/******************************************************/

	/* [zap-zero] what value is DRAW? */
#define W3_GAMERESULT_LOSS	0x00000003
#define W3_GAMERESULT_WIN	0x00000004

	typedef struct
	{
		bn_byte number;
		bn_int  result;
		bn_int  race;
		bn_int  unknown1;
		bn_int  unknown2;
	} PACKED_ATTR() t_client_w3route_gameresult_player;

	typedef struct
	{
		bn_byte unknown0; /* but could also contain info about result */
		bn_int  unknown1;
		bn_int  unknown2;
		bn_int  unknown3;
		bn_int  unknown4;
		bn_int  unit_score;
		bn_int  heroes_score;
		bn_int  resource_score;
		bn_int  units_produced;
		bn_int  units_killed;
		bn_int  buildings_produced;
		bn_int  buildings_razed;
		bn_int  largest_army;
		bn_int  heroes_used_count;
	} PACKED_ATTR() t_client_w3route_gameresult_part2;

	typedef struct
	{
		bn_short level;
		bn_int  race_and_name;
		bn_int  hero_xp;
	} PACKED_ATTR() t_client_w3route_gameresult_hero;

	typedef struct
	{
		bn_int  heroes_killed;
		bn_int  items_obtained;
		bn_int  mercenaries_hired;
		bn_int  total_hero_xp;
		bn_int  gold_mined;
		bn_int  lumber_harvested;
		bn_int  resources_traded_given;
		bn_int  resources_traded_taken;
		bn_int  tech_percentage;
		bn_int  gold_lost_to_upkeep;
	} PACKED_ATTR() t_client_w3route_gameresult_part3;

	/******************************************************/
	/*
							f7 04 1e 00 07 00 00 b2 72 76   ..............rv
							0040  ec cc cc 02 02 00 04 0b d9 e9 5f ac 00 00 00 00   .........._.....
							0050  00 00 00 00                                       ....
							*/
#define SERVER_W3ROUTE_ACK 0x04f7
	typedef struct
	{
		t_w3route_header h;
		bn_byte unknown1;	/* 07 */
		bn_short unknown2;	/* 00 00 */
		bn_int unknown3;	/* random stuff */
		bn_short unknown4;	/* cc cc */
		bn_byte playernum;	/* 1-4 */
		bn_short unknown5;	/* 0x0002 */
		bn_short port;		/* client port */
		bn_int ip;		/* client ip */
		bn_int unknown7;	/* 00 00 00 00 */
		bn_int unknown8;	/* 00 00 00 00 */
	} PACKED_ATTR() t_server_w3route_ack;
	/******************************************************/

#define SERVER_W3ROUTE_ACK_UNKNOWN3	0x484e2637

	/******************************************************/
	/*
							f7 06 3b 00 01 00 00 00 01 **   ........;......P
							0040  ** ** ** ** ** ** ** 00 08 2d 09 9d 04 08 00 00   layer00..-......
							0050  00 02 00 17 e0 [censored]  00 00 00 00 00 00 00   .......g........
							0060  00 02 00 17 e0 [censored]  00 00 00 00 00 00 00   .......g........
							0070  00

							*/
#define SERVER_W3ROUTE_PLAYERINFO 0x06f7
	typedef struct
	{
		t_w3route_header h;
		bn_int handle;
		bn_byte playernum;
		/* then:
		   opponent name
		   playerinfo2
		   playerinfo_addr (external addr)
		   playerinfo_addr (local lan addr)
		   */
	} PACKED_ATTR() t_server_w3route_playerinfo;

	typedef struct
	{
		bn_byte unknown1;		/* 8 (length?) */
		bn_int id;			/* id from FINDANONGAME_SEARCH packet */
		bn_int race;			/* see defines */
	} PACKED_ATTR() t_server_w3route_playerinfo2;

#define SERVER_W3ROUTE_LEVELINFO 0x47f7
	typedef struct
	{
		t_w3route_header h;
		bn_byte numplayers;
		/* then: levelinfo2 for each player */
	} PACKED_ATTR() t_server_w3route_levelinfo;

	typedef struct
	{
		bn_byte plnum;
		bn_byte unknown1;		/* 3 (length?) */
		bn_byte level;
		bn_short unknown2;
	} PACKED_ATTR() t_server_w3route_levelinfo2;


	typedef struct
	{
		bn_short unknown1;		/* 2 */
		bn_short port;
		bn_int ip;
		bn_int unknown2;		/* 0 */
		bn_int unknown3;		/* 0 */
	} PACKED_ATTR() t_server_w3route_playerinfo_addr;


	typedef struct
	{
		t_w3route_header h;	/* f7 0a 04 00 */
	} PACKED_ATTR() t_server_w3route_startgame1;
#define SERVER_W3ROUTE_STARTGAME1 0x0af7

	typedef struct
	{
		t_w3route_header h;	/* f7 0b 04 00 */
	} PACKED_ATTR() t_server_w3route_startgame2;
#define SERVER_W3ROUTE_STARTGAME2 0x0bf7

	/*******************************************************/
	/*
		ALL ANONGAME PACKET INFO (0x44ff) MOVED TO anongame_protocol.h
		[Omega]
		*/
	/******************************************************/
	/*
	These two dumps are from the original unpatched Starcraft client:

	FF 05 28 00 01 00 00 00            ..(.....
	D1 43 88 AA DA 9D 1B 00   9A F7 69 AB 4A 41 32 30    .C........i.JA20
	35 43 2D 30 34 00 6C 61   62 61 73 73 69 73 74 00    5C-04.labassist.

	FF 05 24 00 01 00 00 00   D1 43 88 AA DA 9D 1B 00    ..$......C......
	9A F7 69 AB 42 4F 42 20   20 20 20 20 20 20 20 00    ..i.BOB        .
	42 6F 62 00                                          Bob.

	??? note it sends NO host and user strings
	FF 05 14 00 01 00 00 00   D8 94 F6 07 B3 2C 6E 02    .............,n.
	B4 E0 3B 6C                                          ..;l
	??? sent right after it... request for session key?
	FF 28 08 00   F6 0F 08 00                    .(......

	Diablo II 1.03 ... note it sends NO host and user strings
	FF 05 14 00 01 00   00 00 D1 43 88 AA DA 9D      ...... ...C....
	1B 00 9A F7 69 AB                                    ....i.
	*/
#define CLIENT_COMPINFO1 0x05ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        reg_version;  /* 01 00 00 00 */
		bn_int        reg_auth;     /* D1 43 88 AA */ /* looks like server ip */
		bn_int        client_id;    /* DA 9D 1B 00 */
		bn_int        client_token; /* 9A F7 69 AB */
		/* host */ /* optional */
		/* user */ /* optional */
	} PACKED_ATTR() t_client_compinfo1;
#define CLIENT_COMPINFO1_REG_VERSION  0x00000001
#define CLIENT_COMPINFO1_REG_AUTH     0xaa8843d1
#define CLIENT_COMPINFO1_CLIENT_ID    0x001b9dda
#define CLIENT_COMPINFO1_CLIENT_TOKEN 0xab69f79a
	/******************************************************/


	/******************************************************/
	/*
	CLIENT_COMPINFO2 was first seen in Starcraft 1.05

	FF 1E 24 00 01 00 00 00            ..$.....
	01 00 00 00 D1 43 88 AA   1C B9 48 00 31 8A F2 89    .....C....H.1...
	43 4C 4F 55 44 00 63 6C   6F 75 64 00                CLOUD.cloud.

	FF 1E 28 00 01 00 00 00   01 00 00 00 D1 43 88 AA    ..(..........C..
	DA 9D 1B 00 9A F7 69 AB   42 4F 42 20 20 20 20 20    ......i.BOB
	20 20 20 00 42 6F 62 00                                 .Bob.

	Diablo II 1.03 ... note it sends empty host and user strings
	FF 1E 1A 00 01 00   00 00 01 00 00 00 D1 43      ............C
	88 AA DA 9D 1B 00 9A F7   69 AB 00 00                .......i...
	*/
#define CLIENT_COMPINFO2 0x1eff
	typedef struct
	{
		t_bnet_header h;
		bn_int        unknown1;     /* 01 00 00 00 */ /* ??? Version */
		bn_int        reg_version;  /* 01 00 00 00 */
		bn_int        reg_auth;     /* D1 43 88 AA */ /* looks like server ip */
		bn_int        client_id;    /* 1C B9 48 00 */ /* DA 9D 1B 00 */
		bn_int        client_token; /* 31 8A f2 89 */ /* 9A F7 69 AB */
		/* host */
		/* user */
	} PACKED_ATTR() t_client_compinfo2;
#define CLIENT_COMPINFO2_UNKNOWN1     0x00000001
#define CLIENT_COMPINFO2_REG_VERSION  0x00000001
#define CLIENT_COMPINFO2_REG_AUTH     0xaa8843d1
#define CLIENT_COMPINFO2_CLIENT_ID    0x001b9dda
#define CLIENT_COMPINFO2_CLIENT_TOKEN 0xab69f79a
	/******************************************************/


	/******************************************************/
	/*
	Sent in response to CLIENT_COMPINFO[12] along with sessionkey

	FF 05 14 00 01 00 00 00            ........
	D1 43 88 AA 1C B9 48 00   31 8A F2 89                .C....H.1...

	FF 05 14 00 01 00 00 00   D8 94 F6 07 3F 62 6E 02    ............?bn.
	CA A4 0D 99                                          ....

	FF 05 14 00 01 00 00 00   D8 94 F6 07 B3 2C 6E 02    .............,n.
	B4 E0 3B 6C                                          ..;l

	To D2 Beta 1.02:
	FF 05 14 00 01 00 00 00   D1 43 88 AA 42 8D 2E 02    .........C..B...
	2B 81 8C 2B                                          +..+
	*/
#define SERVER_COMPREPLY 0x05ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        reg_version;  /* 01 00 00 00 */
		bn_int        reg_auth;     /* D1 43 88 AA */ /* looks like server ip */
		bn_int        client_id;    /* DA 9D 1B 00 */ /* 1C B9 48 00 */
		bn_int        client_token; /* 9A F7 69 AB */ /* 31 8A F2 89 */
	} PACKED_ATTR() t_server_compreply;
#define SERVER_COMPREPLY_REG_VERSION  0x00000001
#define SERVER_COMPREPLY_REG_AUTH     0xaa8843d1
#define SERVER_COMPREPLY_CLIENT_ID    0x001b9dda
#define SERVER_COMPREPLY_CLIENT_TOKEN 0xab69f79a
	/******************************************************/


	/******************************************************/
	/*
	Sent in repsonse to COMPINFO1 along with COMPINFOREPLY.
	Used for password hashing by the client.
	*/
#define SERVER_SESSIONKEY1 0x28ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        sessionkey;
	} PACKED_ATTR() t_server_sessionkey1;
	/******************************************************/


	/******************************************************/
	/*
	Sent in response to COMPINFO2 along with COMPINFOREPLY.
	Used for password hashing by the client.

	FF 1D 0C 00 40 24 02 00            ....@$..
	99 F3 FD 78                                          ...x

	FF 1D 0C 00 0C 67 08 00   7A 3C D8 75                .....g..z<.u

	FF 1D 0C 00 58 77 00 00   27 45 44 7A                ....Xw..'EDz

	FF 1D 0C 00 9D DF 01 00   7A 11 07 ED                ........z...
	*/
#define SERVER_SESSIONKEY2 0x1dff
	typedef struct
	{
		t_bnet_header h;
		bn_int        sessionnum;
		bn_int        sessionkey;
	} PACKED_ATTR() t_server_sessionkey2;
#define SERVER_SESSIONKEY2_UNKNOWN1 0x00004df3
	/******************************************************/


	/******************************************************/
	/*
	FF 12 3C 00 E0 28 02 E4   0A 37 BE 01 E0 50 A3 37    ..<..(...7...P.7
	D0 36 BE 01 A4 01 00 00   09 04 00 00 09 04 00 00    .6..............
	09 04 00 00 65 6E 75 00   31 00 55 53 41 00 55 6E    ....enu.1.USA.Un
	69 74 65 64 20 53 74 61   74 65 73 00                ited States.

	still original client, but at a later date
	FF 12 3C 00 60 C5 4B 8B   19 DE BE 01 60 55 B1 40    ..<.`.K.....`U.@
	E7 DD BE 01 A4 01 00 00   09 04 00 00 09 04 00 00    ................
	09 04 00 00 65 6E 75 00   31 00 55 53 41 00 55 6E    ....enu.1.USA.Un
	69 74 65 64 20 53 74 61   74 65 73 00                ited States.

	FF 12 3C 00 60 EA 02 23   F5 DE BE 01 60 7A 68 D8    ..<.`..#....`zh.
	C2 DE BE 01 A4 01 00 00   09 04 00 00 09 04 00 00    ................
	09 04 00 00 65 6E 75 00   31 00 55 53 41 00 55 6E    ....enu.1.USA.Un
	69 74 65 64 20 53 74 61   74 65 73 00                ited States.

	FF 12 35 00                ..5.
	20 BA B0 55 F2 7B BE 01   20 62 98 C5 3D 7C BE 01     ..U.{.. b..=|..
	E4 FD FF FF 12 04 00 00   12 04 00 00 12 04 00 00    ................
	6B 6F 72 00 38 32 00 4B   4F 52 00 4B 6F 72 65 61    kor.82.KOR.Korea
	00                                                   .

	FF 12 37 00 E0 D4 72 97   2F 8C BF 01 E0 3C 37 F9    ..7...r./....<7.
	37 8C BF 01 C4 FF FF FF   07 04 00 00 07 04 00 00    7...............
	07 04 00 00 64 65 75 00   34 39 00 44 45 55 00 47    ....deu.49.DEU.G
	65 72 6D 61 6E 79 00                                 ermany.

	FF 12 36 00 20 F3 31 08   40 A7 BF 01 20 C3 BA CB    ..6. .1.@... ...
	50 A7 BF 01 C4 FF FF FF   1D 04 00 00 1D 04 00 00    P...............
	1D 04 00 00 73 76 65 00   34 36 00 53 57 45 00 53    ....sve.46.SWE.S
	77 65 64 65 6E 00                                    weden.

	Diablo II 1.03
	FF 12 39 00 A0 DB   AA 45 51 3F C0 01 A0 EB      ..9....EQ?....
	56 17 A5 3F C0 01 A8 FD   FF FF 09 0C 00 00 09 0C    V..?............
	00 00 09 0C 00 00 65 6E   61 00 36 31 00 41 55 53    ......ena.61.AUS
	00 41 75 73 74 72 61 6C   69 61 00                   .Australia.
	*/
#define CLIENT_COUNTRYINFO1 0x12ff
	typedef struct
	{
		t_bnet_header h;
		bn_long       systemtime; /* GMT */
		bn_long       localtime;  /* time in local timezone */
		bn_int        bias;       /* (gmt-local)/60  (using signed math) */
		bn_int        langid1;    /* 09 04 00 00 */   /* 12 04 00 00 */
		bn_int        langid2;    /* 09 04 00 00 */   /* 12 04 00 00 */
		bn_int        langid3;    /* 09 04 00 00 */   /* 12 04 00 00 */
		/* langstr */
		/* countrycode (long distance phone) */
		/* countryabbrev */
		/* countryname */
	} PACKED_ATTR() t_client_countryinfo1;
	/******************************************************/


	/******************************************************/
	/*
	  First seen in Diablo II (and LoD) 1.09
	  FF 50 34 00 00 00 00 00   36 38 58 49 50 58 32 44    .P4.....68XIPX2D
	  09 00 00 00 00 00 00 00   00 00 00 00 C4 FF FF FF    ................
	  07 04 00 00 07 04 00 00   44 45 55 00 47 65 72 6D    ........DEU.Germ
	  61 6E 79 00                                          any.

	  FF 50 47 00 00 00 00 00   36 38 58 49 56 44 32 44    .PG.....68XIVD2D
	  09 00 00 00 00 00 00 00   00 00 00 00 20 FE FF FF    ............ ...
	  04 08 00 00 04 08 00 00   43 48 4E 00 50 65 6F 70    ........CHN.Peop
	  6C 65 27 73 20 52 65 70   75 62 6C 69 63 20 6F 66    le's Republic of
	  20 43 68 69 6E 61 00                                  China.
	  */
#define CLIENT_COUNTRYINFO_109 0x50ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        protocol;  /* 00 00 00 00 always zero */
		bn_int        archtag;
		bn_int        clienttag;
		bn_int        versionid; /* 09 00 00 00 */ /* FIXME: what is this? */
		bn_int        gamelang;  /* 00 00 00 00 always zero */
		bn_int        localip;  /* 00 00 00 00 always zero */
		bn_int        bias;      /* (gmt-local)/60  (using signed math) */
		bn_int        lcid;      /* Win32 LCID */
		bn_int        langid;    /* Win32 LangID */
		/* langstr */
		/* countryname */
	} PACKED_ATTR() t_client_countryinfo_109;
	/******************************************************/


	/******************************************************/
	/*
	FF 2A 20 00 91 4F 93 DF   57 74 B5 C8 48 0F 4D 9B    .* ..O..Wt..H.M.
	A2 28 A6 03 C1 D9 DA 11   42 69 6D 42 6F 3A 29 00    .(......BimBo:).
	*/
#define CLIENT_CREATEACCTREQ1 0x2aff
	typedef struct
	{
		t_bnet_header h;
		bn_int        password_hash1[5]; /* hash of lowercase password w/o null */
		/* player name */
	} PACKED_ATTR() t_client_createacctreq1;
	/******************************************************/


	/******************************************************/
	/*
							  FF 2A 18 00 01 00 00 00            .*......
							  13 00 00 00 78 52 82 02   00 00 00 00 00 00 00 00    ................
							  ---120 82 130 2---

							  FF 2A 08 00 01 00 00 00                              .*......
							  */
#define SERVER_CREATEACCTREPLY1 0x2aff
	typedef struct
	{
		t_bnet_header h;
		bn_int        result;
	} PACKED_ATTR() t_server_createacctreply1;
#define SERVER_CREATEACCTREPLY1_RESULT_OK 0x00000001
#define SERVER_CREATEACCTREPLY1_RESULT_NO 0x00000000
	/******************************************************/


	/******************************************************/
	/*
							  FF 2B 20 00 01 00 00 00            .+ .....
							  00 00 00 00 4D 00 00 00   0E 01 00 00 20 00 00 00    ....M....... ...
							  CE 01 00 00 DD 07 00 00                              ........

							  FF 2B 20 00 01 00 00   00 00 00 00 00 06 00 00     .+ ............
							  00 72 01 00 00 40 00 00   00 A9 07 00 00 FF 07 00    .r...@..........
							  00                                                   .

							  from Starcraft 1.05
							  FF 2B 20 00 01 00 00 00   00 00 00 00 06 00 00 00    .+ .............
							  7C 01 00 00 20 00 00 00   00 02 00 00 FF 07 00 00    |... ...........
							  */
#define CLIENT_UNKNOWN_2B 0x2bff /* FIXME: what is this? */
	typedef struct
	{
		t_bnet_header h;
		bn_int        unknown1; /* 01 00 00 00 */ /* 01 00 00 00 */
		bn_int        unknown2; /* 00 00 00 00 */ /* 00 00 00 00 */
		bn_int        unknown3; /* 4D 00 00 00 */ /* 06 00 00 00 */
		bn_int        unknown4; /* 0E 01 00 00 */ /* 72 01 00 00 */
		bn_int        unknown5; /* 20 00 00 00 */ /* 40 00 00 00 */
		bn_int        unknown6; /* CE 01 00 00 */ /* A9 07 00 00 */
		bn_int        unknown7; /* DD 07 00 00 */ /* FF 07 00 00 */
	} PACKED_ATTR() t_client_unknown_2b;
#define CLIENT_UNKNOWN_2B_UNKNOWN1 0x00000001
#define CLIENT_UNKNOWN_2B_UNKNOWN2 0x00000000
#define CLIENT_UNKNOWN_2B_UNKNOWN3 0x0000004d
#define CLIENT_UNKNOWN_2B_UNKNOWN4 0x0000010e
#define CLIENT_UNKNOWN_2B_UNKNOWN5 0x00000020
#define CLIENT_UNKNOWN_2B_UNKNOWN6 0x000001ce
#define CLIENT_UNKNOWN_2B_UNKNOWN7 0x000007dd
	/******************************************************/


	/******************************************************/
	/*
	later replaced by progident2 and the authreq packets

	FF 06 14 00 36 38 58 49            ....68XI
	50 58 45 53 BB 00 00 00   00 00 00 00                PXES........

	sent by 1.05 Starcraft
	FF 06 14 00 36 38 58 49   52 41 54 53 BD 00 00 00    ....68XIRATS....
	00 00 00 00                                          ....

	Diablo II 1.03
	FF 06 14 00 36 38   58 49 56 44 32 44 03 00      ....68XIVD2D..
	00 00 00 00 00 00                                    ......
	*/
#define CLIENT_PROGIDENT 0x06ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        archtag;
		bn_int        clienttag; /* see tag.h */
		bn_int        versionid; /* FIXME: how does the versionid work? */
		bn_int        unknown1;  /* FIXME: always zero? spawn flag? */
	} PACKED_ATTR() t_client_progident;
	/******************************************************/


	/******************************************************/
	/*
	These formulas are for authenticating the client version.

	FF 06 5A 00 00 86 BA E3            ..Z.....
	09 28 BC 01 49 58 38 36   76 65 72 32 2E 6D 70 71    .(..IX86ver2.mpq
	00 41 3D 32 30 31 39 34   39 38 38 39 39 20 42 3D    .A=2019498899 B=
	33 34 32 33 32 39 32 33   39 34 20 43 3D 31 37 31    3423292394 C=171
	39 30 31 31 32 32 32 20   34 20 41 3D 41 5E 53 20    9011222 4 A=A^S
	42 3D 42 2D 43 20 43 3D   43 5E 41 20 41 3D 41 5E    B=B-C C=C^A A=A^
	42 00                                                B.

	FF 06 59 00 00 C1 12 EC   09 28 BC 01 49 58 38 36    ..Y......(..IX86
	76 65 72 35 2E 6D 70 71   00 41 3D 31 38 37 35 35    ver5.mpq.A=18755
	39 31 33 34 31 20 42 3D   32 34 39 31 30 39 39 38    91341 B=24910998
	30 39 20 43 3D 36 33 34   38 35 36 36 30 34 20 34    09 C=634856604 4
	20 41 3D 41 2D 53 20 42   3D 42 5E 43 20 43 3D 43     A=A-S B=B^C C=C
	2B 41 20 41 3D 41 5E 42   00                         +A A=A^B.

	FF 06 5A 00 00 C1 12 EC   09 28 BC 01 49 58 38 36    ..Z......(..IX86
	76 65 72 35 2E 6D 70 71   00 41 3D 31 37 31 32 39    ver5.mpq.A=17129
	34 38 34 32 36 20 42 3D   33 36 30 30 30 33 30 36    48426 B=36000306
	30 37 20 43 3D 33 33 39   30 34 31 37 39 35 39 20    07 C=3390417959
	34 20 41 3D 41 2D 53 20   42 3D 42 5E 43 20 43 3D    4 A=A-S B=B^C C=
	43 2D 41 20 41 3D 41 2D   42 00                      C-A A=A-B.

	FF 06 5A 00 00 3A 7F E8   09 28 BC 01 49 58 38 36    ..Z..:...(..IX86
	76 65 72 34 2E 6D 70 71   00 41 3D 31 31 38 36 39    ver4.mpq.A=11869
	35 38 31 34 31 20 42 3D   31 33 37 37 34 34 31 34    58141 B=13774414
	35 37 20 43 3D 31 37 37   32 37 38 37 37 30 35 20    57 C=1772787705
	34 20 41 3D 41 5E 53 20   42 3D 42 5E 43 20 43 3D    4 A=A^S B=B^C C=
	43 2B 41 20 41 3D 41 5E   42 00                      C+A A=A^B.

	FF 06 5A 00 00 56 CD F6   09 28 BC 01 49 58 38 36    ..Z..V...(..IX86
	76 65 72 37 2E 6D 70 71   00 41 3D 31 30 32 36 30    ver7.mpq.A=10260
	34 34 33 35 34 20 42 3D   34 31 33 32 36 33 30 37    44354 B=41326307
	31 31 20 43 3D 32 33 30   32 34 31 31 33 32 38 20    11 C=2302411328
	34 20 41 3D 41 5E 53 20   42 3D 42 5E 43 20 43 3D    4 A=A^S B=B^C C=
	43 5E 41 20 41 3D 41 2B   42 00                      C^A A=A+B.
	*/
#define SERVER_AUTHREQ1 0x06ff
	typedef struct
	{
		t_bnet_header h;
		bn_long       timestamp; /* FIXME: file modification time? */
		/* versioncheck filename */
		/* equation */
	} PACKED_ATTR() t_server_authreq1;
	/******************************************************/


	/******************************************************/
	/*
	  First seen in Diablo II (and LoD) 1.09
	  FF 50 65 00 00 00 00 00   36 1A 6C 45 76 BC 00 00    .Pe.....6.lEv...
	  00 48 A6 EF 09 28 BC 01   49 58 38 36 76 65 72 36    .H...(..IX86ver6
	  2E 6D 70 71 00 41 3D 33   38 34 35 35 38 31 36 33    .mpq.A=384558163
	  34 20 42 3D 38 38 30 38   32 33 35 38 30 20 43 3D    4 B=880823580 C=
	  31 33 36 33 39 33 37 31   30 33 20 34 20 41 3D 41    1363937103 4 A=A
	  2D 53 20 42 3D 42 2D 43   20 43 3D 43 2D 41 20 41    -S B=B-C C=C-A A
	  3D 41 2D 42 00                                       =A-B.

	  FF 50 65 00 00 00 00 00            .Pe.....
	  30 4B C1 33 10 EB 09 00   00 A5 C4 DD 09 28 BC 01    0K.3.........(..
	  49 58 38 36 76 65 72 30   2E 6D 70 71 00 41 3D 31    IX86ver0.mpq.A=1
	  34 33 32 36 36 32 34 37   38 20 42 3D 36 35 32 32    432662478 B=6522
	  37 38 36 32 35 20 43 3D   31 37 36 31 35 31 35 38    78625 C=17615158
	  36 39 20 34 20 41 3D 41   5E 53 20 42 3D 42 2B 43    69 4 A=A^S B=B+C
	  20 43 3D 43 2B 41 20 41   3D 41 5E 42 00              C=C+A A=A^B.
	  */
#define SERVER_AUTHREQ_109 0x50ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        logontype;  /* 00 00 00 00 always zero */
		bn_int        sessionkey;
		bn_int        sessionnum;
		bn_long       timestamp;
		/* versioncheck filename */
		/* equation */
	} PACKED_ATTR() t_server_authreq_109;
#define SERVER_AUTHREQ_109_LOGONTYPE 		0x0000000
#define SERVER_AUTHREQ_109_LOGONTYPE_W3 	0x00000002
#define SERVER_AUTHREQ_109_LOGONTYPE_W3XP 	0x00000002
	/******************************************************/

	/* ADDED BY UNDYING SOULZZ 4/3/02 */
#define VERSIONTAG_WARCRAFT3_113	"WAR3_113"

	/******************************************************/
	/*
	FF 07 40 00 36 38 58 49   52 41 54 53 BD 00 00 00    ..@.68XIRATS....
	00 05 00 01 1E 88 D7 08   73 74 61 72 63 72 61 66    ........starcraf
	74 2E 65 78 65 20 30 33   2F 30 38 2F 39 39 20 32    t.exe 03/08/99 2
	32 3A 34 31 3A 35 30 20   31 30 34 32 34 33 32 00    2:41:50 1042432.

	sent by the 1.05 Starcraft
	FF 07 40 00 36 38 58 49   52 41 54 53 BD 00 00 00    ..@.68XIRATS....
	00 05 00 01 AE AC DE 87   73 74 61 72 63 72 61 66    ........starcraf
	74 2E 65 78 65 20 30 33   2F 30 38 2F 39 39 20 32    t.exe 03/08/99 2
	32 3A 34 31 3A 35 30 20   31 30 34 32 34 33 32 00    2:41:50 1042432.

	sent by the 1.08alpha Brood War (Starcraft game) in response to
	A=2521522835 B=3428392135 C=218673704 4 A=A^S B=B-C C=C+A A=A-B
	with IX86ver1.mpq
	FF 07 40 00 36 38 58 49   52 41 54 53 C3 00 00 00    ..@.68XIRATS....
	01 08 00 01 A1 52 CE FE   73 74 61 72 63 72 61 66    .....R..starcraf
	74 2E 65 78 65 20 31 32   2F 32 38 2F 30 30 20 31    t.exe 12/28/00 1
	31 3A 32 38 3A 35 32 20   31 30 38 32 33 36 38 00    1:28:52 1082368.

	sent by the 1.07 Diablo
	FF 07 3C 00 36 38 58 49   4C 54 52 44 26 00 00 00    ..<.68XILTRD&...
	01 06 05 62 8C 56 E6 21   64 69 61 62 6C 6F 2E 65    ...b.V.!diablo.e
	78 65 20 30 39 2F 31 37   2F 39 38 20 31 38 3A 30    xe 09/17/98 18:0
	30 3A 34 30 20 37 36 30   33 32 30 00                0:40 760320.

	FF 07 45 00 36 38 58 49   4E 42 32 57 4B 00 00 00    ..E.68XINB2WK...
	99 00 00 02 3D 51 C4 AA   57 61 72 63 72 61 66 74    ....=Q..Warcraft
	20 49 49 20 42 4E 45 2E   65 78 65 20 31 30 2F 31     II BNE.exe 10/1
	35 2F 39 39 20 30 30 3A   33 37 3A 35 34 20 37 30    5/99 00:37:54 70
	34 35 31 32 00                                       4512.

	sent by the 1.03 Diablo II
	FF 07 3A 00 36 38 58 49            ..:.68XI
	56 44 32 44 03 00 00 00   00 03 00 01 47 3E 26 73    VD2D........G>&s
	47 61 6D 65 2E 65 78 65   20 30 38 2F 30 35 2F 30    Game.exe 08/05/0
	30 20 30 31 3A 34 32 3A   32 38 20 32 39 34 39 31    0 01:42:28 29491
	32 00                                                2.
	*/
#define CLIENT_AUTHREQ1 0x07ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        archtag;
		bn_int        clienttag;
		bn_int        versionid;
		bn_int        gameversion;
		bn_int        checksum;
		/* executable info */
	} PACKED_ATTR() t_client_authreq1;

	/******************************************************/


	/******************************************************/
	/*
							  FF 07 0A 00 02 00 00 00            ........
							  00 00                                                ..

							  FF 07 0A 00 02 00 00 00   00 00                      ..........
							  */
#define SERVER_AUTHREPLY1 0x07ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        message;
		/* filename */
		/* unknown */
	} PACKED_ATTR() t_server_authreply1;
#define SERVER_AUTHREPLY1_MESSAGE_BADVERSION  0x00000000
#define SERVER_AUTHREPLY1_MESSAGE_UPDATE      0x00000001 /* initiate auto-update */
#define SERVER_AUTHREPLY1_MESSAGE_OK          0x00000002
	/******************************************************/


	/******************************************************/
	/*
	  First seen in Diablo II (and LoD) 1.09
	  FF 51 09 00 00 00 00 00   00                         .Q.......
	  */
#define SERVER_AUTHREPLY_109 0x51ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        message;
		/* message string? */
	} PACKED_ATTR() t_server_authreply_109;
#define SERVER_AUTHREPLY_109_MESSAGE_OK         0x00000000
#define SERVER_AUTHREPLY_109_MESSAGE_UPDATE     0x00000100
#define SERVER_AUTHREPLY_109_MESSAGE_BADVERSION 0x00000101
	/* we should check the first 10 values or so to see what they mean */
	/******************************************************/


	/******************************************************/
	/*
	  First seen in Diablo II (and LoD) 1.09
	  FF 51 67 00 C9 88 DA 42   00 09 00 01 46 97 62 9A    .Qg....B....F.b.
	  01 00 00 00 00 00 00 00   10 00 00 00 06 00 00 00    ................
	  A5 E7 39 00 00 00 00 00   ED CD 4F F7 6A 7A 4F 96    ..9.......O.jzO.
	  85 7A 2D A2 7F 1F B1 D6   81 B3 8D 50 47 61 6D 65    .z-........PGame
	  2E 65 78 65 20 30 38 2F   31 36 2F 30 31 20 32 33    .exe 08/16/01 23
	  3A 30 34 3A 34 30 20 34   32 34 30 36 37 00 74 73    :04:40 424067.ts
	  69 6E 67 68 75 61 00                                 inghua.
	  */
#define CLIENT_AUTHREQ_109 0x51ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        ticks;
		bn_int        gameversion;
		bn_int        checksum;
		bn_int        cdkey_number; /* count of cdkeys, d2 = 1, lod = 2 */
		bn_int        spawn; 	/* set if using spawn copy */
		/* cdkey info(s) */
		/* executable info */
		/* cdkey owner */
	} PACKED_ATTR() t_client_authreq_109;
	/* values are the same as in CLIENT_AUTHREQ1 */

	typedef struct
	{
		bn_int len;
		bn_int type;
		bn_int checksum;
		bn_int u1;
		bn_int hash[5];
	} PACKED_ATTR() t_cdkey_info;
	/******************************************************/


	/******************************************************/
	/* Batle.net used to send requests for registry and email info.
	   Thanks to bnetanon, I have a dump of these old packets.
	   */
	/*
	FF 18 41 00 00 00 00 00   01 00 00 80 53 6F 66 74    ..A.........Soft
	77 61 72 65 5C 4D 69 63   72 6F 73 6F 66 74 5C 4D    ware\Microsoft\M
	53 20 53 65 74 75 70 20   28 41 43 4D 45 29 5C 55    S Setup (ACME)\U
	73 65 72 20 49 6E 66 6F   00 44 65 66 4E 61 6D 65    ser Info.DefName
	00                                                   .

	FF 18 48 00 00 00 00 00   01 00 00 80 53 6F 66 74    ..H.........Soft
	77 61 72 65 5C 4D 69 63   72 6F 73 6F 66 74 5C 4D    ware\Microsoft\M
	65 64 69 61 50 6C 61 79   65 72 5C 43 6F 6E 74 72    ediaPlayer\Contr
	6F 6C 5C 50 6C 61 79 42   61 72 00 43 6C 72 42 61    ol\PlayBar.ClrBa
	63 6B 43 6F 6C 6F 72 00                              ckColor.
	*/
#define SERVER_REGSNOOPREQ 0x18ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        unknown1; /* 00 00 00 00 */ /* sequence match like in other packets? */
		bn_int        hkey;
		/* registry key */
		/* value name */
	} PACKED_ATTR() t_server_regsnoopreq;
#define SERVER_REGSNOOPREQ_UNKNOWN1 0x00000000
#define SERVER_REGSNOOPREQ_HKEY_CLASSES_ROOT        0x80000000
#define SERVER_REGSNOOPREQ_HKEY_CURRENT_USER        0x80000001
#define SERVER_REGSNOOPREQ_HKEY_LOCAL_MACHINE       0x80000002
#define SERVER_REGSNOOPREQ_HKEY_USERS               0x80000003
#define SERVER_REGSNOOPREQ_HKEY_PERFORMANCE_DATA    0x80000004
#define SERVER_REGSNOOPREQ_HKEY_CURRENT_CONFIG      0x80000005
#define SERVER_REGSNOOPREQ_HKEY_DYN_DATA            0x80000006
#define SERVER_REGSNOOPREQ_HKEY_PERFORMANCE_TEXT    0x80000050
#define SERVER_REGSNOOPREQ_HKEY_PERFORMANCE_NLSTEXT 0x80000060
#define SERVER_REGSNOOPREQ_REGKEY "Software\\Microsoft\\MS Setup (ACME)\\User Info"
#define SERVER_REGSNOOPREQ_REGVALNAME "DefName"
	/******************************************************/


	/******************************************************/
	/* If the key exists, the client send this back */
	/*
	FF 18 0C 00 00 00 00 00   42 6F 62 00                ........Bob.

	FF 18 0C 00 00 00 00 00   A0 9C A0 00                ............
	*/
#define CLIENT_REGSNOOPREPLY 0x18ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        unknown1; /* 00 00 00 00 */ /* same as request? */
		/* registry value (string, dword, or binary */
	} PACKED_ATTR() t_client_regsnoopreply;
	/******************************************************/


	/******************************************************/
	/*
	FF 07 0A 00 02 00 00 00   00 00                      ..........
	*/
#define CLIENT_ICONREQ 0x2dff
	typedef struct
	{
		t_bnet_header h;
	} PACKED_ATTR() t_client_iconreq;
	/******************************************************/


	/******************************************************/
	/*
							  FF 2D 16 00 76 34 1F 8F            .-..v4..
							  C0 D6 BD 01 69 63 6F 6E   73 2E 62 6E 69 00          ....icons.bni.

							  FF 2D 16 00 00 77 D0 01   C7 B1 BE 01 69 63 6F 6E    .-...w......icon
							  73 2E 62 6E 69 00                                    s.bni.
							  */
#define SERVER_ICONREPLY 0x2dff
	typedef struct
	{
		t_bnet_header h;
		bn_long       timestamp; /* file modification time? */
		/* filename */
	} PACKED_ATTR() t_server_iconreply;
	/******************************************************/


	/******************************************************/
#define CLIENT_LADDERSEARCHREQ 0x2fff
	typedef struct
	{
		t_bnet_header h;
		bn_int        clienttag;
		bn_int        id;   /* (AKA ladder type) 1==standard, 3==ironman */
		bn_int        type; /* (AKA ladder sort) */
		/* player name */
	} PACKED_ATTR() t_client_laddersearchreq;
#define CLIENT_LADDERSEARCHREQ_ID_STANDARD       0x00000001
#define CLIENT_LADDERSEARCHREQ_ID_IRONMAN        0x00000003
#define CLIENT_LADDERSEARCHREQ_TYPE_HIGHESTRATED 0x00000000
#define CLIENT_LADDERSEARCHREQ_TYPE_MOSTWINS     0x00000002
#define CLIENT_LADDERSEARCHREQ_TYPE_MOSTGAMES    0x00000003
	/******************************************************/


	/******************************************************/
#define SERVER_LADDERSEARCHREPLY 0x2fff
	typedef struct /* FIXME: how does client know how many names?
			  do we send separate replies for each name in the request? */
	{
		t_bnet_header h;
		bn_int        rank; /* 0 means 1st, etc */
	} PACKED_ATTR() t_server_laddersearchreply;
#define SERVER_LADDERSEARCHREPLY_RANK_NONE 0xffffffff
	/******************************************************/


	/******************************************************/
	/*
							  FF 30 1C 00 00 00 00 00            .0......
							  32 37 34 34 37 37 32 39   31 34 38 32 38 00 63 6C    2744772914828.cl
							  6F 75 64 00                                          oud.
							  */
#define CLIENT_CDKEY 0x30ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        spawn; /* FIXME: not sure if this is correct, but cdkey2 does it this way */
		/* cd key */
		/* owner name */ /* Was this always here? */
	} PACKED_ATTR() t_client_cdkey;
#define CLIENT_CDKEY_UNKNOWN1 0x00000000
	/******************************************************/


	/******************************************************/
	/*
							  FF 30 0E 00 01 00 00 00            .0......
							  63 6C 6F 75 64 00                                    cloud.
							  */
#define SERVER_CDKEYREPLY 0x30ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        message;
		/* owner name */
	} PACKED_ATTR() t_server_cdkeyreply;
#define SERVER_CDKEYREPLY_MESSAGE_OK       0x00000001
#define SERVER_CDKEYREPLY_MESSAGE_BAD      0x00000002
#define SERVER_CDKEYREPLY_MESSAGE_WRONGAPP 0x00000003
#define SERVER_CDKEYREPLY_MESSAGE_ERROR    0x00000004 /* disabled */
#define SERVER_CDKEYREPLY_MESSAGE_INUSE    0x00000005
	/* (any other value seems to correspond to ok) */
	/******************************************************/


	/******************************************************/
	/*
	FF 36 34 00 00 00 00 00   0D 00 00 00 01 00 00 00    .64.............
	B5 AE 23 00 50 E5 D5 C0   DB 55 1E 38 0A F5 58 B9    ..#.P....U.8..X.
	47 64 C6 C2 9F BB FF B8   81 E7 EB EC 1B 13 C6 38    Gd.............8
	52 6F 62 00                                          Rob.

	FF 36 34 00 00 00 00 00   0D 00 00 00 01 00 00 00    .64.............
	7F D7 00 00 90 64 77 2F   D7 5B 42 38 1F A1 A2 6F    .....dw/.[B8...o
	E8 FA BE F8 B6 0B BA 0F   CA 64 3A 17 14 56 83 AB    .........d:..V..
	42 6F 62 00                                          Bob.

	FF 36 35 00 00 00 00 00   10 00 00 00 04 00 00 00    .65.............
	0D 43 03 00 7A 11 07 ED   7C 9E 1E 38 E5 87 8B 3B    .C..z...|..8...;
	9C 19 91 D9 0D 10 FC C1   C0 86 8C 8D DA A4 45 0B    ..............E.
	XX XX XX XX 00                                       XXXX.

	FF 36 34 00 00 00 00 00   10 00 00 00 04 00 00 00    .64.............
	70 F9 02 00 58 F9 B6 E6   38 49 5C 38 38 9C 31 E4    p...X...8I\88.1.
	1D 3D 40 05 66 AD 4C C8   1D 12 8E 49 9E 60 1A CB    .=@.f.L....I.`..
	42 6F 62 00                                          Bob.
	*/
#define CLIENT_CDKEY2 0x36ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        spawn;
		bn_int        keylen; /* without terminating NUL */
		bn_int        productid;
		bn_int        keyvalue1;
		bn_int        sessionkey;
		bn_int        ticks;
		bn_int        key_hash[5];
		/* owner name */
	} PACKED_ATTR() t_client_cdkey2;
#define CLIENT_CDKEY2_SPAWN_TRUE  0x00000001
#define CLIENT_CDKEY2_SPAWN_FALSE 0x00000000
	/******************************************************/


	/******************************************************/
	/*
	From Diablo II 1.08?
	FF 42 43 00 AB 4C A4 3B            .BC..L.;
	01 00 00 00 00 00 00 00   10 00 00 00 06 00 00 00    ................
	XX 60 12 00 00 00 00 00   5D 82 82 C4 F4 8F D0 91    X`......].......
	E1 5B AB 95 D9 EE EF 18   44 3E F1 C9 XX XX XX XX    .[......D>..XXXX
	XX XX XX XX XX XX XX XX   XX XX 00                   XXXXXXXXXX.

	FF 42 44 00 17 78 42 77   01 00 00 00 00 00 00 00    .BD..xBw........
	10 00 00 00 06 00 00 00   XX F3 10 00 00 00 00 00    ........X.......
	A8 29 8B C4 41 BD 33 AB   74 4C 1F 1E 5C XX CA 83    .)..A.3.tL..\X..
	7F E5 36 14 XX XX XX XX   XX XX XX XX XX XX XX XX    ..6.XXXXXXXXXXXX
	XX XX XX 00                                          XXX.

	FF 42 44 00 C6 25 A1 3B   01 00 00 00 00 00 00 00    .BD..%.;........
	10 00 00 00 06 00 00 00   XX F3 10 00 00 00 00 00    ........X.......
	C4 3F FB 05 94 0C AC D4   3B 63 B1 90 E4 XX 53 B9    .?......;c...XS.
	70 C3 6F 2E XX XX XX XX   XX XX XX XX XX XX XX XX    p.o.XXXXXXXXXXXX
	XX XX XX 00                                          XXX.
	*/
#define CLIENT_CDKEY3 0x42ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        unknown1; /* FIXME: some kind of salt? */
		bn_int        unknown2; /* 01 00 00 00 */
		bn_int        unknown3; /* 00 00 00 00 */
		bn_int        unknown4; /* 10 00 00 00 */
		bn_int        unknown5; /* 06 00 00 00 */
		bn_int        unknown6; /* FIXME: value1? */
		bn_int        unknown7; /* 00 00 00 00 */
		bn_int        key_hash[5];
		/* owner name */
	} PACKED_ATTR() t_client_cdkey3;
#define CLIENT_CDKEY3_UNKNOWN1  0xffffffff
#define CLIENT_CDKEY3_UNKNOWN2  0x00000001
#define CLIENT_CDKEY3_UNKNOWN3  0x00000000
#define CLIENT_CDKEY3_UNKNOWN4  0x00000010
#define CLIENT_CDKEY3_UNKNOWN5  0x00000006
#define CLIENT_CDKEY3_UNKNOWN6  0x00123456
#define CLIENT_CDKEY3_UNKNOWN7  0x00000000
	/******************************************************/


	/******************************************************/
	/*
							  FF 42 09 00 00 00 00 00            .B......
							  00

							  FF 42 09 00 00 00 00 00   00                         .B.......
							  */
#define SERVER_CDKEYREPLY3 0x42ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        message;
		/* owner name */ /* FIXME: or error message, or ... */
	} PACKED_ATTR() t_server_cdkeyreply3;
#define SERVER_CDKEYREPLY3_MESSAGE_OK       0x00000000
	/******************************************************/


	/******************************************************/
	/*
	FF 34 0D 00 00 00 00 00   00 00 00 00 00             .4...........
	*/
#define CLIENT_REALMLISTREQ 0x34ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        unknown1;
		bn_int        unknown2;
	} PACKED_ATTR() t_client_realmlistreq;
	/******************************************************/


	/******************************************************/
	/*
	0000:   FF 40 04 00                                          .@..

	*/
#define CLIENT_REALMLISTREQ_110 0x40ff
	typedef struct
	{
		t_bnet_header h;
	} PACKED_ATTR() t_client_realmlistreq_110;
	/******************************************************/


	/******************************************************/
	/*
	FF 34 5E 00 00 00 00 00   01 00 00 00 00 00 00 C0    .4^.............
	00 00 00 00 00 00 00 00   00 00 00 00 10 82 01 00    ................
	FF FF FF FF 00 00 00 00   42 65 74 61 57 65 73 74    ........BetaWest
	00 50 6C 65 61 73 65 20   73 65 6C 65 63 74 20 74    .Please select t
	68 69 73 20 61 73 20 79   6F 75 72 20 72 65 61 6C    his as your real
	6D 20 64 75 72 69 6E 67   20 62 65 74 61 00          m during beta.

	ff 34 5e 00 00 00 00 00   01 00 00 00 00 00 00 c0    .4^.............
	00 00 00 00 00 00 00 00   00 00 00 00 bc 95 01 00    ................
	ff ff ff ff 00 00 00 00   42 65 74 61 57 65 73 74    ........BetaWest
	00 50 6c 65 61 73 65 20   73 65 6c 65 63 74 20 74    .Please select t
	68 69 73 20 61 73 20 79   6f 75 72 20 72 65 61 6c    his as your real
	6d 20 64 75 72 69 6e 67   20 62 65 74 61 00          m during beta.

	ff 34 5e 00 00 00 00 00   01 00 00 00 00 00 00 c0    .4^.............
	00 00 00 00 00 00 00 00   00 00 00 00 c8 99 01 00    ................
	ff ff ff ff 00 00 00 00   42 65 74 61 57 65 73 74    ........BetaWest
	00 50 6c 65 61 73 65 20   73 65 6c 65 63 74 20 74    .Please select t
	68 69 73 20 61 73 20 79   6f 75 72 20 72 65 61 6c    his as your real
	6d 20 64 75 72 69 6e 67   20 62 65 74 61 00          m during beta.

	from bnetd-0.3.23pre18 to Diablo II 1.03
	FF 34 4B 00 00 00   00 00 01 00 00 00 00 00      .4K...........
	00 C0 00 00 00 00 00 00   00 00 00 00 00 00 10 82    ................
	01 00 FF FF FF FF 00 00   00 00 51 61 72 61 74 68    ..........Qarath
	52 65 61 6C 6D 00 54 48   45 20 43 68 6F 69 63 65    Realm.THE Choice
	20 46 6F 72 20 4E 6F 77   28 74 6D 29 00              For Now(tm).
	*/
#define SERVER_REALMLISTREPLY 0x34ff /* realm list reply? */
	typedef struct
	{
		t_bnet_header h;
		bn_int        unknown1;
		bn_int        count;
		/* realm entries */
	} PACKED_ATTR() t_server_realmlistreply;
#define SERVER_REALMLISTREPLY_UNKNOWN1 0x00000000

	typedef struct
	{
		bn_int unknown3;
		bn_int unknown4;
		bn_int unknown5;
		bn_int unknown6;
		bn_int unknown7; /* this one is always different... 00 01 XX XX.. what is it? */
		bn_int unknown8;
		bn_int unknown9;
		/* realm name */
		/* realm description */
	} PACKED_ATTR() t_server_realmlistreply_data;
#define SERVER_REALMLISTREPLY_DATA_UNKNOWN3 0xc0000000
#define SERVER_REALMLISTREPLY_DATA_UNKNOWN4 0x00000000
#define SERVER_REALMLISTREPLY_DATA_UNKNOWN5 0x00000000
#define SERVER_REALMLISTREPLY_DATA_UNKNOWN6 0x00000000
#define SERVER_REALMLISTREPLY_DATA_UNKNOWN7 0x00018210 /* 98832 or 1;33296 */
#define SERVER_REALMLISTREPLY_DATA_UNKNOWN8 0xffffffff
#define SERVER_REALMLISTREPLY_DATA_UNKNOWN9 0x00000000
	/******************************************************/



	/******************************************************/
	/*
	# 44 packet from server: type=0x40ff(unknown) length=40 class=bnet
	0000:   FF 40 28 00 00 00 00 00   01 00 00 00 01 00 00 00    .@(.............
	0010:   45 75 72 6F 70 65 00 52   65 61 6C 6D 20 66 6F 72    Europe.Realm for
	0020:   20 45 75 72 6F 70 65 00                               Europe.
	*/
#define SERVER_REALMLISTREPLY_110 0x40ff

	typedef struct
	{
		t_bnet_header h;
		bn_int        unknown1;
		bn_int        count;
		/* realm entries */
	} PACKED_ATTR() t_server_realmlistreply_110;
#define SERVER_REALMLISTREPLY_110_UNKNOWN1 0x00000000

	typedef struct
	{
		bn_int unknown1;
		/* realm name */
		/* realm description */
	} PACKED_ATTR() t_server_realmlistreply_110_data;
#define SERVER_REALMLISTREPLY_110_DATA_UNKNOWN1 0x00000001


#define CLIENT_PROFILEREQ 0x35ff
	typedef struct /* join realm request */
	{
		t_bnet_header h;
		bn_int	  count;
		/* player name */
	} PACKED_ATTR() t_client_profilereq;


#define SERVER_PROFILEREPLY 0x35ff
	typedef struct /* realm join reply? */
	{
		t_bnet_header h;
		bn_int	  count;	/* same as in req */
		bn_byte	  fail;		/* != 0 if a problem occured */
		/* profile-description */
		/* profile-location */
		/* bn_int clanTAG */
	} PACKED_ATTR() t_server_profilereply;

#define CLIENT_UNKNOWN_37 0x37ff
	typedef struct /* character list request, character list upload? */
	{
		t_bnet_header h;
		bn_int        opencount;  /* Number of OPEN characters on user's machine!    */
		/* Always zero for "closed" connections.           */
		/* unknown2 */            /* subsequent blocks of t_d2char_info or something */
		/* similar, so server could read this list and     */
		/* include in the 0x37ff reply as a choice (this   */
		/* makes sense cuz the server does NOT store open  */
		/* character details - this also explains why      */
		/* unknown1 is always 0 in the beta, and the 0x00  */
		/* of unknown2 acts as a EOF when client read the  */
		/* t_d2char_info structures                        */
	} PACKED_ATTR() t_client_unknown_37;
	/******************************************************/


	/******************************************************/
	/*
	FF 37 01 01 00 00 00 00   08 00 00 00 04 00 00 00    .7..............
	42 65 74 61 57 65 73 74   2C 4D 6F 4E 6B 00 87 80    BetaWest,MoNk...
	06 01 01 01 01 50 FF FF   02 02 FF FF FF FF FF FF    .....P..........
	02 49 50 50 50 50 FF FF   FF 50 50 FF FF FF FF FF    .IPPPP...PP.....
	FF 14 88 82 80 80 FF FF   FF 00 42 65 74 61 57 65    ..........BetaWe
	73 74 2C 4D 6F 4E 6B 2D   65 00 83 80 05 02 02 01    st,MoNk-e.......
	01 2B FF 1B 02 02 FF FF   FF FF FF FF 03 FF FF FF    .+..............
	FF FF FF FF A8 FF FF FF   FF FF FF FF FF 10 80 82    ................
	80 80 FF FF FF 00 42 65   74 61 57 65 73 74 2C 4D    ......BetaWest,M
	6F 4E 6B 2D 65 65 00 83   80 06 01 01 01 01 FF 4C    oNk-ee.........L
	FF 02 02 FF FF FF FF FF   FF 01 FF 48 48 48 48 FF    ...........HHHH.
	A6 FF 48 48 FF FF FF FF   FF FF 0F 80 80 80 80 FF    ..HH............
	FF FF 00 42 65 74 61 57   65 73 74 2C 4D 6F 4E 6B    ...BetaWest,MoNk
	2D 74 77 6F 00 87 80 01   01 01 01 01 FF FF FF 01    -two............
	01 FF FF FF FF FF FF 02   FF FF FF FF FF FF FF FF    ................
	FF FF FF FF FF FF FF FF   01 84 80 FF FF FF 80 80    ................
	00                                                   .

	^-- 1: (BetaWest) MoNk
	2: (BetaWest) MoNk-e
	3: (BetaWest) MoNk-ee
	4: (BetaWest) MoNk-two

	ff 37 4e 00 00 00 00 00   08 00 00 00 01 00 00 00    .7N.............
	42 65 74 61 57 65 73 74   2c 4c 69 66 65 6c 69 6b    BetaWest,Lifelik
	65 00 87 80 01 01 01 01   01 ff ff ff 01 01 ff ff    e...............
	ff ff ff ff 03 ff ff ff   ff ff ff ff ff ff ff ff    ................
	ff ff ff ff ff 01 80 80   ff ff ff 80 80 00          ..............

	ff 37 4e 00 00 00 00 00   08 00 00 00 01 00 00 00    .7N.............
	42 65 74 61 57 65 73 74   2c 51 6c 65 78 54 45 53    BetaWest,QlexTES
	54 00 83 80 ff ff ff ff   ff 30 ff 1b ff ff ff ff    T........0......
	ff ff ff ff 04 ff ff ff   ff ff ff ff ff ff ff ff    ................
	ff ff ff ff ff 01 80 80   80 80 ff ff ff 00          ..............

	from bnetd-0.3.23pre18 to Diablo II 1.03
	"Char1 {BNE}" [lvl 20, amaz]
	"Char2 {BNE}" [lvl 21, sorc]
	"Char3 {BNE}" [lvl 22, necro]
	FF 37 D9 00 00 00   00 00 08 00 00 00 03 00    Gv.7............
	00 00 51 61 72 61 74 68   52 65 61 6C 6D 2C 43 68    ..QarathRealm,Ch
	61 72 31 00 87 80 01 01   01 01 01 01 01 01 01 01    ar1.............
	01 01 01 01 01 01 01 01   01 01 01 01 01 01 01 01    ................
	01 01 01 01 01 01 01 14   85 86 01 FF FF FF FF 42    ...............B
	4E 45 54 44 00 51 61 72   61 74 68 52 65 61 6C 6d    NETD.QarathRealm
	2C 43 68 61 72 32 00 87   80 01 01 01 01 01 01 01    ,Char2..........
	01 01 01 01 01 01 01 01   01 02 01 01 01 01 01 01    ................
	01 01 01 01 01 01 01 01   01 01 15 85 86 01 FF FF    ................
	FF FF 42 4E 45 54 44 00   51 61 72 61 74 68 52 65    ..BNETD.QarathRe
	61 6C 6D 2C 43 68 61 72   33 00 87 80 01 01 01 01    alm,Char3.......
	01 01 01 01 01 01 01 01   01 01 01 01 03 01 01 01    ................
	01 01 01 01 01 01 01 01   01 01 01 01 01 16 85 86    ................
	01 FF FF FF FF 42 4E 45   54 44 00                   .....BNETD.
	*/
#define SERVER_UNKNOWN_37 0x37ff
	typedef struct /* character list reply? */
	{
		t_bnet_header h;
		bn_int        unknown1;
		bn_int        unknown2; /* _bucky_: max chars allowed? */
		bn_int        count;    /* # of chars, same number of  */
		/* t_char_info to follow in    */
		/* packet                      */
		/* d2char_info blocks */
	} PACKED_ATTR() t_server_unknown_37;
#define SERVER_UNKNOWN_37_UNKNOWN1 0x00000000
#define SERVER_UNKNOWN_37_UNKNOWN2 0x00000008

	/* The ONLY 0x00 that should appear should be the terminating NUL for  */
	/* the character name string and the guild tag string, they're used as */
	/* delimiters to separate character name and the character structure   */
	/* If you got any other NUL's in here the next character's info will   */
	/* be royally fucked up - using 0x01 or 0xff for unknowns seem to work */
	/* well                                                                */
	typedef struct
	{
		/* "RealmName,CharacterName" - for closed characters */
		/* - OR -                                            */
		/* "CharacterName" - for open characters             */
		/* - strlen(CharacterName) must be <= 15 -           */
		bn_byte unknownb1;     /* 0x83, 0x87? */
		bn_byte unknownb2;     /* 0x80...? */
		bn_byte helmgfx;
		bn_byte bodygfx;
		bn_byte leggfx;
		bn_byte lhandweapon;
		bn_byte lhandgfx;
		bn_byte rhandweapon;

		/* Partial weapon code list:
				  0x2f: 1H Axe
				  0x30: 1H Sword
				  0x50: 2H Staff
				  0x51: Another 2H Staff
				  0x52: Another 2H Staff
				  0x53: Another 2H Staff
				  0x54: 2H Axe
				  0x55: Scythe
				  0x56: empty?
				  0x57: Another 2H Axe
				  0x58: Halberd?
				  0x59: empty?
				  0x5a: Another 2H Axe
				  0x5b: Another Halberd
				  0x5c: empty?
				  0x5d: 1H club?
				  0x5e: empty?
				  0x5f: empty?
				  */

		bn_byte rhandgfx;
		bn_byte unknownb3;
		bn_byte unknownb4;
		bn_byte unknownb5;
		bn_byte unknownb6;
		bn_byte unknownb7;
		bn_byte unknownb8;
		bn_byte unknownb9;
		bn_byte unknownb10;
		bn_byte unknownb11;
		bn_byte chclass;     /* 0x01=Amazon, 0x02=Sor, 0x03=Nec, 0x04=Pal, 0x05=Bar */

		bn_int  unknown1;
		bn_int  unknown2;
		bn_int  unknown3;
		bn_int  unknown4;

		bn_byte level;     /* yes, byte, not short/int/long  */
		bn_byte status;    /* 0x01-03 = Norm & alive         */
		/* 0x04-07 = HC & alive           */
		/* 0x08-0b = Norm & "dead"?       */
		/* 0x0c+   = HC & dead, chat only */
		/* Add 0x80 to get same effect    */
		bn_byte title;     /* 0x01=none
							  0x02=Sir/Dame?
							  0x03=Sir/Dame?
							  0x04=Lord?
							  0x05=Lord?
							  0x06=Baron?
							  0x07=Baron? */
		/* Same codes for HC chars                            */
		/* Add 0x80 to get same effect    */
		bn_byte unknownb13;
		bn_byte emblembgc; /* Guild emblem background colour */
		bn_byte emblemfgc; /* Guild emblem foreground colour */
		bn_byte emblemnum; /* Guild emblem type number       */

		/* emblem number corresponds to D2DATA.MPQ/data/global/ui/Emblems/iconXXa.dc6 */
		/* where XX = emblem number - 1 (ie, 0x0A corresponds to icon09a.dc6) use     */
		/* for dummy values seem safe... 0x01 won't work, you'll get an emblem...     */

		bn_byte unknownb14;
		/* Guild Tag */ /* must not be longer than 3 chars */
	} PACKED_ATTR() t_d2char_info;
#define D2CHAR_INFO_UNKNOWNB1 0x83
#define D2CHAR_INFO_UNKNOWNB2 0x80
#define D2CHAR_INFO_FILLER 0xff /* non-zero padding */
#define D2CHAR_INFO_CLASS_AMAZON      0x01
#define D2CHAR_INFO_CLASS_SORCERESS   0x02
#define D2CHAR_INFO_CLASS_NECROMANCER 0x03
#define D2CHAR_INFO_CLASS_PALADIN     0x04
#define D2CHAR_INFO_CLASS_BARBARIAN   0x05
#define D2CHAR_INFO_CLASS_DRUID       0x06
#define D2CHAR_INFO_CLASS_ASSASSIN    0x07
	/******************************************************/


	/******************************************************/
	/* D2 packet... not sent very often and the client doesn't
	 * seem to expect an answer */
	/* FIXME: what the hell does this one do? */
	/*
	FF 39 13 00 42 65 74 61   57 65 73 74 2C 62 75 73    .9..BetaWest,bus
	74 61 00                                             ta.

	this one was sent after a closed character was deleted on the auth
	server... maybe a notifier for the gateway server?
	FF 39 17 00 42 6F 62 73   57 6F 72 6C 64 2C 63 68    .9..BobsWorld,ch
	61 72 6E 61 6D 65 00                                 arname.
	*/
#define CLIENT_UNKNOWN_39 0x39ff
	typedef struct
	{
		t_bnet_header h;
		/* character name */ /* what about open chars? */
	} PACKED_ATTR() t_client_unknown_39;
	/******************************************************/


	/******************************************************/
	/*
	FF 3A 2E 00 58 4C F2 00   19 C2 08 00 D7 33 37 D3    .:..XL.......37.
	42 8C 92 37 C2 26 08 A9   3E 92 05 28 A1 5A 18 B9    B..7.&..>..(.Z..
	6D 61 73 74 6F 64 6F 6E   74 66 69 6C 6D 00          mastodontfilm.

	FF 3A 28 00 2B 73 1C 01   88 91 F2 0D AF 22 43 25    .:(.+s......."C%
	BF E4 2D 45 42 37 04 DB   AF 95 66 71 16 85 67 60    ..-EB7....fq..g`
	51 6C 65 78 53 5A 47 00                              QlexSZG.
	*/
#define CLIENT_LOGINREQ2 0x3aff
	typedef struct
	{
		t_bnet_header h;
		bn_int        ticks; /* is it really? */
		bn_int        sessionkey;
		bn_int        password_hash2[5];
		/* player name */
	} PACKED_ATTR() t_client_loginreq2;
	/******************************************************/


	/******************************************************/
	/*
	# 21 packet from client: type=0x46ff(unknown) length=8 class=bnet
	0000:   FF 46 08 00 00 00 00 00                              .F......
	*/
#define CLIENT_MOTD_W3 0x46ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        last_news_time; /* date of the last news item the client has */
	} PACKED_ATTR() t_client_motd_w3;
	/******************************************************/


	/******************************************************/
	/*
	# 22 packet from server: type=0x46ff(unknown) length=225 class=bnet
	0000:   FF 46 E1 00 01 16 3A 6C   3C FF FF FF FF 00 00 00    .F....:l<.......
	0010:   00 00 00 00 00 57 65 6C   63 6F 6D 65 20 74 6F 20    .....Welcome to
	0020:   42 61 74 74 6C 65 2E 6E   65 74 21 0A 54 68 69 73    Battle.net!.This
	0030:   20 73 65 72 76 65 72 20   69 73 20 68 6F 73 74 65     server is hoste
	0040:   64 20 62 79 20 41 54 26   54 2E 0A 54 68 65 72 65    d by AT&T..There
	0050:   20 61 72 65 20 63 75 72   72 65 6E 74 6C 79 20 36     are currently 6
	0060:   32 38 20 75 73 65 72 73   20 70 6C 61 79 69 6E 67    28 users playing
	0070:   20 31 35 39 20 67 61 6D   65 73 20 6F 66 20 57 61     159 games of Wa
	0080:   72 63 72 61 66 74 20 49   49 49 2C 20 61 6E 64 20    rcraft III, and
	0090:   31 37 37 33 34 36 20 75   73 65 72 73 20 70 6C 61    177346 users pla
	00A0:   79 69 6E 67 20 37 37 38   33 37 20 67 61 6D 65 73    ying 77837 games
	00B0:   20 6F 6E 20 42 61 74 74   6C 65 2E 6E 65 74 2E 0A     on Battle.net..
	00C0:   4C 61 73 74 20 6C 6F 67   6F 6E 3A 20 54 68 75 20    Last logon: Thu
	00D0:   46 65 62 20 31 34 20 20   35 3A 32 38 20 50 4D 0A    Feb 14  5:28 PM.
	00E0:   00                                                   .

	# Match 4, 2002
	# 92 packet from server: type=0x46ff(unknown) length=859 class=bnet
	0000:   FF 46 5B 03 01 B4 B2 82   3C 20 B6 83 3C 20 B6 83    .F[.....< ..< ..
	0010:   3C 20 B6 83 3C 57 65 20   68 61 76 65 20 62 65 65    < ..<We have bee
	# 93 packet from server: type=0x46ff(unknown) length=223 class=bnet
	0000:   FF 46 DF 00 01 B4 B2 82   3C 20 B6 83 3C 20 B6 83    .F......< ..< ..
	0010:   3C 00 00 00 00 57 65 6C   63 6F 6D 65 20 74 6F 20    <....Welcome to

	*/
#define SERVER_MOTD_W3 0x46ff
	typedef struct
	{
		t_bnet_header h;
		bn_byte        msgtype; /* we only saw "1" type so far */
		bn_int         curr_time; /* as seen by the server */
		bn_int         first_news_time; /* the oldest news item's timestamp */
		bn_int         timestamp; /* the timestamp of this news item */
		/* it is equal with the latest news item timestamp for
		the welcome message */
		bn_int         timestamp2; /* always equal with the timestamp except the
						last packet which shows in the right panel */
		/* text */
	} PACKED_ATTR() t_server_motd_w3;
#define SERVER_MOTD_W3_MSGTYPE  0x01
#define SERVER_MOTD_W3_WELCOME  0x00000000
	/******************************************************/

	/******************************************************/
	/*
	# Jon/bbbbb
	# 28 packet from client: type=0x53ff(unknown) length=40 class=bnet
	0000:   FF 53 28 00 6F FD 5F 61   C3 D1 C4 78 E6 2E 24 8B    .S(.o._a...x..$.
	0010:   32 EB 36 9C 39 57 D8 BA   57 84 67 5E E7 78 5B 01    2.6.9W..W.g^.x[.
	0020:   6D 99 87 15 4A 6F 6E 00                              m...Jon.
	*/
#define CLIENT_LOGINREQ_W3 0x53ff
	typedef struct
	{
		t_bnet_header h;
		bn_byte        client_public_key[32];
		/* player name */
	} PACKED_ATTR() t_client_loginreq_w3;
	/******************************************************/

	/******************************************************/
	/*
	12:33:56.255569 63.241.83.11.6112 > ws-2-11.1038: P 190:262(72) ack 272 win 65264
	** ** ** ** ** ** ** **   ** ** ** ** ** ** ** **
	** ** ** ** ** ** ** **   ** ** ** ** ** ** ** **
	** ** ** ** ** ** ** **   FF 53 48 00 00 00 00 00            .SH.....
	4B A8 FF 5D 1E 5D 2D 50   D1 2B B2 95 74 AD 5F 4E    K..].]-P.+..t._N
	88 A4 88 48 18 27 89 50   F1 AA 1B D5 D7 B6 47 BC    ...H.'.P......G.
	30 8B 2A 54 AA 99 23 96   75 8A 5E 67 35 8E 5B 22    0.*T..#.u.^g5.["
	2C 0E 68 2E C2 95 E9 D7   A1 82 F1 2C 1E 2B 28 36    ,.h........,.+(6
	*/
#define SERVER_LOGINREPLY_W3 0x53ff
	typedef struct
	{
		t_bnet_header h;
		bn_int       message;
		/* seems to be response to client-challenge */
		bn_byte       salt[32];
		bn_byte       server_public_key[32];
	} PACKED_ATTR() t_server_loginreply_w3;
#define SERVER_LOGINREPLY_W3_MESSAGE_SUCCESS 0x00000000
#define SERVER_LOGINREPLY_W3_MESSAGE_ALREADY 0x00000001 /* Account already logged on */
#define SERVER_LOGINREPLY_W3_MESSAGE_BADACCT 0x00000001 /* Accoutn does not exist */
	/******************************************************/

	/******************************************************/
	/* single player crack based:
	# 34 packet from server: type=0x54ff(unknown) length=40 class=bnet
	0000:   FF 54 28 00 00 00 00 00   00 00 00 00 00 00 00 00    .T(.............
	0010:   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
	0020:   00 00 00 00 00 00 00 00                              ........

	* Password Checksum ? *
	-- client --
	0x54ff - 2 bytes
	size - 2 bytes (0x0018)
	unknown1 - 20 bytes
	-- server --
	0x54ff - 2 bytes
	size - 2 bytes
	msgid - 4 bytes
	{
	0x00000000	accept
	0x00000002	password incorrect
	}
	unknown1 - 20 bytes

	Packet #13
	0x0000   FF 54 1C 00 00 00 00 00-3A D5 B9 B1 2B D9 B5 D9   T......:չ?ٵ?
	0x0010   87 3B 2B 3D 28 57 C0 2E-02 93 5F 8B               ?+=(W?._?
	*/
#define CLIENT_LOGONPROOFREQ 0x54ff
	typedef struct
	{
		t_bnet_header h;
		bn_byte	client_password_proof[20];
	} PACKED_ATTR() t_client_logonproofreq;

#define SERVER_LOGONPROOFREPLY 0x54ff
	typedef struct
	{
		t_bnet_header h;
		bn_int	response;
		bn_byte	server_password_proof[20];
	} PACKED_ATTR() t_server_logonproofreply;
#define SERVER_LOGONPROOFREPLY_RESPONSE_OK      0x00000000
#define SERVER_LOGONPROOFREPLY_RESPONSE_BADPASS 0x00000002
#define SERVER_LOGONPROOFREPLY_RESPONSE_EMAIL   0x0000000E
#define SERVER_LOGONPROOFREPLY_RESPONSE_CUSTOM  0x0000000F
	/******************************************************/

	/******************************************************/
#define CLIENT_PASSCHANGEREQ 0x55ff
	typedef struct
	{
		t_bnet_header h;
		bn_byte client_public_key[32];
		/* username */
	} PACKED_ATTR() t_client_passchangereq;

#define SERVER_PASSCHANGEREPLY 0x55ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        message;
		bn_byte       salt[32];
		bn_byte       server_public_key[32];
	} PACKED_ATTR() t_server_passchangereply;
#define SERVER_PASSCHANGEREPLY_MESSAGE_ACCEPT 0x00000000
#define SERVER_PASSCHANGEREPLY_MESSAGE_REJECT 0x00000001 /* No such account */
	/******************************************************/

	/******************************************************/
#define CLIENT_PASSCHANGEPROOFREQ 0x56ff
	typedef struct
	{
		t_bnet_header h;
		bn_byte       client_password_proof[20];
		bn_byte       salt[32];
		bn_byte       password_verifier[32];
	} PACKED_ATTR() t_client_passchangeproofreq;

#define SERVER_PASSCHANGEPROOFREPLY 0x56ff
	typedef struct
	{
		t_bnet_header h;
		bn_int	response;
		bn_byte       server_password_proof[20];
	} PACKED_ATTR() t_server_passchangeproofreply;
#define SERVER_PASSCHANGEPROOFREPLY_RESPONSE_OK      0x00000000
#define SERVER_PASSCHANGEPROOFREPLY_RESPONSE_BADPASS 0x00000002
	/******************************************************/

	/******************************************************/
	/*
	# 13 packet from client: type=0x52ff(unknown) length=83 class=bnet
	0000:   FF 52 53 00 2B 63 B9 05   CA F3 E1 BA 58 5C ED 65    .RS.+c......X\.e
	0010:   BE 8F 0E 89 A9 B8 C7 FE   75 2C 44 10 AE 19 B5 14    ........u,D.....
	0020:   E8 CA E9 C7 37 50 7D 0F   9A 89 00 FF 2F 10 BB EE    ....7P}...../...
	0030:   A8 0C 81 64 AD AF DC C7   3F 58 F1 20 A1 05 E2 38    ...d....?X. ...8
	0040:   18 87 85 5B 74 68 65 61   63 63 6F 75 6E 74 6E 61    ...[theaccountna
	0050:   6D 65 00                                             me.
	*/
#define CLIENT_CREATEACCOUNT_W3 0x52ff
	typedef struct
	{
		t_bnet_header h;
		bn_byte       salt[32];
		bn_byte       password_verifier[32];
		/* player name */
	} PACKED_ATTR() t_client_createaccount_w3;
	/******************************************************/


	/******************************************************/

	/******************************************************/

	/*
	# 20 packet from client: type=0x45ff(unknown) length=6 class=bnet
	0000:   FF 45 06 00 E0 17                                    .E....
	*/
#define CLIENT_CHANGEGAMEPORT 0x45ff
	typedef struct
	{
		t_bnet_header	h;
		bn_short	port;
	} PACKED_ATTR() t_client_changegameport;




	/******************************************************/
	/*
	RECV-> 0000   FF 52 08 00 00 00 00 00                            .R......
	*/
#define SERVER_CREATEACCOUNT_W3 0x52ff
	typedef struct
	{
		t_bnet_header h;
		bn_int       result;
	} PACKED_ATTR() t_server_createaccount_w3;
#define SERVER_CREATEACCOUNT_W3_RESULT_OK    0x00000000
#define SERVER_CREATEACCOUNT_W3_RESULT_EXIST 0x00000004
#define SERVER_CREATEACCOUNT_W3_RESULT_EMPTY 0x00000007
#define SERVER_CREATEACCOUNT_W3_RESULT_INVALID 0x00000008
#define SERVER_CREATEACCOUNT_W3_RESULT_BANNED 0x00000009
#define SERVER_CREATEACCOUNT_W3_RESULT_SHORT 0x0000000A
#define SERVER_CREATEACCOUNT_W3_RESULT_PUNCTUATION 0x0000000B
#define SERVER_CREATEACCOUNT_W3_RESULT_PUNCTUATION2 0x0000000C
	/******************************************************/

	/******************************************************/
	/*
	FF 3A 08 00 00 00 00 00                              .:......
	*/
#define SERVER_LOGINREPLY2 0x3aff
	typedef struct
	{
		t_bnet_header h;
		bn_int        message;
		/* reason for account lock */
	} PACKED_ATTR() t_server_loginreply2;
#define SERVER_LOGINREPLY2_MESSAGE_SUCCESS  0x00000000
#define SERVER_LOGINREPLY2_MESSAGE_NONEXIST 0x00000001 /* Account does not exist */
#define SERVER_LOGINREPLY2_MESSAGE_BADPASS  0x00000002 /* Bad password */
#define SERVER_LOGINREPLY2_MESSAGE_LOCKED   0x00000006 /* Account is locked */
	/******************************************************/


	/******************************************************/
	/* Diablo II 1.03 */
	/* sent when registering new player with open Battle.net */
	/* and when logging in with closed Battle.net */
	/*
	(closed) b.net login:
	enter name/password -> 3dff packet
	(open) b.net login:
	enter name/password -> "login" packet (bad account) <-- CORRECT
	"Create new account" -> TOS grab -> Enter password -> 3dff packet
	*/
	/*
	FF 3D 20 00 B8 C0 A1 2F   56 B1 47 65 CF 55 09 62    .= ..../V.Ge.U.b
	8E 21 3C 59 57 BC E8 EA   45 6C 66 6C 6F 72 64 00    .!<YW...Elflord.
	*/
#define CLIENT_CREATEACCTREQ2 0x3dff
	typedef struct
	{
		t_bnet_header h;
		bn_int        password_hash1[5];
		/* username (charactername?) */
	} PACKED_ATTR() t_client_createacctreq2;
	/******************************************************/


	/******************************************************/
	/* Diablo II 1.03 */
#define SERVER_CREATEACCTREPLY2 0x3dff
	typedef struct
	{
		t_bnet_header h;
		bn_int        result;
	} PACKED_ATTR() t_server_createacctreply2;
#define SERVER_CREATEACCTREPLY2_RESULT_OK    0x00000000
#define SERVER_CREATEACCTREPLY2_RESULT_SHORT 0x00000001 /* Username must be a minimum of 2 characters */
#define SERVER_CREATEACCTREPLY2_RESULT_INVALID 0x00000002
#define SERVER_CREATEACCTREPLY2_RESULT_BANNED 0x00000003
#define SERVER_CREATEACCTREPLY2_RESULT_EXIST 0x00000004
#define SERVER_CREATEACCTREPLY2_RESULT_LAST_CREATE_IN_PROGRESS 0x00000005
#define SERVER_CREATEACCTREPLY2_RESULT_ALPHANUM 0x00000006
#define SERVER_CREATEACCTREPLY2_RESULT_PUNCTUATION 0x00000007
#define SERVER_CREATEACCTREPLY2_RESULT_PUNCTUATION2 0x00000008
	/******************************************************/


	/******************************************************/
	/*
	FF 36 0C 00 01 00 00 00   52 6F 62 00                .6......Rob.

	FF 36 0C 00 01 00 00 00   42 6F 62 00                .6......Bob.
	*/
#define SERVER_CDKEYREPLY2 0x36ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        message;
		/* owner name */
	} PACKED_ATTR() t_server_cdkeyreply2;
#define SERVER_CDKEYREPLY2_MESSAGE_OK       0x00000001
#define SERVER_CDKEYREPLY2_MESSAGE_BAD      0x00000002
#define SERVER_CDKEYREPLY2_MESSAGE_WRONGAPP 0x00000003
#define SERVER_CDKEYREPLY2_MESSAGE_ERROR    0x00000004
#define SERVER_CDKEYREPLY2_MESSAGE_INUSE    0x00000005
	/* (any other value seems to correspond to ok) */
	/******************************************************/


	/******************************************************/
	/*
	FF 14 08 00 74 65 6E 62                              ....tenb
	*/
#define CLIENT_UDPOK 0x14ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        echo; /* echo of what the server sent, normally "tenb" */
	} PACKED_ATTR() t_client_udpok;
	/******************************************************/


	/******************************************************/
	/*
	FF 33 18 00 1A 00 00 00   00 00 00 00 74 6F 73 5F    .3..........tos_
	55 53 41 2E 74 78 74 00                              USA.txt.

	Diablo II 1.03
	FF 33 19 00 04 00   00 80 00 00 00 00 62 6E      .3..........bn
	73 65 72 76 65 72 2E 69   6E 69 00                   server.ini.
	*/
#define CLIENT_FILEINFOREQ 0x33ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        type;     /* type of file (TOS,icons,etc.) */
		bn_int        unknown2; /* 00 00 00 00 */ /* always zero? */
		/* filename */          /* default/suggested filename? */
	} PACKED_ATTR() t_client_fileinforeq;
#define CLIENT_FILEINFOREQ_TYPE_TOS          0x0000001a /* tos_USA.txt */
#define CLIENT_FILEINFOREQ_TYPE_GATEWAYS     0x0000001b /* STAR bnserver.ini */
#define CLIENT_FILEINFOREQ_TYPE_GATEWAYS_D2  0x80000004 /* D2XP bnserver-D2DV.ini */
	/* FIXME: what about icons.bni? */
#define CLIENT_FILEINFOREQ_TYPE_ICONS        0x0000001d /* STAR icons_STAR.bni */
#define CLIENT_FILEINFOREQ_UNKNOWN2          0x00000000
#define CLIENT_FILEINFOREQ_FILE_TOSUSA        "tos_USA.txt"
#define CLIENT_FILEINFOREQ_FILE_TOSUNICODEUSA "tos-unicode_USA.txt"
#define CLIENT_FILEINFOREQ_FILE_BNSERVER      "bnserver.ini"
	/******************************************************/


	/******************************************************/
	/*
	FF 33 1C 00 1A 00 00 00   00 00 00 00 30 C3 89 86    .3..........0...
	09 4F BD 01 74 6F 73 2E   74 78 74 00                .O..tos.txt.

	FF 33 20 00 1A 00 00 00   00 00 00 00 00 38 51 E2    .3 ..........8Q.
	30 A1 BD 01 74 6F 73 5F   55 53 41 2E 74 78 74 00    0...tos_USA.txt.
	*/
#define SERVER_FILEINFOREPLY 0x33ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        type;      /* type of file (TOS,icons,etc.) */
		bn_int        unknown2;  /* 00 00 00 00 */ /* same as in TOSREQ */
		bn_long       timestamp; /* file modification time */
		/* filename */
	} PACKED_ATTR() t_server_fileinforeply;
#define SERVER_FILEINFOREPLY_TYPE_TOSFILE      0x0000001a /* tos_USA.txt */
#define SERVER_FILEINFOREPLY_TYPE_GATEWAYS     0x0000001b /* STAR bnserver.ini */
#define SERVER_FILEINFOREPLY_TYPE_GATEWAYS_D2  0x80000004 /* D2XP bnserver-D2DV.ini */
#define SERVER_FILEINFOREPLY_TYPE_ICONS        0x0000001d /* STAR icons_STAR.bni */
#define SERVER_FILEINFOREPLY_UNKNOWN2          0x00000000 /* always zero */
	/******************************************************/


	/******************************************************/
	/*
				FF 26 9E 01   01 00 00 00 13 00 00 00        .&..........
				78 52 82 02 42 6F 62 00   70 72 6F 66 69 6C 65 5C    xR..Bob.profile\
				73 65 78 00 70 72 6F 66   69 6C 65 5C 61 67 65 00    sex.profile\age.
				70 72 6F 66 69 6C 65 5C   6C 6F 63 61 74 69 6F 6E    profile\location
				00 70 72 6F 66 69 6C 65   5C 64 65 73 63 72 69 70    .profile\descrip
				74 69 6F 6E 00 52 65 63   6F 72 64 5C 53 45 58 50    tion.Record\SEXP
				5C 30 5C 77 69 6E 73 00   52 65 63 6F 72 64 5C 53    \0\wins.Record\S
				45 58 50 5C 30 5C 6C 6F   73 73 65 73 00 52 65 63    EXP\0\losses.Rec
				6F 72 64 5C 53 45 58 50   5C 30 5C 64 69 73 63 6F    ord\SEXP\0\disco
				6E 6E 65 63 74 73 00 52   65 63 6F 72 64 5C 53 45    nnects.Record\SE
				58 50 5C 30 5C 6C 61 73   74 20 67 61 6D 65 00 52    XP\0\last game.R
				65 63 6F 72 64 5C 53 45   58 50 5C 30 5C 6C 61 73    ecord\SEXP\0\las
				74 20 67 61 6D 65 20 72   65 73 75 6C 74 00 52 65    t game result.Re
				63 6F 72 64 5C 53 45 58   50 5C 31 5C 77 69 6E 73    cord\SEXP\1\wins
				00 52 65 63 6F 72 64 5C   53 45 58 50 5C 31 5C 6C    .Record\SEXP\1\l
				6F 73 73 65 73 00 52 65   63 6F 72 64 5C 53 45 58    osses.Record\SEX
				50 5C 31 5C 64 69 73 63   6F 6E 6E 65 63 74 73 00    P\1\disconnects.
				52 65 63 6F 72 64 5C 53   45 58 50 5C 31 5C 72 61    Record\SEXP\1\ra
				74 69 6E 67 00 52 65 63   6F 72 64 5C 53 45 58 50    ting.Record\SEXP
				5C 31 5C 68 69 67 68 20   72 61 74 69 6E 67 00 52    \1\high rating.R
				65 63 6F 72 64 5C 53 45   58 50 5C 31 5C 72 61 6E    ecord\SEXP\1\ran
				6B 00 52 65 63 6F 72 64   5C 53 45 58 50 5C 31 5C    k.Record\SEXP\1\
				68 69 67 68 20 72 61 6E   6B 00 52 65 63 6F 72 64    high rank.Record
				5C 53 45 58 50 5C 31 5C   6C 61 73 74 20 67 61 6D    \SEXP\1\last gam
				65 00 52 65 63 6F 72 64   5C 53 45 58 50 5C 31 5C    e.Record\SEXP\1\
				6C 61 73 74 20 67 61 6D   65 20 72 65 73 75 6C 74    last game result
				00 00                                                ..

				FF 26 C2 01   05 00 00 00 13 00 00 00        .&..........
				EE E4 84 03 6E 73 6C 40   63 6C 6F 75 64 00 63 6C    ....nsl@cloud.cl
				6F 75 64 00 67 75 65 73   74 00 48 65 72 6E 40 73    oud.guest.Hern@s
				65 65 6D 65 00 6F 72 69   6F 6E 40 00 70 72 6F 66    eeme.orion@.prof
				69 6C 65 5C 73 65 78 00   70 72 6F 66 69 6C 65 5C    ile\sex.profile\
				61 67 65 00 70 72 6F 66   69 6C 65 5C 6C 6F 63 61    age.profile\loca
				74 69 6F 6E 00 70 72 6F   66 69 6C 65 5C 64 65 73    tion.profile\des
				63 72 69 70 74 69 6F 6E   00 52 65 63 6F 72 64 5C    cription.Record\
				53 74 61 72 5C 30 5C 77   69 6E 73 00 52 65 63 6F    Star\0\wins.Reco
				72 64 5C 53 74 61 72 5C   30 5C 6C 6F 73 73 65 73    rd\Star\0\losses
				00 52 65 63 6F 72 64 5C   53 74 61 72 5C 30 5C 64    .Record\Star\0\d
				69 73 63 6F 6E 6E 65 63   74 73 00 52 65 63 6F 72    isconnects.Recor
				64 5C 53 74 61 72 5C 30   5C 6C 61 73 74 20 67 61    d\Star\0\last ga
				6D 65 00 52 65 63 6F 72   64 5C 53 74 61 72 5C 30    me.Record\Star\0
				5C 6C 61 73 74 20 67 61   6D 65 20 72 65 73 75 6C    \last game resul
				74 00 52 65 63 6F 72 64   5C 53 74 61 72 5C 31 5C    t.Record\Star\1\
				77 69 6E 73 00 52 65 63   6F 72 64 5C 53 74 61 72    wins.Record\Star
				5C 31 5C 6C 6F 73 73 65   73 00 52 65 63 6F 72 64    \1\losses.Record
				5C 53 74 61 72 5C 31 5C   64 69 73 63 6F 6E 6E 65    \Star\1\disconne
				63 74 73 00 52 65 63 6F   72 64 5C 53 74 61 72 5C    cts.Record\Star\
				31 5C 72 61 74 69 6E 67   00 52 65 63 6F 72 64 5C    1\rating.Record\
				53 74 61 72 5C 31 5C 68   69 67 68 20 72 61 74 69    Star\1\high rati
				6E 67 00 52 65 63 6F 72   64 5C 53 74 61 72 5C 31    ng.Record\Star\1
				5C 72 61 6E 6B 00 52 65   63 6F 72 64 5C 53 74 61    \rank.Record\Sta
				72 5C 31 5C 68 69 67 68   20 72 61 6E 6B 00 52 65    r\1\high rank.Re
				63 6F 72 64 5C 53 74 61   72 5C 31 5C 6C 61 73 74    cord\Star\1\last
				20 67 61 6D 65 00 52 65   63 6F 72 64 5C 53 74 61     game.Record\Sta
				72 5C 31 5C 6C 61 73 74   20 67 61 6D 65 20 72 65    r\1\last game re
				73 75 6C 74 00 00                                    sult..
				*/
#define CLIENT_STATSREQ 0x26ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        name_count;
		bn_int        key_count;
		bn_int        requestid; /* 78 52 82 02 */
		/* player name */
		/* field key ... */
	} PACKED_ATTR() t_client_statsreq;
#define CLIENT_STATSREQ_UNKNOWN1 0x02825278
	/******************************************************/


	/******************************************************/
	/*
							  FF 26 23 00 01 00 00 00            .&#.....
							  13 00 00 00 78 52 82 02   00 00 00 00 00 00 00 00    ....xR..........
							  00 00 00 00 00 00 00 00   00 00 00 00 00 00          ..............

							  FF 26 13 02   05 00 00 00 13 00 00 00        .&..........
							  EE E4 84 03 20 20 A1 F0   00 20 20 A1 F0 00 68 74    ....  ...  ...ht
							  74 70 3A 2F 2F 6E 73 6C   2E 6B 6B 69 72 69 2E 6F    tp://nsl.kkiri.o
							  72 67 00 20 20 20 20 20   20 20 20 20 20 20 A2 CB    rg.           ..
							  20 50 72 6F 74 6F 73 73   20 69 73 20 54 68 65 20     Protoss is The
							  42 65 73 74 20 A2 CB 20   0D 0A 0D 0A 20 20 20 49    Best .. ....   I
							  66 20 59 6F 75 20 57 61   6E 74 20 54 6F 20 4B 6E    f You Want To Kn
							  6F 77 20 41 62 6F 75 74   20 55 73 2C 20 47 6F 0D    ow About Us, Go.
							  0A 0D 0A 20 20 20 20 20   20 20 20 20 20 20 20 20    ...
							  20 68 74 74 70 3A 2F 2F   6E 73 6C 2E 6B 6B 69 72     http://nsl.kkir
							  69 2E 6F 72 67 00 38 38   00 37 30 00 33 00 32 39    i.org.88.70.3.29
							  32 36 30 33 31 30 20 33   39 36 31 36 37 38 32 34    260310 396167824
							  30 00 4C 4F 53 53 00 30   00 30 00 30 00 30 00 00    0.LOSS.0.0.0.0..
							  00 00 00 00 00 6D 00 31   35 00 53 69 6E 67 61 70    .....m.15.Singap
							  6F 72 65 20 00 00 32 37   00 31 00 36 00 32 39 32    ore ..27.1.6.292
							  35 34 32 33 37 20 32 34   35 37 32 33 30 39 38 00    54237 245723098.
							  44 52 41 57 00 30 00 30   00 30 00 30 00 00 00 00    DRAW.0.0.0.0....
							  00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
							  00 00 00 00 00 00 00 00   00 00 31 00 00 00 32 39    ..........1...29
							  32 35 39 38 31 32 20 31   31 35 33 38 33 37 39 34    259812 115383794
							  36 00 57 49 4E 00 30 00   30 00 30 00 30 00 00 00    6.WIN.0.0.0.0...
							  00 00 00 00 B0 C5 BD C3   B1 E2 00 3F 3F 00 B1 D9    ...........??...
							  B0 C5 C1 F6 20 BE F8 C0   BD 2E 00 BA B0 C0 DA B8    .... ...........
							  AE 20 6F 72 69 6F 6E 20   2C 2C 20 C3 CA C4 DA C6    . orion ,, .....
							  C4 C0 CC B0 A1 20 BE C6   B3 E0 BF EB 2E 2E 0D 0A    ..... ..........
							  C4 ED C4 ED 2E 2E 0D 0A   0D 0A C1 B9 B6 F3 20 C0    .............. .
							  DF C7 CF B4 C2 20 B3 D1   20 3A 20 6E 73 6C B3 D1    ..... .. : nsl..
							  B5 E9 0D 0A C0 DF C7 CF   B4 C2 20 C7 C1 C5 E4 20    .......... ....
							  3A 20 6E 73 6C 40 74 6F   74 6F 72 6F 00 35 31 00    : nsl@totoro.51.
							  34 38 00 36 00 32 39 32   35 39 32 36 33 20 39 35    48.6.29259263 95
							  36 35 30 35 39 30 32 00   4C 4F 53 53 00 36 00 36    6505902.LOSS.6.6
							  00 32 00 39 39 30 00 31   30 32 37 00 00 00 32 39    .2.990.1027...29
							  32 35 39 32 35 38 20 33   31 32 32 37 39 38 38 32    259258 312279882
							  00 4C 4F 53 53 00 00                                 .LOSS..
							  */
#define SERVER_STATSREPLY 0x26ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        name_count;
		bn_int        key_count;
		bn_int        requestid; /* 78 52 82 02 */ /* EE E4 84 03 */ /* same as request */
		/* field values ... */
	} PACKED_ATTR() t_server_statsreply;
	/******************************************************/


	/******************************************************/
	/*
	FF 29 25 00 CF 17 28 00   A3 D3 2C 5C F4 18 02 40    .)%...(...,\...@
	F9 B8 EA F4 A5 B1 3F 39   85 89 2D DB 18 2D B9 D4    ......?9..-..-..
	52 6F 73 73 00                                       Ross.
	*/
#define CLIENT_LOGINREQ1 0x29ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        ticks;
		bn_int        sessionkey;
		bn_int        password_hash2[5]; /* hash of ticks, key, and hash1 */
		/* player name */
	} PACKED_ATTR() t_client_loginreq1;
	/******************************************************/


	/******************************************************/
#define SERVER_LOGINREPLY1 0x29ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        message;
	} PACKED_ATTR() t_server_loginreply1;
#define SERVER_LOGINREPLY1_MESSAGE_FAIL    0x00000000
#define SERVER_LOGINREPLY1_MESSAGE_SUCCESS 0x00000001
	/******************************************************/


	/******************************************************/
	/*
	FF 31 3B 00 22 1A 9A 00   64 B7 C5 21 2C 82 57 F4    .1;."...d..!,.W.
	0A 36 73 25 1E A5 42 5F   FA 36 54 97 BC 65 3F E1    .6s%..B_.6T..e?.
	7D 5A 54 17 4C 33 B9 1A   09 25 49 45 99 52 69 45    }ZT.L3...%IE.RiE
	B1 E6 5C 9C 77 72 73 70   6F 69 00                   ..\.wrspoi.

	FF 31 3B 00 3A 5B 9B 00   64 B7 C5 21 99 32 14 B7    .1;.:[..d..!.2..
	89 02 3C 28 4A 75 84 05   70 EF B5 A7 99 CA 7E 12    ..<(Ju..p.....~.
	7D 5A 54 17 4C 33 B9 1A   09 25 49 45 99 52 69 45    }ZT.L3...%IE.RiE
	B1 E6 5C 9C 77 72 73 70   6F 69 00                   ..\.wrspoi.

	from D2 LoD 1.08
	FF 31 38 00 79 8E 09 00   F5 1C EB 4E 2E 7A 9C 6A    .18.y......N.z.j
	13 43 4A 49 2C CE 49 24   2E 65 FB 95 44 FC C3 B2    .CJI,.I$.e..D...
	E1 75 1A DA 19 36 EE 9B   AA EE 23 99 F0 82 4F F8    .u...6....#...O.
	B9 6B 09 55 62 6F 62 00                              .k.Ubob.
	*/
#define CLIENT_CHANGEPASSREQ 0x31ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        ticks; /* FIXME: upper two bytes seem constant for each client? */
		bn_int        sessionkey;
		bn_int        oldpassword_hash2[5]; /* hash of ticks, key, hash1 */
		bn_int        newpassword_hash1[5]; /* hash of lowercase password w/o null */
		/* player name */
	} PACKED_ATTR() t_client_changepassreq;
	/******************************************************/


	/******************************************************/
	/*
	FF 31 08 00 01 00 00 00                              .1......

	FF 31 08 00 00 00 00 00                              .1......
	*/
#define SERVER_CHANGEPASSACK 0x31ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        message;
	} PACKED_ATTR() t_server_changepassack;
#define SERVER_CHANGEPASSACK_MESSAGE_FAIL    0x00000000
#define SERVER_CHANGEPASSACK_MESSAGE_SUCCESS 0x00000001
	/******************************************************/


	/******************************************************/
	/*
							 FF 0A 0F 00 0F                   .....
							 4D 79 41 63 63 6F 75 6E          MyAccoun
							 74 00 00                                             t..

							 ff 0a 1f 00 4c 69 66 65   6c 69 6b 65 00 42 65 74    ....Lifelike.Bet
							 61 57 65 73 74 2c 4c 69   66 65 6c 69 6b 65 00       aWest,Lifelike.

							 Diablo II 1.03 - amazon just after creation
							 ff 0a 20 00 47 6f   64 64 65 73 73 00 51 61      .. .Goddess.Qa
							 72 61 74 68 52 65 61 6c   6d 2c 47 6f 64 64 65 73    rathRealm,Goddes
							 73 00                                                s.
							 */
#define CLIENT_PLAYERINFOREQ 0x0aff
	typedef struct
	{
		t_bnet_header h;
		/* player name */
		/* player info */ /* used by Diablo and D2 (character,Realm) */
	} PACKED_ATTR() t_client_playerinforeq;
	/******************************************************/


	/******************************************************/
	/*
							  FF 0A 29 00 4D 79 41 63            ..).MyAc
							  63 6F 75 6E 74 00 50 58   45 53 20 30 20 30 20 30    count.PXES 0 0 0
							  20 30 20 30 20 30 00 4D   79 41 63 63 6F 75 6E 74     0 0 0.MyAccount
							  00                                                   .

							  FF 0A 2F 00 6C 61 77 75   65 66 00 4C 54 52 44 20    ../.lawuef.LTRD
							  31 20 30 20 30 20 33 30   20 31 30 20 32 30 20 32    1 0 0 30 10 20 2
							  35 20 31 30 30 20 30 00   6C 61 77 75 65 66 00       5 100 0.lawuef.

							  ff 0a 53 00 4d 6f 4e   6b 32 6b 00 56 44 32 44     ..S.MoNk2k.VD2D
							  42 65 74 61 57 65 73 74   2c 4d 6f 4e 6b 2d 65 65    BetaWest,MoNk-ee
							  2c 87 80 06 01 01 01 01   ff 4c ff 02 02 ff ff ff    ,........L......
							  ff ff ff 01 ff 48 48 48   48 ff a6 ff 48 48 ff ff    .....HHHH...HH..
							  ff ff ff ff 0f 88 80 80   80 ff ff ff 00 4d 6f 4e    .............MoN
							  6b 32 6b 00                                          k2k.

							  ff 0a 5c 00 47 61 6d 65   6f 66 4c 69 66 65 00 56    ..\.GameofLife.V
							  44 32 44 42 65 74 61 57   65 73 74 2c 4c 69 66 65    D2DBetaWest,Life
							  6c 69 6b 65 2c 87 80 01   01 01 01 01 ff ff ff 01    like,...........
							  01 ff ff ff ff ff ff 03   ff ff ff ff ff ff ff ff    ................
							  ff ff ff ff ff ff ff ff   01 80 80 ff ff ff 80 80    ................
							  00 47 61 6d 65 6f 66 4c   69 66 65 00 ff 0f 3c 00    .GameofLife...<.
							  07 00 00 00 21 00 00 00   7d 00 00 00 00 00 00 00    ....!...}.......
							  d8 94 f6 08 55 9e 77 02   47 61 6d 65 6f 66 4c 69    ....U.w.GameofLi
							  66 65 00 44 69 61 62 6c   6f 20 49 49 20 42 65 74    fe.Diablo II Bet
							  61 57 65 73 74 2d 31 00   ff 0f 68 00 01 00 00 00    aWest-1...h.....
							  00 00 00 00 10 00 00 00   00 00 00 00 d8 94 f6 08    ................
							  b1 65 77 02 65 76 69 6c   67 72 75 73 73 6c 65 72    .ew.evilgrussler
							  00 56 44 32 44 42 65 74   61 57 65 73 74 2c 74 61    .VD2DBetaWest,ta
							  72 61 6e 2c 83 80 ff ff   ff ff ff 2f ff ff ff ff    ran,......./....
							  ff ff ff ff ff ff 03 ff   ff ff ff ff ff ff ff ff    ................
							  ff ff ff ff ff ff ff 07   80 80 80 80 ff ff ff 00    ................

							  ff 0a 59 00 47 61 6d 65   6f 66 4c 69 66 65 00 56    ..Y.GameofLife.V
							  44 32 44 42 65 74 61 57   65 73 74 2c 42 4e 45 54    D2DBetaWest,BNET
							  44 2c 87 80 01 01 01 01   01 ff ff ff 01 01 ff ff    D,..............
							  ff ff ff ff 02 ff ff ff   ff ff ff ff ff ff ff ff    ................
							  ff ff ff ff ff 01 80 80   ff ff ff 80 80 00 47 61    ..............Ga
							  6d 65 6f 66 4c 69 66 65   00                         meofLife.
							  */
#define SERVER_PLAYERINFOREPLY 0x0aff
	typedef struct
	{
		t_bnet_header h;
		/* player name */
		/* status */
		/* player name?! (maybe character name?) */
	} PACKED_ATTR() t_server_playerinforeply;
	/*
	 * status string:
	 *
	 * for STAR, SEXP, SSHR < 1.10:
	 * "%s %u %u %u %u %u"
	 *  client tag (RATS, PXES, RHSS)
	 *  rating
	 *  number (ladder rank)
	 *  stars  (normal wins)
	 *  unknown3 (always zero?)
	 *  unknown4 (always zero?) FIXME: I don't see this last one in any dumps...
	 is this only a SEXP thing?

	 * for STAR, SEXP, SSHR >= 1.10:
	 * "%s %u %u %u %u %u %u %u %u %u %s"
	 *  client tag (RATS, PXES, RHSS)
	 *  rating
	 *  number (ladder rank)
	 *  stars  (normal wins)
	 *  spawned (1 of spawned, 0 otherwise)
	 *  unknown4 (always zero?)
	 *  highest ladder rating
	 *  unknown6 (always zero?)
	 *  unknown7 (always zero?)
	 *  icon tag (usually client tag)


	 *
	 * for DRTL:
	 * "%s %u %u %u %u %u %u %u %u %u"
	 *  client tag (LTRD)
	 *  level
	 *  class (0==warrior, 1==rogue, 2==sorcerer)
	 *  dots (times killed diablo)
	 *  strength
	 *  magic
	 *  dexterity
	 *  vitality
	 *  gold
	 *  unknown2 (always zero?)

	 *
	 * for D2DV:
	 * "%s%s,%s,"
	 * client tag (VD2D)
	 * realm
	 * character name
	 * 43 unknown bytes
	 */
#define PLAYERINFO_DRTL_CLASS_WARRIOR  0
#define PLAYERINFO_DRTL_CLASS_ROGUE    1
#define PLAYERINFO_DRTL_CLASS_SORCERER 2
	/******************************************************/


	/******************************************************/
#define CLIENT_PROGIDENT2 0x0bff
	typedef struct
	{
		t_bnet_header h;
		bn_int        clienttag;
	} PACKED_ATTR() t_client_progident2;
	/******************************************************/


	/******************************************************/
#define CLIENT_JOINCHANNEL 0x0cff
	typedef struct
	{
		t_bnet_header h;
		bn_int        channelflag;
	} PACKED_ATTR() t_client_joinchannel;
#define CLIENT_JOINCHANNEL_NORMAL  0x00000000
#define CLIENT_JOINCHANNEL_GENERIC 0x00000001
#define CLIENT_JOINCHANNEL_CREATE  0x00000002
	/******************************************************/


	/******************************************************/
#define SERVER_CHANNELLIST 0x0bff
	typedef struct
	{
		t_bnet_header h;
		/* channel names */
	} PACKED_ATTR() t_server_channellist;
	/******************************************************/


	/******************************************************/
	/*
	We don't use this for now. It makes the client put
	the list of IPs/hostnames into the registry.

	FF 04 8F 00 00 00 00 00   32 30 39 2E 36 37 2E 31    ........209.67.1
	33 36 2E 31 37 34 3B 32   30 37 2E 36 39 2E 31 39    36.174;207.69.19
	34 2E 32 31 30 3B 32 30   37 2E 36 39 2E 31 39 34    4.210;207.69.194
	2E 31 38 39 3B 32 31 36   2E 33 32 2E 37 33 2E 31    .189;216.32.73.1
	37 34 3B 32 30 39 2E 36   37 2E 31 33 36 2E 31 37    74;209.67.136.17
	31 3B 32 30 36 2E 37 39   2E 32 35 34 2E 31 39 32    1;206.79.254.192
	3B 32 30 37 2E 31 33 38   2E 33 34 2E 33 3B 32 30    ;207.138.34.3;20
	39 2E 36 37 2E 31 33 36   2E 31 37 32 3B 65 78 6F    9.67.136.172;exo
	64 75 73 2E 62 61 74 74   6C 65 2E 6E 65 74 00       dus.battle.net.
	*/
#define SERVER_SERVERLIST 0x04ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        unknown1; /* 00 00 00 00 */
		/* list */
	} PACKED_ATTR() t_server_serverlist;
#define SERVER_SERVERLIST_UNKNOWN1 0x00000000
	/******************************************************/


	/******************************************************/
	/*
	FF 0F 30 00 01 00 00 00   00 00 00 00 00 00 00 00    ..0.............
	00 00 00 00 00 00 00 00   00 00 00 00 52 6F 73 73    ............Ross
	00 52 41 54 53 20 30 20   30 20 30 20 30 20 30 00    .RATS 0 0 0 0 0.

	FF 0F 38 00 07 00 00 00   21 00 00 00 64 00 00 00    ..8.....!...d...
	00 00 00 00 D8 94 F6 07   B3 2C 6E 02 4D 6F 4E 6B    .........,n.MoNk
	32 6B 00 44 69 61 62 6C   6F 20 49 49 20 42 65 74    2k.Diablo II Bet
	61 57 65 73 74 2D 31 00                              aWest-1.

	MT_ADD:
	0x0000: ff 0a 53 00 4d 6f 4e 6b   32 6b 00 56 44 32 44 42    ..S.MoNk2k.VD2DB
	0x0010: 65 74 61 57 65 73 74 2c   4d 6f 4e 6b 2d 65 65 2c    etaWest,MoNk-ee,
	0x0020: 83 80 06 01 01 01 01 ff   4c ff 02 02 ff ff ff ff    ........L.......
	0x0030: ff ff 01 ff 48 48 48 48   ff a6 ff 48 48 ff ff ff    ....HHHH...HH...
	0x0040: ff ff ff 10 80 80 80 80   ff ff ff 00 4d 6f 4e 6b    ............MoNk
	0x0050: 32 6b 00 ff 0f 38 00 07   00 00 00 21 00 00 00 6d    2k...8.....!...m
	0x0060: 00 00 00 00 00 00 00 d8   94 f6 08 a4 46 6e 02 4d    ............Fn.M
	0x0070: 6f 4e 6b 32 6b 00 44 69   61 62 6c 6f 20 49 49 20    oNk2k.Diablo II
	0x0080: 42 65 74 61 57 65 73 74   2d 31 00 ff 0f 62 00 01    BetaWest-1...b..
	0x0090: 00 00 00 00 00 00 00 28   00 00 00 00 00 00 00 d8    .......(........
	0x00a0: 94 f6 07 69 fb 6d 02 4e   6f 72 62 62 6f 00 56 44    ...i.m.Norbbo.VD
	0x00b0: 32 44 42 65 74 61 57 65   73 74 2c 44 6f 6f 73 68    2DBetaWest,Doosh
	0x00c0: 2c 87 80 05 02 01 01 01   2b ff 1b 02 02 ff ff ff    ,.......+.......
	0x00d0: ff ff ff 03 ff ff ff ff   ff ff ff ff ff ff ff ff    ................
	0x00e0: ff ff ff ff 0a 80 80 80   80 ff ff ff 00 ff 0f 65    ...............e
	0x00f0: 00 01 00 00 00 00 00 00   00 32 00 00 00 00 00 00    .........2......
	0x0100: 00 d8 94 f6 07 23 00 6e   02 6e 6a 67 6f 61 6c 69    .....#.n.njgoali
	0x0110: 65 00 56 44 32 44 42 65   74 61 57 65 73 74 2c 73    e.VD2DBetaWest,s
	0x0120: 68 65 69 6b 61 2c 83 80   05 02 01 02 02 ff 4c ff    heika,........L.
	0x0130: 02 02 ff ff ff ff ff ff   02 ff ff ff ff ff ff ff    ................
	0x0140: ff ff ff ff ff ff ff ff   ff 09 80 80 80 80 ff ff    ................
	0x0150: ff 00 ff 0f 67 00 01 00   00 00 00 00 00 00 1f 00    ....g...........
	0x0160: 00 00 00 00 00 00 d8 94   f6 09 eb 24 6e 02 72 6f    ...........$n.ro
	0x0170: 62 6d 6d 73 64 00 56 44   32 44 42 65 74 61 57 65    bmmsd.VD2DBetaWe
	0x0180: 73 74 2c 56 61 6e 63 6f   75 76 65 72 2c 83 80 05    st,Vancouver,...
	0x0190: 02 02 01 01 30 ff 1b 02   02 ff ff ff ff ff ff 04    ....0...........
	0x01a0: 4f ff ff ff ff ff ff a9   ff ff ff ff ff ff ff ff    O...............
	0x01b0: 09 80 80 80 80 ff ff ff   00 ff 0f 62 00 01 00 00    ...........b....
	0x01c0: 00 00 00 00 00 6e 00 00   00 00 00 00 00 ce 4f fe    .....n........O.
	0x01d0: c0 f9 02 15 01 4c 79 63   74 68 69 73 00 56 44 32    .....Lycthis.VD2
	0x01e0: 44 42 65 74 61 57 65 73   74 2c 45 6c 6c 65 2c 83    DBetaWest,Elle,.
	0x01f0: 80 04 02 01 01 01 ff 4d   ff 02 02 ff ff ff ff ff    .......M........
	0x0200: ff 01 ff ff ff ff ff ff   ff ff ff ff ff ff ff ff    ................
	0x0210: ff ff 06 80 80 80 80 ff   ff ff 00 ff 0f 6b 00 01    .............k..
	0x0220: 00 00 00 00 00 00 00 19   01 00 00 00 00 00 00 ce    ................
	0x0230: 4f fe c1 e2 1b 9c 00 52   6f 62 4d 69 74 63 68 65    O......RobMitche
	0x0240: 6c 6c 00 56 44 32 44 42   65 74 61 57 65 73 74 2c    ll.VD2DBetaWest,
	0x0250: 53 6f 6f 6e 65 72 64 65   64 2c 83 80 06 02 02 01    Soonerded,......
	0x0260: 01 46 46 ff 02 02 ff ff   ff ff ff ff 05 ff ff ff    .FF.............
	0x0270: ff ff ff 29 ff ff ff ff   ff ff ff ff ff 11 80 82    ...)............
	0x0280: 80 80 ff ff ff 00 ff 0f   66 00 01 00 00 00 00 00    ........f.......
	0x0290: 00 00 bc 00 00 00 00 00   00 00 d8 94 f6 09 8f 3f    ...............?
	0x02a0: 6e 02 4d 61 72 6c 6f 63   6b 31 00 56 44 32 44 42    n.Marlock1.VD2DB
	0x02b0: 65 74 61 57 65 73 74 2c   6d 61 72 6c 6f 63 6b 2c    etaWest,marlock,
	0x02c0: 83 80 ff ff ff ff ff ff   ff ff ff ff ff ff ff ff    ................
	0x02d0: ff ff 05 ff ff ff ff ff   ff ff ff ff ff ff ff ff    ................
	0x02e0: ff ff ff 01 80 80 80 80   ff ff ff 00 ff 0f 63 00    ..............c.
	0x02f0: 01 00 00 00 00 00 00 00   c8 00 00 00 00 00 00 00    ................
	0x0300: d1 43 88 aa bd ce 3c 00   42 2d 57 61 74 74 7a 00    .C....<.B-Wattz.

	From bnetd-0.4.23pre18 to Diablo II 1.03
	FF 0F 69 00 09 00  00 00 00 00 00 00 12 00   I@..i... ........
	00 00 00 00 00 00 00 00  00 00 00 00 00 00 45 6C   ........ ......El
	66 6C 6F 72 64 00 56 44  32 44 51 61 72 61 74 68   flord.VD 2DQarath
	52 65 61 6C 6D 2C 46 61  6B 65 43 68 61 72 2C 83   Realm,Fa keChar,.
	80 FF FF FF FF FF 2F FF  FF FF FF FF FF FF FF FF   ....../. ........
	FF 03 FF FF FF FF FF FF  FF FF FF FF FF FF FF FF   ........ ........
	FF FF 07 80 80 80 80 FF  FF FF 00                  ........ ...
	*/
#define SERVER_MESSAGE 0x0fff
	typedef struct
	{
		t_bnet_header h;
		bn_int        type;
		bn_int        flags;     /* player flags (or channel flags for MT_CHANNEL) */
		bn_int        latency;
		bn_int        player_ip;  /* always zero? */
		bn_int        account_num; /* player's IP (big endian), no longer used, always 0D F0 AD BA */
		bn_int        reg_auth;  /* server ip and/or reg auth? CD key and/or account number? */
		/* player name */
		/* text */
	} PACKED_ATTR() t_server_message;
#define SERVER_MESSAGE_PLAYER_IP_DUMMY 0x00000000
	/* nok */
#define SERVER_MESSAGE_REG_AUTH 0xBAADF00D /* 0D F0 AD BA */
	/* #define SERVER_MESSAGE_REG_AUTH 0x07f694d8 */
#define SERVER_MESSAGE_ACCOUNT_NUM 0x0df0adba
	/* For MT_ADD, MT_JOIN, the text portion looks like:
	 *
	 * for STAR, SEXP, SSHR:
	 * "%4c %u %u %u %u %u"
	 *  client tag (RATS, PXES, RHSS)
	 *  rating
	 *  number (ladder rank)
	 *  stars  (normal wins)
	 *  unknown3 (always zero?)
	 *  unknown4 (always zero?)
	 *
	 * for DRTL:
	 * "%4c %u %u %u %u %u %u %u %u %u"
	 *  client tag (LTRD) FIXME: RHSD?
	 *  level
	 *  class (0==warrior, 1==rogue, 2==sorcerer)
	 *  dots (times killed diablo)
	 *  strength
	 *  magic
	 *  dexterity
	 *  vitality
	 *  gold
	 *  unknown2 (always zero?)
	 *
	 * for CHAT:
	 * FIXME: ???  "%4c"
	 * client tag (TAHC)
	 *
	 * FIXME: Warcraft II?
	 *
	 * for D2DV:
	 * open:
	 * "%4c"
	 * client tag (VD2D)
	 * closed:
	 * "%4c%s,%s,%s"
	 * client tag (VD2D)
	 * realm name
	 * character name
	 * character info
	 */
#define SERVER_MESSAGE_TYPE_ADDUSER             0x00000001 /* ADD,USER,SHOWUSER */
#define SERVER_MESSAGE_TYPE_JOIN                0x00000002
#define SERVER_MESSAGE_TYPE_PART                0x00000003 /* LEAVE */
#define SERVER_MESSAGE_TYPE_WHISPER             0x00000004
#define SERVER_MESSAGE_TYPE_TALK                0x00000005 /* MESSAGE */
#define SERVER_MESSAGE_TYPE_BROADCAST           0x00000006
#define SERVER_MESSAGE_TYPE_CHANNEL             0x00000007 /* JOINING */
	/* unused?                                      0x00000008 */
#define SERVER_MESSAGE_TYPE_USERFLAGS           0x00000009
#define SERVER_MESSAGE_TYPE_WHISPERACK          0x0000000a /* WHISPERSENT */
	/* unused?                                      0x0000000b */
	/* unused?                                      0x0000000c */
#define SERVER_MESSAGE_TYPE_CHANNELFULL         0x0000000d
#define SERVER_MESSAGE_TYPE_CHANNELDOESNOTEXIST 0x0000000e
#define SERVER_MESSAGE_TYPE_CHANNELRESTRICTED   0x0000000f
	/* unused?                                      0x00000010 */
	/* unused?                                      0x00000011 */
#define SERVER_MESSAGE_TYPE_INFO                0x00000012
#define SERVER_MESSAGE_TYPE_ERROR               0x00000013
	/* unused?                                      0x00000014 */
	/* unused?                                      0x00000015 */
	/* unused?                                      0x00000016 */
#define SERVER_MESSAGE_TYPE_EMOTE               0x00000017

	/****** Player Flags ******/
	/* flag bits for above struct */

	/* ADDED BY UNDYING SOULZZ 4/7/02 */
#define W3_ICON_SET					0x00000000
#define MAX_STR_RACELEN				20
#define MAX_STR_ACCTPASSLEN         10
#define W3_RACE_RANDOM				32
#define W3_RACE_HUMANS				1
#define W3_RACE_ORCS				2
#define W3_RACE_UNDEAD				8
#define W3_RACE_NIGHTELVES			4
#define W3_RACE_DEMONS				16

#define W3_ICON_RANDOM				0 /* - Although when client presses random in PG and it sends "32" its "0" for icon */
#define W3_ICON_HUMANS				1
#define W3_ICON_ORCS				2
#define W3_ICON_UNDEAD				3 /* - Although when client presses undead in PG and it sends "8" its "3" for icon */
#define W3_ICON_NIGHTELVES			4
#define W3_ICON_DEMONS				5

	/* Icon setup  3RAW then <accounts level> <Race> <race wins> */
	/* Races: 1 = human, 2 = orc, 8 = undead, 4 = nightelf  32 = random */
	/* Misc icons are 6-9 * - There might be some icons not defined*/
	/* If you find them let us know pse,  forums.cheatlist.com in War3 Hacking/Development */

	/*Human Icons*/
#define W3_ICON_HUMAN_FOOTMAN					"3RAW 1 1 11"
#define W3_ICON_HUMAN_KNIGHT					"3RAW 1 1 100"
#define W3_ICON_HUMAN_ARCHMAGE					"3RAW 1 1 250"
#define W3_ICON_HUMAN_HERO						"3RAW 1 1 1000"

	/*Orc Icons*/
#define W3_ICON_ORC_PEON						"3RAW 1 2 00" /* default icon , unless u change it in code*/
#define W3_ICON_ORC_GRUNT						"3RAW 1 2 10"
#define W3_ICON_ORC_TAUREN						"3RAW 1 2 100"
#define W3_ICON_ORC_FARSEER						"3RAW 1 2 250"
#define W3_ICON_ORC_HERO						"3RAW 1 2 1000"

	/*Undead Icons*/
#define W3_ICON_UNDEAD_GHOUL					"3RAW 1 3 10"
#define W3_ICON_UNDEAD_ABOM						"3RAW 1 3 100"
#define W3_ICON_UNDEAD_LICH						"3RAW 1 3 250"
#define W3_ICON_UNDEAD_HERO						"3RAW 1 3 1000"

	/*Night Elf Icons*/
#define W3_ICON_ELF_ARCHER						"3RAW 1 4 10"
#define W3_ICON_ELF_DRUIDCLAW					"3RAW 1 4 100"
#define W3_ICON_ELF_PRIESMOON					"3RAW 1 4 250"
#define W3_ICON_ELF_HERO						"3RAW 1 4 1000"

	/*Random Icons , like NPC's, Creeps*/
#define W3_ICON_RANDOM_GREENDRAGON				"3RAW 1 9 10"
#define W3_ICON_RANDOM_BLACKDRAGON				"3RAW 1 9 100"
#define W3_ICON_RANDOM_REDDRAGON				"3RAW 1 9 250"
#define W3_ICON_RANDOM_BLUEDRAGON				"3RAW 1 9 1000"

	/* End of Undying Edits */

	/*      Blizzard Entertainment employee */
#define MF_BLIZZARD 0x00000001 /* blue Blizzard logo */
	/*      Channel operator */
#define MF_GAVEL    0x00000002 /* gavel */
	/*      Speaker in moderated channel */
#define MF_VOICE    0x00000004 /* megaphone */
	/*      System operator */
#define MF_BNET     0x00000008 /* (old: blue Blizzard, new: green b.net) or red BNETD logo */
	/*      Chat bot or other user without UDP support */
#define MF_PLUG     0x00000010 /* tiny plug to right of icon, no UDP */
	/*      Squelched/Ignored user */
#define MF_X        0x00000020 /* big red X */
	/*      Special guest of Blizzard Entertainment */
#define MF_SHADES   0x00000040 /* sunglasses */
	/* unused           0x00000080 */
	/*      Use BEL character in error codes. Some bots use it as a flag. Battle.net */
	/*      stopped supporting it recently. */
#define MF_BEEP     0x00000100 /* no change in icon */
	/*      Registered Professional Gamers League player */
#define MF_PGLPLAY  0x00000200 /* PGL player logo */
	/*      Registered Professional Gamers League official */
#define MF_PGLOFFL  0x00000400 /* PGL official logo */
	/*      Registered KBK player */
#define MF_KBKPLAY  0x00000800 /* KBK player logo */
	/*      Official KBK Referee */
#define MF_KBKREF   0x00001000 /* KBK referee logo */ /* FIXME: this number may be wrong */
	/* unused... FIXME: how many bits work? 16 or 32? */

	/****** Channel Flags ******/
	/* flag bits for MT_CHANNEL message */
#define CF_PUBLIC     0x00000001 /* public channel */
#define CF_MODERATED  0x00000002 /* moderated channel */
#define CF_RESTRICTED 0x00000004 /* ? restricted channel ? */
#define CF_THEVOID    0x00000008 /* "The Void" */
#define CF_SYSTEM     0x00000020 /* system channel */
#define CF_OFFICIAL   0x00001000 /* official channel */
	/*
	 * Examples:
	 * 0x00001003 Blizzard Tech Support
	 * 0x00001001 Open Tech Support
	 * 0x00000021 Diablo II USA-1  or  War2BNE USA-1
	 * 0x0000000D warez
	 * 0x00000009 The Void
	 * 0x00000001 War2 Ladder Challenges  or  Diablo II PvP
	 * 0x00000000 clan randomchannel
	 * 0x00000000 randomchannel
	 */
	/******************************************************/


	/******************************************************/
#define CLIENT_MESSAGE 0x0eff
	typedef struct
	{
		t_bnet_header h;
		/* text */
	} PACKED_ATTR() t_client_message;
	/******************************************************/


	/******************************************************/
	/*
	FF 09 17 00 03 00 00 00   FF FF 00 00 00 00 00 00    ................
	19 00 00 00 00 00 00                                 .......

	FF 09 24 00 00 00 00 00   00 00 00 00 00 00 00 00    ..$.............
	01 00 00 00 4C 61 64 64   65 72 20 31 20 6F 6E 20    ....Ladder 1 on
	31 00 00 00                                          1...
	*/
#define CLIENT_GAMELISTREQ 0x09ff
	typedef struct
	{
		t_bnet_header h;
		bn_short      gametype;
		bn_short      unknown1;
		bn_int        unknown2;
		bn_int        unknown3;
		bn_int        maxgames;
		/* game name */
	} PACKED_ATTR() t_client_gamelistreq;
#define CLIENT_GAMELISTREQ_ALL       0x0000
#define CLIENT_GAMELISTREQ_MELEE     0x0002
#define CLIENT_GAMELISTREQ_FFA       0x0003
#define CLIENT_GAMELISTREQ_ONEONONE  0x0004
#define CLIENT_GAMELISTREQ_CTF       0x0005
#define CLIENT_GAMELISTREQ_GREED     0x0006
#define CLIENT_GAMELISTREQ_SLAUGHTER 0x0007
#define CLIENT_GAMELISTREQ_SDEATH    0x0008
#define CLIENT_GAMELISTREQ_LADDER    0x0009
#define CLIENT_GAMELISTREQ_IRONMAN   0x0010
#define CLIENT_GAMELISTREQ_MAPSET    0x000a
#define CLIENT_GAMELISTREQ_TEAMMELEE 0x000b
#define CLIENT_GAMELISTREQ_TEAMFFA   0x000c
#define CLIENT_GAMELISTREQ_TEAMCTF   0x000d
#define CLIENT_GAMELISTREQ_PGL       0x000e
#define CLIENT_GAMELISTREQ_TOPVBOT   0x000f
#define CLIENT_GAMELISTREQ_DIABLO    0x0409 /* FIXME: this should be the langid */
#define CLIENT_GAMELISTREQ_LOADED    0x0a00
	/* FIXME: Diablo reports differently than it is listed in GAMELIST */
#define CLIENT_GAMETYPE_DIABLO_0     0x00000000 /* Level 1 Char */
#define CLIENT_GAMETYPE_DIABLO_1     0x00000001 /* Level 2 Char */
#define CLIENT_GAMETYPE_DIABLO_2     0x00000002 /* Level ? Char */
#define CLIENT_GAMETYPE_DIABLO_3     0x00000003 /* Level 4 Char */
#define CLIENT_GAMETYPE_DIABLO_4     0x00000004 /* Level ? Char */
#define CLIENT_GAMETYPE_DIABLO_5     0x00000005 /* Level ? Char */
#define CLIENT_GAMETYPE_DIABLO_6     0x00000006 /* Level ? Char */
#define CLIENT_GAMETYPE_DIABLO_7     0x00000007 /* Level ? Char */
#define CLIENT_GAMETYPE_DIABLO_8     0x00000008 /* Level 20-24 Char */
#define CLIENT_GAMETYPE_DIABLO_9     0x00000009 /* Level 25-? Char */
#define CLIENT_GAMETYPE_DIABLO_a     0x0000000a /* Level ? Char */
#define CLIENT_GAMETYPE_DIABLO_b     0x0000000b /* Level ? Char */
#define CLIENT_GAMETYPE_DIABLO_c     0x0000000c /* Level ? Char */
#define CLIENT_GAMETYPE_DIABLO_d     0x0000000d /* Level ? Char */
	/* list might continue - what is maximum diablo level ? */
	/* FIXME: Not sure how Diablo II does things yet */
#define CLIENT_GAMETYPE_DIABLO2_CLOSE 		0x00000000 /* close game */
#define CLIENT_GAMETYPE_DIABLO2_OPEN_NORMAL	0X00000008 /* open, normal difficulty */
#define CLIENT_GAMETYPE_DIABLO2_OPEN_NIGHTMARE	0X00000009 /* open, nightmare difficulty */
#define CLIENT_GAMETYPE_DIABLO2_OPEN_HELL	0X0000000a /* open, hell difficulty */

	/******************************************************/


	/******************************************************/
	/*
	FF 09 35 00 01 00 00 00   00 00 00 00 03 00 01 00    ..5.............
	00 00 00 00 02 00 17 E0   80 7B 4F 0D 00 00 00 00    .........{O.....
	00 00 00 00 04 00 00 00   64 00 00 00 4D 79 47 61    ........d...MyGa
	6D 65 00 00 00                                       me...

	FF 09 5B 00 01 00 00 00   00 00 00 00 03 00 01 00    ..[.............
	02 00 17 E0 80 7B 4F 0D   00 00 00 00 00 00 00 00    .....{O.........
	04 00 00 00 2B 00 00 00   47 61 6D 65 00 50 61 73    ....+...Game.Pas
	73 00 2C 33 34 2C 31 32   2C 35 2C 31 2C 33 2C 31    s.,34,12,5,1,3,1
	2C 63 63 63 33 36 34 30   36 2C 2C 42 6F 62 0D 43    ,ccc36406,,Bob.C
	68 61 6C 6C 65 6E 67 65   72 0D 00                   hallenger..

	FF 09 D4 03 0A 00 00 00   0C 00 00 00 09 04 00 00    ................
	02 00 17 E0 CD E8 B5 E1   00 00 00 00 00 00 00 00    ................
	00 00 00 00 3C 00 00 00   4A 65 73 73 65 27 73 20    ....<...Jesse's
	57 6F 72 6C 64 00 00 32   0D 4C 69 7A 7A 69 65 2E    World..2.Lizzie.
	42 6F 72 64 65 6E 0D 4C   54 52 44 20 34 30 20 31    Borden.LTRD 40 1
	20 33 20 31 32 31 20 31   32 36 20 33 30 36 20 31     3 121 126 306 1
	33 36 20 35 34 37 30 32   20 30 00                   ...

	FF 09 70 00 01 00 00 00   0F 00 04 00 09 04 00 00    ..p.............
	02 00 17 E0 C6 0B 13 3C   00 00 00 00 00 00 00 00    .......<........
	04 00 00 00 C5 00 00 00   4C 61 64 64 65 72 20 31    ........Ladder 1
	20 6F 6E 20 31 00 00 2C   2C 2C 36 2C 32 2C 66 2C     on 1..,,,6,2,f,
	34 2C 66 63 63 35 38 65   34 61 2C 37 32 30 30 2C    4,fcc58e4a,7200,
	49 63 65 36 39 62 75 72   67 0D 46 6F 72 65 73 74    Ice69burg.Forest
	20 54 72 61 69 6C 20 42   4E 45 2E 70 75 64 0D 00     Trail BNE.pud..

	# war3
	# 66 packet from server: type=0x09ff(SERVER_GAMELISTREPLY) length=131 class=bnet
	0000:   FF 09 83 00 01 00 00 00   01 00 00 00 09 04 00 00    ................
	0010:   02 00 17 E0 18 CF BF 9B   00 00 00 00 00 00 00 00    ................
	0020:   10 00 00 00 0F 00 00 00   33 20 6F 6E 20 33 20 64    ........3 on 3 d
	0030:   61 72 6B 20 66 6F 72 65   73 74 00 00 35 31 30 30    ark forest..5100
	0040:   30 30 30 30 30 01 03 01   01 81 01 81 01 73 27 25    00000........s'%
	0050:   15 29 4D 61 71 53 73 5D   63 65 75 61 5D A9 29 37    .)MaqSs]ceua].)7
	0060:   29 45 61 73 6B 69 21 47   6F 73 65 73 75 DD 2F 77    )Easki!Gosesu./w
	0070:   33 6D 01 4B 61 D7 69 73   69 69 6F 5B 53 07 4B 5D    3m.Ka.isiio[S.K]
	0080:   01 01 00                                             ...

	[23:32] <@nok-> 0000:   FF 09 E9 01 04 00 00 00   01 00 00 00 09 04 00 00    ................
	[23:32] <@nok-> 0010:   02 00 17 E0 40 69 1B 07   00 00 00 00 00 00 00 00    ....@i..........

	0000:   FF 09 7A 01 03 00 00 00   01 00 00 00 09 04 00 00    ..z.............
	0010:   02 00 17 E0 18 2C 7E 7B   00 00 00 00 00 00 00 00    .....,~{........
	0020:   10 00 00 00 09 00 00 00   34 20 6F 6E 20 34 20 4D    ........4 on 4 M
	0030:   69 73 74 00 00 37 31 30   30 30 30 30 30 30 01 03    ist..710000000..
	0040:   01 01 89 01 89 01 75 4D   7B 27 A1 4D 61 71 53 73    ......uM{'.MaqSs
	0050:   5D 63 65 75 61 5D B9 29   39 29 47 6F 6D 65 17 6D    ]ceua].)9)Gome.m
	0060:   73 21 69 6F 21 75 75 69   65 21 4D 69 73 75 1D 2F    s!io!uuie!Misu./
	0070:   77 33 6D 01 51 73 BB 69   6F 63 65 2D 4D 75 17 63    w3m.Qs.ioce-Mu.c
	0080:   69 67 65 73 01 01 00 01   00 00 00 09 04 00 00 02    iges............

	*/
#define SERVER_GAMELISTREPLY 0x09ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        gamecount;
		bn_int	  sstatus; /* when reply with error to specific game */
		/* games */
	} PACKED_ATTR() t_server_gamelistreply;

#define SERVER_GAMELISTREPLY_GAME_SSTATUS_NOTFOUND	0x0 /* but also any other value diff from the ones bellow */
#define SERVER_GAMELISTREPLY_GAME_SSTATUS_PASS		0x2 /* password incorrect */
#define SERVER_GAMELISTREPLY_GAME_SSTATUS_FULL		0x3 /* game full */
#define SERVER_GAMELISTREPLY_GAME_SSTATUS_STARTED	0x4 /* game started */
#define SERVER_GAMELISTREPLY_GAME_SSTATUS_NOSPAWNCDKEY	0x5 /* trying to use a spawn install join invalid cdkey creator game */
#define SERVER_GAMELISTREPLY_GAME_SSTATUS_LOADED        0x0a00 /* Loaded game */

	typedef struct
	{
		/*	if yak doesn't like this... then the client doesn't also =) (bbf)
			bn_int   unknown7; */
		bn_short gametype;
		bn_short unknown1; /* langid under Diablo... */
		bn_short unknown3;
		/*  bn_int   deleted; */ /* they changed the structure at one point */
		bn_short port;     /* big endian byte order... at least they are consistent! */
		bn_int   game_ip;  /* big endian byte order */
		bn_int   unknown4;
		bn_int   unknown5; /* FIXME: got to figure out where latency is */
		bn_int   status;
		bn_int   unknown6;
		/* game name */
		/* clear password */
		/* info */
	} PACKED_ATTR() t_server_gamelistreply_game;
#define SERVER_GAMELISTREPLY_GAME_UNKNOWN7       0x00000000 //0x00000409 // 0x0000000c
#define SERVER_GAMELISTREPLY_GAME_UNKNOWN1           0x0001 //0x0000 //0x0001 // 0x0000
#define SERVER_GAMELISTREPLY_GAME_UNKNOWN3           0x0002
#define SERVER_GAMELISTREPLY_GAME_UNKNOWN4       0x00000000
#define SERVER_GAMELISTREPLY_GAME_UNKNOWN5       0x00000000
#define SERVER_GAMELISTREPLY_GAME_STATUS_OPEN    0x00000004
#define SERVER_GAMELISTREPLY_GAME_STATUS_FULL    0x00000006
#define SERVER_GAMELISTREPLY_GAME_STATUS_STARTED 0x0000000e
#define SERVER_GAMELISTREPLY_GAME_STATUS_DONE    0x0000000c
#define SERVER_GAMELISTREPLY_GAME_UNKNOWN6       0x0000002b /* latency? */

#define SERVER_GAMELISTREPLY_TYPE_DIABLO2_OPEN 		0x0704 /* open game */
	/******************************************************/


	/******************************************************/
#define CLIENT_STARTGAME1 0x08ff /* original starcraft or shareware (1.01) */
	typedef struct
	{
		t_bnet_header h;
		bn_int        status;
		bn_int        unknown3;
		bn_short      gametype;
		bn_short      unknown1;
		bn_int        unknown4;
		bn_int        unknown5;
		/* game name */
		/* game password */
		/* game info */
	} PACKED_ATTR() t_client_startgame1;
	/* I have also seen 1,5,7,f */
#define CLIENT_STARTGAME1_STATUSMASK     0x0000000f
#define CLIENT_STARTGAME1_STATUS_OPEN    0x00000004
#define CLIENT_STARTGAME1_STATUS_FULL    0x00000006
#define CLIENT_STARTGAME1_STATUS_STARTED 0x0000000e
#define CLIENT_STARTGAME1_STATUS_DONE    0x0000000c
	/******************************************************/


	/******************************************************/
	/*
	FF 1B 14 00 02 00 17 E0   80 7B 3F 54 00 00 00 00    .........{?T....
	00 00 00 00                                          ....
	*/
#define CLIENT_UNKNOWN_1B 0x1bff
	typedef struct
	{
		t_bnet_header h;
		bn_short      unknown1; /* FIXME: This "2" is the same as in the game
					   listings. What do they mean? */
		bn_short      port;     /* big endian byte order */
		bn_int        ip;       /* big endian byte order */
		bn_int        unknown2;
		bn_int        unknown3;
	} PACKED_ATTR() t_client_unknown_1b;
#define CLIENT_UNKNOWN_1B_UNKNOWN1 0x0002
#define CLIENT_UNKNOWN_1B_UNKNOWN2 0x00000000
#define CLIENT_UNKNOWN_1B_UNKNOWN3 0x00000000
	/******************************************************/


	/******************************************************/
	/*
	FF 1A 4C 00 01 00 00 00   00 00 00 00 00 00 00 00    ..L.............
	0F 00 00 00 00 00 00 00   E0 17 00 00 61 6E 73 00    ............ans.
	65 6C 6D 6F 00 30 0D 77   61 72 72 69 6F 72 0D 4C    elmo.0.warrior.L
	54 52 44 20 31 20 30 20   30 20 33 30 20 31 30 20    TRD 1 0 0 30 10
	32 30 20 32 35 20 31 30   30 20 30 00                20 25 100 0.
	*/
#define CLIENT_STARTGAME3 0x1aff /* Starcraft 1.03, Diablo 1.07 */
	typedef struct
	{
		t_bnet_header h;
		bn_int        status;
		bn_int        unknown3;
		bn_short      gametype;
		bn_short      unknown1;
		bn_int        unknown6;
		bn_int        unknown4; /* port # under Diablo */
		bn_int        unknown5;
		/* game name */
		/* game password */
		/* game info */
	} PACKED_ATTR() t_client_startgame3;
#define CLIENT_STARTGAME3_STATUSMASK      0x0000000f
#define CLIENT_STARTGAME3_STATUS_OPEN1    0x00000001 /* used by Diablo */
#define CLIENT_STARTGAME3_STATUS_OPEN     0x00000004
#define CLIENT_STARTGAME3_STATUS_FULL     0x00000006
#define CLIENT_STARTGAME3_STATUS_STARTED  0x0000000e
#define CLIENT_STARTGAME3_STATUS_DONE     0x0000000c
	/******************************************************/


	/******************************************************/
	/*
	FF 1C 49 00 00 00 00 00   00 00 00 00 03 00 01 00    ..I.............
	00 00 00 00 00 00 00 00   74 65 61 6D 6D 65 6C 65    ........teammele
	65 00 00 2C 2C 2C 2C 31   2C 33 2C 31 2C 33 65 33    e..,,,,1,3,1,3e3
	37 61 38 34 63 2C 37 2C   61 6E 73 65 6C 6D 6F 0D    7a84c,7,anselmo.
	4F 63 74 6F 70 75 73 0D   00                         Octopus..

	Brood War 1.04 ladder, disconnect==loss
	FF 1C 45 00 10 00 00 00   00 00 00 00 09 00 02 00    ..E.............
	00 00 00 00 01 00 00 00   54 45 53 54 00 00 2C 34    ........TEST..,4
	34 2C 31 34 2C 2C 32 2C   39 2C 32 2C 33 65 33 37    4,14,,2,9,2,3e37
	61 38 34 63 2C 33 2C 52   6F 73 73 0D 41 73 68 72    a84c,3,Ross.Ashr
	69 67 6F 0D 00                                       igo..

	Brood War 1.04 ladder, disconnect==disconnect
	FF 1C 45 00 00 00 00 00   00 00 00 00 09 00 01 00    ..E.............
	00 00 00 00 01 00 00 00   54 45 53 54 00 00 2C 34    ........TEST..,4
	34 2C 31 34 2C 2C 32 2C   39 2C 31 2C 33 65 33 37    4,14,,2,9,1,3e37
	61 38 34 63 2C 33 2C 52   6F 73 73 0D 41 73 68 72    a84c,3,Ross.Ashr
	69 67 6F 0D 00                                       igo..

	Brood War 1.04 greed, minerals==10000
	FF 1C 4C 00 00 00 00 00   00 00 00 00 06 00 04 00    ..L.............
	00 00 00 00 00 00 00 00   74 65 73 74 31 30 30 30    ........test1000
	30 00 00 2C 33 34 2C 31   32 2C 2C 31 2C 36 2C 34    0..,34,12,,1,6,4
	2C 33 65 33 37 61 38 34   63 2C 2C 52 6F 73 73 0D    ,3e37a84c,,Ross.
	43 68 61 6C 6C 65 6E 67   65 72 0D 00                Challenger..

	FF 1C 4F 00                                          ..O.
	00 00 00 00 00 00 00 00   02 00 01 00 1F 00 00 00    ................
	00 00 00 00 74 65 73 74   00 00 2C 34 34 2C 31 34    ....test..,44,14
	2C 35 2C 32 2C 32 2C 31   2C 32 31 30 34 62 62 33    ,5,2,2,1,2104bb3
	36 2C 34 2C 48 6F 6D 65   72 0D 54 68 65 20 4C 6F    6,4,Homer.The.Lo
	73 74 20 54 65 6D 70 6C   65 0D 00                   st.Temple..

	Diablo II 1.03 (level diff 0)
	FF 1C 20 00 00 00   00 00 00 00 00 00 00 00      .. ... ........
	00 00 00 00 00 00 00 00   00 00 54 65 73 74 00 00    ........ ..Test..
	31 00                                                1.
	*/
#define CLIENT_STARTGAME4 0x1cff /* Brood War or newer Starcraft (1.04, 1.05) */
	typedef struct
	{
		t_bnet_header h;
		bn_short      status; /* 0x0001 - private war3 game */
		bn_short      flag;
		bn_int        unknown2; /* 00 00 00 00 */
		bn_short      gametype;
		bn_short      option;   /* 01 00 */
		bn_int        unknown4; /* 00 00 00 00 */
		bn_int        unknown5; /* 00 00 00 00 */
		/* game name */
		/* game password */
		/* game info */
	} PACKED_ATTR() t_client_startgame4;
#define CLIENT_STARTGAME4_UNKNOWN2		    0x00000000
#define CLIENT_STARTGAME4_STATUSMASK_16             0x000000ff
#define CLIENT_STARTGAME4_STATUSMASK_INIT_VALID     0x00000093
#define CLIENT_STARTGAME4_STATUSMASK_OPEN_VALID     0x0000009f
#define CLIENT_STARTGAME4_STATUS_INIT               0x00000000
#define CLIENT_STARTGAME4_STATUS_PRIVATE            0x00000001
#define CLIENT_STARTGAME4_STATUS_FULL               0x00000002
#define CLIENT_STARTGAME4_STATUS_OPEN               0x00000004
#define CLIENT_STARTGAME4_STATUS_START              0x00000008
#define CLIENT_STARTGAME4_STATUS_DISC_IS_LOSS       0x00000010
#define CLIENT_STARTGAME4_STATUS_REPLAY             0x00000080
#define CLIENT_STARTGAME4_OPTION_MELEE_NORMAL       0x0001
#define CLIENT_STARTGAME4_OPTION_FFA_NORMAL         0x0001
#define CLIENT_STARTGAME4_OPTION_ONEONONE_NORMAL    0x0001
#define CLIENT_STARTGAME4_OPTION_CTF_NORMAL         0x0001
#define CLIENT_STARTGAME4_OPTION_GREED_10000        0x0004
#define CLIENT_STARTGAME4_OPTION_GREED_7500         0x0003
#define CLIENT_STARTGAME4_OPTION_GREED_5000         0x0002
#define CLIENT_STARTGAME4_OPTION_GREED_2500         0x0001
#define CLIENT_STARTGAME4_OPTION_SLAUGHTER_60       0x0004
#define CLIENT_STARTGAME4_OPTION_SLAUGHTER_45       0x0003
#define CLIENT_STARTGAME4_OPTION_SLAUGHTER_30       0x0002
#define CLIENT_STARTGAME4_OPTION_SLAUGHTER_15       0x0001
#define CLIENT_STARTGAME4_OPTION_SDEATH_NORMAL      0x0001
#define CLIENT_STARTGAME4_OPTION_LADDER_COUNTASLOSS 0x0002
#define CLIENT_STARTGAME4_OPTION_LADDER_NOPENALTY   0x0001
#define CLIENT_STARTGAME4_OPTION_IRONMAN_           0x000 /* FIXME */
#define CLIENT_STARTGAME4_OPTION_MAPSET_NORMAL      0x0001
#define CLIENT_STARTGAME4_OPTION_TEAMMELEE_4        0x0003
#define CLIENT_STARTGAME4_OPTION_TEAMMELEE_3        0x0002
#define CLIENT_STARTGAME4_OPTION_TEAMMELEE_2        0x0001
#define CLIENT_STARTGAME4_OPTION_TEAMFFA_4          0x0003
#define CLIENT_STARTGAME4_OPTION_TEAMFFA_3          0x0002
#define CLIENT_STARTGAME4_OPTION_TEAMFFA_2          0x0001
#define CLIENT_STARTGAME4_OPTION_TEAMCTF_4          0x0003
#define CLIENT_STARTGAME4_OPTION_TEAMCTF_3          0x0002
#define CLIENT_STARTGAME4_OPTION_TEAMCTF_2          0x0001
#define CLIENT_STARTGAME4_OPTION_PGL_               0x000 /* FIXME */
#define CLIENT_STARTGAME4_OPTION_TOPVBOT_1          0x0001 /* 1 vs all [ 1x1, 1x2, 1x3 ...]         */
#define CLIENT_STARTGAME4_OPTION_TOPVBOT_2          0x0002 /* f.e. for (8) The Hunters.scm 1 vs all */
#define CLIENT_STARTGAME4_OPTION_TOPVBOT_3          0x0003 /*      means 1x7                        */
#define CLIENT_STARTGAME4_OPTION_TOPVBOT_4          0x0004 /* 4 vs all                              */
#define CLIENT_STARTGAME4_OPTION_TOPVBOT_5          0x0005
#define CLIENT_STARTGAME4_OPTION_TOPVBOT_6          0x0006
#define CLIENT_STARTGAME4_OPTION_TOPVBOT_7          0x0007
#define CLIENT_STARTGAME4_UNKNOWN4		    0x00000000
#define CLIENT_STARTGAME4_UNKNOWN5		    0x00000000
#define CLIENT_STARTGAME4_OPTION_NONE               0x000 /* FIXME */

#define CLIENT_STARTGAME4_FLAG_PRIVATE              0x0001
#define CLIENT_STARTGAME4_FLAG_PRIVATE_PASSWORD     "password"

#define CLIENT_MAPTYPE_SELFMADE     0
#define CLIENT_MAPTYPE_BLIZZARD     1
#define CLIENT_MAPTYPE_LADDER       2
#define CLIENT_MAPTYPE_PGL          3
#define CLIENT_MAPTYPE_KBK          4
#define CLIENT_MAPTYPE_CompUSA      5

	/* CLIENT_GAMESPEED_FAST is NULL for "fast" games, I convert it into 4 */
#define CLIENT_GAMESPEED_SLOWEST  0
#define CLIENT_GAMESPEED_SLOWER   1
#define CLIENT_GAMESPEED_SLOW     2
#define CLIENT_GAMESPEED_NORMAL   3
#define CLIENT_GAMESPEED_FAST     4
#define CLIENT_GAMESPEED_FASTER   5
#define CLIENT_GAMESPEED_FASTEST  6

	/* The tileset is NULL for BADLANDS, I'm using zero here */
#define CLIENT_TILESET_BADLANDS       0
#define CLIENT_TILESET_SPACE          1
#define CLIENT_TILESET_INSTALLATION   2
#define CLIENT_TILESET_ASHWORLD       3
#define CLIENT_TILESET_JUNGLE         4
#define CLIENT_TILESET_DESERT         5
#define CLIENT_TILESET_ICE            6
#define CLIENT_TILESET_TWILIGHT       7

	/* Diablo II Difficulty */
#define CLIENT_DIFFICULTY_NORMAL             1
#define CLIENT_DIFFICULTY_NIGHTMARE          2
#define CLIENT_DIFFICULTY_HELL               3 /* assumed */
#define CLIENT_DIFFICULTY_HARDCORE_NORMAL    4
#define CLIENT_DIFFICULTY_HARDCORE_NIGHTMARE 5 /* assumed */
#define CLIENT_DIFFICULTY_HARDCORE_HELL      6 /* assumed */
	/******************************************************/


	/******************************************************/
#define SERVER_STARTGAME1_ACK 0x08ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        reply;
	} PACKED_ATTR() t_server_startgame1_ack;
#define SERVER_STARTGAME1_ACK_NO 0x00000000
#define SERVER_STARTGAME1_ACK_OK 0x00000001
	/******************************************************/


	/******************************************************/
#define SERVER_STARTGAME3_ACK 0x1aff
	typedef struct
	{
		t_bnet_header h;
		bn_int        reply;
	} PACKED_ATTR() t_server_startgame3_ack;
#define SERVER_STARTGAME3_ACK_NO 0x00000000
#define SERVER_STARTGAME3_ACK_OK 0x00000001
	/******************************************************/


	/******************************************************/
#define SERVER_STARTGAME4_ACK 0x1cff
	typedef struct
	{
		t_bnet_header h;
		bn_int        reply;
	} PACKED_ATTR() t_server_startgame4_ack;
#define SERVER_STARTGAME4_ACK_NO 0x00000001
#define SERVER_STARTGAME4_ACK_OK 0x00000000
	/******************************************************/


	/******************************************************/
#define CLIENT_CLOSEGAME 0x02ff
#define CLIENT_CLOSEGAME2 0x1fff
	typedef struct
	{
		t_bnet_header h;
	} PACKED_ATTR() t_client_closegame;
	/******************************************************/


	/******************************************************/
#define CLIENT_LEAVECHANNEL 0x10ff
	typedef struct
	{
		t_bnet_header h;
	} PACKED_ATTR() t_client_leavechannel;

	/******************************************************/
	/*
		packets 0x60ff - 0x64ff moved to anongame_protocal.h
		[Omega]
		*/
	/******************************************************/

	/*
	FF 32 2A 00 1A 29 25 72   77 C3 3C 25 6B 4D 7A A4    .2*..)%rw.<%kMz.
	3B 92 38 D5 01 F4 A5 6B   28 32 29 43 68 61 6C 6C    ;.8....k(2)Chall
	65 6E 67 65 72 2E 73 63   6D 00                      enger.scm.

	FF 32 2C 00 21 F8 16 2D   99 D9 BC A4 A6 5C BA 60    .2,.!..-.....\.`
	71 DE 6D 64 6F BC A5 03   28 34 29 44 69 72 65 20    q.mdo...(4)Dire
	53 74 72 61 69 74 73 2E   73 63 6D 00                Straits.scm.
	*/
#define CLIENT_MAPAUTHREQ1 0x32ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        file_checksum[5];
		/* mapfile */
	} PACKED_ATTR() t_client_mapauthreq1;
	/******************************************************/


	/******************************************************/
	/*
	FF 32 08 00 01 00 00 00                              .2......
	*/
#define SERVER_MAPAUTHREPLY1 0x32ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        response;
	} PACKED_ATTR() t_server_mapauthreply1;
#define SERVER_MAPAUTHREPLY1_NO        0x00000000
#define SERVER_MAPAUTHREPLY1_OK        0x00000001
#define SERVER_MAPAUTHREPLY1_LADDER_OK 0x00000002
	/******************************************************/


	/******************************************************/
	/*
	From BW1.08alpha:
	FF 3C 31 00 7A 20 01 00   3B B7 C6 27 0D 61 C3 79    .<1.z ..;..'.a.y
	79 BE 24 5E 9C 07 05 7D   0B 6A A0 78 28 35 29 4A    y.$^...}.j.x(5)J
	65 77 65 6C 65 64 20 52   69 76 65 72 2E 73 63 6D    eweled River.scm
	00                                                   .
	*/
#define CLIENT_MAPAUTHREQ2 0x3cff
	typedef struct
	{
		t_bnet_header h;
		bn_int        unknown;
		bn_int        file_hash[5];
		/* mapfile */
	} PACKED_ATTR() t_client_mapauthreq2;
	/******************************************************/


	/******************************************************/
	/*
	assuming it looks like the REPLY1 packet....
	*/
#define SERVER_MAPAUTHREPLY2 0x3cff
	typedef struct
	{
		t_bnet_header h;
		bn_int        response;
	} PACKED_ATTR() t_server_mapauthreply2;
#define SERVER_MAPAUTHREPLY2_NO        0x00000000 /* FIXME: these values are guesses */
#define SERVER_MAPAUTHREPLY2_OK        0x00000001
#define SERVER_MAPAUTHREPLY2_LADDER_OK 0x00000002
	/******************************************************/


	/******************************************************/
	/*
	FF 15 14 00 36 38 58 49 52 41 54 53 00 00 00 00  ....68XIRATS....
	AF 14 55 36                                      ..U6
	*/
#define CLIENT_ADREQ 0x15ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        archtag;
		bn_int        clienttag;
		bn_int        prev_adid; /* zero if first request */
		bn_int        ticks;     /* Unix-style time in seconds */
	} PACKED_ATTR() t_client_adreq;
	/******************************************************/


	/******************************************************/
	/*
	Sent in response to a CLIENT_ADREQ to tell the client which
	banner to display next

	FF 15 3A 00 72 00 00 00   2E 70 63 78 50 15 7A 1C    ..:.r....pcxP.z.
	CE 0F BD 01 61 64 30 30   30 30 37 32 2E 70 63 78    ....ad000072.pcx
	00 68 74 74 70 3A 2F 2F   77 77 77 2E 62 6C 69 7A    .http://www.bliz
	7A 61 72 64 2E 63 6F 6D   2F 00                      zard.com/.

	FF 15 3C 00 C3 00 00 00   2E 73 6D 6B 00 9B 36 A6    ..<......smk..6.
	8F 5B BE 01 61 64 30 30   30 30 63 33 2E 73 6D 6B    .[..ad0000c3.smk
	00 68 74 74 70 3A 2F 2F   77 77 77 2E 66 61 74 68    .http://www.fath
	65 72 68 6F 6F 64 2E 6F   72 67 2F 00                erhood.org/.

	FF 15 36 00 2B 51 02 00            ..6.+Q..
	2E 70 63 78 00 00 00 00   58 01 B2 00 61 64 30 32    .pcx....X...ad02
	35 31 32 62 2E 70 63 78   00 68 74 74 70 3A 2F 2F    512b.pcx.http://
	77 77 77 2E 66 73 67 73   2E 63 6F 6D 2F 00          www.fsgs.com/.
	*/
#define SERVER_ADREPLY 0x15ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        adid;
		bn_int        extensiontag; /* unlike other tags, this one is "forward" */
		bn_long       timestamp;    /* file modification time? */
		/* filename */
		/* link URL */
	} PACKED_ATTR() t_server_adreply;
	/******************************************************/


	/******************************************************/
#define CLIENT_ADACK 0x21ff
	/*
	Sent after client has displayed the banner

	0000:   FF 21 36 00 36 38 58 49   52 41 54 53 72 00 00 00    .!6.68XIRATSr...
	0010:   61 64 30 30 30 30 37 32   2E 70 63 78 00 68 74 74    ad000072.pcx.htt
	0020:   70 3A 2F 2F 77 77 77 2E   62 6C 69 7A 7A 61 72 64    p://www.blizzard
	0030:   2E 63 6F 6D 2F 00                                    .com/.

	0000:   FF 21 38 00 36 38 58 49   4C 54 52 44 C3 00 00 00    .!8.68XILTRD....
	0010:   61 64 30 30 30 30 63 33   2E 73 6D 6B 00 68 74 74    ad0000c3.smk.htt
	0020:   70 3A 2F 2F 77 77 77 2E   66 61 74 68 65 72 68 6F    p://www.fatherho
	0030:   6F 64 2E 6F 72 67 2F 00                              od.org/.
	*/
	typedef struct
	{
		t_bnet_header h;
		bn_int        archtag;
		bn_int        clienttag;
		bn_int        adid;
		/* adfile */
		/* adlink */
	} PACKED_ATTR() t_client_adack;
	/******************************************************/


	/******************************************************/
	/*
	Sent if the user clicks on the adbanner
	*/
#define CLIENT_ADCLICK 0x16ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        adid;
		bn_int        unknown1;
	} PACKED_ATTR() t_client_adclick;
	/******************************************************/


	/******************************************************/
	/* first seen in Diablo II? */
	/*
	FF 41 08 00 01 00 00 00                              .A......
	*/
#define CLIENT_ADCLICK2 0x41ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        adid;
	} PACKED_ATTR() t_client_adclick2;
	/******************************************************/


	/******************************************************/
	/* first seen in Diablo II? */
	/*
					  FF 41 2C   00 0B 20 00 00 68 74   .qT....A,.. ..ht
					  74 70 3A 2F 2F 77 77 77 2E   62 6C 69 7A 7A 61 72   tp://www.blizzar
					  64 2E 63 6F 6D 2F 64 69 61   62 6C 6F 32 65 78 70   d.com/diablo2exp
					  2F 00                                               /.
					  */
#define SERVER_ADCLICKREPLY2 0x41ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        adid;
		/* link URL */
	} PACKED_ATTR() t_server_adclickreply2;
	/******************************************************/


	/******************************************************/
	/* seen in SC107a */
#define CLIENT_READMEMORY 0x17ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        request_id;
		/* Memory */
	} PACKED_ATTR() t_client_readmemory;

#define SERVER_READMEMORY 0x17ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        request_id;
		bn_int        address;
		bn_int        length;
	} PACKED_ATTR() t_server_readmemory;
	/******************************************************/


	/******************************************************/
	/* seen in SC107a */
#define CLIENT_UNKNOWN_24 0x24ff
	typedef struct
	{
		t_bnet_header h;
		/* FIXME: what is in here... is there a cooresponding
		   server packet? */
	} PACKED_ATTR() t_client_unknown_24;
	/******************************************************/


	/******************************************************/
	/*
	FF 2E 18 00 4E 42 32 57   01 00 00 00 00 00 00 00    ....NB2W........
	00 00 00 00 0A 00 00 00                              ........
	*/
#define CLIENT_LADDERREQ 0x2eff
	typedef struct
	{
		t_bnet_header h;
		bn_int        clienttag;
		bn_int        id; /* (AKA ladder type) 1==standard, 3==ironman */
		bn_int        type; /* (AKA ladder sort) */
		bn_int        startplace; /* start listing on this entries */
		bn_int        count; /* how many entries to list */
	} PACKED_ATTR() t_client_ladderreq;
#define CLIENT_LADDERREQ_ID_STANDARD       0x00000001
#define CLIENT_LADDERREQ_ID_IRONMAN        0x00000003
#define CLIENT_LADDERREQ_TYPE_HIGHESTRATED 0x00000000
#define CLIENT_LADDERREQ_TYPE_MOSTWINS     0x00000002
#define CLIENT_LADDERREQ_TYPE_MOSTGAMES    0x00000003
	/******************************************************/


	/******************************************************/
	/*
	Sent in repsonse to CLIENT_LADDERREQ

	FF 2E AB 03 52 41 54 53   01 00 00 00 00 00 00 00    ....RATS........
	00 00 00 00 0A 00 00 00   27 00 00 00 01 00 00 00    ........'.......
	01 00 00 00 74 06 00 00   00 00 00 00 27 00 00 00    ....t.......'...
	01 00 00 00 01 00 00 00   74 06 00 00 00 00 00 00    ........t.......
	00 00 00 00 00 00 00 00   FF FF FF FF 74 06 00 00    ............t...
	00 00 00 00 00 00 00 00   12 27 4E FF 2D DD BE 01    .........'N.-...
	12 27 4E FF 2D DD BE 01   6E 65 6D 62 69 3A 29 6B    .'N.-...nembi:)k
	69 6C 6C 65 72 00 1D 00   00 00 03 00 00 00 00 00    iller...........
	00 00 67 06 00 00 00 00   00 00 1D 00 00 00 03 00    ..g.............
	00 00 00 00 00 00 67 06   00 00 00 00 00 00 01 00    ......g.........
	00 00 00 00 00 00 FF FF   FF FF 67 06 00 00 01 00    ..........g.....
	00 00 00 00 00 00 D8 5B   9E D0 32 DD BE 01 D8 5B    .......[..2....[
	9E D0 32 DD BE 01 53 4B   45 4C 54 4F 4E 00 1F 00    ..2...SKELTON...
	00 00 03 00 00 00 00 00   00 00 EF 05 00 00 00 00    ................
	00 00 1F 00 00 00 03 00   00 00 00 00 00 00 EF 05    ................
	00 00 00 00 00 00 02 00   00 00 00 00 00 00 FF FF    ................
	FF FF EF 05 00 00 02 00   00 00 00 00 00 00 62 26    ..............b&
	55 0C 51 DC BE 01 62 26   55 0C 51 DC BE 01 7A 69    U.Q...b&U.Q...zi
	7A 69 62 65 5E 2E 7E 00   19 00 00 00 02 00 00 00    zibe^.~.........
	00 00 00 00 FC 05 00 00   00 00 00 00 18 00 00 00    ................
	02 00 00 00 00 00 00 00   EE 05 00 00 00 00 00 00    ................
	03 00 00 00 00 00 00 00   FF FF FF FF FC 05 00 00    ................
	03 00 00 00 00 00 00 00   A0 25 4F 31 90 DE BE 01    .........%O1....
	8C F1 7F 9F 66 DD BE 01   59 4F 4F 4A 49 4E 27 53    ....f...YOOJIN'S
	00 1D 00 00 00 02 00 00   00 00 00 00 00 EC 05 00    ................
	00 00 00 00 00 1D 00 00   00 02 00 00 00 00 00 00    ................
	00 EC 05 00 00 00 00 00   00 04 00 00 00 00 00 00    ................
	00 FF FF FF FF EC 05 00   00 03 00 00 00 00 00 00    ................
	00 F4 58 78 82 2F D7 BE   01 F4 58 78 82 2F D7 BE    ..Xx./....Xx./..
	01 3D 7B 5F 7C 5F 7D 3D   00 1A 00 00 00 00 00 00    .={_|_}=........
	00 00 00 00 00 E2 05 00   00 00 00 00 00 1A 00 00    ................
	00 00 00 00 00 00 00 00   00 E2 05 00 00 00 00 00    ................
	00 05 00 00 00 00 00 00   00 FF FF FF FF E2 05 00    ................
	00 05 00 00 00 00 00 00   00 F2 DC 0D 1F 6C DD BE    .............l..
	01 F2 DC 0D 1F 6C DD BE   01 5B 53 50 41 43 45 5D    .....l...[SPACE]
	2D 31 2D 54 2E 53 2E 4A   00 15 00 00 00 02 00 00    -1-T.S.J........
	00 01 00 00 00 E0 05 00   00 00 00 00 00 15 00 00    ................
	00 02 00 00 00 01 00 00   00 E0 05 00 00 00 00 00    ................
	00 06 00 00 00 00 00 00   00 FF FF FF FF E0 05 00    ................
	00 06 00 00 00 00 00 00   00 7A C9 F6 6E 0B DE BE    .........z..n...
	01 7A C9 F6 6E 0B DE BE   01 5B 46 65 77 5D 2D 44    .z..n....[Few]-D
	2E 73 00 23 00 00 00 00   00 00 00 00 00 00 00 DF    .s.#............
	05 00 00 00 00 00 00 23   00 00 00 00 00 00 00 00    .......#........
	00 00 00 DF 05 00 00 00   00 00 00 07 00 00 00 00    ................
	00 00 00 FF FF FF FF E2   05 00 00 04 00 00 00 00    ................
	00 00 00 6E D9 DC 3A E2   DA BE 01 6E D9 DC 3A E2    ...n..:....n..:.
	DA BE 01 5B 4C 2E 73 5D   2D 43 6F 6F 6C 00 1F 00    ...[L.s]-Cool...
	00 00 07 00 00 00 02 00   00 00 DD 05 00 00 00 00    ................
	00 00 1F 00 00 00 07 00   00 00 02 00 00 00 DD 05    ................
	00 00 00 00 00 00 08 00   00 00 00 00 00 00 FF FF    ................
	FF FF 07 06 00 00 02 00   00 00 00 00 00 00 B6 CE    ................
	B1 D8 7A D8 BE 01 B6 CE   B1 D8 7A D8 BE 01 52 6F    ..z.......z...Ro
	60 4C 65 58 7E 50 72 4F   27 5A 65 4E 00 18 00 00    `LeX~PrO'ZeN....
	00 04 00 00 00 00 00 00   00 DD 05 00 00 00 00 00    ................
	00 18 00 00 00 04 00 00   00 00 00 00 00 DD 05 00    ................
	00 00 00 00 00 09 00 00   00 00 00 00 00 FF FF FF    ................
	FF DD 05 00 00 04 00 00   00 00 00 00 00 50 76 4C    .............PvL
	F7 AD D7 BE 01 50 76 4C   F7 AD D7 BE 01 48 61 6E    .....PvL.....Han
	5F 65 53 54 68 65 72 2E   27 27 00                   _eSTher.''.
	*/
#define SERVER_LADDERREPLY 0x2eff
	typedef struct
	{
		t_bnet_header h;
		bn_int        clienttag;
		bn_int        id; /* (AKA ladder type) 1==standard, 3==ironman */
		bn_int        type; /* (AKA ladder sort) */
		bn_int        startplace; /* start listing on this entries */
		bn_int        count; /* how many entries to list */
		/* ladder entry */
		/* player name */
	} PACKED_ATTR() t_server_ladderreply;
#define CLIENT_LADDERREPLY_ID_STANDARD 0x00000001
#define CLIENT_LADDERREPLY_ID_IRONMAN  0x00000003

	typedef struct
	{
		bn_int wins;
		bn_int loss;
		bn_int disconnect;
		bn_int rating;
		bn_int rank;
	} PACKED_ATTR() t_ladder_data;

	typedef struct
	{
		t_ladder_data current;
		t_ladder_data active;
		bn_int        ttest[6]; /* 00 00 00 00  00 00 00 00  FF FF FF FF  74 06 00 00  00 00 00 00 00  00 00 00 */
		bn_long       lastgame_current; /* timestamp */
		bn_long       lastgame_active;  /* timestamp */
	} PACKED_ATTR() t_ladder_entry;
	/******************************************************/


	/******************************************************/
	/*
										FF 25 08 00 EA 7F DB 02   .%......
										*/
#define CLIENT_ECHOREPLY 0x25ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        ticks;
	} PACKED_ATTR() t_client_echoreply;
	/******************************************************/


	/******************************************************/
#define SERVER_ECHOREQ 0x25ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        ticks;
	} PACKED_ATTR() t_server_echoreq;
	/******************************************************/



	/******************************************************/
	/*
	What I'm calling the ping happens every 90 seconds during gameplay.  I'm
	not exactly sure what it is, but it didn't hurt that we didn't respond up
	to now...  I went ahead and coded a response in.  This packet is sent
	at other times as well, even before login.

	This is probably a keepalive packet and the UDP is sent to make
	sure the UDP entry on the NAT gateway remains valid.

	Prolix calls these null packets and says they are sent every 60
	seconds.

	This seems to be associated with a UDP packet 7 from the client:
	7: cli class=bnet[0x01] type=CLIENT_PINGREQ[0x00ff] length=4
	0000:   FF 00 04 00                                          ....
	5: clt prot=udp[0x07] from=128.123.62.23:6112 to=128.123.62.23:6112 length=8
	0000:   07 00 00 00 2E 95 9D 00                              ........
	7: srv class=bnet[0x01] type=SERVER_PINGREPLY[0x00ff] length=4
	0000:   FF 00 04 00                                          ....

	FF 00 04 00                                          ....
	*/
#define CLIENT_PINGREQ 0x00ff
	typedef struct
	{
		t_bnet_header h;
	} PACKED_ATTR() t_client_pingreq;
	/******************************************************/


	/******************************************************/
	/*
	FF 00 04 00                                          ....
	*/
#define SERVER_PINGREPLY 0x00ff
	typedef struct
	{
		t_bnet_header h;
	} PACKED_ATTR() t_server_pingreply;
	/******************************************************/


	/******************************************************/
	/*
	FF 2C 3D 02 00 00 00 00   08 00 00 00 03 00 00 00    .,=.............
	00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
	00 00 00 00 00 00 00 00   00 00 00 00 52 6F 73 73    ............Ross
	5F 43 4D 00 00 00 00 00   00 00 00 4F 6E 20 6D 61    _CM........On ma
	70 20 22 43 68 61 6C 6C   65 6E 67 65 72 22 3A 0A    p "Challenger":.
	00 52 6F 73 73 5F 43 4D   20 77 61 73 20 50 72 6F    .Ross_CM was Pro
	74 6F 73 73 20 61 6E 64   20 70 6C 61 79 65 64 20    toss and played
	66 6F 72 20 33 31 20 6D   69 6E 75 74 65 73 0A 0A    for 31 minutes..
	20 20 4F 76 65 72 61 6C   6C 20 53 63 6F 72 65 20      Overall Score
	32 38 31 30 35 0A 20 20   20 20 20 20 20 20 20 31    28105.         1
	31 37 30 30 20 66 6F 72   20 55 6E 69 74 73 0A 20    1700 for Units.
	20 20 20 20 20 20 20 20   20 31 34 37 35 20 66 6F             1475 fo
	72 20 53 74 72 75 63 74   75 72 65 73 0A 20 20 20    r Structures.
	20 20 20 20 20 20 31 34   39 33 30 20 66 6F 72 20          14930 for
	52 65 73 6F 75 72 63 65   73 0A 0A 20 20 55 6E 69    Resources..  Uni
	74 73 20 53 63 6F 72 65   20 31 31 37 30 30 0A 20    ts Score 11700.
	20 20 20 20 20 20 20 20   20 20 20 37 30 20 55 6E               70 Pn
	*/
#define CLIENT_GAME_REPORT 0x2cff
	typedef struct
	{
		t_bnet_header h;
		bn_int        unknown1; /* 0x00000000 */
		bn_int        count;    /* (number of player slots (will be 8 for now) */
		/* results... */
		/* names... */
		/* report header */
		/* report body */
	} PACKED_ATTR() t_client_game_report;

	typedef struct
	{
		bn_int result;
	} PACKED_ATTR() t_client_game_report_result;
#define CLIENT_GAME_REPORT_RESULT_PLAYING    0x00000000
#define CLIENT_GAME_REPORT_RESULT_WIN        0x00000001
#define CLIENT_GAME_REPORT_RESULT_LOSS       0x00000002
#define CLIENT_GAME_REPORT_RESULT_DRAW       0x00000003
#define CLIENT_GAME_REPORT_RESULT_DISCONNECT 0x00000004
#define CLIENT_GAME_REPORT_RESULT_OBSERVER   0x00000005
	/******************************************************/


	/******************************************************/
	/*
	War Craft II BNE (original):
	FF 22 1B 00 4E 42 32 57   4B 00 00 00 4C 61 64 64    ."..NB2WK...Ladd
	65 72 20 31 20 6F 6E 20   31 00 00                   er 1 on 1..
	*/
#define CLIENT_JOIN_GAME 0x22ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        clienttag;
		bn_int        versiontag;
		/* game name */
		/* game password */
	} PACKED_ATTR() t_client_join_game;
	/******************************************************/


	/******************************************************/
	/*
	FF 27 84 00 01 00 00 00  04 00 00 00 52 6F 73 73    .'..........Ross
	5F 43 4D 00 70 72 6F 66  69 6C 65 5C 73 65 78 00    _CM.profile\sex.
	70 72 6F 66 69 6C 65 5C  61 67 65 00 70 72 6F 66    profile\age.prof
	69 6C 65 5C 6C 6F 63 61  74 69 6F 6E 00 70 72 6F    ile\location.pro
	66 69 6C 65 5C 64 65 73  63 72 69 70 74 69 6F 6E    file\description
	00 61 73 64 66 00 61 73  66 00 61 73 64 66 00 61    .asdf.asf.asdf.a
	73 64 66 61 73 64 66 61  73 64 66 61 73 64 66 0D    sdfasdfasdfasdf.
	0A 61 73 64 0D 0A 66 61  73 64 0D 0A 66 61 73 64    .asd..fasd..fasd
	66 0D 0A 00                                         f...
	*/
#define CLIENT_STATSUPDATE 0x27ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        name_count;
		bn_int        key_count;
		/* names... */
		/* values... */
	} PACKED_ATTR() t_client_statsupdate;
	/******************************************************/

	/******************************************************/
#define CLIENT_REALMJOINREQ_109 0x3eff
	typedef struct
	{
		t_bnet_header h;
		bn_int        seqno;
		bn_int        seqnohash[5];
		/* Realm Name */
	} PACKED_ATTR() t_client_realmjoinreq_109;
	/******************************************************/

	/******************************************************/
#define SERVER_REALMJOINREPLY_109 0x3eff
	typedef struct
	{
		t_bnet_header h;
		bn_int        seqno;
		bn_int        u1;
		bn_int        bncs_addr1;
		bn_int        sessionnum;
		bn_int        addr;
		bn_short      port;
		bn_short      u3;
		bn_int        sessionkey;     /* zero */
		bn_int        u5;
		bn_int        u6;
		bn_int        clienttag;
		bn_int        versionid;
		bn_int        bncs_addr2;
		bn_int        u7;             /* zero */
		bn_int        secret_hash[5];
		/* account name */
	} PACKED_ATTR() t_server_realmjoinreply_109;
	/******************************************************/

#define CLIENT_SEARCH_LAN_GAMES 0x2ff7
	typedef struct
	{
		t_bnet_header h;
		bn_byte game_tag[4]; /* 3WAR */
		bn_int unknown1;
		bn_int unknown2;
	} PACKED_ATTR() t_client_search_lan_games;

#define CLIENT_CHANGECLIENT	0x5cff
	typedef struct
	{
		t_bnet_header h;
		bn_int clienttag;
	} t_client_changeclient;

#define CLIENT_SETEMAILREPLY		0x59ff
	typedef struct
	{
		t_bnet_header	h;
		/* email address */
	} PACKED_ATTR() t_client_setemailreply;

#define SERVER_SETEMAILREQ		0x59ff
	/* send this packet to client before login ok packet will cause
	client to enter input email screen */
	typedef struct
	{
		t_bnet_header	h;
	} PACKED_ATTR() t_server_setemailreq;

#define CLIENT_GETPASSWORDREQ		0x5aff
	typedef struct
	{
		t_bnet_header	h;
		/* account name */
		/* email address */
	} PACKED_ATTR() t_client_getpasswordreq;

#define CLIENT_CHANGEEMAILREQ		0x5bff
	typedef struct
	{
		t_bnet_header	h;
		/* account name */
		/* old email address */
		/* new email address */
	} PACKED_ATTR() t_client_changeemailreq;

#define CLIENT_MOTDREQ			0x46ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        last_news_time;	/* date of the last news item the client has */
	} PACKED_ATTR() t_client_motdreq;
	/*
	this packet is sent right after cdkey and version auth reply success and crashdump exist
	0x0000: ff 5d 14 00 01 01 00 27   00 0a 01 05 00 00 c0 00    .].....'........
	0x0010: 00 00 00 00                                          ....
	*/

#define CLIENT_CRASHDUMP		0x5dff
	typedef struct
	{
		t_bnet_header	h;
		/* crashdump file data */
		/* contains data like client version, exception code, code address */
	} PACKED_ATTR() t_client_crashdump;

#define CLIENT_CLANINFOREQ		0x82ff
	typedef struct
	{
		t_bnet_header	h;
		bn_int		count;
		bn_int		clantag;
		/* player name */
	} PACKED_ATTR() t_client_claninforeq;

#define SERVER_CLANINFOREPLY		0x82ff
	typedef struct
	{
		t_bnet_header	h;
		bn_int		count;
		bn_byte		fail;
		/* string  - clan name */
		/* bn_byte - clan rank */
		/* bn_int  - join time */
	} PACKED_ATTR() t_server_claninforeply;

#define CLIENT_ARRANGEDTEAM_ACCEPT_INVITE 0xfdff
	typedef struct
	{
		t_bnet_header h;
	} PACKED_ATTR() t_client_arrangedteam_accept_invite;

	/* clan handling */

#define SERVER_CLAN_MEMBER_CHIEFTAIN 0x04
#define SERVER_CLAN_MEMBER_SHAMAN 0x03
#define SERVER_CLAN_MEMBER_GRUNT 0x02
#define SERVER_CLAN_MEMBER_PEON 0x01
#define SERVER_CLAN_MEMBER_NEW 0x00
#define SERVER_CLAN_MEMBER_OFFLINE 0x00
#define SERVER_CLAN_MEMBER_ONLINE 0x01
#define SERVER_CLAN_MEMBER_CHANNEL 0x02
#define SERVER_CLAN_MEMBER_GAME 0x03
#define SERVER_CLAN_MEMBER_PRIVATE_GAME 0x04

	/*Paquet #267
	0x0000   FF 75 0A 00 00 00 42 54-54 04                     u....BTT.
	*/
	/*
	300: recv class=bnet[0x02] type=unknown[0x70ff] length=12
	0000:   FF 70 0C 00 01 00 00 00   00 64 73 66                .p.......dsf
	300: send class=bnet[0x02] type=unknown[0x70ff] length=56
	0000:   FF 70 38 00 01 00 00 00   00 09 44 4A 50 32 00 44    .p8.......DJP2.D
	0010:   4A 50 33 00 44 4A 50 34   00 44 4A 50 35 00 44 4A    JP3.DJP4.DJP5.DJ
	0020:   50 36 00 44 4A 50 37 00   44 4A 50 38 00 44 4A 50    P6.DJP7.DJP8.DJP
	0030:   39 00 44 4A 50 31 30 00                              9.DJP10.
	300: recv class=bnet[0x02] type=CLIENT_FRIENDINFOREQ[0x66ff] length=5
	0000:   FF 66 05 00 01                                       .f...
	300: send class=bnet[0x02] type=unknown[0x66ff] length=12
	0000:   FF 66 0C 00 01 00 00 00   00 00 00 00                .f..........
	300: recv class=bnet[0x02] type=unknown[0x71ff] length=64
	0000:   FF 71 40 00 01 00 00 00   74 65 73 74 00 00 64 73    .q@.....test..ds
	0010:   66 09 44 4A 50 32 00 44   4A 50 33 00 44 4A 50 34    f.DJP2.DJP3.DJP4
	0020:   00 44 4A 50 35 00 44 4A   50 36 00 44 4A 50 37 00    .DJP5.DJP6.DJP7.
	0030:   44 4A 50 38 00 44 4A 50   39 00 44 4A 50 31 30 00    DJP8.DJP9.DJP10.
	300: send class=bnet[0x02] type=unknown[0x71ff] length=10
	0000:   FF 71 0A 00 01 00 00 00   00 00                      .q........
	300: send class=bnet[0x02] type=unknown[0x75ff] length=12
	0000:   FF 75 0C 00 00 00 00 00   64 73 66 00                .u......dsf.
	300: recv class=bnet[0x02] type=CLIENT_FRIENDINFOREQ[0x66ff] length=5
	0000:   FF 66 05 00 02                                       .f...
	300: send class=bnet[0x02] type=unknown[0x66ff] length=12
	0000:   FF 66 0C 00 02 00 00 00   00 00 00 00                .f..........
	*/

#define CLIENT_CLAN_CREATEREQ 0x70ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		bn_int               clantag;
	} PACKED_ATTR() t_client_clan_createreq;


#define SERVER_CLAN_CREATEREPLY 0x70ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		bn_byte			   check_result;
		bn_byte			   friend_count;
		/* player name in chan or mutual
		char player_name[sizeof (friend_name)]; */
	} PACKED_ATTR() t_server_clan_createreply;
#define SERVER_CLAN_CREATEREPLY_CHECK_OK 0x00
#define SERVER_CLAN_CREATEREPLY_CHECK_ALLREADY_IN_USE 0x01
#define SERVER_CLAN_CREATEREPLY_CHECK_TIME_LIMIT 0x02
#define SERVER_CLAN_CREATEREPLY_CHECK_EXCEPTION 0x04
#define SERVER_CLAN_CREATEREPLY_CHECK_INVALID_CLAN_TAG 0x0a

	/*
	3852: recv class=bnet[0x02] type=unknown[0x71ff] length=70
	0000:   FF 71 46 00 01 00 00 00   53 75 62 57 61 72 5A 6F    .qF.....SubWarZo
	0010:   6E 65 00 00 5A 57 53 09   44 4A 50 32 00 44 4A 50    ne..ZWS.DJP2.DJP
	0020:   33 00 44 4A 50 34 00 44   4A 50 35 00 44 4A 50 36    3.DJP4.DJP5.DJP6
	0030:   00 44 4A 50 37 00 44 4A   50 38 00 44 4A 50 31 30    .DJP7.DJP8.DJP10
	0040:   00 44 4A 50 39 00                                    .DJP9.
	*/
#define CLIENT_CLAN_CREATEINVITEREQ 0x71ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		/* Clan Name (\0 terminated string)
		   bn_int			    clantag;
		   bn_byte			friend_count; //Number of friend selected
		   Name of friend (\0 terminated string)
		   */
	} PACKED_ATTR() t_client_clan_createinvitereq;

	/*3756: send class=bnet[0x02] type=unknown[0x71ff] length=14
	0000:   FF 71 0E 00 02 00 00 00   05 44 4A 50 32 00          .q.......DJP2.
	<- Unable to receive invitation ( PG search, already in a clan, etc... )*/
	/*3756:
	Paquet #266
	0x0000   FF 71 0A 00 05 00 00 00-00 00                     q........
	<- Clan invitation = Sucessfully done */
#define SERVER_CLAN_CREATEINVITEREPLY 0x71ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		bn_byte			   status; /* 0x05 = Cannot contact(not in channel screen) or already in clan | 0x04 = Decline | 0x00 = OK :)
		Name of failed member(\0 terminated string) */
	} PACKED_ATTR() t_server_clan_createinvitereply;

#define SERVER_CLAN_CREATEINVITEREQ 0x72ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		bn_int			   clantag;
		/* Clan Name (\0 terminated string)
		   Clan Creator (\0 terminated string)
		   bn_byte			friend_count; //Number of friend selected
		   Name of friend (\0 terminated string) */
	} PACKED_ATTR() t_server_clan_createinvitereq;

#define CLIENT_CLAN_CREATEINVITEREPLY 0x72ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		bn_int			   clantag;
		/* Clan Creator (\0 terminated string)
		   bn_byte			reply */ /* 0x04--decline 0x05--Cannot contact(not in channel screen) or already in clan 0x06--accept*/
	} PACKED_ATTR() t_client_clan_createinvitereply;

	/*
	3876: recv class=bnet[0x02] type=unknown[0x73ff] length=8
	0000:   FF 73 08 00 01 00 00 00                              .s......      */
#define CLIENT_CLAN_DISBANDREQ 0x73ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
	} PACKED_ATTR() t_client_clan_disbandreq;

#define SERVER_CLAN_DISBANDREPLY 0x73ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		bn_byte              result;   /* 0-- Success  1-- Exception raised  2-- Clan exists less than 1 week, cannot remove */
	} PACKED_ATTR() t_server_clan_disbandreply;
#define SERVER_CLAN_DISBANDREPLY_RESULT_OK 0x0
#define SERVER_CLAN_DISBANDREPLY_RESULT_EXCEPTION 0x1
#define SERVER_CLAN_DISBANDREPLY_RESULT_FAILED 0x2

#define CLIENT_CLAN_MEMBERNEWCHIEFREQ 0x74ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		/*Player_Name deleted(\0 terminated) */
	} PACKED_ATTR() t_client_clan_membernewchiefreq;

#define SERVER_CLAN_MEMBERNEWCHIEFREPLY 0x74ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		bn_byte              result; /* 0-successful 1-failed */
	} PACKED_ATTR() t_server_clan_membernewchiefreply;
#define SERVER_CLAN_MEMBERNEWCHIEFREPLY_SUCCESS 0x00
#define SERVER_CLAN_MEMBERNEWCHIEFREPLY_FAILED 0x01

#define SERVER_CLAN_CLANACK 0x75ff
	typedef struct{
		t_bnet_header		h;
		bn_byte		unknow1; /* 0x00 */
		bn_int		clantag;
		bn_byte		status;  /* member status */
	} PACKED_ATTR() t_server_clan_clanack;

#define SERVER_CLANQUITNOTIFY 0x76ff
	typedef struct{
		t_bnet_header        h;
		bn_byte              status;
	} PACKED_ATTR() t_server_clanquitnotify;
#define SERVER_CLANQUITNOTIFY_STATUS_REMOVED_FROM_CLAN 0x01

	/*
	3876: recv class=bnet[0x02] type=unknown[0x77ff] length=13
	0000:   FF 77 0D 00 01 00 00 00   44 4A 50 31 00             .w......DJP1.  */
#define CLIENT_CLAN_INVITEREQ 0x77ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		/*Player_Name invited */
	} PACKED_ATTR() t_client_clan_invitereq;

#define SERVER_CLAN_INVITEREPLY 0x77ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		bn_byte              result;  /* 0x04--decline 0x05--Cannot contact(not in channel screen) or already in clan */
	} PACKED_ATTR() t_server_clan_invitereply;

#define CLIENT_CLANMEMBER_REMOVE_REQ 0x78ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		/*Player_Name deleted(\0 terminated) */
	} PACKED_ATTR() t_client_clanmember_remove_req;

#define SERVER_CLANMEMBER_REMOVE_REPLY 0x78ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		bn_byte              result; /* 0-successful 1-failed */
	} PACKED_ATTR() t_server_clanmember_remove_reply;
#define SERVER_CLANMEMBER_REMOVE_SUCCESS 0x00
#define SERVER_CLANMEMBER_REMOVE_FAILED 0x01

#define SERVER_CLAN_INVITEREQ 0x79ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		bn_int               clantag;
		/*Clan_Name (\0 terminated)
		Player_Name inviter (\0 terminated) */
	} PACKED_ATTR() t_server_clan_invitereq;

#define CLIENT_CLAN_INVITEREPLY 0x79ff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		bn_int               clantag;
		/*Player_Name inviter (\0 terminated)
		bn_byte            reply */
	} PACKED_ATTR() t_client_clan_invitereply;

	/*
		clan codes according to bnetdocs

		0x00: Success
		0x01: In use
		0x02: Too soon
		0x03: Not enough members
		0x04: Invitation was declined
		0x05: Decline
		0x06: Accept
		0x07: Not authorized
		0x08: User not found
		0x09: Clan is full
		0x0A: Bad tag
		0x0B: Bad name
		0x0C: User not found in that clan
		*/

#define CLAN_RESPONSE_SUCCESS 		0x00
#define CLAN_RESPONSE_FAIL			0x01
#define CLAN_RESPONSE_TOO_SOON 		0x02
#define CLAN_RESPONSE_TOO_SMALL 	0x03
#define CLAN_RESPONSE_DECLINED 		0x04
#define CLAN_RESPONSE_DECLINE 		0x05
#define CLAN_RESPONSE_ACCEPT 		0x06
#define CLAN_RESPONSE_NOT_AUTHORIZED 0x07
#define CLAN_RESPONSE_NOT_FOUND 	0x08
#define CLAN_RESPONSE_CLAN_FULL 	0x09
#define CLAN_RESPONSE_BAD_TAG		0x0a
#define CLAN_RESPONSE_BAD_NAME		0x0b
#define CLAN_RESPONSE_NOT_MEMBER	0x0c

#define CLIENT_CLANMEMBER_RANKUPDATE_REQ 0x7aff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		/*Player_Name invited(\0 terminated)
		  Player_Status(bn_byte: 1~4) */
	} PACKED_ATTR() t_client_clanmember_rankupdate_req;

#define SERVER_CLANMEMBER_RANKUPDATE_REPLY 0x7aff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		bn_byte              result; /* 0-successful 1-failed */
	} PACKED_ATTR() t_server_clanmember_rankupdate_reply;
#define SERVER_CLANMEMBER_RANKUPDATE_SUCCESS 0x00
#define SERVER_CLANMEMBER_RANKUPDATE_FAILED 0x01

#define CLIENT_CLAN_MOTDCHG 0x7bff
	typedef struct{
		t_bnet_header        h;
		bn_int               unknow1;
		/* Motd en string ^^ */
	} PACKED_ATTR() t_client_clan_motdchg;
#define SERVER_CLAN_MOTDREPLY_UNKNOW1 0x00000000

#define SERVER_CLAN_MOTDREPLY 0x7cff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		bn_int			   unknow1; /* 0x00000000 */
		/* MOTD */
	} PACKED_ATTR() t_server_clan_motdreply;

#define CLIENT_CLAN_MOTDREQ 0x7cff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
	} PACKED_ATTR() t_client_clan_motdreq;

	/*
	Paquet #52
	0x0000   FF 7D 10 01 01 00 00 00-13 4D 79 73 74 69 2E 53   }.......Mysti.S
	0x0010   77 5A 00 04 01 00 73 61-75 72 6F 6E 2E 73 77 7A   wZ....sauron.swz
	0x0020   00 03 00 00 73 69 6D 6F-6E 2E 53 77 5A 00 03 00   ....simon.SwZ...
	0x0030   00 4E 65 6F 2D 56 61 67-72 61 6E 74 2E 73 77 7A   .Neo-Vagrant.swz
	0x0040   00 02 00 00 77 4D 7A 00-02 00 00 6B 61 74 6E 6F   ....wMz....katno
	0x0050   6D 61 64 2E 53 77 5A 00-02 00 00 47 7A 62 65 75   mad.SwZ....Gzbeu
	0x0060   68 2E 53 77 5A 00 02 00-00 53 69 6C 76 65 72 2E   h.SwZ....Silver.
	0x0070   53 77 5A 00 02 00 00 4D-61 67 67 65 75 73 00 02   SwZ....Maggeus..
	0x0080   00 00 4F 6E 69 2D 4D 75-73 68 61 2E 53 77 5A 00   ..Oni-Musha.SwZ.
	0x0090   02 00 00 4D 61 67 67 65-75 53 2E 53 77 5A 00 02   ...MaggeuS.SwZ..
	0x00A0   00 00 53 69 72 65 5F 4C-6F 75 70 00 02 00 00 6B   ..Sire_Loup....k
	0x00B0   69 6C 6C 69 62 6F 79 00-03 00 00 52 65 64 2E 44   illiboy....Red.D
	0x00C0   72 61 4B 65 00 02 00 00-53 69 72 65 2E 53 77 5A   raKe....Sire.SwZ
	0x00D0   00 03 00 00 73 74 72 61-69 67 68 74 5F 63 6F 75   ....straight_cou
	0x00E0   67 61 72 00 02 00 00 52-65 64 44 72 61 6B 65 2E   gar....RedDrake.
	0x00F0   53 77 5A 00 02 00 00 54-72 6F 6C 6C 6F 00 02 00   SwZ....Trollo...
	0x0100   00 53 69 6C 76 65 72 62-65 61 72 64 00 00 00 00   .Silverbeard....
	*/
#define SERVER_CLANMEMBERLIST_REPLY 0x7dff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
		bn_byte			   member_count;
		/* player repeat start
		 * Name of player(\0 terminated string)
		 * bn_byte		   CHIEFTAIN = 0x04
		 *				   SHAMANS = 0x03
		 *				   GRUNT = 0x02
		 *				   PEON = 0x01
		 *				   NEW_MEMBER = 0x00 <- can't be promoted/devoted
		 * bn_byte		   online status
		 * unknown(always \0)
		 * repeat end
		 */
	} PACKED_ATTR() t_server_clanmemberlist_reply;

	/*
	Paquet #51
	0x0000   FF 7D 08 00 01 00 00 00-                          }......
	*/

#define CLIENT_CLANMEMBERLIST_REQ 0x7dff
	typedef struct{
		t_bnet_header        h;
		bn_int               count;
	} PACKED_ATTR() t_client_clanmemberlist_req;

#define SERVER_CLANMEMBER_REMOVED_NOTIFY 0x7eff
	typedef struct{
		t_bnet_header        h;
		/* Player_Name deleted(\0 terminated) */
	} PACKED_ATTR() t_server_clanmember_removed_notify;

#define SERVER_CLANMEMBERUPDATE 0x7fff
	typedef struct{
		t_bnet_header        h;
		/* Player_Name invited(\0 terminated)
		 * Player_Status(bn_byte: 1~4)
		 * Player_Online(bn_short: 0x0/0x1)
		 */
	} PACKED_ATTR() t_server_clanmemberupdate;



#define SERVER_MESSAGEBOX 0x19ff
	typedef struct
	{
		t_bnet_header h;
		bn_int        style;
		/* Text */
		/* Caption */
	} PACKED_ATTR() t_server_messagebox;
#define SERVER_MESSAGEBOX_OK    0x00000000
#define SERVER_MESSAGEBOX_OKCANCEL 0x00000001
#define SERVER_MESSAGEBOX_YESNO 0x00000004


}

#endif
