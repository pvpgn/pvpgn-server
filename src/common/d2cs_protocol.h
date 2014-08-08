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
#ifndef INCLUDED_D2CS_PROTOCOL_H
#define INCLUDED_D2CS_PROTOCOL_H

#include "common/bn_type.h"

namespace pvpgn
{

	typedef struct
	{
		bn_short		size;
		bn_byte			type;
	} t_d2cs_client_header;

	typedef struct
	{
		t_d2cs_client_header	h;
	} t_d2cs_client_generic;

#define CLIENT_D2CS_LOGINREQ			0x01
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_int			seqno;
		bn_int			u1;
		bn_int			bncs_addr1;
		bn_int			sessionnum;
		bn_int			sessionkey;			/* always zero,self define */
		bn_int			cdkey_id;
		bn_int			u5;
		bn_int			clienttag;
		bn_int			bnversion;
		bn_int			bncs_addr2;
		bn_int			u6;	/* zero */
		bn_int			secret_hash[5];
		/* account name */
	} t_client_d2cs_loginreq;

#define D2CS_CLIENT_LOGINREPLY			0x01
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_int			reply;
	} t_d2cs_client_loginreply;
#define D2CS_CLIENT_LOGINREPLY_SUCCEED			0x00
#define D2CS_CLIENT_LOGINREPLY_BADPASS			0x0c

#define CLIENT_D2CS_CREATECHARREQ		0x02
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		chclass;	/* character class */
		bn_short		u1;		/* always zero */
		bn_short		status;		/* same as in .d2s file */
		/* character name */
	} t_client_d2cs_createcharreq;

#define D2CS_CLIENT_CREATECHARREPLY		0x02
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_int			reply;
	} t_d2cs_client_createcharreply;
#define D2CS_CLIENT_CREATECHARREPLY_SUCCEED		0x00
#define D2CS_CLIENT_CREATECHARREPLY_FAILED		0x01
#define D2CS_CLIENT_CREATECHARREPLY_ALREADY_EXIST	0x14
#define D2CS_CLIENT_CREATECHARREPLY_NAME_REJECT		0x15


#define CLIENT_D2CS_CREATEGAMEREQ		0x03
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		seqno;
		bn_int			gameflag;		/* just difficulty is set here */
		bn_byte			u1;			/* always 1 */
		bn_byte			leveldiff;		/* Only allow people of +/- n level to join */
		bn_byte			maxchar;		/* Maximum number of chars allowed in game */
		/* game name */
		/* game pass */
		/* game desc */
	} t_client_d2cs_creategamereq;

#define D2CS_CLIENT_CREATEGAMEREPLY		0x03
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		seqno;
		bn_short		gameid;
		bn_short		u1;			/* always zero */
		bn_int			reply;
	} t_d2cs_client_creategamereply;
#define D2CS_CLIENT_CREATEGAMEREPLY_SUCCEED		0x00
#define D2CS_CLIENT_CREATEGAMEREPLY_FAILED		0x01
#define D2CS_CLIENT_CREATEGAMEREPLY_INVALID_NAME	0x1e
#define D2CS_CLIENT_CREATEGAMEREPLY_NAME_EXIST		0x1f
#define D2CS_CLIENT_CREATEGAMEREPLY_SERVER_DOWN		0x20
#define D2CS_CLIENT_CREATEGAMEREPLY_NOT_AVAILABLE	0x32
#define D2CS_CLIENT_CREATEGAMEREPLY_U1			0x33

#define CLIENT_D2CS_JOINGAMEREQ			0x04
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		seqno;
		/* game name */
		/* game pass */
	} t_client_d2cs_joingamereq;

#define D2CS_CLIENT_JOINGAMEREPLY		0x04
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		seqno;
		bn_short		gameid;
		bn_short		u1;			/* always zero */
		bn_int			addr;
		bn_int			token;
		bn_int			reply;
	} t_d2cs_client_joingamereply;
#define D2CS_CLIENT_JOINGAMEREPLY_SUCCEED		0x00
#define D2CS_CLIENT_JOINGAMEREPLY_BAD_PASS		0x29
#define D2CS_CLIENT_JOINGAMEREPLY_NOT_EXIST		0x2a
#define D2CS_CLIENT_JOINGAMEREPLY_GAME_FULL		0x2b
#define D2CS_CLIENT_JOINGAMEREPLY_LEVEL_LIMIT		0x2c
#define D2CS_CLIENT_JOINGAMEREPLY_HARDCORE_SOFTCORE	0x71
#define D2CS_CLIENT_JOINGAMEREPLY_NORMAL_NIGHTMARE	0x73
#define D2CS_CLIENT_JOINGAMEREPLY_NIGHTMARE_HELL	0x74
#define D2CS_CLIENT_JOINGAMEREPLY_CLASSIC_EXPANSION	0x78
#define D2CS_CLIENT_JOINGAMEREPLY_EXPANSION_CLASSIC	0x79
#define D2CS_CLIENT_JOINGAMEREPLY_NORMAL_LADDER		0x7D
#define D2CS_CLIENT_JOINGAMEREPLY_FAILED		0x01

#define CLIENT_D2CS_GAMELISTREQ				0x05
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		seqno;
		bn_int			gameflag;		/* only hardcore is set here */
		/* bn_byte		u1;			string game name to search? */
	} t_client_d2cs_gamelistreq;

#define D2CS_CLIENT_GAMELISTREPLY		0x05
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		seqno;
		bn_int			token;
		bn_byte			currchar;		/* current number of characters in game */
		bn_int			gameflag;		/* 0x04 is always set here */
		/* game name */
		/* game desc */
	} t_d2cs_client_gamelistreply;

#define CLIENT_D2CS_GAMEINFOREQ			0x06
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		seqno;
		/* game name */
	} t_client_d2cs_gameinforeq;

#define D2CS_CLIENT_GAMEINFOREPLY		0x06
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		seqno;
		bn_int			gameflag;
		bn_int			etime;
		bn_byte			charlevel;
		bn_byte			leveldiff;
		bn_byte			maxchar;
		bn_byte			currchar;
		bn_byte			chclass[16];		/* 16 character class */
		bn_byte			level[16];		/* 16 character level */
		/* game description */
		/* currchar number of character names */
	} t_d2cs_client_gameinforeply;

#define CLIENT_D2CS_CHARLOGINREQ		0x07
	typedef struct
	{
		t_d2cs_client_header	h;
		/* character name */
	} t_client_d2cs_charloginreq;

#define D2CS_CLIENT_CHARLOGINREPLY		0x07
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_int			reply;
	} t_d2cs_client_charloginreply;
#define D2CS_CLIENT_CHARLOGINREPLY_SUCCEED		0x00
#define D2CS_CLIENT_CHARLOGINREPLY_FAILED		0x01
#define D2CS_CLIENT_CHARLOGINREPLY_NOT_FOUND		0x46
#define D2CS_CLIENT_CHARLOGINREPLY_EXPIRED		0x7b

#define CLIENT_D2CS_DELETECHARREQ		0x0a
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		u1;			/* always zero */
		/* character name */
	} t_client_d2cs_deletecharreq;

#define D2CS_CLIENT_DELETECHARREPLY		0x0a
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		u1;			/* always zero */
		bn_int			reply;
	} t_d2cs_client_deletecharreply;
#define D2CS_CLIENT_DELETECHARREPLY_SUCCEED		0x00
#define D2CS_CLIENT_DELETECHARREPLY_FAILED		0x01


#define CLIENT_D2CS_LADDERREQ			0x11
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_byte			type;			/* jadder type request */
		bn_short		start_pos;		/* list ladder from what position */
	} t_client_d2cs_ladderreq;

#define D2CS_CLIENT_LADDERREPLY			0x11
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_byte			type;			/* ladder type request */
		bn_short		total_len;		/* total length of the ladder data */

		bn_short		curr_len;		/* length of ladder data in this packet */
		bn_short		cont_len;		/* length of ladder data in previous packets */
		/* length here include the header but
		exclude this packet */
	} t_d2cs_client_ladderreply;

	typedef struct
	{
		bn_short		start_pos;		/* start position of ladder */
		bn_short		u1;			/* always zero */
		bn_int			count1;			/* always 0x10 */
	} t_d2cs_client_ladderheader;

	typedef struct
	{
		bn_int			count2;			/* 0x10 for first packet or 0x0 for continue */
	} t_d2cs_client_ladderinfoheader;

	typedef struct
	{
		bn_int			explow;
		bn_int			exphigh;		/* always zero */
		bn_short		status;
		bn_byte			level;
		bn_byte			u1;			/* always zero */
		char			charname[16];
	} t_d2cs_client_ladderinfo;
#define LADDERSTATUS_FLAG_DEAD		0x10
#define LADDERSTATUS_FLAG_HARDCORE	0x20
#define LADDERSTATUS_FLAG_EXPANSION	0x40
#define LADDERSTATUS_FLAG_DIFFICULTY	0x0f00

#define CLIENT_D2CS_MOTDREQ			0x12
	typedef struct
	{
		t_d2cs_client_header	h;
	} t_client_d2cs_motdreq;

#define D2CS_CLIENT_MOTDREPLY			0x12
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_byte			u1;
		/* message */
	} t_d2cs_client_motdreply;

#define CLIENT_D2CS_CANCELCREATEGAME		0x13
	typedef struct
	{
		t_d2cs_client_header	h;
	} t_client_d2cs_cancelcreategame;

#define D2CS_CLIENT_CREATEGAMEWAIT		0x14
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_int			position;
	} t_d2cs_client_creategamewait;

#define D2CS_CLIENT_CHARLADDERREQ		0x16
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_int			hardcore;
		bn_int			expansion;
		/* character name */
	} t_client_d2cs_charladderreq;
	/* use 0x11 LADDER reply for this request */

#define CLIENT_D2CS_CHARLISTREQ			0x17
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		maxchar;
		bn_short		u1;
	} t_client_d2cs_charlistreq;

#define D2CS_CLIENT_CHARLISTREPLY		0x17
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		maxchar;
		bn_short		currchar;
		bn_short		u1;			/* always zero */
		bn_short		currchar2;
		/* character name */
		/* character portrait blocks */
		/* each block is 0x22 bytes static length */
	} t_d2cs_client_charlistreply;

#define CLIENT_D2CS_CONVERTCHARREQ		0x18
	typedef struct
	{
		t_d2cs_client_header	h;
		/* character name */
	} t_client_d2cs_convertcharreq;

#define D2CS_CLIENT_CONVERTCHARREPLY		0x18
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_int			reply;
	} t_d2cs_client_convertcharreply;
#define D2CS_CLIENT_CONVERTCHARREPLY_SUCCEED		0x00
#define D2CS_CLIENT_CONVERTCHARREPLY_FAILED		0x01

#define CLIENT_D2CS_CHARLISTREQ_110		0x19
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		maxchar;
		bn_short		u1;
	} t_client_d2cs_charlistreq_110;


#define D2CS_CLIENT_CHARLISTREPLY_110		0x19
	typedef struct
	{
		t_d2cs_client_header	h;
		bn_short		maxchar;
		bn_short		currchar;
		bn_short		u1;			/* always zero */
		bn_short		currchar2;
	} t_d2cs_client_charlistreply_110;

	typedef struct
	{
		bn_int			expire_time;		/* character expire time (in seconds) */
		/* character name */
		/* character portrait blocks */
		/* each block is 0x22 bytes static length */
	} t_d2cs_client_chardata;

}

#endif
