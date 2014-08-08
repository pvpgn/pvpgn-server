/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
 * Copyright (C) 2001		faster	(lqx@cic.tsinghua.edu.cn)
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
#ifndef INCLUDED_D2CS_D2GS_PROTOCOL_H
#define INCLUDED_D2CS_D2GS_PROTOCOL_H
/*
 * there is only little comments in this header file
 * check the documents for more details
 */

/* include your own header files here */
#include "common/bn_type.h"

#ifdef D2GS
# include "bn_types.h"
#pragma pack(push, pack01, 1)
#endif

namespace pvpgn
{

	typedef struct
	{
		bn_short	size;
		bn_short	type;
		bn_int		seqno;			/* seqno, set by the sender */
	} t_d2cs_d2gs_header;

	typedef struct
	{
		t_d2cs_d2gs_header	h;
	} t_d2cs_d2gs_generic;

#define D2CS_D2GS_AUTHREQ		0x10
	typedef struct
	{
		t_d2cs_d2gs_header	h;
		bn_int			sessionnum;
		bn_int			signlen;
		/* realm name */
		/* key check sum, maybe 128 bytes */
	} t_d2cs_d2gs_authreq;

#define D2GS_D2CS_AUTHREPLY		0x11
	typedef struct
	{
		t_d2cs_d2gs_header	h;
		bn_int			version;
		bn_int			checksum;
		bn_int			randnum;
		bn_int			signlen;
		bn_basic		sign[128];
	} t_d2gs_d2cs_authreply;

#define D2CS_D2GS_AUTHREPLY		0x11
	typedef struct
	{
		t_d2cs_d2gs_header	h;
		bn_int			reply;
	} t_d2cs_d2gs_authreply;
#define D2CS_D2GS_AUTHREPLY_SUCCEED		0x00
#define D2CS_D2GS_AUTHREPLY_BAD_VERSION		0x01
#define D2CS_D2GS_AUTHREPLY_BAD_CHECKSUM	0x02

#define D2GS_D2CS_SETGSINFO		0x12
	typedef struct
	{
		t_d2cs_d2gs_header	h;
		bn_int			maxgame;
		bn_int			gameflag;
	} t_d2gs_d2cs_setgsinfo;

#define D2CS_D2GS_SETGSINFO		0x12
	typedef struct
	{
		t_d2cs_d2gs_header      h;
		bn_int                  maxgame;
		bn_int                  gameflag;
	} t_d2cs_d2gs_setgsinfo;

#define D2CS_D2GS_ECHOREQ		0x13
	typedef struct
	{
		t_d2cs_d2gs_header	h;
	} t_d2cs_d2gs_echoreq;

#define D2GS_D2CS_ECHOREPLY		0x13
	typedef struct
	{
		t_d2cs_d2gs_header	h;
	} t_d2gs_d2cs_echoreply;

#define D2CS_D2GS_CONTROL		0x14
	typedef struct
	{
		t_d2cs_d2gs_header      h;
		bn_int                  cmd;
		bn_int                  value;
	} t_d2cs_d2gs_control;
#define D2CS_D2GS_CONTROL_CMD_RESTART	0x01
#define D2CS_D2GS_CONTROL_CMD_SHUTDOWN	0x02
#define D2CS_D2GS_CONTROL_VALUE_DEFAULT	0x00

#define D2CS_D2GS_SETINITINFO		0x15
	typedef struct
	{
		t_d2cs_d2gs_header	h;
		bn_int			time;
		bn_int			gs_id;
		bn_int			ac_version;
		/* ac_checksum */
	} t_d2cs_d2gs_setinitinfo;

#define D2CS_D2GS_SETCONFFILE		0x16
	typedef struct
	{
		t_d2cs_d2gs_header	h;
		bn_int			size;
		bn_int			reserved1;
		/* conf file (null terminated string) */
	} t_d2cs_d2gs_setconffile;

#define D2CS_D2GS_CREATEGAMEREQ		0x20
	typedef struct
	{
		t_d2cs_d2gs_header	h;
		bn_byte			ladder;
		bn_byte			expansion;
		bn_byte			difficulty;
		bn_byte			hardcore;
		/* gamename */
		/* gamepass */
		/* gamedesc */
		/* create by acctname */
		/* create by charname */
		/* create by ip address(string) */
	} t_d2cs_d2gs_creategamereq;
#define D2GAME_DIFFICULTY_NORMAL	0
#define D2GAME_DIFFICULTY_NIGHTMARE	1
#define D2GAME_DIFFICULTY_HELL		2

#define D2GS_D2CS_CREATEGAMEREPLY	0x20
	typedef struct
	{
		t_d2cs_d2gs_header	h;
		bn_int			result;
		bn_int			gameid;
	} t_d2gs_d2cs_creategamereply;
#define D2GS_D2CS_CREATEGAME_SUCCEED		0
#define D2GS_D2CS_CREATEGAME_FAILED		1


#define D2CS_D2GS_JOINGAMEREQ		0x21
	typedef struct
	{
		t_d2cs_d2gs_header	h;
		bn_int			gameid;
		bn_int			token;
		/* character name */
		/* account name */
		/* client ip address(string) */
	} t_d2cs_d2gs_joingamereq;

#define D2GS_D2CS_JOINGAMEREPLY		0x21
	typedef struct
	{
		t_d2cs_d2gs_header	h;
		bn_int			result;
		bn_int			gameid;
	} t_d2gs_d2cs_joingamereply;
#define D2GS_D2CS_JOINGAME_SUCCEED			0
#define D2GS_D2CS_JOINGAME_FAILED			1
#define D2GS_D2CS_JOINGAME_GAME_FULL			2

#define D2GS_D2CS_UPDATEGAMEINFO	0x22
	typedef struct
	{
		t_d2cs_d2gs_header	h;
		bn_int			flag;
		bn_int			gameid;
		bn_int			charlevel;
		bn_int			charclass;
		/* charname */
	} t_d2gs_d2cs_updategameinfo;
#define D2GS_D2CS_UPDATEGAMEINFO_FLAG_UPDATE		0
#define D2GS_D2CS_UPDATEGAMEINFO_FLAG_ENTER		1
#define D2GS_D2CS_UPDATEGAMEINFO_FLAG_LEAVE		2

#define D2GS_D2CS_CLOSEGAME		0x23
	typedef struct
	{
		t_d2cs_d2gs_header	h;
		bn_int			gameid;
	} t_d2gs_d2cs_closegame;

#ifdef D2GS
#pragma pack(pop, pack01)
#endif

}

#endif
