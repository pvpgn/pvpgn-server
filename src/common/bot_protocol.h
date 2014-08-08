/*
 * Copyright (C) 1999,2001  Ross Combs (rocombs@cs.nmsu.edu)
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


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_BOT_PROTOCOL_PROTOS
#define INCLUDED_BOT_PROTOCOL_PROTOS

/*
 * The bot protocol has no real structure.  It basically different ASCII
 * formatted lines.
 */

/***********************************
*/
#define EID_SHOWUSER            1001
/***********************************/


/***********************************
*/
#define EID_JOIN                1002
/***********************************/


/***********************************
*/
#define EID_LEAVE               1003
/***********************************/


/***********************************
*/
#define EID_WHISPER             1004
/***********************************/


/***********************************
*/
#define EID_TALK                1005
/***********************************/


/***********************************
*/
#define EID_BROADCAST           1006
/***********************************/


/***********************************
*/
#define EID_CHANNEL             1007
/***********************************/


/***********************************
*/
#define EID_USERFLAGS           1009
/***********************************/


/***********************************
*/
#define EID_WHISPERSENT         1010
/***********************************/


/***********************************
*/
#define EID_CHANNELFULL         1013
/***********************************/


/***********************************
*/
#define EID_CHANNELDOESNOTEXIST 1014
/***********************************/


/***********************************
*/
#define EID_CHANNELRESTRICTED   1015
/***********************************/


/***********************************
1018 INFO "You are Anonymous#11, using Chat in a private game."
*/
#define EID_INFO                1018 /* 1016? */
/***********************************/


/***********************************
1019 ERROR "That is not a valid command. Type /help or /? for more info."
*/
#define EID_ERROR               1019
/***********************************/


/***********************************
*/
#define EID_EMOTE               1023
/***********************************/


/***********************************
2000 NULL
*/
#define EID_NULL                2000
/***********************************/


/***********************************
2010 NAME Anonymous#11
*/
#define EID_UNIQUENAME          2010
/***********************************/


#endif
#endif
