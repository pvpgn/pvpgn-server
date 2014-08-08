/*
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
#ifndef INCLUDED_FIELD_SIZES_TYPES
#define INCLUDED_FIELD_SIZES_TYPES

const unsigned MAX_PACKET_SIZE = 3072;
const unsigned MAX_WOL_GAMERES_PACKET_SIZE = 65535;
const unsigned MAX_NORMAL_TYPE = 0xffff;
const unsigned MAX_FILE_TYPE = 0xffff;
const int MAX_AUTH_TYPE = 0xff;
const int MAX_GAMES_LIST = 20;
const unsigned MAX_MESSAGE_LEN = 255;
const int MAX_COMMAND_LEN = 32;
const unsigned MAX_USERNAME_LEN = 16; /* including terminating NUL char */
const unsigned MIN_USERNAME_LEN = 2;
const unsigned MAX_USERPASS_LEN = 12; /* max password length as allowed by clients */
const int UNCHECKED_NAME_STR = 32; /* including terminating NUL char */
/* (longer than USER_NAME_MAX and CHAR_NAME_LEN so that
 *  proper error packets can be returned) */
const int CLANSHORT_NAME_MAX = 4;
const unsigned CLAN_NAME_MAX = 24; /* including terminating NUL char */
const unsigned CLAN_MOTD_MAX = 224; /* including terminating NUL char */
const int MAX_CHANNELNAME_LEN = 32; /* including terminating NUL char */
const unsigned MAX_CHARNAME_LEN = 16;
const unsigned MIN_CHARNAME_LEN = 2;
const int MAX_GAMENAME_LEN = 32;
const int MAX_GAMEPASS_LEN = 32;
const int MAX_GAMEINFO_LEN = 128;
const int MAX_GAMEDESC_LEN = 32;
const int MAP_NAME_LEN = 64;
const int BNETHASH_LEN = 20; /* uint32*5, see bnethash.h */
const int MAX_EXEINFO_STR = 256; /* including terminating NUL char */
const int MAX_OWNER_STR = 128; /* including terminating NUL char */
const int MAX_CDKEY_STR = 128; /* including terminating NUL char */
const int MAX_EMAIL_STR = 128; /* including terminating NUL char */
const int MAX_WINHOST_STR = 128; /* including terminating NUL char */
const int MAX_WINUSER_STR = 128; /* including terminating NUL char */
const int MAX_LANG_STR = 64; /* including terminating NUL char */
const int MAX_COUNTRYNAME_STR = 128; /* including terminating NUL char */
const int MAX_FILENAME_STR = 2048; /* including terminating NUL char */
const int MAX_GAMEREP_HEAD_STR = 2048; /* including terminating NUL char */
const int MAX_GAMEREP_BODY_STR = 8192; /* including terminating NUL char */
const int MAX_PLAYERINFO_STR = 2048; /* including terminating NUL char */
const int MAX_COUNTRYCODE_STR = 32; /* including terminating NUL char */
const int MAX_COUNTRY_STR = 32; /* including terminating NUL char */
const int MAX_ATTRKEY_STR = 1024; /* including terminating NUL char */
const int MAX_ATTRVAL_STR = 4096; /* including terminating NUL char */
const unsigned MAX_IRC_MESSAGE_LEN = 512; /* including CRLF (according to RFC 2812) */
const unsigned MAX_TOPIC_LEN = 201; /* including terminating NUL char */
const int MAX_REALMNAME_LEN = 32;
const int MAX_WOLV1_CHANNELNAME_LEN = 18; /* including '#' char */

#endif
