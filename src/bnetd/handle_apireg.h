/*
 * Copyright (C) 2007  Pelish (pelish@gmail.com)
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

#ifndef JUST_NEED_TYPES

#ifndef INCLUDED_APIREGISTER_TYPES
#define INCLUDED_APIREGISTER_TYPES

#include "common/packet.h"
#include "connection.h"
#endif

namespace pvpgn
{

	namespace bnetd
	{

		typedef struct apiregmember
#ifdef APIREGISTER_INTERNAL_ACCESS
		{
			t_connection *  conn;
			char const *    email;
			char const *    bday;
			char const *    bmonth;
			char const *    byear;
			char const *    langcode;
			char const *    sku;          /* here are SKUs of all installed games */
			char const *    ver;          /* same as with SKU - versions of all installed games */
			char const *    serial;       /* also serials of all installed games */
			char const *    sysid;
			char const *    syscheck;
			char const *    oldnick;      /* client send also all nicks that was registerd in the past */
			char const *    oldpass;      /* and passwords for oldnicks */
			char const *    newnick;
			char const *    newpass;
			char const *    newpass2;
			char const *    parentemail;
			bool            newsletter;   /* do user want to sending news by e-mail? */
			bool            shareinfo;    /* can EA/Westwood shared e-mail contact for sending news? :) */
			char const *    request;      /* API Register request (knowed requests are defined below) */
		}
#endif
		t_apiregmember;

#ifndef INCLUDED_HANDLE_APIREG_PROTOS
#define INCLUDED_HANDLE_APIREG_PROTOS

#define REQUEST_AGEVERIFY    "apireg_ageverify"
#define REQUEST_GETNICK      "apireg_getnick"

		extern int apireglist_create(void);
		extern int apireglist_destroy(void);

		extern int handle_apireg_packet(t_connection * conn, t_packet const * const packet);

	}

}

#endif
#endif
