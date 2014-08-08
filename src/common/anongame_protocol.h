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
#ifndef INCLUDED_ANONGAME_PROTOCOL_TYPES
#define INCLUDED_ANONGAME_PROTOCOL_TYPES

#ifdef JUST_NEED_TYPES
# include "common/bn_type.h"
#else
# define JUST_NEED_TYPES
# include "common/bn_type.h"
# undef JUST_NEED_TYPES
#endif

namespace pvpgn
{

	/***********************************************************************************/
	/* first packet recieved from client - option decides which struct to use next */
#define CLIENT_FINDANONGAME 			0x44ff
#define SERVER_FINDANONGAME 			0x44ff
	typedef struct
	{
		t_bnet_header	h;
		bn_byte		option;
		/* rest of packet data */
	} PACKED_ATTR() t_client_anongame;

	/***********************************************************************************/
#define CLIENT_FINDANONGAME_SEARCH              0x00
#define CLIENT_FINDANONGAME_INFOS               0x02
#define CLIENT_FINDANONGAME_CANCEL		0x03
#define CLIENT_FINDANONGAME_PROFILE             0x04
#define CLIENT_FINDANONGAME_AT_SEARCH           0x05
#define CLIENT_FINDANONGAME_AT_INVITER_SEARCH   0x06
#define CLIENT_ANONGAME_TOURNAMENT		0X07
#define	CLIENT_FINDANONGAME_PROFILE_CLAN	0x08
#define CLIENT_FINDANONGAME_GET_ICON            0x09
#define CLIENT_FINDANONGAME_SET_ICON            0x0A

#define SERVER_FINDANONGAME_SEARCH              0x00
#define SERVER_FINDANONGAME_FOUND		0x01
#define SERVER_FINDANONGAME_CANCEL              0x03
	/***********************************************************************************/
	/* option 00 - anongame search */
	/*
	=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	Tournament PG 1v1 - battle.net
	# 440 packet from client: type=0x44ff(CLIENT_FINDANONGAME) length=28 class=bnet
	0000:   FF 44 1C 00 00 01 00 00   00 00 00 00 00[02 13]01    .D..............
	0010:   00 00 00 08 13 C7 30 04   04 00 00 00                ......0.....
	PG 1v1
	# 438 packet from client: type=0x44ff(CLIENT_FINDANONGAME) length=28 class=bnet
	0000:   FF 44 1C 00 00 01 00 00   00 00 00 00 00[00 00]FF    .D..............
	0010:   00 00 00 08 C2 13 A6 02   20 00 00 00                ........ ...
	PG 2v2
	# 456 packet from client: type=0x44ff(CLIENT_FINDANONGAME) length=28 class=bnet
	0000:   FF 44 1C 00 00 02 00 00   00 00 00 00 00[00 01]FF    .D..............
	0010:   07 00 00 08 A0 45 A6 02   20 00 00 00                .....E.. ...
	PG 3v3
	# 482 packet from client: type=0x44ff(CLIENT_FINDANONGAME) length=28 class=bnet
	0000:   FF 44 1C 00 00 03 00 00   00 00 00 00 00[00 02]FF    .D..............
	0010:   03 00 00 08 E8 7D A6 02   20 00 00 00                .....}.. ...
	PG 4v4
	# 516 packet from client: type=0x44ff(CLIENT_FINDANONGAME) length=28 class=bnet
	0000:   FF 44 1C 00 00 04 00 00   00 00 00 00 00[00 03]FF    .D..............
	0010:   01 00 00 08 48 B0 A6 02   20 00 00 00                ....H... ...
	PG sffa
	# 550 packet from client: type=0x44ff(CLIENT_FINDANONGAME) length=28 class=bnet
	0000:   FF 44 1C 00 00 05 00 00   00 00 00 00 00[00 04]FF    .D..............
	0010:   01 00 00 08 E6 F4 A6 02   20 00 00 00                ........ ...
	=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	inviter 2v2 - battle.net
	# 74059 packet from client: type=0x44ff(CLIENT_FINDANONGAME) length=57 class=bnet
	0000:   FF 44 39 00 06 09 00 00   00 A0 03 00 0C 72 40 27    .D9..........r@'
	0010:   3F 02 B9 DA 6B 9A A4 22   5D 60 49 EF 15 6F 44 25    ?...k.."]`I..oD%
	0020:   02 4E F9 13 BC 4D 00 00   00 00[01 00]FF 07 00 00    .N...M..........
	0030:   08 7E 98 E6 04 04 00 00   00                         .~.......
	inviter 2v2 - battle.net
	# 83545 packet from client: type=0x44ff(CLIENT_FINDANONGAME) length=57 class=bnet
	0000:   FF 44 39 00 06 0A 00 00   00 E4 03 00 0C 9E 45 27    .D9...........E'
	0010:   3F 02 1E 4E E7 53 80 F9   01 00 4F 68 A1 58 C5 1B    ?..N.S....Oh.X..
	0020:   44 31 BD EE 47 7F 00 00   00 00[01 00]FF 07 00 00    D1..G...........
	0030:   08 48 07 FB 04 04 00 00   00                         .H.......
	=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	AT 3v3 - pvpgn - with uid of team members
	# 378 packet from client: type=0x44ff(CLIENT_FINDANONGAME) length=57 class=bnet
	0000:   FF 44 39 00 06 01 00 00   00[B7 36 C4 19]1B 02 2F    .D9.......6..../
	0010:   3F 03[19 00 00 00|02 00   00 00|01 00 00 00|FF FF    ?...............
	0020:   FF FF|FF FF FF FF]00 00   00 00 01 02 FF 03 00 00    ................
	0030:   08 AA A4 E2 0C 20 00 00   00                         ..... ...

	# 379 packet from client: type=0x44ff(CLIENT_FINDANONGAME) length=51 class=bnet
	0000:   FF 44 33 00 05 01 00 00   00[B7 36 C4 19]1B 02 2F    .D3.......6..../
	0010:   3F 03[19 00 00 00|02 00   00 00|01 00 00 00|FF FF    ?...............
	0020:   FF FF|FF FF FF FF]00 00   00 00 08 59 C6 0E 00 20    ...........Y...
	0030:   00 00 00                                             ...

	# 382 packet from client: type=0x44ff(CLIENT_FINDANONGAME) length=51 class=bnet
	0000:   FF 44 33 00 05 01 00 00   00[B7 36 C4 19]1B 02 2F    .D3.......6..../
	0010:   3F 03[19 00 00 00|02 00   00 00|01 00 00 00|FF FF    ?...............
	0020:   FF FF|FF FF FF FF]00 00   00 00 08 45 2C 13 00 20    ...........E,..
	0030:   00 00 00                                             ...
	=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
	*/

	typedef struct
	{
		t_bnet_header	h;
		bn_byte		option;
		bn_int		count;     /* Goes up each time client clicks search */
		bn_int		unknown2;  /* 00 00 00 00 */
		bn_byte		type;      /* 0 = PG  , 1 = AT  , 2 = TY - from TYPE */
		bn_byte		gametype;  /* 0 = 1v1 , 1 = 2v2 , 2 = 3v3 , 3 = 4v4 , 4 = sffa - from TYPE */
		bn_int		map_prefs; /* map preferences bitmask */
		bn_byte		unknown3;  /* 8 */
		bn_int		id;        /* client id */
		bn_int       	race;      /* 1 = H , 2 = O , 4 = N , 8 = U , 0x20 = R */
	} PACKED_ATTR() t_client_findanongame;

	typedef struct
	{
		t_bnet_header h;
		bn_byte      option;
		bn_int       count;
		bn_int       tid;        /* team id */
		bn_int       timestamp;
		bn_byte	 teamsize;
		bn_int       info[5];   /* client get this info from SERVER_ARRANGEDTEAM_INVITE_FRIEND_ACK (0x61ff) */
		bn_int       unknown2;  /* 00 00 00 00 */
		bn_byte      type;      /* 0 = PG  , 1 = AT  , 2 = TY - from TYPE */
		bn_byte      gametype;  /* 0 = 1v1 , 1 = 2v2 , 2 = 3v3 , 3 = 4v4 , 4 = sffa - from TYPE */
		bn_int       map_prefs; /* map preferences bitmask */
		bn_byte      unknown3;  /* 08 */
		bn_int       id;	    /* client id */
		bn_int       race;      /* 1 = H , 2 = O , 4 = N , 8 = U , 0x20 = R */
	} PACKED_ATTR() t_client_findanongame_at_inv;

	typedef struct
	{
		t_bnet_header h;
		bn_byte      option;
		bn_int       count;     /* Goes up each time client clicks search */
		bn_int       tid;       /* team id */
		bn_int       timestamp;
		bn_byte      teamsize;
		bn_int       info[5];   /* client get this info from inviter form SERVER_ARRANGEDTEAM_INVITE_FRIEND_ACK (0x61ff) */
		bn_int       unknown2;  /* 00 00 00 00 */
		bn_byte      unknown3;  /* 08 */
		bn_int       id;        /* client id */
		bn_int       race;      /* 1 = H , 2 = O , 4 = N , 8 = U , 0x20 = R */
	} PACKED_ATTR() t_client_findanongame_at;

#define SERVER_ANONGAME_SEARCH_REPLY		0x44ff
	typedef struct
	{
		t_bnet_header h;
		bn_byte   option;
		bn_int    count;
		bn_int    reply;
		/*      bn_short  avgtime; - only in W3XP so far average time in seconds of search */
	} PACKED_ATTR() t_server_anongame_search_reply;

	/***********************************************************************************/
	/* option 01 - anongame found */
	/*
	tournament 1v1
	# 490 packet from server: type=0x44ff(unknown) length=84 class=bnet
	0000:   FF 44 54 00 01 01 00 00   00 00 00 00 00 3F F1 53    .DT..........?.S
	0010:   CD E0 17 F8 CC 00 00 3E   4F 8E 12 06[02 13]4D 61    .......>O.....Ma
	0020:   70 73 5C 46 72 6F 7A 65   6E 54 68 72 6F 6E 65 5C    ps\FrozenThrone\
	0030:   28 34 29 46 6C 6F 6F 64   70 6C 61 69 6E 73 31 76    (4)Floodplains1v
	0040:   31 2E 77 33 78 00 FF FF   FF FF 31 44 4C 46 02 00    1.w3x.....1DLF..
	0050:   00 00 02 02                                          ....
	PG 2v2
	# 28332 packet from server: type=0x44ff(unknown) length=83 class=bnet
	0000:   FF 44 53 00 01 02 00 00   00 00 00 00 00 D3 E9 00    .DS.............
	0010:   DC E0 17 2E 67 00 00 9A   7D 56 0F 06 00 01 4D 61    ....g...}V....Ma
	0020:   70 73 5C 46 72 6F 7A 65   6E 54 68 72 6F 6E 65 5C    ps\FrozenThrone\
	0030:   28 36 29 53 63 6F 72 63   68 65 64 42 61 73 69 6E    (6)ScorchedBasin
	0040:   2E 77 33 78 00 FF FF FF   FF 4D 41 45 54 04 02 00    .w3x.....MAET...
	0050:   00 02 02                                             ...
	PG 4v4
	# 28827 packet from server: type=0x44ff(unknown) length=82 class=bnet
	0000:   FF 44 52 00 01 02 00 00   00 00 00 00 00 3F F1 53    .DR..........?.S
	0010:   DA E0 17 77 D6 00 00 92   D4 B8 56 06 00 03 4D 61    ...w......V...Ma
	0020:   70 73 5C 46 72 6F 7A 65   6E 54 68 72 6F 6E 65 5C    ps\FrozenThrone\
	0030:   28 38 29 42 61 74 74 6C   65 67 72 6F 75 6E 64 2E    (8)Battleground.
	0040:   77 33 78 00 FF FF FF FF   4D 41 45 54 08 02 00 00    w3x.....MAET....
	0050:   02 02                                                ..
	PG 3v3
	# 357 packet from server: type=0x44ff(unknown) length=84 class=bnet
	0000:   FF 44 54 00 01 01 00 00   00 00 00 00 00 3F F1 53    .DT..........?.S
	0010:   CE E0 17 10 22 01 00 29   0F 62 A0 06 00 02 4D 61    ...."..).b....Ma
	0020:   70 73 5C 46 72 6F 7A 65   6E 54 68 72 6F 6E 65 5C    ps\FrozenThrone\
	0030:   28 36 29 42 6C 6F 6F 64   73 74 6F 6E 65 4D 65 73    (6)BloodstoneMes
	0040:   61 2E 77 33 78 00 FF FF   FF FF 4D 41 45 54 06 02    a.w3x.....MAET..
	0050:   00 00 02 02                                          ....
	PG 4v4
	# 620 packet from server: type=0x44ff(unknown) length=79 class=bnet
	0000:   FF 44 4F 00 01 02 00 00   00 00 00 00 00 3F F1 53    .DO..........?.S
	0010:   DA E0 17 94 22 01 00 4C   29 5F 62 06 00 03 4D 61    ...."..L)_b...Ma
	0020:   70 73 5C 46 72 6F 7A 65   6E 54 68 72 6F 6E 65 5C    ps\FrozenThrone\
	0030:   28 38 29 53 61 6E 63 74   75 61 72 79 2E 77 33 78    (8)Sanctuary.w3x
	0040:   00 FF FF FF FF 4D 41 45   54 08 02 00 00 02 02       .....MAET......
	PG sffa
	# 810 packet from server: type=0x44ff(unknown) length=82 class=bnet
	0000:   FF 44 52 00 01 03 00 00   00 00 00 00 00 3F F1 53    .DR..........?.S
	0010:   C9 E0 17 3E 22 01 00 93   44 ED 22 06 00 04 4D 61    ...>"...D."...Ma
	0020:   70 73 5C 46 72 6F 7A 65   6E 54 68 72 6F 6E 65 5C    ps\FrozenThrone\
	0030:   28 38 29 42 61 74 74 6C   65 67 72 6F 75 6E 64 2E    (8)Battleground.
	0040:   77 33 78 00 FF FF FF FF   20 41 46 46 05 00 00 00    w3x..... AFF....
	0050:   02 02                                                ..
	=-=-=-=-==-=-=-=-=
	AT 2v2
	# 74077 packet from server: type=0x44ff(unknown) length=87 class=bnet
	0000:   FF 44 57 00 01 09 00 00   00 00 00 00 00 3F F1 53    .DW..........?.S
	0010:   D3 E0 17 D8 D1 00 00 B4   2B 57 C6 06[01 00]4D 61    ........+W....Ma
	0020:   70 73 5C 46 72 6F 7A 65   6E 54 68 72 6F 6E 65 5C    ps\FrozenThrone\
	0030:   28 36 29 53 74 72 61 6E   67 6C 65 74 68 6F 72 6E    (6)Stranglethorn
	0040:   56 61 6C 65 2E 77 33 78   00 FF FF FF FF 32 53 56    Vale.w3x.....2SV
	0050:   32 04 02 00 00 02 02                                 2......

	AT 2v2
	# 83549 packet from server: type=0x44ff(unknown) length=79 class=bnet
	0000:   FF 44 4F 00 01 0A 00 00   00 00 00 00 00 3F F1 53    .DO..........?.S
	0010:   CF E0 17 BB D1 00 00 01   CB E4 BA 06[01 00]4D 61    ..............Ma
	0020:   70 73 5C 46 72 6F 7A 65   6E 54 68 72 6F 6E 65 5C    ps\FrozenThrone\
	0030:   28 36 29 47 6E 6F 6C 6C   57 6F 6F 64 2E 77 33 78    (6)GnollWood.w3x
	0040:   00 FF FF FF FF 32 53 56   32 04 02 00 00 02 02       .....2SV2......
	*/
#define SERVER_ANONGAME_FOUND			0x44ff
	typedef struct
	{
		t_bnet_header	h;
		bn_byte		option;     /* 1: anongame found */
		bn_int		count;
		bn_int		unknown1;   /* 00 00 00 00 */
		bn_int		ip;
		bn_short		port;
		bn_byte		unknown2;
		bn_byte             unknown3;
		bn_short		unknown4;   /* usually 00 00 , seen 01 00 */
		bn_int		id;         /* random val for identifying client */
		bn_byte		unknown5;   /* 0x06 */
		bn_byte		type;       /* 0 = PG  , 1 = AT  , 2 = TY - from TYPE */
		bn_byte		gametype;   /* for PG - 0 = 1v1 , 1 = 2v2 , 2 = 3v3 , 3 = 4v4 , 4 = sffa
						 * for AT - 0 = 2v2 , 2 = 3v3 , 3 = 4v3
						 * for TY - set to 0
						 * from TYPE
						 */
		/* char *		mapname */
		/* t_saf_pt2 * 	pt2 */
	} PACKED_ATTR() t_server_anongame_found;

	/* MISC PACKET APPEND DATA's */
	typedef struct
	{
		bn_int	unknown1;		/* 0xFFFFFFFF */
		bn_int	anongame_string;	/* ie. SOLO, TEAM, 2VS2, etc. */
		bn_byte	totalplayers;
		bn_byte	totalteams;		/* 1v1 & sffa = 0, rest 2 */
		bn_short	unknown2;		/* 0x0000 */
		bn_byte	visibility;		/* 0x01 = dark - 0x02 = default */
		bn_byte	unknown3;		/* 0x02 */
	} PACKED_ATTR() t_saf_pt2;

	/***********************************************************************************/
	/* option 02 - info request */
#define CLIENT_FINDANONGAME_INFOREQ             0x44ff
	typedef struct
	{
		t_bnet_header       h;
		bn_byte             option;         /* type of request:
						 * 0x02 for matchmaking infos */
		bn_int              count;          /* 0x00000001 increments each request of same type */
		bn_byte             noitems;
	} PACKED_ATTR() t_client_findanongame_inforeq;

#define SERVER_FINDANONGAME_INFOREPLY           0x44ff
	typedef struct {
		t_bnet_header       h;
		bn_byte             option; /* as received from client */
		bn_int              count; /* as received from client */
		bn_byte             noitems; /* not very sure about it */
		/* data */
		/*
		for type 0x02 :
		<type of info><unknown int><info>
		if <type of info> is :
		URL\0 : <info> contains 3 NULL terminated urls/strings
		MAP\0 : <info> contains 7 (seen so far) maps names
		TYPE  : <info> unknown 38 bytes probably meaning anongame types
		DESC  : <info>
		*/
	} PACKED_ATTR() t_server_findanongame_inforeply;

	/***********************************************************************************/
	/* option 03 - playgame cancel */
#define SERVER_FINDANONGAME_PLAYGAME_CANCEL 	0x44ff
	typedef struct
	{
		t_bnet_header h; /* header */
		bn_byte cancel; /* Cancel byte always 03 */
		bn_int  count;
	} PACKED_ATTR() t_server_findanongame_playgame_cancel;


	/* option 04 - profile request */
	typedef struct
	{
		t_bnet_header h;
		bn_byte     option;
		bn_int          count;
		/* USERNAME TO LOOKUP
		 * CLIENT TAG */
	} PACKED_ATTR() t_client_findanongame_profile;

#define SERVER_FINDANONGAME_PROFILE		0x44ff
	/*
	typedef struct
	{
	t_bnet_header h; //header
	bn_byte option; // in this case it will be 0x04 (for profile)
	bn_int count; // count that goes up each time user clicks on someones profile
	// REST OF PROFILE STATS - THIS WILL BE SET IN HANDLE_BNET.C after
	// SERVER LOOKS UP THE USER ACCOUNT
	} PACKED_ATTR() t_server_findanongame_profile;
	*/
	typedef struct
	{
		t_bnet_header	h;
		bn_byte		option;
		bn_int		count;
		bn_int		icon;
		bn_byte		rescount;
		/* REST OF PROFILE STATS - THIS WILL BE SET IN HANDLE_BNET.C after
		 * SERVER LOOKS UP THE USER ACCOUNT */
	} PACKED_ATTR() t_server_findanongame_profile2;

	/***********************************************************************************/
	/* option 07 - tournament request */
#define CLIENT_FINDANONGAME_TOURNAMENT_REQUEST  0x44ff
	typedef struct
	{
		t_bnet_header       h;
		bn_byte             option; /* 07 */
		bn_int              count;  /* 01 00 00 00 */
	} PACKED_ATTR() t_client_anongame_tournament_request;

#define SERVER_FINDANONGAME_TOURNAMENT_REPLY    0x44ff
	typedef struct
	{
		t_bnet_header       h;
		bn_byte             option;     /* 07 */
		bn_int              count;      /* 00 00 00 01 reply with same number */
		bn_byte             type;	    /* type - 	01 = notice - time = prelim round begins
						 *		02 = signups - time = signups end
						 *		03 = signups over - time = prelim round ends
						 *		04 = prelim over - time = finals round 1 begins
						 */
		bn_byte             unknown;    /* 00 */
		bn_short            unknown4;   /* random ? might be part of time/date ? */
		bn_int              timestamp;
		bn_byte             unknown5;   /* 01 effects time/date */
		bn_short            countdown;  /* countdown until next timestamp (seconds) */
		bn_short            unknown2;   /* 00 00 */
		bn_byte		wins;	    /* during prelim */
		bn_byte		losses;     /* during prelim */
		bn_byte             ties;	    /* during prelim */
		bn_byte             unknown3;   /* 00 = notice.  08 = signups thru prelim over (02-04) */
		bn_byte             selection;  /* matches anongame_TY_section of DESC */
		bn_byte             descnum;    /* matches desc_count of DESC */
		bn_byte             nulltag;    /* 00 */
	} PACKED_ATTR() t_server_anongame_tournament_reply;

	/***********************************************************************************/

	/* option 08 - clan profile request */
	typedef struct
	{
		t_bnet_header h;
		bn_byte	option;
		bn_int	count;
		bn_int	clantag;
		bn_int	clienttag;
	} PACKED_ATTR() t_client_findanongame_profile_clan;

#define SERVER_FINDANONGAME_PROFILE_CLAN	0x44ff

	typedef struct
	{
		t_bnet_header	h;
		bn_byte		option;
		bn_int		count;
		bn_byte		rescount;
		/* REST OF PROFILE STATS - THIS WILL BE SET IN HANDLE_BNET.C after
		 * SERVER LOOKS UP THE USER ACCOUNT */
	} PACKED_ATTR() t_server_findanongame_profile_clan;


	/***********************************************************************************/
	/* option 9 - icon request */
#define SERVER_FINDANONGAME_ICONREPLY           0x44ff
	typedef struct{
		t_bnet_header         h;
		bn_byte               option;                 /* as received from client */
		bn_int                count;                  /* as received from client */
		bn_int                curricon;               /* current icon code */
		bn_byte               table_width;            /* the icon table width */
		bn_byte               table_size;             /* the icon table total size */
		/* table data */
	} PACKED_ATTR() t_server_findanongame_iconreply;

	/***********************************************************************************/
#define SERVER_ANONGAME_SOLO_STR        	0x534F4C4F /* "SOLO" */
#define SERVER_ANONGAME_TEAM_STR        	0x5445414D /* "TEAM" */
#define SERVER_ANONGAME_SFFA_STR        	0x46464120 /* "FFA " */
#define SERVER_ANONGAME_AT2v2_STR       	0x32565332 /* "2VS2" */
#define SERVER_ANONGAME_AT3v3_STR       	0x33565333 /* "3VS3" */
#define SERVER_ANONGAME_AT4v4_STR       	0x34565334 /* "4VS4" */
#define SERVER_ANONGAME_TY_STR			0X54592020 /* "TY  " FIXME-TY: WHAT TO PUT HERE */

#define CLIENT_FINDANONGAME_INFOTAG_URL         0x55524c        /*  URL\0 */
#define CLIENT_FINDANONGAME_INFOTAG_MAP         0x4d4150        /*  MAP\0 */
#define CLIENT_FINDANONGAME_INFOTAG_TYPE        0x54595045      /*  TYPE */
#define CLIENT_FINDANONGAME_INFOTAG_DESC        0x44455343      /*  DESC */
#define CLIENT_FINDANONGAME_INFOTAG_LADR        0x4c414452      /*  LADR */
#define CLIENT_FINDANONGAME_INFOTAG_SOLO        0x534f4c4f      /*  SOLO */
#define CLIENT_FINDANONGAME_INFOTAG_TEAM        0x5445414d      /*  TEAM */
#define CLIENT_FINDANONGAME_INFOTAG_FFA         0x46464120      /*  FFA\20 */

#define ANONGAME_TYPE_1V1       0
#define ANONGAME_TYPE_2V2       1
#define ANONGAME_TYPE_3V3       2
#define ANONGAME_TYPE_4V4       3
#define ANONGAME_TYPE_SMALL_FFA 4
#define ANONGAME_TYPE_AT_2V2    5
#define ANONGAME_TYPE_TEAM_FFA  6
#define ANONGAME_TYPE_AT_3V3    7
#define ANONGAME_TYPE_AT_4V4    8
	/* Added by Omega */
#define ANONGAME_TYPE_TY	9
#define ANONGAME_TYPE_5V5	10
#define ANONGAME_TYPE_6V6	11
#define ANONGAME_TYPE_2V2V2	12
#define ANONGAME_TYPE_3V3V3	13
#define ANONGAME_TYPE_4V4V4	14
#define ANONGAME_TYPE_2V2V2V2	15
#define ANONGAME_TYPE_3V3V3V3	16
#define ANONGAME_TYPE_AT_2V2V2	17

#define ANONGAME_TYPES 18

#define SERVER_FINDANONGAME_PROFILE_UNKNOWN2    0x6E736865 /* Sheep */

	/***********************************************************************************/
	/***********************************************************************************/
	/* This is a blank packet - includes just type and size */
#define CLIENT_ARRANGEDTEAM_FRIENDSCREEN 0x60ff
	typedef struct
	{
		t_bnet_header h;
	} PACKED_ATTR() t_client_arrangedteam_friendscreen;

	/***********************************************************************************/
#define SERVER_ARRANGEDTEAM_FRIENDSCREEN 0x60ff
	typedef struct
	{
		t_bnet_header h;
		bn_byte f_count;
		/* usernames get appended here */
	} PACKED_ATTR() t_server_arrangedteam_friendscreen;

#define SERVER_ARRANGED_TEAM_ADDNAME 0x01

	/***********************************************************************************/
	/*
	0000:   FF 61 1C 00 01 00 00 00   C9 7B A0 02 01 00 00 00    .........{......
	0010:   01 74 72 65 6E 64 65 63   69 64 65 00                .trendecide.
	*/
#define CLIENT_ARRANGEDTEAM_INVITE_FRIEND 0x61ff
	typedef struct
	{
		t_bnet_header	h;
		bn_int		count;
		bn_int		id;
		bn_int		unknown1;	/* 01 00 00 00 */
		bn_byte 	numfriends;	/* next is a byte, that is the number of friends to invite */
		/* usernames get appended here */
	} PACKED_ATTR() t_client_arrangedteam_invite_friend;

	/***********************************************************************************/
#define SERVER_ARRANGEDTEAM_INVITE_FRIEND_ACK 0x61ff
	typedef struct
	{
		t_bnet_header	h;
		bn_int		count;
		bn_int 		id;          /* client id */
		bn_int		timestamp;
		bn_byte 	teamsize;    /* numfriends + 1 */
		bn_int		info[5];
	} PACKED_ATTR() t_server_arrangedteam_invite_friend_ack;

	/***********************************************************************************/
#define SERVER_ARRANGEDTEAM_SEND_INVITE 0x63ff
	typedef struct
	{
		t_bnet_header h;
		bn_int count;
		bn_int id;          /* client id of inviter */
		bn_int inviterip;   /* IP address of the person who invited them into the game */
		bn_short port;      /* Port of the person who invited them into the game */
		bn_byte numfriends; /* Number of friends that got invited to the game */
		/* username of the inviter */
		/* usernames of the others who got invited */
	} PACKED_ATTR() t_server_arrangedteam_send_invite;

	/***********************************************************************************/
#define CLIENT_ARRANGEDTEAM_ACCEPT_DECLINE_INVITE 0x63ff
	typedef struct
	{
		t_bnet_header h;
		bn_int count;
		bn_int id;
		bn_int option;   /* accept or decline */
		/* username of the inviter */
	} PACKED_ATTR() t_client_arrangedteam_accept_decline_invite;

#define CLIENT_ARRANGEDTEAM_ACCEPT		0x00000003
#define CLIENT_ARRANGEDTEAM_DECLINE		0x00000002

	/***********************************************************************************/
#define SERVER_ARRANGEDTEAM_MEMBER_DECLINE 0x62ff
	typedef struct
	{
		t_bnet_header h;
		bn_int count;
		bn_int action; /* number assigned to player? playernum? */
		/* username of the person who declined invitation */
	} PACKED_ATTR() t_server_arrangedteam_member_decline;

#define SERVER_ARRANGEDTEAM_ACCEPT		0x00000003
#define SERVER_ARRANGEDTEAM_DECLINE		0x00000002

	/***********************************************************************************/
	/* not used ?? [Omega] */
	/*
	0000:   FF 64 1C 00 02 00 00 00  00 00 00 00 01 00 00 00     .d..............
	0010:   00 00 00 00 03 00 00 00  00 00 00 00                 ............
	*/
	/* THIS NEEDS FINISHED */
	/*
	#define SERVER_ARRANGEDTEAM_TEAM_STATS 0x64ff
	typedef struct
	{
	t_bnet_header h;
	}
	*/
	/***********************************************************************************/
	/***********************************************************************************/
	/*
	# 144 packet from client: type=0x65ff(unknown) length=4 class=bnet
	0000:   FF 65 04 00                                          .e..
	*/
#define CLIENT_FRIENDSLISTREQ 0x65ff
	typedef struct
	{
		t_bnet_header h;
	} PACKED_ATTR() t_client_friendslistreq;

	/*
	# 158 packet from server: type=0x65ff(unknown) length=16 class=bnet
	0000:   FF 65 10 00     01 66 6F 6F   00 00 00 00 00 00 00 00    .e.. .foo. .......


	*/
#define SERVER_FRIENDSLISTREPLY 0x65ff
	typedef struct
	{
		t_bnet_header h;
		bn_byte friendcount;
		/* 1 byte status, 0-terminated name, 6 bytes unknown, ... */
	} PACKED_ATTR() t_server_friendslistreply;

	typedef struct
	{
		bn_byte status;
		bn_byte location;
		bn_int clienttag;
	} PACKED_ATTR() t_server_friendslistreply_status;

	/*
	# 124 packet from client: type=0x66ff(unknown) length=5 class=bnet
	0000:   FF 66 05 00 00                                       .f...
	*/
	/* FF 66-05 00 00 40 - AT */
#define CLIENT_FRIENDINFOREQ 0x66ff
	typedef struct
	{
		t_bnet_header h;
		bn_byte friendnum;
	} PACKED_ATTR() t_client_friendinforeq;

	/*
	# 126 packet from server: type=0x66ff(unknown) length=12 class=bnet
	0000:   FF 66 0C 00 00 00 00 00   00 00 00 00                .f..........

	Arranged Team sends this to each inviter
	FF 66-1A 00 00 01 03 33 52 41   ..g....f.....3RA
	0x0040   57 41 72 72 61 6E 67 65-64 20 54 65 61 6D 73 00   WArranged Teams.
	and this to the inviter
	FF 66-18 00 00 01 02 33 52 41         ..i....f.....3RA
	0x0040   57 57 61 72 63 72 61 66-74 20 49 49 49 00         WWarcraft III.
	*/

#define SERVER_FRIENDINFOREPLY 0x66ff
	typedef struct
	{
		t_bnet_header h;
		bn_byte friendnum;
		bn_byte type;
		bn_byte status;
		bn_int clienttag;
		/* game name */
	} PACKED_ATTR() t_server_friendinforeply;
#define FRIEND_TYPE_NON_MUTUAL 0x00
#define FRIEND_TYPE_MUTUAL     0x01
#define FRIEND_TYPE_DND	       0x02
#define FRIEND_TYPE_AWAY       0x04

	/******************************************************/
	/*
	# 126 packet from server: type=0x67ff(unknown) length=15 class=bnet
	0000:   FF 67 0F 00 66 6F 6F 00   00 00 00 00 00 00 00       .g..foo........
	*/
#define SERVER_FRIENDADD_ACK 0x67ff
	typedef struct
	{
		t_bnet_header h;
		/* friend name, status */
	} PACKED_ATTR() t_server_friendadd_ack;
	/******************************************************/
	/*
	# 114 packet from server: type=0x68ff(unknown) length=5 class=bnet
	0000:   FF 68 05 00 01                                       .h...
	*/
#define SERVER_FRIENDDEL_ACK 0x68ff
	typedef struct
	{
		t_bnet_header h;
		bn_byte friendnum;
	} PACKED_ATTR() t_server_frienddel_ack;
	/******************************************************/

#define SERVER_FRIENDMOVE_ACK 0x69ff
	typedef struct
	{
		t_bnet_header h;
		bn_byte pos1;
		bn_byte pos2;
	} PACKED_ATTR() t_server_friendmove_ack;

#define FRIENDSTATUS_OFFLINE    	0x00
#define FRIENDSTATUS_ONLINE     	0x01
#define FRIENDSTATUS_CHAT       	0x02
#define FRIENDSTATUS_PUBLIC_GAME	0x03
#define FRIENDSTATUS_PRIVATE_GAME	0x05

}

#endif
