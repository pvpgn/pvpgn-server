/*
 * Copyright (C) 1998,1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 1999,2000,2001  Marco Ziech (mmz@gmx.net)
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
#include "common/setup_before.h"
#include "common/packet.h"

#include <cstring>

#include "common/eventlog.h"
#include "common/bn_type.h"
#include "common/field_sizes.h"
#include "common/xalloc.h"
#include "common/lstr.h"
#include "common/setup_after.h"


namespace pvpgn
{

	extern t_packet * packet_create(t_packet_class pclass)
	{
		t_packet * temp;

		if (pclass != packet_class_init &&
			pclass != packet_class_bnet &&
			pclass != packet_class_file &&
			pclass != packet_class_udp &&
			pclass != packet_class_raw &&
			pclass != packet_class_d2game &&
			pclass != packet_class_d2cs &&
			pclass != packet_class_d2gs &&
			pclass != packet_class_d2cs_bnetd &&
			pclass != packet_class_w3route &&
			pclass != packet_class_wolgameres)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "invalid packet class %d", (int)pclass);
			return NULL;
		}

		temp = (t_packet*)xmalloc(sizeof(t_packet));
		temp->ref = 1;
		temp->pclass = pclass;
		temp->flags = 0;
		packet_set_size(temp, 0);

		return temp;
	}


	extern void packet_destroy(t_packet const * packet)
	{
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return;
		}

		xfree((void *)packet); /* avoid warning */
	}


	extern t_packet * packet_add_ref(t_packet * packet)
	{
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return NULL;
		}

		packet->ref++;
		return packet;
	}


	extern void packet_del_ref(t_packet * packet)
	{
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return;
		}

		if (packet->ref < 2) /* if would go to zero */
			packet_destroy(packet);
		else
			packet->ref--;
	}


	extern t_packet_class packet_get_class(t_packet const * packet)
	{
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return packet_class_none;
		}

		switch (packet->pclass)
		{
		case packet_class_init:
			return packet_class_init;
		case packet_class_bnet:
			return packet_class_bnet;
		case packet_class_file:
			return packet_class_file;
		case packet_class_udp:
			return packet_class_udp;
		case packet_class_raw:
			return packet_class_raw;
		case packet_class_d2game:
			return packet_class_d2game;
		case packet_class_d2cs:
			return packet_class_d2cs;
		case packet_class_d2gs:
			return packet_class_d2gs;
		case packet_class_d2cs_bnetd:
			return packet_class_d2cs_bnetd;
		case packet_class_w3route:
			return packet_class_w3route;
		case packet_class_wolgameres:
			return packet_class_wolgameres;
		case packet_class_none:
			return packet_class_none;
		default:
			eventlog(eventlog_level_error, __FUNCTION__, "packet has invalid class %d", (int)packet->pclass);
			return packet_class_none;
		}
	}


	extern char const * packet_get_class_str(t_packet const * packet)
	{
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return "unknown";
		}

		switch (packet->pclass)
		{
		case packet_class_init:
			return "init";
		case packet_class_bnet:
			return "bnet";
		case packet_class_file:
			return "file";
		case packet_class_udp:
			return "udp";
		case packet_class_raw:
			return "raw";
		case packet_class_d2game:
			return "d2game";
		case packet_class_d2gs:
			return "d2gs";
		case packet_class_d2cs_bnetd:
			return "d2cs_bnetd";
		case packet_class_d2cs:
			return "d2cs";
		case packet_class_w3route:
			return "w3route";
		case packet_class_wolgameres:
			return "wolgameres";
		case packet_class_none:
			return "none";
		default:
			eventlog(eventlog_level_error, __FUNCTION__, "packet has invalid class %d", (int)packet->pclass);
			return "unknown";
		}
	}


	extern int packet_set_class(t_packet * packet, t_packet_class pclass)
	{
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return -1;
		}
		if (packet->pclass != packet_class_raw)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got non-raw packet");
			return -1;
		}
		if (pclass != packet_class_init &&
			pclass != packet_class_bnet &&
			pclass != packet_class_file &&
			pclass != packet_class_udp &&
			pclass != packet_class_raw &&
			pclass != packet_class_d2game &&
			pclass != packet_class_d2cs &&
			pclass != packet_class_d2gs &&
			pclass != packet_class_d2cs_bnetd &&
			pclass != packet_class_w3route &&
			pclass != packet_class_wolgameres)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "invalid packet class %d", (int)pclass);
			return -1;
		}

		packet->pclass = pclass;
		return 0;
	}


	extern unsigned int packet_get_type(t_packet const * packet)
	{
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return 0;
		}

		switch (packet->pclass)
		{
		case packet_class_init:
			return CLIENT_INITCONN; /* all init packets are of this type */

		case packet_class_bnet:
			if (packet_get_size(packet) < sizeof(t_bnet_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bnet packet is shorter than header (len=%u)", packet_get_size(packet));
				return 0;
			}
			return (unsigned int)bn_short_get(packet->u.bnet.h.type);

		case packet_class_file:
			if (packet_get_size(packet) < sizeof(t_file_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "file packet is shorter than header (len=%u)", packet_get_size(packet));
				return 0;
			}
			return (unsigned int)bn_short_get(packet->u.file.h.type);

		case packet_class_udp:
			if (packet_get_size(packet) < sizeof(t_udp_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "udp packet is shorter than header (len=%u)", packet_get_size(packet));
				return 0;
			}
			return bn_int_get(packet->u.udp.h.type);

		case packet_class_raw:
			return 0; /* raw packets don't have a type, but don't warn because the packet dump tries anyway */

		case packet_class_d2game:
			if (packet_get_size(packet) < sizeof(t_d2game_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "d2game packet is shorter than header (len=%u)", packet_get_size(packet));
				return 0;
			}
			return bn_byte_get(packet->u.d2game.h.type);

		case packet_class_d2gs:
			if (packet_get_size(packet) < sizeof(t_d2cs_d2gs_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "d2gs packet is shorter than header (len=%u)", packet_get_size(packet));
				return 0;
			}
			return bn_short_get(packet->u.d2cs_d2gs.h.type);
		case packet_class_d2cs_bnetd:
			if (packet_get_size(packet) < sizeof(t_d2cs_bnetd_header)) {
				eventlog(eventlog_level_error, __FUNCTION__, "d2cs_bnetd packet shorter than header (len=%u)", packet_get_size(packet));
				return 0;
			}
			return bn_short_get(packet->u.d2cs_d2gs.h.type);

		case packet_class_d2cs:
			if (packet_get_size(packet) < sizeof(t_d2cs_client_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "d2cs packet is shorter than header (len=%u)", packet_get_size(packet));
				return 0;
			}
			return bn_byte_get(packet->u.d2cs_client.h.type);

		case packet_class_w3route:
			if (packet_get_size(packet) < sizeof(t_w3route_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "w3route packet is shorter than header (len=%u)", packet_get_size(packet));
				return 0;
			}
			return bn_short_get(packet->u.w3route.h.type);
		case packet_class_wolgameres:
			return 0; /* wolgameres packets don't have a type */

		default:
			eventlog(eventlog_level_error, __FUNCTION__, "packet has invalid class %d", (int)packet->pclass);
			return 0;
		}
	}


	extern char const * packet_get_type_str(t_packet const * packet, t_packet_dir dir)
	{
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return "unknown";
		}

		switch (dir)
		{
		case packet_dir_from_client:
			switch (packet->pclass)
			{
			case packet_class_init:
				return "CLIENT_INITCONN";
			case packet_class_bnet:
				if (packet_get_size(packet) < sizeof(t_bnet_header))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "packet is shorter than header (len=%u)", packet_get_size(packet));
					return "unknown";
				}
				switch (bn_short_get(packet->u.bnet.h.type))
				{
				case CLIENT_COMPINFO1:
					return "CLIENT_COMPINFO1";
				case CLIENT_COMPINFO2:
					return "CLIENT_COMPINFO2";
				case CLIENT_COUNTRYINFO1:
					return "CLIENT_COUNTRYINFO1";
				case CLIENT_COUNTRYINFO_109:
					return "CLIENT_COUNTRYINFO_109";
				case CLIENT_CREATEACCTREQ1:
					return "CLIENT_CREATEACCTREQ1";
				case CLIENT_UNKNOWN_2B:
					return "CLIENT_UNKNOWN_2B";
				case CLIENT_PROGIDENT:
					return "CLIENT_PROGIDENT";
				case CLIENT_AUTHREQ1:
					return "CLIENT_AUTHREQ1";
				case CLIENT_AUTHREQ_109:
					return "CLIENT_AUTHREQ_109";
				case CLIENT_REGSNOOPREPLY:
					return "CLIENT_REGSNOOPREPLY";
				case CLIENT_ICONREQ:
					return "CLIENT_ICONREQ";
				case CLIENT_LADDERSEARCHREQ:
					return "CLIENT_LADDERSEARCHREQ";
				case CLIENT_CDKEY:
					return "CLIENT_CDKEY";
				case CLIENT_CDKEY2:
					return "CLIENT_CDKEY2";
				case CLIENT_CDKEY3:
					return "CLIENT_CDKEY3";
				case CLIENT_REALMLISTREQ:
					return "CLIENT_REALMLISTREQ";
				case CLIENT_REALMLISTREQ_110:
					return "CLIENT_REALMLISTREQ_110";
				case CLIENT_PROFILEREQ:
					return "CLIENT_PROFILEREQ";
				case CLIENT_UNKNOWN_37:
					return "CLIENT_UNKNOWN_37";
				case CLIENT_UNKNOWN_39:
					return "CLIENT_UNKNOWN_39";
				case CLIENT_LOGINREQ2:
					return "CLIENT_LOGINREQ2";
				case CLIENT_MOTD_W3:
					return "CLIENT_MOTD_W3";
				case CLIENT_LOGINREQ_W3:
					return "CLIENT_LOGINREQ_W3";
				case CLIENT_LOGONPROOFREQ:
					return "CLIENT_LOGONPROOFREQ";
				case CLIENT_CREATEACCOUNT_W3:
					return "CLIENT_CREATEACCOUNT_W3";
				case CLIENT_PASSCHANGEREQ:
					return "CLIENT_PASSCHANGEREQ";
				case CLIENT_PASSCHANGEPROOFREQ:
					return "CLIENT_PASSCHANGEPROOFREQ";
				case CLIENT_CHANGEGAMEPORT:
					return "CLIENT_CHANGEGAMEPORT";
				case CLIENT_CREATEACCTREQ2:
					return "CLIENT_CREATEACCTREQ2";
				case CLIENT_UDPOK:
					return "CLIENT_UDPOK";
				case CLIENT_FILEINFOREQ:
					return "CLIENT_FILEINFOREQ";
				case CLIENT_STATSREQ:
					return "CLIENT_STATSREQ";
				case CLIENT_LOGINREQ1:
					return "CLIENT_LOGINREQ1";
				case CLIENT_CHANGEPASSREQ:
					return "CLIENT_CHANGEPASSREQ";
				case CLIENT_PLAYERINFOREQ:
					return "CLIENT_PLAYERINFOREQ";
				case CLIENT_PROGIDENT2:
					return "CLIENT_PROGIDENT2";
				case CLIENT_JOINCHANNEL:
					return "CLIENT_JOINCHANNEL";
				case CLIENT_MESSAGE:
					return "CLIENT_MESSAGE";
				case CLIENT_GAMELISTREQ:
					return "CLIENT_GAMELISTREQ";
				case CLIENT_STARTGAME1:
					return "CLIENT_STARTGAME1";
				case CLIENT_UNKNOWN_1B:
					return "CLIENT_UNKNOWN_1B";
				case CLIENT_STARTGAME3:
					return "CLIENT_STARTGAME3";
				case CLIENT_STARTGAME4:
					return "CLIENT_STARTGAME4";
				case CLIENT_CLOSEGAME:
					return "CLIENT_CLOSEGAME";
				case CLIENT_CLOSEGAME2:
					return "CLIENT_CLOSEGAME2";
				case CLIENT_LEAVECHANNEL:
					return "CLIENT_LEAVECHANNEL";
				case CLIENT_MAPAUTHREQ1:
					return "CLIENT_MAPAUTHREQ1";
				case CLIENT_MAPAUTHREQ2:
					return "CLIENT_MAPAUTHREQ2";
				case CLIENT_ADREQ:
					return "CLIENT_ADREQ";
				case CLIENT_ADACK:
					return "CLIENT_ADACK";
				case CLIENT_ADCLICK:
					return "CLIENT_ADCLICK";
				case CLIENT_ADCLICK2:
					return "CLIENT_ADCLICK2";
				case CLIENT_READMEMORY:
					return "CLIENT_READMEMORY";
				case CLIENT_UNKNOWN_24:
					return "CLIENT_UNKNOWN_24";
				case CLIENT_LADDERREQ:
					return "CLIENT_LADDERREQ";
				case CLIENT_ECHOREPLY:
					return "CLIENT_ECHOREPLY";
				case CLIENT_PINGREQ:
					return "CLIENT_PINGREQ";
				case CLIENT_GAME_REPORT:
					return "CLIENT_GAME_REPORT";
				case CLIENT_JOIN_GAME:
					return "CLIENT_JOIN_GAME";
				case CLIENT_STATSUPDATE:
					return "CLIENT_STATSUPDATE";
				case CLIENT_REALMJOINREQ_109:
					return "CLIENT_REALMJOINREQ_109";
				case CLIENT_CHANGECLIENT:
					return "CLIENT_CHANGECLIENT";
				case CLIENT_SETEMAILREPLY:
					return "CLIENT_SETEMAILREPLY";
				case CLIENT_GETPASSWORDREQ:
					return "CLIENT_GETPASSWORDREQ";
				case CLIENT_CHANGEEMAILREQ:
					return "CLIENT_CHANGEEMAILREQ";
				case CLIENT_CRASHDUMP:
					return "CLIENT_CRASHDUMP";
				case CLIENT_FINDANONGAME:
					return "CLIENT_FINDANONGAME";
				case CLIENT_ARRANGEDTEAM_FRIENDSCREEN:
					return "CLIENT_ARRANGEDTEAM_FRIENDSCREEN";
				case CLIENT_ARRANGEDTEAM_INVITE_FRIEND:
					return "CLIENT_ARRANGEDTEAM_INVITE_FRIEND";
				case CLIENT_ARRANGEDTEAM_ACCEPT_DECLINE_INVITE:
					return "CLIENT_ARRANGEDTEAM_ACCEPT_DECLINE_INVITE";
				case CLIENT_FRIENDSLISTREQ:
					return "CLIENT_FRIENDSLISTREQ";
				case CLIENT_FRIENDINFOREQ:
					return "CLIENT_FRIENDINFOREQ";
				case CLIENT_CLANINFOREQ:
					return "CLIENT_CLANINFOREQ";
				case CLIENT_CLAN_CREATEREQ:
					return "CLIENT_CLAN_CREATEREQ";
				case CLIENT_CLAN_CREATEINVITEREQ:
					return "CLIENT_CLAN_CREATEINVITEREQ";
				case CLIENT_CLAN_CREATEINVITEREPLY:
					return "CLIENT_CLAN_CREATEINVITEREPLY";
				case CLIENT_CLAN_DISBANDREQ:
					return "CLIENT_CLAN_DISBANDREQ";
				case CLIENT_CLAN_MEMBERNEWCHIEFREQ:
					return "CLIENT_CLAN_MEMBERNEWCHIEFREQ";
				case CLIENT_CLAN_INVITEREQ:
					return "CLIENT_CLAN_INVITEREQ";
				case CLIENT_CLANMEMBER_REMOVE_REQ:
					return "CLIENT_CLANMEMBER_REMOVE_REQ";
				case CLIENT_CLAN_INVITEREPLY:
					return "CLIENT_CLAN_INVITEREPLY";
				case CLIENT_CLANMEMBER_RANKUPDATE_REQ:
					return "CLIENT_CLANMEMBER_RANKUPDATE_REQ";
				case CLIENT_CLAN_MOTDCHG:
					return "CLIENT_CLAN_MOTDCHG";
				case CLIENT_CLAN_MOTDREQ:
					return "CLIENT_CLAN_MOTDREQ";
				case CLIENT_CLANMEMBERLIST_REQ:
					return "CLIENT_CLANMEMBERLIST_REQ";
				}
				return "unknown";

			case packet_class_file:
				if (packet_get_size(packet) < sizeof(t_file_header))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "packet is shorter than header (len=%u)", packet_get_size(packet));
					return "unknown";
				}
				switch (bn_short_get(packet->u.file.h.type))
				{
				case CLIENT_FILE_REQ:
					return "CLIENT_FILE_REQ";
				}
				return "unknown";

			case packet_class_udp:
				if (packet_get_size(packet) < sizeof(t_udp_header))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "packet is shorter than header (len=%u)", packet_get_size(packet));
					return "unknown";
				}
				switch (bn_int_get(packet->u.udp.h.type))
				{
				case SERVER_UDPTEST: /* we get these if we send stuff to ourself */
					return "SERVER_UDPTEST";
				case CLIENT_UDPPING:
					return "CLIENT_UDPPING";
				case CLIENT_SESSIONADDR1:
					return "CLIENT_SESSIONADDR1";
				case CLIENT_SESSIONADDR2:
					return "CLIENT_SESSIONADDR2";
				}
				return "unknown";

			case packet_class_raw:
				return "CLIENT_RAW";

			case packet_class_d2game:
				if (packet_get_size(packet) < sizeof(t_d2game_header))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "packet is shorter than header (len=%u)", packet_get_size(packet));
					return "unknown";
				}
				switch (bn_byte_get(packet->u.d2game.h.type))
				{
				default:
					return "CLIENT_D2GAME";
				}
				return "unknown";

			case packet_class_d2cs:
				return "D2CS";
			case packet_class_d2gs:
				return "D2GS";
			case packet_class_d2cs_bnetd:
				return "D2CS_BNETD";

			case packet_class_w3route:
				if (packet_get_size(packet) < sizeof(t_w3route_header))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "packet is shorter than header (len=%u)", packet_get_size(packet));
					return "unknown";
				}
				switch (bn_short_get(packet->u.bnet.h.type))
				{
				case CLIENT_W3ROUTE_REQ:
					return "CLIENT_W3ROUTE_REQ";
				case CLIENT_W3ROUTE_LOADINGDONE:
					return "CLIENT_W3ROUTE_LOADINGDONE";
				case CLIENT_W3ROUTE_ABORT:
					return "CLIENT_W3ROUTE_ABORT";
				case CLIENT_W3ROUTE_CONNECTED:
					return "CLIENT_W3ROUTE_CONNECTED";
				case CLIENT_W3ROUTE_ECHOREPLY:
					return "CLIENT_W3ROUTE_ECHOREPLY";
				case CLIENT_W3ROUTE_GAMERESULT:
					return "CLIENT_W3ROUTE_GAMERESULT";
				case CLIENT_W3ROUTE_GAMERESULT_W3XP:
					return "CLIENT_W3ROUTE_GAMERESULT_W3XP";
				}
				return "unknown";

			case packet_class_none:
				return "unknown";
			}

			eventlog(eventlog_level_error, __FUNCTION__, "packet has invalid class %d", (int)packet->pclass);
			return "unknown";

		case packet_dir_from_server:
			switch (packet->pclass)
			{
			case packet_class_init:
				return "unknown";
			case packet_class_bnet:
				if (packet_get_size(packet) < sizeof(t_bnet_header))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "packet is shorter than header (len=%u)", packet_get_size(packet));
					return "unknown";
				}
				switch (bn_short_get(packet->u.bnet.h.type))
				{
				case SERVER_COMPREPLY:
					return "SERVER_COMPREPLY";
				case SERVER_SESSIONKEY1:
					return "SERVER_SESSIONKEY1";
				case SERVER_SESSIONKEY2:
					return "SERVER_SESSIONKEY2";
				case SERVER_CREATEACCTREPLY1:
					return "SERVER_CREATEACCTREPLY1";
				case SERVER_AUTHREQ1:
					return "SERVER_AUTHREQ1";
				case SERVER_AUTHREQ_109:
					return "SERVER_AUTHREQ_109";
				case SERVER_AUTHREPLY1:
					return "SERVER_AUTHREPLY1";
				case SERVER_AUTHREPLY_109:
					return "SERVER_AUTHREPLY_109";
				case SERVER_REGSNOOPREQ:
					return "SERVER_REGSNOOPREQ";
				case SERVER_ICONREPLY:
					return "SERVER_ICONREPLY";
				case SERVER_LADDERSEARCHREPLY:
					return "SERVER_LADDERSEARCHREPLY";
				case SERVER_CDKEYREPLY:
					return "SERVER_CDKEYREPLY";
				case SERVER_CDKEYREPLY2:
					return "SERVER_CDKEYREPLY2";
				case SERVER_CDKEYREPLY3:
					return "SERVER_CDKEYREPLY3";
				case SERVER_REALMLISTREPLY:
					return "SERVER_REALMLISTREPLY";
				case SERVER_REALMLISTREPLY_110:
					return "SERVER_REALMLISTREPLY_110";
				case SERVER_PROFILEREPLY:
					return "SERVER_PROFILEREPLY";
				case SERVER_UNKNOWN_37:
					return "SERVER_UNKNOWN_37";
				case SERVER_MOTD_W3:
					return "SERVER_MOTD_W3";
				case SERVER_LOGINREPLY_W3:
					return "SERVER_LOGINREPLY_W3";
				case SERVER_LOGONPROOFREPLY:
					return "SERVER_LOGONPROOFREPLY";
				case SERVER_CREATEACCOUNT_W3:
					return "SERVER_CREATEACCOUNT_W3";
				case SERVER_PASSCHANGEREPLY:
					return "SERVER_PASSCHANGEREPLY";
				case SERVER_PASSCHANGEPROOFREPLY:
					return "SERVER_PASSCHANGEPROOFREPLY";
				case SERVER_LOGINREPLY2:
					return "SERVER_LOGINREPLY2";
				case SERVER_CREATEACCTREPLY2:
					return "SERVER_CREATEACCTREPLY2";
				case SERVER_FILEINFOREPLY:
					return "SERVER_FILEINFOREPLY";
				case SERVER_STATSREPLY:
					return "SERVER_STATSREPLY";
				case SERVER_LOGINREPLY1:
					return "SERVER_LOGINREPLY1";
				case SERVER_CHANGEPASSACK:
					return "SERVER_CHANGEPASSACK";
				case SERVER_PLAYERINFOREPLY:
					return "SERVER_PLAYERINFOREPLY";
				case SERVER_CHANNELLIST:
					return "SERVER_CHANNELLIST";
				case SERVER_SERVERLIST:
					return "SERVER_SERVERLIST";
				case SERVER_MESSAGE:
					return "SERVER_MESSAGE";
				case SERVER_GAMELISTREPLY:
					return "SERVER_GAMELISTREPLY";
				case SERVER_STARTGAME1_ACK:
					return "SERVER_STARTGAME1_ACK";
				case SERVER_STARTGAME3_ACK:
					return "SERVER_STARTGAME3_ACK";
				case SERVER_STARTGAME4_ACK:
					return "SERVER_STARTGAME4_ACK";
				case SERVER_MAPAUTHREPLY1:
					return "SERVER_MAPAUTHREPLY1";
				case SERVER_MAPAUTHREPLY2:
					return "SERVER_MAPAUTHREPLY2";
				case SERVER_ADREPLY:
					return "SERVER_ADREPLY";
				case SERVER_ADCLICKREPLY2:
					return "SERVER_ADCLICKREPLY2";
				case SERVER_READMEMORY:
					return "SERVER_READMEMORY";
				case SERVER_LADDERREPLY:
					return "SERVER_LADDERREPLY";
				case SERVER_ECHOREQ:
					return "SERVER_ECHOREQ";
				case SERVER_PINGREPLY:
					return "SERVER_PINGREPLY";
				case SERVER_REALMJOINREPLY_109:
					return "SERVER_REALMJOINREPLY_109";
				case SERVER_SETEMAILREQ:
					return "SERVER_SETEMAILREQ";
				case SERVER_FINDANONGAME:
					return "SERVER_FINDANONGAME";
				case SERVER_ARRANGEDTEAM_FRIENDSCREEN:
					return "SERVER_ARRANGEDTEAM_FRIENDSCREEN";
				case SERVER_ARRANGEDTEAM_INVITE_FRIEND_ACK:
					return "SERVER_ARRANGEDTEAM_INVITE_FRIEND_ACK";
				case SERVER_ARRANGEDTEAM_SEND_INVITE:
					return "SERVER_ARRANGEDTEAM_SEND_INVITE";
				case SERVER_ARRANGEDTEAM_MEMBER_DECLINE:
					return "SERVER_ARRANGEDTEAM_MEMBER_DECLINE";
				case SERVER_FRIENDSLISTREPLY:
					return "SERVER_FRIENDSLISTREPLY";
				case SERVER_FRIENDINFOREPLY:
					return "SERVER_FRIENDINFOREPLY";
				case SERVER_FRIENDADD_ACK:
					return "SERVER_FRIENDADD_ACK";
				case SERVER_FRIENDDEL_ACK:
					return "SERVER_FRIENDDEL_ACK";
				case SERVER_FRIENDMOVE_ACK:
					return "SERVER_FRIENDMOVE_ACK";
				case SERVER_CLANINFOREPLY:
					return "SERVER_CLANINFO_REPLY";
				case SERVER_CLAN_CREATEREPLY:
					return "SERVER_CLAN_CREATEREPLY";
				case SERVER_CLAN_CREATEINVITEREPLY:
					return "SERVER_CLAN_CREATEINVITEREPLY";
				case SERVER_CLAN_CREATEINVITEREQ:
					return "SERVER_CLAN_CREATEINVITEREQ";
				case SERVER_CLAN_DISBANDREPLY:
					return "SERVER_CLAN_DISBANDREPLY";
				case SERVER_CLAN_MEMBERNEWCHIEFREPLY:
					return "SERVER_CLAN_MEMBERNEWCHIEFREPLY";
				case SERVER_CLAN_CLANACK:
					return "SERVER_CLAN_CLANACK";
				case SERVER_CLANQUITNOTIFY:
					return "SERVER_CLANQUITNOTIFY";
				case SERVER_CLAN_INVITEREPLY:
					return "SERVER_CLAN_INVITEREPLY";
				case SERVER_CLANMEMBER_REMOVE_REPLY:
					return "SERVER_CLANMEMBER_REMOVE_REPLY";
				case SERVER_CLAN_INVITEREQ:
					return "SERVER_CLAN_INVITEREQ";
				case SERVER_CLANMEMBER_RANKUPDATE_REPLY:
					return "SERVER_CLANMEMBER_RANKUPDATE_REPLY";
				case SERVER_CLAN_MOTDREPLY:
					return "SERVER_CLAN_MOTDREPLY";
				case SERVER_CLANMEMBERLIST_REPLY:
					return "SERVER_CLANMEMBERLIST_REPLY";
				case SERVER_CLANMEMBER_REMOVED_NOTIFY:
					return "SERVER_CLANMEMBER_REMOVED_NOTIFY";
				case SERVER_CLANMEMBERUPDATE:
					return "SERVER_CLANMEMBERUPDATE";
				case SERVER_MESSAGEBOX:
					return "SERVER_MESSAGEBOX";
				}
				return "unknown";

			case packet_class_file:
				if (packet_get_size(packet) < sizeof(t_file_header))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "packet is shorter than header (len=%u)", packet_get_size(packet));
					return "unknown";
				}
				switch (bn_short_get(packet->u.file.h.type))
				{
				case SERVER_FILE_REPLY:
					return "SERVER_FILE_REPLY";
				}
				return "unknown";

			case packet_class_udp:
				if (packet_get_size(packet) < sizeof(t_udp_header))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "packet is shorter than header (len=%u)", packet_get_size(packet));
					return "unknown";
				}
				switch (bn_int_get(packet->u.udp.h.type))
				{
				case SERVER_UDPTEST:
					return "SERVER_UDPTEST";
				}
				return "unknown";

			case packet_class_raw:
				return "SERVER_RAW";

			case packet_class_d2game:
				if (packet_get_size(packet) < sizeof(t_d2game_header))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "packet is shorter than header (len=%u)", packet_get_size(packet));
					return "unknown";
				}
				switch (bn_byte_get(packet->u.d2game.h.type))
				{
				default:
					return "SERVER_D2GAME";
				}
				return "unknown";

			case packet_class_d2cs:
				return "D2CS";
			case packet_class_d2gs:
				return "D2GS";
			case packet_class_d2cs_bnetd:
				return "D2CS_BNETD";

			case packet_class_w3route:
				if (packet_get_size(packet) < sizeof(t_w3route_header))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "packet is shorter than header (len=%u)", packet_get_size(packet));
					return "unknown";
				}
				switch (bn_short_get(packet->u.bnet.h.type))
				{
				case SERVER_W3ROUTE_READY:
					return "SERVER_W3ROUTE_READY";
				case SERVER_W3ROUTE_LOADINGACK:
					return "SERVER_W3ROUTE_LOADINGACK";
				case SERVER_W3ROUTE_ECHOREQ:
					return "SERVER_W3ROUTE_ECHOREQ";
				case SERVER_W3ROUTE_ACK:
					return "SERVER_W3ROUTE_ACK";
				case SERVER_W3ROUTE_PLAYERINFO:
					return "SERVER_W3ROUTE_PLAYERINFO";
				case SERVER_W3ROUTE_LEVELINFO:
					return "SERVER_W3ROUTE_LEVELINFO";
				case SERVER_W3ROUTE_STARTGAME1:
					return "SERVER_W3ROUTE_STARTGAME1";
				case SERVER_W3ROUTE_STARTGAME2:
					return "SERVER_W3ROUTE_STARTGAME2";
				}
				return "unknown";
			case packet_class_wolgameres:
				return "CLIENT_WOLGAMERES";
			case packet_class_none:
				return "unknown";
			}

			eventlog(eventlog_level_error, __FUNCTION__, "packet has invalid class %d", (int)packet->pclass);
			return "unknown";
		}

		eventlog(eventlog_level_error, __FUNCTION__, "got unknown direction %d", (int)dir);
		return "unknown";
	}


	extern int packet_set_type(t_packet * packet, unsigned int type)
	{
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return -1;
		}

		switch (packet->pclass)
		{
		case packet_class_init:
			if (type != CLIENT_INITCONN)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "init packet type 0x%08x is not valid", type);
				return -1;
			}
			return 0;

		case packet_class_bnet:
			if (packet_get_size(packet)<sizeof(t_bnet_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bnet packet is shorter than header (len=%u)", packet_get_size(packet));
				return -1;
			}
			if (type>MAX_NORMAL_TYPE)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "bnet packet type 0x%08x is too large", type);
				return -1;
			}
			bn_short_set(&packet->u.bnet.h.type, (unsigned short)type);
			return 0;

		case packet_class_file:
			if (packet_get_size(packet)<sizeof(t_file_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "file packet is shorter than header (len=%u)", packet_get_size(packet));
				return -1;
			}
			if (type>MAX_FILE_TYPE)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "file packet type 0x%08x is too large", type);
				return -1;
			}
			bn_short_set(&packet->u.file.h.type, (unsigned short)type);
			return 0;

		case packet_class_udp:
			if (packet_get_size(packet) < sizeof(t_udp_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "udp packet is shorter than header (len=%u)", packet_get_size(packet));
				return -1;
			}
			bn_int_set(&packet->u.udp.h.type, type);
			return 0;

		case packet_class_d2game:
			if (packet_get_size(packet) < sizeof(t_d2game_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "d2game packet is shorter than header (len=%u)", packet_get_size(packet));
				return -1;
			}
			bn_byte_set(&packet->u.d2game.h.type, type);
			return 0;

		case packet_class_d2gs:
			if (packet_get_size(packet) < sizeof(t_d2cs_d2gs_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "d2gs packet is shorter than header (len=%u)", packet_get_size(packet));
				return -1;
			}
			bn_short_set(&packet->u.d2cs_d2gs.h.type, type);
			return 0;

		case packet_class_d2cs_bnetd:
			if (packet_get_size(packet) < sizeof(t_d2cs_bnetd_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "d2cs_bnetd packet is shorter than header (len=%u)", packet_get_size(packet));
				return -1;
			}
			bn_short_set(&packet->u.d2cs_bnetd.h.type, type);
			return 0;

		case packet_class_d2cs:
			if (packet_get_size(packet) < sizeof(t_d2cs_client_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "d2cs packet is shorter than header (len=%u)", packet_get_size(packet));
				return -1;
			}
			bn_byte_set(&packet->u.d2cs_client.h.type, type);
			return 0;

		case packet_class_w3route:
			if (packet_get_size(packet) < sizeof(t_w3route_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "w3route packet is shorter than header (len=%u)", packet_get_size(packet));
				return -1;
			}
			bn_short_set(&packet->u.w3route.h.type, (unsigned short)type);
			return 0;

		case packet_class_wolgameres:
			eventlog(eventlog_level_error, __FUNCTION__, "can not set packet type for wol gameres packet");
			return 0;

		case packet_class_raw:
			eventlog(eventlog_level_error, __FUNCTION__, "can not set packet type for raw packet");
			return 0;

		default:
			eventlog(eventlog_level_error, __FUNCTION__, "packet has invalid class %d", (int)packet->pclass);
			return -1;
		}
	}


	/* size of the _complete_ packet, not the amount currently received or sent */
	extern unsigned int packet_get_size(t_packet const * packet)
	{
		unsigned int size;

		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return 0;
		}

		switch (packet->pclass)
		{
		case packet_class_init:
			size = sizeof(t_client_initconn);
			break;
		case packet_class_bnet:
			size = (unsigned int)bn_short_get(packet->u.bnet.h.size);
			break;
		case packet_class_file:
			size = (unsigned int)bn_short_get(packet->u.file.h.size);
			break;
		case packet_class_udp:
			size = packet->len;
			break;
		case packet_class_raw:
			size = packet->len;
			break;
		case packet_class_d2game:
			size = packet->len; /* FIXME: does header not contain the size? */
			break;
		case packet_class_d2gs:
			size = (unsigned int)bn_short_get(packet->u.d2cs_d2gs.h.size);
			break;
		case packet_class_d2cs_bnetd:
			size = (unsigned int)bn_short_get(packet->u.d2cs_bnetd.h.size);
			break;
		case packet_class_d2cs:
			size = (unsigned int)bn_short_get(packet->u.d2cs_client.h.size);
			break;
		case packet_class_w3route:
			size = (unsigned int)bn_short_get(packet->u.w3route.h.size);
			break;
		case packet_class_wolgameres:
			/* PELISH: We check and return size explicitly in this case,
					   because we need to check MAX_WOL_GAMERES_PACKET_SIZE */
		{
										size = (unsigned int)bn_short_nget(packet->u.wolgameres.h.size);
										if (size == 0) /* RNGD sends size on rngd_size */
											size = (unsigned int)bn_short_nget(packet->u.wolgameres.h.rngd_size);
										//            if (size>MAX_WOL_GAMERES_PACKET_SIZE) { /* PELISH: Fix for bug but also disable WOL gameres */
										if (size > MAX_PACKET_SIZE) {
											// eventlog(eventlog_level_error,__FUNCTION__,"packet has bad size %u",size);
											return 0;
										}
		}
			return size;
		default:
			eventlog(eventlog_level_error, __FUNCTION__, "packet has invalid class %d", (int)packet->pclass);
			return 0;
		}

		if (size > MAX_PACKET_SIZE)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "packet has bad size %u", size);
			return 0;
		}
		return size;
	}


	extern int packet_set_size(t_packet * packet, unsigned int size)
	{
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return -1;
		}
		if (size > MAX_PACKET_SIZE)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got bad size %u", size);
			return -1;
		}

		switch (packet->pclass)
		{
		case packet_class_init:
			if (size != 0 && size != sizeof(t_client_initconn))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "invalid size %u for init packet", size);
				return -1;
			}
			packet->len = size;
			return 0;
		case packet_class_bnet:
			if (size != 0 && size < sizeof(t_bnet_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "invalid size %u for bnet packet", size);
				return -1;
			}
			bn_short_set(&packet->u.bnet.h.size, size);
			return 0;
		case packet_class_file:
			if (size != 0 && size < sizeof(t_file_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "invalid size %u for file packet", size);
				return -1;
			}
			bn_short_set(&packet->u.file.h.size, size);
			return 0;
		case packet_class_udp:
			if (size != 0 && size < sizeof(t_udp_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "invalid size %u for udp packet", size);
				return -1;
			}
			packet->len = size;
			return 0;
		case packet_class_raw:
			packet->len = size;
			return 0;
		case packet_class_d2game:
			packet->len = size; /* FIXME: does header not contain the size? */
			return 0;
		case packet_class_d2cs:
			bn_short_set(&packet->u.d2cs_client.h.size, size);
			return 0;
		case packet_class_d2gs:
			bn_short_set(&packet->u.d2cs_d2gs.h.size, size);
			return 0;
		case packet_class_d2cs_bnetd:
			bn_short_set(&packet->u.d2cs_bnetd.h.size, size);
			return 0;
		case packet_class_w3route:
			if (size != 0 && size < sizeof(t_w3route_header))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "invalid size %u for w3route packet", size);
				return -1;
			}
			bn_short_set(&packet->u.w3route.h.size, size);
			return 0;
		case packet_class_wolgameres:
			/* PELISH: useless - there is no server packet for wolgameres */
			packet->len = size;
			return 0;
		default:
			eventlog(eventlog_level_error, __FUNCTION__, "packet has invalid class %d", (int)packet->pclass);
			return -1;
		}
	}


	extern unsigned int packet_get_header_size(t_packet const * packet)
	{
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return MAX_PACKET_SIZE;
		}

		switch (packet_get_class(packet))
		{
		case packet_class_init:
			return 0;
		case packet_class_bnet:
			return sizeof(t_bnet_header);
		case packet_class_file:
			return sizeof(t_file_header);
		case packet_class_udp:
			return sizeof(t_udp_header);
		case packet_class_raw:
			return 0;
		case packet_class_d2game:
			return 0; /* FIXME: is there no game packet header? */
		case packet_class_d2cs:
			return sizeof(t_d2cs_client_header);
		case packet_class_d2gs:
			return sizeof(t_d2cs_d2gs_header);
		case packet_class_d2cs_bnetd:
			return sizeof(t_d2cs_bnetd_header);
		case packet_class_w3route:
			return sizeof(t_w3route_header);
		case packet_class_wolgameres:
			return sizeof(t_wolgameres_header);
		default:
			eventlog(eventlog_level_error, __FUNCTION__, "packet has bad class %d", (int)packet_get_class(packet));
			return MAX_PACKET_SIZE;
		}
	}


	extern unsigned int packet_get_flags(t_packet const * packet)
	{
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return 0;
		}

		return packet->flags;
	}


	extern int packet_set_flags(t_packet * packet, unsigned int flags)
	{
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return -1;
		}

		packet->flags = flags;
		return 0;
	}


	extern int packet_append_string(t_packet * packet, char const * str)
	{
		unsigned int   len;
		unsigned short addlen;
		unsigned short size;

		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return -1;
		}
		if (!str)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL string");
			return -1;
		}

		len = std::strlen(str) + 1;
		size = packet_get_size(packet);
		if (size >= MAX_PACKET_SIZE)
			return -1;

		if (MAX_PACKET_SIZE - (unsigned int)size > len)
			addlen = len;
		else
			addlen = MAX_PACKET_SIZE - size;
		if (addlen < 1)
			return -1;

		std::memcpy(packet->u.data + size, str, addlen - 1);
		packet->u.data[size + addlen - 1] = '\0';
		packet_set_size(packet, size + addlen);

		return (int)addlen;
	}


	extern int packet_append_ntstring(t_packet * packet, char const * str)
	{
		unsigned int   len;
		unsigned short addlen;
		unsigned short size;

		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return -1;
		}
		if (!str)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL string");
			return -1;
		}

		len = std::strlen(str);
		size = packet_get_size(packet);
		if (size >= MAX_PACKET_SIZE)
			return -1;

		if (MAX_PACKET_SIZE - (unsigned int)size > len)
			addlen = len;
		else
			addlen = MAX_PACKET_SIZE - size;
		if (addlen < 1)
			return -1;

		std::memcpy(packet->u.data + size, str, addlen);
		packet_set_size(packet, size + addlen);

		return (int)addlen;
	}


	extern int packet_append_lstr(t_packet * packet, t_lstr *lstr)
	{
		unsigned short addlen;
		unsigned short size;

		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return -1;
		}
		if (!lstr || !lstr_get_str(lstr))
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL string");
			return -1;
		}

		size = packet_get_size(packet);
		if (size >= MAX_PACKET_SIZE)
			return -1;

		if (MAX_PACKET_SIZE - (unsigned int)size > lstr_get_len(lstr))
			addlen = lstr_get_len(lstr);
		else
			addlen = MAX_PACKET_SIZE - size;
		if (addlen < 1)
			return -1;

		std::memcpy(packet->u.data + size, lstr_get_str(lstr), addlen - 1);
		packet->u.data[size + addlen - 1] = '\0';
		packet_set_size(packet, size + addlen);

		return (int)addlen;
	}


	extern int packet_append_data(t_packet * packet, void const * data, unsigned int len)
	{
		unsigned short addlen;
		unsigned short size;

		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return -1;
		}
		if (!data)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL data");
			return -1;
		}

		size = packet_get_size(packet);
		if (size >= MAX_PACKET_SIZE)
			return -1;

		if (MAX_PACKET_SIZE - (unsigned int)size > len)
			addlen = len;
		else
			addlen = MAX_PACKET_SIZE - size;
		if (addlen < 1)
			return -1;

		std::memcpy(packet->u.data + size, data, addlen);
		packet_set_size(packet, size + addlen);

		return (int)addlen;
	}


	extern void const * packet_get_raw_data_const(t_packet const * packet, unsigned int offset)
	{
		unsigned int size;

		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return NULL;
		}
		size = (unsigned int)packet_get_size(packet);
		if (offset >= size || (((offset >= MAX_PACKET_SIZE) && (packet->pclass != packet_class_wolgameres)) || (offset >= MAX_WOL_GAMERES_PACKET_SIZE)))
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got bad offset %u for packet size %u", offset, size);
			return NULL;
		}

		return packet->u.data + offset;
	}


	extern void * packet_get_raw_data(t_packet * packet, unsigned int offset)
	{
		unsigned int size;

		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return NULL;
		}
		size = (unsigned int)packet_get_size(packet);
		if (offset >= size || (((offset >= MAX_PACKET_SIZE) && (packet->pclass != packet_class_wolgameres)) || (offset >= MAX_WOL_GAMERES_PACKET_SIZE)))
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got bad offset %u for packet size %u", offset, size);
			return NULL;
		}

		return packet->u.data + offset;
	}


	extern void * packet_get_raw_data_build(t_packet * packet, unsigned int offset)
	{
		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return NULL;
		}

		if (((offset >= MAX_PACKET_SIZE) && (packet->pclass != packet_class_wolgameres)) || (offset >= MAX_WOL_GAMERES_PACKET_SIZE))
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got bad offset %u for packet", offset);
			return NULL;
		}

		return packet->u.data + offset;
	}


	/* maxlen includes room for NUL char */
	extern char const * packet_get_str_const(t_packet const * packet, unsigned int offset, unsigned int maxlen)
	{
		unsigned int size;
		unsigned int pos;

		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return NULL;
		}
		size = (unsigned int)packet_get_size(packet);
		if (offset >= size)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got bad offset %u for packet size %u", offset, size);
			return NULL;
		}

		for (pos = offset; packet->u.data[pos] != '\0'; pos++)
		if (pos >= size || pos - offset > maxlen)
			return NULL;
		if (pos >= size || pos - offset > maxlen) /* NUL must be inside too */
			return NULL;
		return packet->u.data + offset;
	}


	extern void const * packet_get_data_const(t_packet const * packet, unsigned int offset, unsigned int len)
	{
		unsigned int size;

		if (!packet)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
			return NULL;
		}
		if (len<1)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got zero length");
			return NULL;
		}
		size = (unsigned int)packet_get_size(packet);
		if (offset + len>size)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got bad offset %u and length %u for packet size %u", offset, len, size);
			return NULL;
		}

		return packet->u.data + offset;
	}


	extern t_packet * packet_duplicate(t_packet const * src)
	{
		t_packet * p;

		if (!(p = packet_create(packet_get_class(src))))
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not create packet");
			return NULL;
		}
		packet_append_data(p, src->u.data, packet_get_size(src));
		packet_set_flags(p, packet_get_flags(src));

		return p;
	}

}
