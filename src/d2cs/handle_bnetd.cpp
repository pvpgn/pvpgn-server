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
#include "common/setup_before.h"
#include "setup.h"
#include "handle_bnetd.h"

#include "common/init_protocol.h"
#include "common/eventlog.h"
/*
#include "common/d2cs_bnetd_protocol.h"
#include "common/d2cs_protocol.h"
#include "common/packet.h"
#include "connection.h"
*/
#include "version.h"
#include "prefs.h"
#include "serverqueue.h"
#include "game.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace d2cs
	{

		DECLARE_PACKET_HANDLER(on_bnetd_accountloginreply)
		DECLARE_PACKET_HANDLER(on_bnetd_charloginreply)
		DECLARE_PACKET_HANDLER(on_bnetd_authreq)
		DECLARE_PACKET_HANDLER(on_bnetd_authreply)
		DECLARE_PACKET_HANDLER(on_bnetd_gameinforeq)

		static t_packet_handle_table bnetd_packet_handle_table[] = {
			/* 0x00 */{ 0, conn_state_none, NULL },
			/* 0x01 */{ sizeof(t_bnetd_d2cs_authreq), conn_state_connected, on_bnetd_authreq },
			/* 0x02 */{ sizeof(t_bnetd_d2cs_authreply), conn_state_connected, on_bnetd_authreply },
			/* 0x03 */{ 0, conn_state_none, NULL },
			/* 0x04 */{ 0, conn_state_none, NULL },
			/* 0x05 */{ 0, conn_state_none, NULL },
			/* 0x06 */{ 0, conn_state_none, NULL },
			/* 0x07 */{ 0, conn_state_none, NULL },
			/* 0x08 */{ 0, conn_state_none, NULL },
			/* 0x09 */{ 0, conn_state_none, NULL },
			/* 0x0a */{ 0, conn_state_none, NULL },
			/* 0x0b */{ 0, conn_state_none, NULL },
			/* 0x0c */{ 0, conn_state_none, NULL },
			/* 0x0d */{ 0, conn_state_none, NULL },
			/* 0x03 */{ 0, conn_state_none, NULL },
			/* 0x0f */{ 0, conn_state_none, NULL },
			/* 0x10 */{ sizeof(t_bnetd_d2cs_accountloginreply), conn_state_authed, on_bnetd_accountloginreply },
			/* 0x11 */{ sizeof(t_bnetd_d2cs_charloginreply), conn_state_authed, on_bnetd_charloginreply },
			/* 0x12 */{ sizeof(t_bnetd_d2cs_gameinforeq), conn_state_authed, on_bnetd_gameinforeq }
		};

		extern int handle_bnetd_packet(t_connection * c, t_packet * packet)
		{
			conn_process_packet(c, packet, bnetd_packet_handle_table, NELEMS(bnetd_packet_handle_table));
			return 0;
		}

		extern int handle_bnetd_init(t_connection * c)
		{
			t_packet * packet;

			packet = packet_create(packet_class_init);
			packet_set_size(packet, sizeof(t_client_initconn));
			bn_byte_set(&packet->u.client_initconn.cclass, CLIENT_INITCONN_CLASS_D2CS_BNETD);
			conn_push_outqueue(c, packet);
			packet_del_ref(packet);
			d2cs_conn_set_state(c, conn_state_connected);
			eventlog(eventlog_level_info, __FUNCTION__, "sent init class packet to bnetd");
			return 0;
		}

		static int on_bnetd_authreq(t_connection * c, t_packet * packet)
		{
			t_packet	* rpacket;
			unsigned int	sessionnum;

			sessionnum = bn_int_get(packet->u.bnetd_d2cs_authreq.sessionnum);
			eventlog(eventlog_level_info, __FUNCTION__, "received bnetd sessionnum %d", sessionnum);
			if ((rpacket = packet_create(packet_class_d2cs_bnetd))) {
				packet_set_size(rpacket, sizeof(t_d2cs_bnetd_authreply));
				packet_set_type(rpacket, D2CS_BNETD_AUTHREPLY);
				bn_int_set(&rpacket->u.d2cs_bnetd_authreply.h.seqno, 1);
				bn_int_set(&rpacket->u.d2cs_bnetd_authreply.version, D2CS_VERSION_NUMBER);
				packet_append_string(rpacket, prefs_get_realmname());
				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
			}
			return 0;
		}

		static int on_bnetd_authreply(t_connection * c, t_packet * packet)
		{
			unsigned int	reply;

			reply = bn_int_get(packet->u.bnetd_d2cs_authreply.reply);
			if (reply == BNETD_D2CS_AUTHREPLY_SUCCEED) {
				eventlog(eventlog_level_info, __FUNCTION__, "authed by bnetd");
				d2cs_conn_set_state(c, conn_state_authed);
			}
			else {
				eventlog(eventlog_level_error, __FUNCTION__, "failed to auth by bnetd (error=%d)", reply);
				d2cs_conn_set_state(c, conn_state_destroy);
			}
			return 0;
		}

		static int on_bnetd_accountloginreply(t_connection * c, t_packet * packet)
		{
			unsigned int	seqno;
			t_sq		* sq;
			t_packet	* opacket, *rpacket;
			t_connection	* client;
			int		result, reply;
			char const	* account;
			t_elem		* elem;

			if (!packet || !c)
				return -1;

			seqno = bn_int_get(packet->u.d2cs_bnetd.h.seqno);
			if (!(sq = sqlist_find_sq(seqno))) {
				eventlog(eventlog_level_error, __FUNCTION__, "seqno %d not found", seqno);
				return -1;
			}
			if (!(client = d2cs_connlist_find_connection_by_sessionnum(sq_get_clientid(sq)))) {
				eventlog(eventlog_level_error, __FUNCTION__, "client %d not found", sq_get_clientid(sq));
				sq_destroy(sq, &elem);
				return -1;
			}
			if (!(opacket = sq_get_packet(sq))) {
				eventlog(eventlog_level_error, __FUNCTION__, "previous packet missing (seqno: %d)", seqno);
				sq_destroy(sq, &elem);
				return -1;
			}
			result = bn_int_get(packet->u.bnetd_d2cs_accountloginreply.reply);
			if (result == BNETD_D2CS_CHARLOGINREPLY_SUCCEED) {
				reply = D2CS_CLIENT_LOGINREPLY_SUCCEED;
				account = packet_get_str_const(opacket, sizeof(t_client_d2cs_loginreq), MAX_CHARNAME_LEN);
				d2cs_conn_set_account(client, account);
				d2cs_conn_set_state(client, conn_state_authed);
				eventlog(eventlog_level_info, __FUNCTION__, "account %s authed", account);
			}
			else {
				eventlog(eventlog_level_warn, __FUNCTION__, "client %d login request was rejected by bnetd", sq_get_clientid(sq));
				reply = D2CS_CLIENT_LOGINREPLY_BADPASS;
			}
			if ((rpacket = packet_create(packet_class_d2cs))) {
				packet_set_size(rpacket, sizeof(t_d2cs_client_loginreply));
				packet_set_type(rpacket, D2CS_CLIENT_LOGINREPLY);
				bn_int_set(&rpacket->u.d2cs_client_loginreply.reply, reply);
				conn_push_outqueue(client, rpacket);
				packet_del_ref(rpacket);
			}
			sq_destroy(sq, &elem);
			return 0;
		}

		static int on_bnetd_charloginreply(t_connection * c, t_packet * packet)
		{
			unsigned int	seqno;
			t_sq		* sq;
			t_connection	* client;
			t_packet	* opacket, *rpacket;
			int		result, reply, type;
			char const	* charname;
			t_elem		* elem;

			if (!packet || !c)
				return -1;

			seqno = bn_int_get(packet->u.d2cs_bnetd.h.seqno);
			if (!(sq = sqlist_find_sq(seqno))) {
				eventlog(eventlog_level_error, __FUNCTION__, "seqno %d not found", seqno);
				return -1;
			}
			if (!(client = d2cs_connlist_find_connection_by_sessionnum(sq_get_clientid(sq)))) {
				eventlog(eventlog_level_error, __FUNCTION__, "client %d not found", sq_get_clientid(sq));
				sq_destroy(sq, &elem);
				return -1;
			}
			if (!(opacket = sq_get_packet(sq))) {
				eventlog(eventlog_level_error, __FUNCTION__, "previous packet missing (seqno: %d)", seqno);
				sq_destroy(sq, &elem);
				return -1;
			}
			type = packet_get_type(opacket);
			result = bn_int_get(packet->u.bnetd_d2cs_charloginreply.reply);
			if (type == CLIENT_D2CS_CREATECHARREQ) {
				charname = packet_get_str_const(opacket, sizeof(t_client_d2cs_createcharreq), MAX_CHARNAME_LEN);
				if (result == BNETD_D2CS_CHARLOGINREPLY_SUCCEED) {
					if (conn_check_multilogin(client, charname) < 0) {
						eventlog(eventlog_level_error, __FUNCTION__, "character %s is already logged in", charname);
						reply = D2CS_CLIENT_CHARLOGINREPLY_FAILED;
					}
					else {
						reply = D2CS_CLIENT_CREATECHARREPLY_SUCCEED;
						eventlog(eventlog_level_info, __FUNCTION__, "character %s authed", charname);
						d2cs_conn_set_charname(client, charname);
						d2cs_conn_set_state(client, conn_state_char_authed);
					}
				}
				else {
					reply = D2CS_CLIENT_CREATECHARREPLY_FAILED;
					eventlog(eventlog_level_error, __FUNCTION__, "failed to auth character %s", charname);
				}
				if ((rpacket = packet_create(packet_class_d2cs))) {
					packet_set_size(rpacket, sizeof(t_d2cs_client_createcharreply));
					packet_set_type(rpacket, D2CS_CLIENT_CREATECHARREPLY);
					bn_int_set(&rpacket->u.d2cs_client_createcharreply.reply, reply);
					conn_push_outqueue(client, rpacket);
					packet_del_ref(rpacket);
				}
			}
			else if (type == CLIENT_D2CS_CHARLOGINREQ) {
				charname = packet_get_str_const(opacket, sizeof(t_client_d2cs_charloginreq), MAX_CHARNAME_LEN);
				if (result == BNETD_D2CS_CHARLOGINREPLY_SUCCEED) {
					if (conn_check_multilogin(client, charname) < 0) {
						eventlog(eventlog_level_error, __FUNCTION__, "character %s is already logged in", charname);
						reply = D2CS_CLIENT_CHARLOGINREPLY_FAILED;
					}
					else {
						reply = D2CS_CLIENT_CHARLOGINREPLY_SUCCEED;
						eventlog(eventlog_level_info, __FUNCTION__, "character %s authed", charname);
						d2cs_conn_set_charname(client, charname);
						d2cs_conn_set_state(client, conn_state_char_authed);
					}
				}
				else {
					reply = D2CS_CLIENT_CHARLOGINREPLY_FAILED;
					eventlog(eventlog_level_error, __FUNCTION__, "failed to auth character %s", charname);
				}
				if ((rpacket = packet_create(packet_class_d2cs))) {
					packet_set_size(rpacket, sizeof(t_d2cs_client_charloginreply));
					packet_set_type(rpacket, D2CS_CLIENT_CHARLOGINREPLY);
					bn_int_set(&rpacket->u.d2cs_client_charloginreply.reply, reply);
					conn_push_outqueue(client, rpacket);
					packet_del_ref(rpacket);
				}
			}
			else {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad packet type %d", type);
				sq_destroy(sq, &elem);
				return -1;
			}
			sq_destroy(sq, &elem);
			return 0;
		}

		int on_bnetd_gameinforeq(t_connection * c, t_packet * packet)
		{
			t_packet *    rpacket;
			t_game *      game;

			char const * gamename;

			if (!(c)) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (!(gamename = packet_get_str_const(packet, sizeof(t_bnetd_d2cs_gameinforeq), MAX_GAMENAME_LEN)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "missing or too long gamename");
				return -1;
			}

			if (!(game = d2cs_gamelist_find_game(gamename)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "request for unknown game \"%s\"", gamename);
				return -1;
			}

			if ((rpacket = packet_create(packet_class_d2cs_bnetd))) {
				packet_set_size(rpacket, sizeof(t_d2cs_bnetd_gameinforeply));
				packet_set_type(rpacket, D2CS_BNETD_GAMEINFOREPLY);
				bn_int_set(&rpacket->u.d2cs_bnetd_gameinforeply.h.seqno, 0);
				packet_append_string(rpacket, gamename);

				bn_byte_set(&rpacket->u.d2cs_bnetd_gameinforeply.difficulty, game_get_gameflag_difficulty(game));

				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
			}
			return 0;
		}

	}

}
