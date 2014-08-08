/*
 * Copyright (C) 2008  Pelish (pelish@gmail.com)
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

#ifndef INCLUDED_ANONGAME_WOL_TYPES
#define INCLUDED_ANONGAME_WOL_TYPES

#ifdef ANONGAME_WOL_INTERNAL_ACCESS

#ifndef JUST_NEED_TYPES
# define JUST_NEED_TYPES
# include "connection.h"
# undef JUST_NEED_TYPES
#endif

#define MATCHTAG_ADDRESS           "ADR"
#define MATCHTAG_PORT              "PRT"
#define MATCHTAG_COUNTRY           "COU"
#define MATCHTAG_COLOUR            "COL"
#define MATCHTAG_MATCHRESOLUTION   "MBR"
#define MATCHTAG_LOCATION          "LOC"
#define MATCHTAG_SCREENRESOLUTION  "RES"

#define RAL2_CHANNEL_FFA      "Lob 38 0"
#define YURI_CHANNEL_FFA      "Lob 40 0"
#define YURI_CHANNEL_COOP     "Lob 39 0"

#endif

namespace pvpgn
{

	namespace bnetd
	{

		typedef struct anongame_wol_player
#ifdef ANONGAME_WOL_INTERNAL_ACCESS
		{
			t_connection       * conn;

			/* Red Alert 2 and Yuri's Revnenge */
			int                  address;
			int                  port;
			int                  country;
			int                  colour;
		}
#endif
		t_anongame_wol_player;

	}

}

#endif

/*******/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_ANONGAME_WOL_PROTOS
#define INCLUDED_ANONGAME_WOL_PROTOS

#define JUST_NEED_TYPES
# include "connection.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern int anongame_wol_matchlist_create(void);
		extern int anongame_wol_matchlist_destroy(void);

		extern int anongame_wol_destroy(t_connection * conn);
		extern int anongame_wol_privmsg(t_connection * conn, int numparams, char ** params, char * text);

	}

}

#endif
#endif

