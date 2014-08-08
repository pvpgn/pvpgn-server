/*
 * Copyright (C) 2000  Otto Chan (kenshin_@hotmail.com)
 * Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2000  Onlyer (onlyer@263.net)
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
#ifndef INCLUDED_D2GAME_PROTOCOL_TYPES
#define INCLUDED_D2GAME_PROTOCOL_TYPES

#ifdef JUST_NEED_TYPES
# include "bn_type.h"
#else
# define JUST_NEED_TYPES
# include "bn_type.h"
# undef JUST_NEED_TYPES
#endif


namespace pvpgn
{

	/*
	 * The protocol for communicating between a Diablo II client
	 * and game server.
	 * FIXME: put the #define's into the PROTO section
	 */


	/******************************************************/
	typedef struct
	{
#ifdef NOTONLYER
		bn_short magic;
		bn_short size;
		bn_byte  type;
		/* FIXME: the two sets of packets don't jive... did the beta have a different
		   game protocol or am I just confused (could easily be the later). */
#else
		bn_byte type;
#endif
	} t_d2game_header;
	/******************************************************/


	/******************************************************/
	typedef struct
	{
		t_d2game_header h;
	} t_d2game_generic;
	/******************************************************/


	/******************************************************/
	/*
	D2 00 05 00 00
	*/
#define SERVER_00 0x00 /* beta-only? */
	typedef struct
	{
		t_d2game_header h;
		bn_byte         unknown1; /* data request #? */
	} PACKED_ATTR() t_server_00_req;
#define SERVER_00 0x00
	/******************************************************/


	/******************************************************/
	/*
	D2 00 20 00 01 E2 5E 13   BE 4C 02 04 CA 00 00 00   .........L......
	6C 69 6C 6A 6F 65 00 00   64 25 42 00 1C E9 50 00   liljoe..d.B...P.
	*/
#define CLIENT_01 0x01 /* beta-only? */
	typedef struct /* game select? */
	{
		t_d2game_header h;
		bn_byte         unknown1; /* data reply #? */
		bn_int          gameid1; /* same as in auth 04 reply */
		bn_short        gameid2; /* same as in auth 04 reply */
		bn_byte         unknown2[5];
		/* character name */
		/* 00 64 25 42 00 1C E9 50 00 unknown... string or numeric? */
	} PACKED_ATTR() t_client_01;
	/******************************************************/


	/******************************************************/
	/*
	0030  22 38 32 5D 00 00 97 20  20 20 20 20               "82]........
	*/
#define D2GAME_SERVER_WELCOME			    0x97
	typedef struct
	{
		t_d2game_header h;
	} t_d2game_server_welcome;


	/*
	0000:   60 00 27 04 18 79 27 04   A8 00 00 00 07 02 00 00    `.'..y'.........
	0010:   B0 01 01 00 00 6F 6E 6C   79 65 72 00 B8 6A F7 BF    .....onlyer..j..
	0020:   00 00 00 00 34 00 00 04   00 00 00 00 00             ....4........

	0000:   60 00 27 04 18 79 27 04   30 0D 00 00 07 02 00 00    `.'..y'.0.......
	0010:   B0 01 04 00 01 41 4C 42   45 52 54 00 B8 6A F7 BF    .....ALBERT..j..
	0020:   00 00 00 00 34 00 00 04   00 00 00 00 00             ....4........

	0000:   60 00 27 04 18 79 27 04   A0 0A 00 00 07 02 00 00    `.'..y'.........
	0010:   B0 01 04 00 02 41 4C 42   45 52 54 00 B8 6A F7 BF    .....ALBERT..j..
	0020:   00 00 00 00 34 00 00 04   00 00 00 00 00             ....4........

	0000:   60 00 AB 04 18 79 AB 04   DE 00 9F 00 0C 02 00 00    `....y..........
	0010:   60 01 04 00 00 62 62 62   00 71 DF 77 A6 C0 E6 77    `....bbb.q.w...w
	0020:   A6 C0 E6 77 34 05 00 04   00 00 00 00 00             ...w4........

	*/


#define D2GAME_CLIENT_CREATEGAMEREQ 0x60

	typedef bn_basic   bn_charname[16];

	typedef struct
	{
		t_d2game_header    h;
		char		    gamename[16];
		bn_byte	    servertype;		/* servertype=0, data not send to client and have no host */
		/* creator changed to newbie data saved in server machine*/
		/* servertype=1, data send to client and have host */
		/* servertype=2, data send to client and have no host */
		/* servertype=3, client all newbie,seems to be wrong */
		/* data is saved in server machine */
		/* client is loaded from server machine */
		bn_byte	    chclass;
		bn_byte	    chtemplate;		/* character template */
		/* affect the reply in 0x01 */
		/* should less than the lines of excel/CharTemplate.txt */
		bn_byte	    difficulty;
		bn_charname	    charname;
		bn_short	    arena;
		bn_int		    gameflag;
		bn_byte	    unknownb2;		/* unused */
		bn_byte	    unknownb3;		/* unused */
	} t_d2game_client_creategamereq;


	typedef struct
	{
		bn_byte	flag1;		    /* "test gameflag,06" should not be zero,or will be bad gameflag */
		/* flag1 should be set with bit 0x04 or will fail or crash	*/
		/* bit 0x10,0x20,0x08 is ignored in reply */

		bn_byte	flag2;		    /* hardcore and softcore */
		/* 0x0 means softcore,0x1 template mode,0x8 means hardcore */
		/* bit 0x02,0x04,0xF0 is ignored in reply  */
		/* have sth to do with char template */
		/* opengame */

		bn_byte	flag3;		    /* guild data (not in reply) */
		/* 0x01 means have guild */
		/* others bits seems all unused */

		bn_byte	flag4;		    /* seems to be unused */

	} t_d2game_gameflag;



	/*
	0030  22 37 86 D7 00 00 61 65  00 00 00 00 00 01 03 00   "7....ae........
	0040  00 00 6F 6E 6C 79 65 72  2D 63 6E 61 61 00 24 E0   ..onlyer-cnaa.$.
	0050  7B 05                                              {.
	*/

#define D2GAME_CLIENT_JOINGAMEREQ		    0x61
	typedef struct
	{
		t_d2game_header  h;
		bn_int	    token;
		bn_short	    gameid;
		bn_byte	    charclass;	    /* 00=Amazon 01=Sor 02=Nec 03=Pal 04=Bar */
		bn_int	    version;
		bn_charname	    charname;
		/*	    16 bytes playe name	(including 0x0 ending)    */
	} t_d2game_client_joingamereq;


	/*
	0030  22 1C C9 65 00 00 20 20  20 20 20 20               "..e........
	*/

#define D2GAME_SERVER_NOOP			    0x20
	typedef struct
	{
		t_d2game_header  h;
		/*	5 \x20 noop	*/
	} t_d2game_server_noop;



	/*
	0030  22 37 17 07 00 00 66 91  4B A1 00 00 00 00 00      "7....f.K......
	*/
#define D2GAME_CLIENT_UNKNOWN_66		    0x66    /* echo message? */
	typedef struct
	{
		t_d2game_header  h;
		bn_int	    unknown1;
		bn_int	    unknown2;
	} t_d2game_client_unknown_66;

	/*
	0000:   8F 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
	0010:   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
	0020:   00                                                   .
	*/

#define D2GAME_SERVER_UNKNOWN_8F		    0x8f    /* echo reply? */
	typedef struct
	{
		t_d2game_header	h;
		bn_int		unknown1;
		bn_int		unknown2;
		bn_int		unknown3;
		bn_int		unknown4;
		bn_int		unknown5;
		bn_int		unknown6;
		bn_int		unknown7;	/* all here for open char is zero */
	} t_d2game_server_unknown_8f;



	/*
	0030  22 1C C4 53 00 00 01 00  04 00 00 00 00 02         "..S..........
	*/
#define D2GAME_SERVER_JOINOK			    0x01
	/* this message will appear after
	 * 1. CLIENT_CREATEGAEM valid
	 * 2. CLIENT_JOINGAME valid
	 */
	typedef struct
	{
		t_d2game_header  h;
		bn_byte	     difficulty;
		bn_short	     gameflag;
		bn_byte	     chtemplate;
		bn_short	     unknown1;
		bn_short	     unknown2;
	} t_d2game_server_joinok;


	/*
	0030  22 2F 65 38 00 00 64                               "/e8..d
	*/

#define D2GAME_CLIENT_JOINACTREQ		    0x64
	typedef struct
	{
		t_d2game_header  h;
		/* none */
	} t_d2game_client_joinactreq;


#define D2GAME_SERVER_UNKNOWN_59		    0x59
	typedef struct
	{
		t_d2game_header  h;
	} t_d2game_server_unknown_59;


	/*
	0030  22 0E 0C 01 00 00 65 82  82 00 00 00 55 AA 55 AA   ".....e.....U.U.
	0040  47 00 00 00 6F 6E 6C 79  65 72 2D 63 6E 61 00 00   G...onlyer-cna..
	0050  00 00 00 00 01 00 00 00  DD 00 10 00 82 00 01 00   ................
	0060  01 00 01 01 01 01 01 FF  FF FF 01 01 FF FF FF FF   ................
	0070  FF FF FF FF FF FF FF FF  FF FF FF FF FF FF FF FF   ................
	0080  FF FF 00 00 00 00 00 00  00 00 00 00 00 00 00 00   ................
	0090  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00   ................
	00A0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00   ................
	00B0  00 00 00 00 00 00 00 00  00 00 00 00 00 00 FC      ...............
	*/


#define D2GAME_CLIENT_PLAYERSAVE	    0x65
	typedef struct
	{
		t_d2game_header	h;
		bn_byte		size;
		bn_int		total_size;
		/*	    player save file	*/
		/*	    an append	    */
	} t_d2game_client_playersave;


#define D2GAME_SERVER_ERROR			    0x9c
	typedef struct
	{
		t_d2game_header	h;
		bn_int		errorno;
	} t_d2game_server_error;

#define D2GAME_SERVER_ERROR_UNKNOWN_FAILURE	    0	    /* error biger than 22 is also unknown */
#define D2GAME_SERVER_ERROR_CHAR_VER		    1
#define D2GAME_SERVER_ERROR_QUEST_DATA		    2
#define D2GAME_SERVER_ERROR_WP_DATA		    3
#define D2GAME_SERVER_ERROR_STAT_DATA		    4
#define D2GAME_SERVER_ERROR_SKILL_DATA		    5
#define D2GAME_SERVER_ERROR_UNABLE_ENTER	    6
#define D2GAME_SERVER_ERROR_INVENTORY_DATA	    7
#define D2GAME_SERVER_ERROR_DEAD_BODY		    8
#define D2GAME_SERVER_ERROR_HEADER		    9
#define D2GAME_SERVER_ERROR_HIREABLES		    10
#define D2GAME_SERVER_ERROR_INTRO_DATA		    11
#define D2GAME_SERVER_ERROR_ITEM		    12
#define D2GAME_SERVER_ERROR_DEAD_BODY_ITEM	    13
#define D2GAME_SERVER_ERROR_GENERIC_BAD_FILE	    14
#define D2GAME_SERVER_ERROR_GAME_FULL		    15
#define D2GAME_SERVER_ERROR_GAME_VER		    16
#define D2GAME_SERVER_ERROR_NIGHTMARE		    17
#define D2GAME_SERVER_ERROR_HELL    		    18
#define D2GAME_SERVER_ERROR_NORMAL_HARDCORE	    19
#define D2GAME_SERVER_ERROR_HARDCORE_NORMAL	    20
#define D2GAME_SERVER_ERROR_DEAD_HARDCORE	    21
	/*
	0000:   15 01 00 69 20 77 69 6C   6C 20 67 6F 00 00 00       ...i will go...

	0000:   15 01 00 74 68 61 6E 6B   73 20 66 6F 72 20 79 6F    ...thanks for yo
	0010:   75 72 20 68 65 6C 70 00   00 00                      ur help...

	0000:   15 01 00 73 65 65 20 75   00 00 00                   ...see u...


	*/

#define D2GAME_CLIENT_CHAT_MESSAGE		    0x15
	typedef struct
	{
		t_d2game_header	h;
		bn_short		unknown1;
		/* chat message */
	} t_d2game_client_chat_message;


	/*
	0000:   26 01 00 02 00 00 00 00   00 01 6F 6E 6C 79 65 72    &.........onlyer
	0010:   2D 63 6E 61 61 00 69 20   77 69 6C 6C 20 67 6F 00    -cnaa.i will go.

	0000:   26 01 00 02 00 00 00 00   00 01 6F 6E 6C 79 65 72    &.........onlyer
	0010:   2D 63 6E 61 61 00 74 68   61 6E 6B 73 20 66 6F 72    -cnaa.thanks for
	0020:   20 79 6F 75 72 20 68 65   6C 70 00                    your help.

	0000:   26 01 00 02 00 00 00 00   00 01 6F 6E 6C 79 65 72    &.........onlyer
	0010:   2D 63 6E 61 61 00 73 65   65 20 75 00                -cnaa.see u.

	0000:   26 01 00 02 00 00 00 00   00 17 63 63 00 62 79 65    &.........cc.bye
	0010:   00 67 49 00 00 00 01 86   17 32 12 01 00 07 00 05    .gI......2......

	*/

#define D2GAME_SERVER_CHAT_MESSAGE		    0x26
	typedef struct
	{
		t_d2game_header	h;
		bn_short		unknown1;
		bn_int		unknown2;
		bn_short		unknown3;
		bn_byte		unknown4;   /* id or token? a fixed number for each char*/
		/* player name */
		/* message */
	} t_d2game_server_chat_message;

#define D2GAME_SERVER_CHAT_MESSAGE_UNKNOWN1	    0x0001
#define D2GAME_SERVER_CHAT_MESSAGE_UNKNOWN2	    0x00000002
#define D2GAME_SERVER_CHAT_MESSAGE_UNKNOWN3	    0x0000
#define D2GAME_SERVER_CHAT_MESSAGE_UNKNOWN4	    0x01


	/*
	0000:   62                                                   b
	*/

#define D2GAME_CLIENT_QUITGAME				    0x62
	typedef struct
	{
		t_d2game_header	h;
	} t_d2game_client_quitgame;


	/*
	0000:   9B FF 01 4E 03 00 00 55   AA 55 AA 47 00 00 00 6F    ...N...U.U.G...o
	0010:   6E 6C 79 65 72 2D 63 6E   61 61 00 00 00 00 00 00    nlyer-cnaa......
	0020:   00 00 00 DD 00 10 00 82   00 01 00 01 00 FF FF FF    ................
	0030:   FF FF 53 FF FF FF FF FF   FF FF FF FF FF FF FF FF    ..S.............
	0040:   FF FF FF FF FF FF FF FF   FF FF FF FF FF FF 00 FF    ................
	0050:   00 FF 00 FF 00 FF 00 FF   00 FF 00 FF 00 00 24 00    ..............$.
	0060:   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
	0070:   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
	0080:   00 00 00 00 00 5E 17 41   5F 57 6F 6F 21 06 00 00    .....^.A_Woo!...
	0090:   00 2A 01 01 00 00 00 00   00 00 00 00 00 00 00 00    .*..............
	00A0:   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
	00B0:   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
	00C0:   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
	00D0:   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
	00E0:   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
	00F0:   00 00 00 00 00 00 00 00   00 00 00 00 00 00 00 00    ................
	0100:   00 00 00 00 00 00                                    ......




	*/

#define D2GAME_SERVER_PLAYERSAVE		    0x9b
	typedef struct
	{
		t_d2game_header	h;
		bn_byte		size;
		bn_byte		start;
		bn_int		total_size;
		/* player save file */
	} t_d2game_server_playersave;


	/*
	0000:	98 05 06
	*/

#define D2GAME_SERVER_CLOSEGAME		    0x98
	typedef struct
	{
		t_d2game_header	 h;
		bn_short		 unknown1;
	} t_d2game_server_closegame;

	/*
	0000:   5A 02 04 00 00 00 00 03   61 72 63 68 5F 6E 61 67    Z.......arch_nag
	0010:   61 00 00 00 B0 FD B6 08   00 FF FF FF 78 07 39 04    a...........x.9.
	0020:   D5 16 2D 04 01 FD B6 08                              ..-.....
	*/

#define D2GAME_SERVER_JOINGAME_MESSAGE	    0x5a
	typedef struct
	{
		t_d2game_header	h;
		bn_byte		unknown1;
		bn_byte		unknown2;
		bn_int		unknown3;
		bn_byte	        unknown4;
		/* char name */
	} t_d2game_joingame_message;



#define D2GAME_CLIENT_DIE		    0x41
	typedef struct
	{
		t_d2game_header	h;
	} t_d2game_client_die;


	/*
	0000:   02 73 00 00 00 4E 00 8B   0F 7B 14 00 00             .s...N...{...

	#define D2GAME_SERVER_UNKNOWN_2			    0x2

	*/

	/*
	0000:   67 06 00 00 00 01 B2 0F   6B 14 01 00 07 00 05       g.......k......

	0000:   67 06 00 00 00 01 B2 0F   71 14 01 00 07 00 05       g.......q......

	0000:   67 0A 00 00 00 01 72 0F   63 14 01 00 07 00 05       g.....r.c......


	#define D2GAME_SERVER_UNKNOWN_67		    0x67

	*/


	/*
	0000:   6D 0A 00 00 00 76 0F 64   14 80                      m....v.d..

	0000:   6D 0A 00 00 00 72 0F 63   14 80                      m....r.c..

	0000:   6D 06 00 00 00 B2 0F 71   14 80 8A 01 0B 00 00 00    m......q........

	0000:   6D 06 00 00 00 B2 0F 6B   14 80                      m......k..



	#define D2GAME_SERVER_UNKNOWN_6D		    0x6d

	*/

	/*
	0000:   8A 01 0B 00 00 00 6D 0B   00 00 00 9B 0F 70 14 80    ......m......p..
	0010:   2C 01 0B 00 00 00 11 00                              ,.......

	0000:   8A 01 0B 00 00 00                                    ......


	#define D2GAME_SERVER_UNKNOWN_8A		    0x8a

	*/


	/*
	0000:   96 59 80 CC 07 36 8A 4C   36                         .Y...6.L6

	#define D2GAME_SERVER_UNKNOWN_96		    0x96

	*/

	/*
	0000:   24 62 00 00 00                                       $b...

	#define D2GAME_CLIENT_UNKNOWN_24		    0x24

	*/


	/*
	CLIENT:

	0000:   2F 01 00 00 00 49 00 00   00                         /....I...

	0000:   31 49 00 00 00 00 00 00   00                         1I.......

	0000:   30 01 00 00 00 49 00 00   00                         0....I...

	0000:   03 8C 17 2E 12                                       .....

	0000:   03 87 17 27 12                                       ...'.	command ?



	SERVER:


	*/
}

#endif
