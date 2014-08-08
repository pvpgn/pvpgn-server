/*
 * Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_CLIENT_CONNECT_PROTOS
#define INCLUDED_CLIENT_CONNECT_PROTOS

// Diablo 1.09b
#define CLIENT_VERSIONID_DRTL   0x0000002a
#define CLIENT_GAMEVERSION_DRTL 0x01000902
#define CLIENT_EXEINFO_DRTL     "Diablo.exe 05/18/01 23:10:57 767760"
#define CLIENT_CHECKSUM_DRTL    0x23135c73 

//plain Starcraft 1.15.2
#define CLIENT_VERSIONID_STAR   0x000000d1
#define CLIENT_GAMEVERSION_STAR 0x010f0200
#define CLIENT_EXEINFO_STAR     "StarCraft.exe 01/08/08 23:45:36 1220608"
#define CLIENT_CHECKSUM_STAR    0x91c0f907


#define CLIENT_VERSIONID_SSHR   0x000000a5 /* FIXME: wrong? */
#define CLIENT_GAMEVERSION_SSHR 0x0100080a /* FIXME: wrong? */
#define CLIENT_EXEINFO_SSHR     "starcraft.exe 03/08/99 22:41:50 1042432" /* FIXME: wrong */
#define CLIENT_CHECKSUM_SSHR    0x12345678

//Broodwar 1.15.2
#define CLIENT_VERSIONID_SEXP   0x000000d1
#define CLIENT_GAMEVERSION_SEXP 0x010f0201
#define CLIENT_EXEINFO_SEXP     "StarCraft.exe 01/10/08 20:23:42 1220608"
#define CLIENT_CHECKSUM_SEXP    0x8fbdf18d

//Warcraft 2 Battle.Net Edition 2.02b
#define CLIENT_VERSIONID_W2BN   0x0000004f
#define CLIENT_GAMEVERSION_W2BN 0x02000201
#define CLIENT_EXEINFO_W2BN     "Warcraft II BNE.exe 05/21/01 21:52:22 712704"
#define CLIENT_CHECKSUM_W2BN    0xff0d4c4a

//Diablo 2 1.11b
#define CLIENT_VERSIONID_D2DV   0x0000000b
#define CLIENT_GAMEVERSION_D2DV 0x01000b00
#define CLIENT_EXEINFO_D2DV     "Game.exe 08/17/05 01:11:43 2125824"
#define CLIENT_CHECKSUM_D2DV    0x602f8323

//Diable 2 Expansion 1.11b
#define CLIENT_VERSIONID_D2XP   0x0000000b
#define CLIENT_GAMEVERSION_D2XP 0x01000b00
#define CLIENT_EXEINFO_D2XP     "Game.exe 08/17/05 01:12:38 2129920"
#define CLIENT_CHECKSUM_D2XP    0xbfc36199

//Warcraft 3 1.21b and Warcraft 3 Frozen Throne 1.21b
#define CLIENT_VERSIONID_WAR3   0x00000015
#define CLIENT_GAMEVERSION_WAR3 0x0115019c
#define CLIENT_EXEINFO_WAR3     "war3.exe 07/19/07 18:41:12 409660"
#define CLIENT_CHECKSUM_WAR3    0x1b735294

#define CLIENT_COUNTRYINFO_109_PROTOCOL            0x00000000
#define CLIENT_COUNTRYINFO_109_VERSIONID_D2DV      0x00000009
#define CLIENT_COUNTRYINFO_109_GAMELANG            "enUS"
#define CLIENT_COUNTRYINFO_109_LOCALIP             0x00000000
#define CLIENT_COUNTRYINFO_109_LANGID_USENGLISH    0x00000409
#define CLIENT_COUNTRYINFO_109_LANGSTR_USENGLISH   "ENU"
#define CLIENT_COUNTRYINFO_109_COUNTRYNAME_USA     "United States"

#include "compat/psock.h"

namespace pvpgn
{

	namespace client
	{

		extern int client_connect(char const * progname, char const * servname, unsigned short servport, char const * cdowner, char const * cdkey, char const * clienttag, int ignoreversion, struct sockaddr_in * saddr, unsigned int * sessionkey, unsigned int * sessionnum, char const * archtag, char const * gamelang);

	}
}
#endif
#endif
