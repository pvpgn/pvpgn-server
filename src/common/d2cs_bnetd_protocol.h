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
#ifndef INCLUDED_D2CS_BNETD_PROTOCOL_H
#define INCLUDED_D2CS_BNETD_PROTOCOL_H

#include "common/bn_type.h"

namespace pvpgn
{

	typedef struct
	{
		bn_short	size;
		bn_short	type;
		bn_int		seqno;			/* seqno, set by the sender */
	} t_d2cs_bnetd_header;

	typedef struct
	{
		t_d2cs_bnetd_header	h;
	} t_d2cs_bnetd_generic;

#define BNETD_D2CS_AUTHREQ				0x01
	typedef struct
	{
		t_d2cs_bnetd_header	h;
		bn_int			sessionnum;
	} t_bnetd_d2cs_authreq;

#define D2CS_BNETD_AUTHREPLY				0x02
	typedef struct
	{
		t_d2cs_bnetd_header	h;
		bn_int			version;
		/* realm name */
	} t_d2cs_bnetd_authreply;

#define BNETD_D2CS_AUTHREPLY				0x02
	typedef struct
	{
		t_d2cs_bnetd_header	h;
		bn_int			reply;
	} t_bnetd_d2cs_authreply;
#define BNETD_D2CS_AUTHREPLY_SUCCEED			0x00
#define BNETD_D2CS_AUTHREPLY_BAD_VERSION		0x01

#define D2CS_BNETD_ACCOUNTLOGINREQ			0x10
	typedef struct
	{
		t_d2cs_bnetd_header	h;
		bn_int			seqno;
		bn_int			sessionnum;
		bn_int			sessionkey;
		bn_int			secret_hash[5];
		/* account name */
	} t_d2cs_bnetd_accountloginreq;

#define BNETD_D2CS_ACCOUNTLOGINREPLY			0x10
	typedef struct
	{
		t_d2cs_bnetd_header	h;
		bn_int			reply;
	} t_bnetd_d2cs_accountloginreply;
#define BNETD_D2CS_ACCOUNTLOGINREPLY_SUCCEED		0x00
#define BNETD_D2CS_ACCOUNTLOGINREPLY_FAILED		0x01

#define D2CS_BNETD_CHARLOGINREQ				0x11
	typedef struct
	{
		t_d2cs_bnetd_header	h;
		bn_int			sessionnum;
		/* character name	*/
		/* character portrait	*/
	} t_d2cs_bnetd_charloginreq;

#define BNETD_D2CS_CHARLOGINREPLY			0x11
	typedef struct
	{
		t_d2cs_bnetd_header	h;
		bn_int			reply;
	} t_bnetd_d2cs_charloginreply;
#define BNETD_D2CS_CHARLOGINREPLY_SUCCEED		0x00
#define BNETD_D2CS_CHARLOGINREPLY_FAILED		0x01

#define BNETD_D2CS_GAMEINFOREQ				0x12
	typedef struct
	{
		t_d2cs_bnetd_header	h;
		/* gamename */
	} t_bnetd_d2cs_gameinforeq;

#define D2CS_BNETD_GAMEINFOREPLY			0x12
	typedef struct
	{
		t_d2cs_bnetd_header	h;
		bn_byte			difficulty;
		/* gamename */
	} t_d2cs_bnetd_gameinforeply;

}

#endif
