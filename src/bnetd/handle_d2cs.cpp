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
#include "handle_d2cs.h"

#include <cstring>
#include <cstdio>

#include "compat/strcasecmp.h"
#include "common/eventlog.h"
#include "common/bn_type.h"
#include "common/addr.h"
#include "common/bnethash.h"
#include "common/bnethashconv.h"
#include "common/tag.h"
#include "common/util.h"

#include "realm.h"
#include "prefs.h"
#include "account_wrap.h"
#include "game.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static int on_d2cs_accountloginreq(t_connection * c, t_packet const * packet);
		static int on_d2cs_charloginreq(t_connection * c, t_packet const * packet);
		static int on_d2cs_authreply(t_connection * c, t_packet const * packet);
		static int on_d2cs_gameinforeply(t_connection * c, t_packet const * packet);

		extern int handle_d2cs_packet(t_connection * c, t_packet const * packet)
		{
			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (!packet) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
				return -1;
			}
			if (packet_get_class(packet) != packet_class_d2cs_bnetd) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad packet class %d",
					packet_get_class(packet));
				return -1;
			}
			switch (conn_get_state(c)) {
			case conn_state_connected:
				switch (packet_get_type(packet)) {
				case D2CS_BNETD_AUTHREPLY:
					on_d2cs_authreply(c, packet);
					break;
				default:
					eventlog(eventlog_level_error, __FUNCTION__,
						"got unknown packet type %d", packet_get_type(packet));
					break;
				}
				break;
			case conn_state_loggedin:
				switch (packet_get_type(packet)) {
				case D2CS_BNETD_ACCOUNTLOGINREQ:
					on_d2cs_accountloginreq(c, packet);
					break;
				case D2CS_BNETD_CHARLOGINREQ:
					on_d2cs_charloginreq(c, packet);
					break;
				case D2CS_BNETD_GAMEINFOREPLY:
					on_d2cs_gameinforeply(c, packet);
					break;
				default:
					eventlog(eventlog_level_error, __FUNCTION__,
						"got unknown packet type %d", packet_get_type(packet));
					break;
				}
				break;
			default:
				eventlog(eventlog_level_error, __FUNCTION__,
					"got unknown connection state %d", conn_get_state(c));
				break;
			}
			return 0;
		}

		static int on_d2cs_authreply(t_connection * c, t_packet const * packet)
		{
			t_packet	* rpacket;
			unsigned int	version;
			unsigned int	try_version;
			unsigned int	reply;
			char const	* realmname;
			t_realm		* realm;

			if (packet_get_size(packet) < sizeof(t_d2cs_bnetd_authreply)) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad packet size");
				return -1;
			}
			if (!(realmname = packet_get_str_const(packet, sizeof(t_d2cs_bnetd_authreply), MAX_REALMNAME_LEN))) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad realmname");
				return -1;
			}
			if (!(realm = realmlist_find_realm(realmname))) {
				realm = realmlist_find_realm_by_ip(conn_get_addr(c)); /* should not fail - checked in handle_init_packet() handle_init.c */
				eventlog(eventlog_level_warn, __FUNCTION__, "warn: realm name mismatch %s %s", realm_get_name(realm), realmname);
				if (!(prefs_allow_d2cs_setname())) { /* fail if allow_d2cs_setname = false */
					eventlog(eventlog_level_error, __FUNCTION__, "d2cs not allowed to set realm name");
					return -1;
				}
				if (realm_get_active(realm)) { /* fail if realm already active */
					eventlog(eventlog_level_error, __FUNCTION__, "cannot set realm name to %s (realm already active)", realmname);
					return -1;
				}
				realm_set_name(realm, realmname);
			}
			version = prefs_get_d2cs_version();
			try_version = bn_int_get(packet->u.d2cs_bnetd_authreply.version);
			if (version && version != try_version) {
				eventlog(eventlog_level_error, __FUNCTION__, "d2cs version mismatch 0x%X - 0x%X",
					try_version, version);
				reply = BNETD_D2CS_AUTHREPLY_BAD_VERSION;
			}
			else {
				reply = BNETD_D2CS_AUTHREPLY_SUCCEED;
			}

			if (reply == BNETD_D2CS_AUTHREPLY_SUCCEED) {
				eventlog(eventlog_level_info, __FUNCTION__, "d2cs %s authed",
					addr_num_to_ip_str(conn_get_addr(c)));
				conn_set_state(c, conn_state_loggedin);
				realm_active(realm, c);
			}
			else {
				eventlog(eventlog_level_error, __FUNCTION__, "failed to auth d2cs %s",
					addr_num_to_ip_str(conn_get_addr(c)));
			}
			if ((rpacket = packet_create(packet_class_d2cs_bnetd))) {
				packet_set_size(rpacket, sizeof(t_bnetd_d2cs_authreply));
				packet_set_type(rpacket, BNETD_D2CS_AUTHREPLY);
				bn_int_set(&rpacket->u.bnetd_d2cs_authreply.h.seqno, 1);
				bn_int_set(&rpacket->u.bnetd_d2cs_authreply.reply, reply);
				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
			}
			return 0;
		}

		static int on_d2cs_accountloginreq(t_connection * c, t_packet const * packet)
		{
			unsigned int	sessionkey;
			unsigned int	sessionnum;
			unsigned int	salt;
			char const *	account;
			char const *	tname;
			t_connection	* client;
			int		reply;
			t_packet	* rpacket;
			struct
			{
				bn_int   salt;
				bn_int   sessionkey;
				bn_int   sessionnum;
				bn_int   secret;
				bn_int	 passhash[5];
			} temp;
			t_hash       secret_hash;
			char const * pass_str;
			t_hash	     passhash;
			t_hash	     try_hash;

			if (packet_get_size(packet) < sizeof(t_d2cs_bnetd_accountloginreq)) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad packet size");
				return -1;
			}
			if (!(account = packet_get_str_const(packet, sizeof(t_d2cs_bnetd_accountloginreq), MAX_USERNAME_LEN))) {
				eventlog(eventlog_level_error, __FUNCTION__, "missing or too long account name");
				return -1;
			}
			sessionkey = bn_int_get(packet->u.d2cs_bnetd_accountloginreq.sessionkey);
			sessionnum = bn_int_get(packet->u.d2cs_bnetd_accountloginreq.sessionnum);
			salt = bn_int_get(packet->u.d2cs_bnetd_accountloginreq.seqno);
			if (!(client = connlist_find_connection_by_sessionnum(sessionnum))) {
				eventlog(eventlog_level_error, __FUNCTION__, "sessionnum %d not found", sessionnum);
				reply = BNETD_D2CS_ACCOUNTLOGINREPLY_FAILED;
			}
			else if (sessionkey != conn_get_sessionkey(client)) {
				eventlog(eventlog_level_error, __FUNCTION__, "sessionkey %d not match", sessionkey);
				reply = BNETD_D2CS_ACCOUNTLOGINREPLY_FAILED;
			}
			else if (!(tname = conn_get_username(client))) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL username");
				reply = BNETD_D2CS_ACCOUNTLOGINREPLY_FAILED;
			}
			else if (strcasecmp(account, tname)) {
				eventlog(eventlog_level_error, __FUNCTION__, "username %s not match", account);
				reply = BNETD_D2CS_ACCOUNTLOGINREPLY_FAILED;
			}
			else {
				bn_int_set(&temp.salt, salt);
				bn_int_set(&temp.sessionkey, sessionkey);
				bn_int_set(&temp.sessionnum, sessionnum);
				bn_int_set(&temp.secret, conn_get_secret(client));
				pass_str = account_get_pass(conn_get_account(client));
				if (hash_set_str(&passhash, pass_str) < 0) {
					reply = BNETD_D2CS_ACCOUNTLOGINREPLY_FAILED;
				}
				else {
					hash_to_bnhash((t_hash const *)&passhash, temp.passhash);
					bnet_hash(&secret_hash, sizeof(temp), &temp);
					bnhash_to_hash(packet->u.d2cs_bnetd_accountloginreq.secret_hash, &try_hash);
					if (hash_eq(try_hash, secret_hash) == 1) {
						eventlog(eventlog_level_debug, __FUNCTION__, "user %s loggedin on d2cs",
							account);
						reply = BNETD_D2CS_ACCOUNTLOGINREPLY_SUCCEED;
					}
					else {
						eventlog(eventlog_level_error, __FUNCTION__, "user %s hash not match",
							account);
						reply = BNETD_D2CS_ACCOUNTLOGINREPLY_FAILED;
					}
				}
			}
			if ((rpacket = packet_create(packet_class_d2cs_bnetd))) {
				packet_set_size(rpacket, sizeof(t_bnetd_d2cs_accountloginreply));
				packet_set_type(rpacket, BNETD_D2CS_ACCOUNTLOGINREPLY);
				bn_int_set(&rpacket->u.bnetd_d2cs_accountloginreply.h.seqno,
					bn_int_get(packet->u.d2cs_bnetd_accountloginreq.h.seqno));
				bn_int_set(&rpacket->u.bnetd_d2cs_accountloginreply.reply, reply);
				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
			}
			return 0;
		}

#define CHAR_PORTRAIT_LEN	0x30
		static int on_d2cs_charloginreq(t_connection * c, t_packet const * packet)
		{
			t_connection *	client;
			char const *	charname;
			char const *	portrait;
			char const *	clienttag;
			char *	temp;
			unsigned int	sessionnum;
			t_realm * 	realm;
			char const *	realmname;
			unsigned int	pos, reply;
			t_packet *	rpacket;

			if (packet_get_size(packet) < sizeof(t_d2cs_bnetd_charloginreq)) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad packet size");
				return -1;
			}
			sessionnum = bn_int_get(packet->u.d2cs_bnetd_charloginreq.sessionnum);
			pos = sizeof(t_d2cs_bnetd_charloginreq);
			if (!(charname = packet_get_str_const(packet, pos, MAX_CHARNAME_LEN))) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad character name");
				return -1;
			}
			pos += std::strlen(charname) + 1;
			if (!(portrait = packet_get_str_const(packet, pos, CHAR_PORTRAIT_LEN))) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad character portrait");
				return -1;
			}
			if (!(client = connlist_find_connection_by_sessionnum(sessionnum))) {
				eventlog(eventlog_level_error, __FUNCTION__, "user %d not found", sessionnum);
				reply = BNETD_D2CS_CHARLOGINREPLY_FAILED;
			}
			else if (!(clienttag = clienttag_uint_to_str(conn_get_clienttag(client)))) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clienttag");
				reply = BNETD_D2CS_CHARLOGINREPLY_FAILED;
			}
			else if (!(realm = conn_get_realm(client))) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL realm");
				reply = BNETD_D2CS_CHARLOGINREPLY_FAILED;
			}
			else {
				char revtag[8];

				realmname = realm_get_name(realm);
				temp = (char*)xmalloc(std::strlen(clienttag) + std::strlen(realmname) + 1 + std::strlen(charname) + 1 +
					std::strlen(portrait) + 1);
				reply = BNETD_D2CS_CHARLOGINREPLY_SUCCEED;
				std::strcpy(revtag, clienttag);
				strreverse(revtag);
				std::sprintf(temp, "%4s%s,%s,%s", revtag, realmname, charname, portrait);
				conn_set_charname(client, charname);
				conn_set_realminfo(client, temp);
				xfree(temp);
				eventlog(eventlog_level_debug, __FUNCTION__,
					"loaded portrait for character %s", charname);
			}
			if ((rpacket = packet_create(packet_class_d2cs_bnetd))) {
				packet_set_size(rpacket, sizeof(t_bnetd_d2cs_charloginreply));
				packet_set_type(rpacket, BNETD_D2CS_CHARLOGINREPLY);
				bn_int_set(&rpacket->u.bnetd_d2cs_charloginreply.h.seqno,
					bn_int_get(packet->u.d2cs_bnetd_charloginreq.h.seqno));
				bn_int_set(&rpacket->u.bnetd_d2cs_charloginreply.reply, reply);
				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
			}
			return 0;
		}

		extern int handle_d2cs_init(t_connection * c)
		{
			t_packet	* packet;

			if ((packet = packet_create(packet_class_d2cs_bnetd))) {
				packet_set_size(packet, sizeof(t_bnetd_d2cs_authreq));
				packet_set_type(packet, BNETD_D2CS_AUTHREQ);
				bn_int_set(&packet->u.bnetd_d2cs_authreq.h.seqno, 1);
				bn_int_set(&packet->u.bnetd_d2cs_authreq.sessionnum, conn_get_sessionnum(c));
				conn_push_outqueue(c, packet);
				packet_del_ref(packet);
			}
			eventlog(eventlog_level_info, __FUNCTION__, "sent init packet to d2cs (sessionnum=%d)",
				conn_get_sessionnum(c));
			return 0;
		}

		extern int send_d2cs_gameinforeq(t_connection * c)
		{
			t_packet	* packet;
			t_game		* game;
			t_realm		* realm;

			if (!(c))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return -1;
			}

			if (!(game = conn_get_game(c)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "conn had NULL game");
				return -1;
			}

			if (!(realm = conn_get_realm(c)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "conn had NULL realm");
				return -1;
			}


			if ((packet = packet_create(packet_class_d2cs_bnetd))) {
				packet_set_size(packet, sizeof(t_bnetd_d2cs_gameinforeq));
				packet_set_type(packet, BNETD_D2CS_GAMEINFOREQ);
				bn_int_set(&packet->u.bnetd_d2cs_gameinforeq.h.seqno, 0);
				packet_append_string(packet, game_get_name(game));
				conn_push_outqueue(realm_get_conn(realm), packet);
				packet_del_ref(packet);
			}
			return 0;
		}

		static int on_d2cs_gameinforeply(t_connection * c, t_packet const * packet)
		{
			t_game *		game;
			char const *		gamename;
			unsigned int		difficulty;
			t_game_difficulty	diff;

			if (!(c)) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (!(gamename = packet_get_str_const(packet, sizeof(t_d2cs_bnetd_gameinforeply), MAX_GAMENAME_LEN)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "missing or too long gamename");
				return -1;
			}

			if (!(game = gamelist_find_game(gamename, CLIENTTAG_DIABLO2DV_UINT, game_type_diablo2closed))
				&& !(game = gamelist_find_game(gamename, CLIENTTAG_DIABLO2XP_UINT, game_type_diablo2closed)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "reply for unknown game \"%s\"", gamename);
				return -1;
			}

			difficulty = bn_byte_get(packet->u.d2cs_bnetd_gameinforeply.difficulty);

			switch (difficulty)
			{
			case 0:
				diff = game_difficulty_normal;
				break;
			case 1:
				diff = game_difficulty_nightmare;
				break;
			case 2:
				diff = game_difficulty_hell;
				break;
			default:
				diff = game_difficulty_none;
			}

			game_set_difficulty(game, diff);

			return 0;
		}

	}

}
