/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
 * Copyright (C) 2004		Olaf Freyer (aaron@cs.tu-berlin.de)
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
#include "handle_d2cs.h"

#include <cstring>
#include <ctime>

#include "compat/mkdir.h"
#include "compat/pdir.h"
#include "compat/psock.h"
#include "compat/statmacros.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/d2cs_d2dbs_ladder.h"
#include "game.h"
#include "bnetd.h"
#include "serverqueue.h"
#include "prefs.h"
#include "d2ladder.h"
#include "d2charfile.h"
#include "d2charlist.h"
#include "common/setup_after.h"


namespace pvpgn
{

namespace d2cs
{

static int d2cs_send_client_ladder(t_connection * c, unsigned char type, unsigned short from);
static unsigned int d2cs_try_joingame(t_connection const * c, t_game const * game, char const * gamepass);

DECLARE_PACKET_HANDLER(on_client_loginreq)
DECLARE_PACKET_HANDLER(on_client_createcharreq)
DECLARE_PACKET_HANDLER(on_client_creategamereq)
DECLARE_PACKET_HANDLER(on_client_joingamereq)
DECLARE_PACKET_HANDLER(on_client_gamelistreq)
DECLARE_PACKET_HANDLER(on_client_gameinforeq)
DECLARE_PACKET_HANDLER(on_client_charloginreq)
DECLARE_PACKET_HANDLER(on_client_deletecharreq)
DECLARE_PACKET_HANDLER(on_client_ladderreq)
DECLARE_PACKET_HANDLER(on_client_motdreq)
DECLARE_PACKET_HANDLER(on_client_cancelcreategame)
DECLARE_PACKET_HANDLER(on_client_charladderreq)
DECLARE_PACKET_HANDLER(on_client_charlistreq)
DECLARE_PACKET_HANDLER(on_client_charlistreq_110)
DECLARE_PACKET_HANDLER(on_client_convertcharreq)


static t_packet_handle_table d2cs_packet_handle_table[]={
/* 0x00 */ { 0,                                     conn_state_none,                          NULL                      },
/* 0x01 */ { sizeof(t_client_d2cs_loginreq),        conn_state_connected,                     on_client_loginreq        },
/* 0x02 */ { sizeof(t_client_d2cs_createcharreq),   conn_state_authed|conn_state_char_authed, on_client_createcharreq   },
/* 0x03 */ { sizeof(t_client_d2cs_creategamereq),   conn_state_char_authed,                   on_client_creategamereq   },
/* 0x04 */ { sizeof(t_client_d2cs_joingamereq),     conn_state_char_authed,                   on_client_joingamereq     },
/* 0x05 */ { sizeof(t_client_d2cs_gamelistreq),     conn_state_char_authed,                   on_client_gamelistreq     },
/* 0x06 */ { sizeof(t_client_d2cs_gameinforeq),     conn_state_char_authed,                   on_client_gameinforeq     },
/* 0x07 */ { sizeof(t_client_d2cs_charloginreq),    conn_state_authed|conn_state_char_authed, on_client_charloginreq    },
/* 0x08 */ { 0,                                     conn_state_none,                          NULL                      },
/* 0x09 */ { 0,                                     conn_state_none,                          NULL                      },
/* 0x0a */ { sizeof(t_client_d2cs_deletecharreq),   conn_state_authed|conn_state_char_authed, on_client_deletecharreq   },
/* 0x0b */ { 0,                                     conn_state_none,                          NULL                      },
/* 0x0c */ { 0,                                     conn_state_none,                          NULL                      },
/* 0x0d */ { 0,                                     conn_state_none,                          NULL                      },
/* 0x0e */ { 0,                                     conn_state_none,                          NULL                      },
/* 0x0f */ { 0,                                     conn_state_none,                          NULL                      },
/* 0x10 */ { 0,                                     conn_state_none,                          NULL                      },
/* 0x11 */ { sizeof(t_client_d2cs_ladderreq),       conn_state_char_authed,                   on_client_ladderreq	},
/* 0x12 */ { sizeof(t_client_d2cs_motdreq),         conn_state_char_authed,                   on_client_motdreq         },
/* 0x13 */ { sizeof(t_client_d2cs_cancelcreategame),conn_state_char_authed,                   on_client_cancelcreategame},
/* 0x14 */ { 0,                                     conn_state_none,                          NULL                      },
/* 0x15 */ { 0,                                     conn_state_none,                          NULL                      },
/* 0x16 */ { sizeof(t_client_d2cs_charladderreq),   conn_state_char_authed,                   on_client_charladderreq	},
/* 0x17 */ { sizeof(t_client_d2cs_charlistreq),     conn_state_authed|conn_state_char_authed, on_client_charlistreq	},
/* 0x18 */ { sizeof(t_client_d2cs_convertcharreq),  conn_state_authed|conn_state_char_authed, on_client_convertcharreq  },
/* 0x19 */ { sizeof(t_client_d2cs_charlistreq_110), conn_state_authed|conn_state_char_authed, on_client_charlistreq_110 }
};


extern int d2cs_handle_d2cs_packet(t_connection * c, t_packet * packet)
{
	return conn_process_packet(c,packet,d2cs_packet_handle_table,NELEMS(d2cs_packet_handle_table));
}

static int on_client_loginreq(t_connection * c, t_packet * packet)
{
	t_packet	* bnpacket;
	char const	* account;
	t_sq		* sq;
	unsigned int	sessionnum;

	if (!(account=packet_get_str_const(packet,sizeof(t_client_d2cs_loginreq),MAX_CHARNAME_LEN))) {
		eventlog(eventlog_level_error,__FUNCTION__,"got bad account name");
		return -1;
	}
	if (d2char_check_acctname(account)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"got bad account name");
		return -1;
	}
	if (!bnetd_conn()) {
		eventlog(eventlog_level_warn,__FUNCTION__,"d2cs is offline with bnetd, login request will be rejected");
		return -1;
	}
	sessionnum=bn_int_get(packet->u.client_d2cs_loginreq.sessionnum);
	conn_set_bnetd_sessionnum(c,sessionnum);
	eventlog(eventlog_level_info,__FUNCTION__,"got client (*%s) login request sessionnum=0x%X",account,sessionnum);
	if ((bnpacket=packet_create(packet_class_d2cs_bnetd))) {
		if ((sq=sq_create(d2cs_conn_get_sessionnum(c),packet,0))) {
			packet_set_size(bnpacket,sizeof(t_d2cs_bnetd_accountloginreq));
			packet_set_type(bnpacket,D2CS_BNETD_ACCOUNTLOGINREQ);
			bn_int_set(&bnpacket->u.d2cs_bnetd_accountloginreq.h.seqno,sq_get_seqno(sq));
			bn_int_set(&bnpacket->u.d2cs_bnetd_accountloginreq.seqno,
				bn_int_get(packet->u.client_d2cs_loginreq.seqno));
			bn_int_set(&bnpacket->u.d2cs_bnetd_accountloginreq.sessionkey,
				bn_int_get(packet->u.client_d2cs_loginreq.sessionkey));
			bn_int_set(&bnpacket->u.d2cs_bnetd_accountloginreq.sessionnum,sessionnum);
			std::memcpy(bnpacket->u.d2cs_bnetd_accountloginreq.secret_hash,
				packet->u.client_d2cs_loginreq.secret_hash,
				sizeof(bnpacket->u.d2cs_bnetd_accountloginreq.secret_hash));
			packet_append_string(bnpacket,account);
			conn_push_outqueue(bnetd_conn(),bnpacket);
		}
		packet_del_ref(bnpacket);
	}
	return 0;
}

static int on_client_createcharreq(t_connection * c, t_packet * packet)
{
	t_packet	* rpacket, * bnpacket;
	char const	* charname;
	char const	* account;
	char            * path;
	t_sq		* sq;
	unsigned int	reply;
	unsigned short	status, chclass;
	t_d2charinfo_file	data;

	if (!(charname=packet_get_str_const(packet,sizeof(t_client_d2cs_createcharreq),MAX_CHARNAME_LEN))) {
		eventlog(eventlog_level_error,__FUNCTION__,"got bad character name");
		return -1;
	}
	if (!(account=d2cs_conn_get_account(c))) {
		eventlog(eventlog_level_error,__FUNCTION__,"missing account for character %s",charname);
		return -1;
	}
	chclass=bn_short_get(packet->u.client_d2cs_createcharreq.chclass);
	status=bn_short_get(packet->u.client_d2cs_createcharreq.status);

	path=(char*)xmalloc(std::strlen(prefs_get_charinfo_dir())+1+std::strlen(account)+1);
	d2char_get_infodir_name(path,account);
	try {
		Directory dir(path);
	} catch (const Directory::OpenError&) {
		INFO1("(*%s) charinfo directory do not exist, building it",account);
		p_mkdir(path,S_IRWXU);
	}
	xfree(path);

	if (d2char_create(account,charname,chclass,status)<0) {
		eventlog(eventlog_level_warn,__FUNCTION__,"error create character %s for account %s",charname,account);
		reply=D2CS_CLIENT_CREATECHARREPLY_ALREADY_EXIST;
	} else if (d2charinfo_load(account,charname,&data)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error loading charinfo for character %s(*%s)",charname,account);
		reply=D2CS_CLIENT_CREATECHARREPLY_FAILED;
	} else {
		eventlog(eventlog_level_info,__FUNCTION__,"character %s(*%s) created",charname,account);
		reply=D2CS_CLIENT_CREATECHARREPLY_SUCCEED;
		conn_set_charinfo(c,&data.summary);
		if ((bnpacket=packet_create(packet_class_d2cs_bnetd))) {
			if ((sq=sq_create(d2cs_conn_get_sessionnum(c),packet,0))) {
				packet_set_size(bnpacket,sizeof(t_d2cs_bnetd_charloginreq));
				packet_set_type(bnpacket,D2CS_BNETD_CHARLOGINREQ);
				bn_int_set(&bnpacket->u.d2cs_bnetd_charloginreq.h.seqno,sq_get_seqno(sq));
				bn_int_set(&bnpacket->u.d2cs_bnetd_charloginreq.sessionnum,
					conn_get_bnetd_sessionnum(c));
				packet_append_string(bnpacket,charname);
				packet_append_string(bnpacket,(char const *)&data.portrait);
				conn_push_outqueue(bnetd_conn(),bnpacket);
			}
			packet_del_ref(bnpacket);
		}
		return 0;
	}
	if ((rpacket=packet_create(packet_class_d2cs))) {
		packet_set_size(rpacket,sizeof(t_d2cs_client_createcharreply));
		packet_set_type(rpacket,D2CS_CLIENT_CREATECHARREPLY);
		bn_int_set(&rpacket->u.d2cs_client_createcharreply.reply,reply);
		conn_push_outqueue(c,rpacket);
		packet_del_ref(rpacket);
	}
	return 0;
}

static int on_client_creategamereq(t_connection * c, t_packet * packet)
{
	char const	* gamename;
	char const	* gamepass;
	char const	* gamedesc;
	t_game		* game;
	t_d2gs		* gs;
	t_gq		* gq;
	unsigned int	tempflag,gameflag;
	unsigned int	leveldiff, maxchar, difficulty, expansion, hardcore, ladder;
	unsigned int	seqno, reply;
	unsigned int	pos;
	t_elem		* elem;

	pos=sizeof(t_client_d2cs_creategamereq);
	if (!(gamename=packet_get_str_const(packet,pos,MAX_GAMENAME_LEN))) {
		eventlog(eventlog_level_error,__FUNCTION__,"got bad game name");
		return -1;
	}
	pos+=std::strlen(gamename)+1;
	if (!(gamepass=packet_get_str_const(packet,pos,MAX_GAMEPASS_LEN))) {
		eventlog(eventlog_level_error,__FUNCTION__,"got bad game pass");
		return -1;
	}
	pos+=std::strlen(gamepass)+1;
	if (!(gamedesc=packet_get_str_const(packet,pos,MAX_GAMEDESC_LEN))) {
		eventlog(eventlog_level_error,__FUNCTION__,"got bad game desc");
		return -1;
	}
	tempflag=bn_int_get(packet->u.client_d2cs_creategamereq.gameflag);
	leveldiff=bn_byte_get(packet->u.client_d2cs_creategamereq.leveldiff);
	maxchar=bn_byte_get(packet->u.client_d2cs_creategamereq.maxchar);
	difficulty=gameflag_get_difficulty(tempflag);
	if (difficulty > conn_get_charinfo_difficulty(c)) {
		eventlog(eventlog_level_error,__FUNCTION__,"game difficulty exceed character limit %d %d",difficulty,
			conn_get_charinfo_difficulty(c));
		return 0;
	}
	expansion=conn_get_charinfo_expansion(c);
	hardcore=conn_get_charinfo_hardcore(c);
	ladder=conn_get_charinfo_ladder(c);
	gameflag=gameflag_create(ladder,expansion,hardcore,difficulty);

	gs = NULL;
	game = NULL;
	gq=conn_get_gamequeue(c);
	if (d2cs_gamelist_find_game(gamename)) {
		eventlog(eventlog_level_info,__FUNCTION__,"game name %s is already exist in gamelist",gamename);
		reply=D2CS_CLIENT_CREATEGAMEREPLY_NAME_EXIST;
	} else if (!gq && gqlist_find_game(gamename)) {
		eventlog(eventlog_level_info,__FUNCTION__,"game name %s is already exist in game queue",gamename);
		reply=D2CS_CLIENT_CREATEGAMEREPLY_NAME_EXIST;
	} else if (!(gs=d2gslist_choose_server())) {
		if (gq) {
			eventlog(eventlog_level_error,__FUNCTION__,"client %d is already in game queue",d2cs_conn_get_sessionnum(c));
			conn_set_gamequeue(c,NULL);
			gq_destroy(gq,&elem);
			return 0;
		} else if ((gq=gq_create(d2cs_conn_get_sessionnum(c), packet, gamename))) {
			conn_set_gamequeue(c,gq);
			d2cs_send_client_creategamewait(c,gqlist_get_length());
			return 0;
		}
		reply=D2CS_CLIENT_CREATEGAMEREPLY_FAILED;
	} else if (hardcore && conn_get_charinfo_dead(c)) {
		reply=D2CS_CLIENT_CREATEGAMEREPLY_FAILED;
	} else if (!(game=d2cs_game_create(gamename,gamepass,gamedesc,gameflag))) {
		reply=D2CS_CLIENT_CREATEGAMEREPLY_NAME_EXIST;
	} else {
		reply=D2CS_CLIENT_CREATEGAMEREPLY_SUCCEED;
		game_set_d2gs(game,gs);
		d2gs_add_gamenum(gs, 1);
		game_set_gameflag_ladder(game,ladder);
		game_set_gameflag_expansion(game,expansion);
		game_set_created(game,0);
		game_set_leveldiff(game,leveldiff);
		game_set_charlevel(game,conn_get_charinfo_level(c));
		game_set_maxchar(game,maxchar);
		game_set_gameflag_difficulty(game,difficulty);
		game_set_gameflag_hardcore(game,hardcore);
	}

	seqno=bn_short_get(packet->u.client_d2cs_creategamereq.seqno);
	if (reply!=D2CS_CLIENT_CREATEGAMEREPLY_SUCCEED) {
		t_packet	* rpacket;

		if ((rpacket=packet_create(packet_class_d2cs))) {
			packet_set_size(rpacket,sizeof(t_d2cs_client_creategamereply));
			packet_set_type(rpacket,D2CS_CLIENT_CREATEGAMEREPLY);
			bn_short_set(&rpacket->u.d2cs_client_creategamereply.seqno,seqno);
			bn_short_set(&rpacket->u.d2cs_client_creategamereply.u1,0);
			bn_short_set(&rpacket->u.d2cs_client_creategamereply.gameid,0);
			bn_int_set(&rpacket->u.d2cs_client_creategamereply.reply,reply);
			conn_push_outqueue(c,rpacket);
			packet_del_ref(rpacket);
		}
	} else {
		t_packet	* gspacket;
		t_sq		* sq;
		struct in_addr	addr;

		if ((gspacket=packet_create(packet_class_d2gs))) {
			if ((sq=sq_create(d2cs_conn_get_sessionnum(c),packet,d2cs_game_get_id(game)))) {
				packet_set_size(gspacket,sizeof(t_d2cs_d2gs_creategamereq));
				packet_set_type(gspacket,D2CS_D2GS_CREATEGAMEREQ);
				bn_int_set(&gspacket->u.d2cs_d2gs_creategamereq.h.seqno,sq_get_seqno(sq));
				bn_byte_set(&gspacket->u.d2cs_d2gs_creategamereq.difficulty,difficulty);
				bn_byte_set(&gspacket->u.d2cs_d2gs_creategamereq.hardcore,hardcore);
				bn_byte_set(&gspacket->u.d2cs_d2gs_creategamereq.expansion,expansion);
				bn_byte_set(&gspacket->u.d2cs_d2gs_creategamereq.ladder,ladder);
				packet_append_string(gspacket,gamename);
				packet_append_string(gspacket,gamepass);
				packet_append_string(gspacket,gamedesc);
				packet_append_string(gspacket,d2cs_conn_get_account(c));
				packet_append_string(gspacket,d2cs_conn_get_charname(c));
				addr.s_addr = htonl(d2cs_conn_get_addr(c));
				packet_append_string(gspacket,inet_ntoa(addr));
				conn_push_outqueue(d2gs_get_connection(gs),gspacket);
			}
			packet_del_ref(gspacket);
			eventlog(eventlog_level_info,__FUNCTION__,"request create game %s on gs %d",gamename,d2gs_get_id(gs));
		}
	}
	return 0;
}

static int on_client_joingamereq(t_connection * c, t_packet * packet)
{
	char const	* gamename;
	char const	* gamepass;
	char const	* charname;
	char const	* account;
	t_game		* game;
	t_d2gs		* gs;
	int		reply;
	unsigned int	pos;
	unsigned int	seqno;

	gs = NULL;
	pos=sizeof(t_client_d2cs_joingamereq);
	if (!(gamename=packet_get_str_const(packet,pos,MAX_GAMENAME_LEN))) {
		eventlog(eventlog_level_error,__FUNCTION__,"got bad game name");
		return -1;
	}
	pos+=std::strlen(gamename)+1;
	if (!(gamepass=packet_get_str_const(packet,pos,MAX_GAMEPASS_LEN))) {
		eventlog(eventlog_level_error,__FUNCTION__,"got bad game pass");
		return -1;
	}
	if (!(charname=d2cs_conn_get_charname(c))) {
		eventlog(eventlog_level_error,__FUNCTION__,"missing character name for connection");
		return -1;
	}
	if (!(account=d2cs_conn_get_account(c))) {
		eventlog(eventlog_level_error,__FUNCTION__,"missing account for connection");
		return -1;
	}
	if (conn_check_multilogin(c,charname)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"character %s is already logged in",charname);
		return -1;
	}
	if (!(game=d2cs_gamelist_find_game(gamename))) {
		eventlog(eventlog_level_info,__FUNCTION__,"game %s not found",gamename);
		reply=D2CS_CLIENT_JOINGAMEREPLY_NOT_EXIST;
	} else if (!(gs=game_get_d2gs(game))) {
		eventlog(eventlog_level_error,__FUNCTION__,"missing game server for game %s",gamename);
		reply=D2CS_CLIENT_JOINGAMEREPLY_NOT_EXIST;
	} else {
		reply=d2cs_try_joingame(c,game,gamepass);
	}

	seqno=bn_short_get(packet->u.client_d2cs_joingamereq.seqno);
	if (reply!=D2CS_CLIENT_JOINGAMEREPLY_SUCCEED) {
		t_packet	* rpacket;

		if ((rpacket=packet_create(packet_class_d2cs))) {
			packet_set_size(rpacket,sizeof(t_d2cs_client_joingamereply));
			packet_set_type(rpacket,D2CS_CLIENT_JOINGAMEREPLY);
			bn_short_set(&rpacket->u.d2cs_client_joingamereply.seqno,seqno);
			bn_short_set(&rpacket->u.d2cs_client_joingamereply.u1,0);
			bn_short_set(&rpacket->u.d2cs_client_joingamereply.gameid,0);
			bn_int_set(&rpacket->u.d2cs_client_joingamereply.addr,0);
			bn_int_set(&rpacket->u.d2cs_client_joingamereply.token,0);
			bn_int_set(&rpacket->u.d2cs_client_joingamereply.reply,reply);
			conn_push_outqueue(c,rpacket);
			packet_del_ref(rpacket);
		}
	} else {
		t_packet	* gspacket;
		t_sq		* sq;
		struct in_addr	addr;

		if ((gspacket=packet_create(packet_class_d2gs))) {
			if ((sq=sq_create(d2cs_conn_get_sessionnum(c),packet,d2cs_game_get_id(game)))) {
				packet_set_size(gspacket,sizeof(t_d2cs_d2gs_joingamereq));
				packet_set_type(gspacket,D2CS_D2GS_JOINGAMEREQ);
				bn_int_set(&gspacket->u.d2cs_d2gs_joingamereq.h.seqno,sq_get_seqno(sq));
				bn_int_set(&gspacket->u.d2cs_d2gs_joingamereq.gameid,
					game_get_d2gs_gameid(game));
				sq_set_gametoken(sq,d2gs_make_token(gs));
				bn_int_set(&gspacket->u.d2cs_d2gs_joingamereq.token,sq_get_gametoken(sq));
				packet_append_string(gspacket,charname);
				packet_append_string(gspacket,account);
				addr.s_addr = htonl(d2cs_conn_get_addr(c));
				packet_append_string(gspacket,inet_ntoa(addr));
				conn_push_outqueue(d2gs_get_connection(gs),gspacket);
			}
			packet_del_ref(gspacket);
			eventlog(eventlog_level_info,__FUNCTION__,"request join game %s for character %s on gs %d",gamename,
				charname,d2gs_get_id(gs));
		}
	}
	return 0;
}

static int on_client_gamelistreq(t_connection * c, t_packet * packet)
{
	t_packet	* rpacket;
	t_game		* game;
	unsigned int	count;
	unsigned int	seqno;
	std::time_t		now;
	unsigned int	maxlifetime;
	t_elem const	* start_elem;
	t_elem const	* elem;

	seqno=bn_short_get(packet->u.client_d2cs_gamelistreq.seqno);
	/* if (seqno%2) return 0; */
	count=0;
	now=std::time(NULL);
	maxlifetime=prefs_get_game_maxlifetime();

	elem=start_elem=gamelist_get_curr_elem();
	if (!elem) elem=list_get_first_const(d2cs_gamelist());
	else elem=elem_get_next_const(d2cs_gamelist(),elem);

	for (; elem != start_elem; elem=elem_get_next_const(d2cs_gamelist(),elem)) {
		if (!elem) {
			elem=list_get_first_const(d2cs_gamelist());
			if (elem == start_elem) break;
		}
		if (!(game=(t_game*)elem_get_data(elem))) {
			eventlog(eventlog_level_error,__FUNCTION__,"got NULL game");
			break;
		}
		if (maxlifetime && (now-game->create_time>maxlifetime)) continue;
		if (!game_get_currchar(game)) continue;
		if (!prefs_allow_gamelist_showall()) {
			if (conn_get_charinfo_difficulty(c)!=game_get_gameflag_difficulty(game)) continue;
		}
		if (prefs_hide_pass_games())
			if (d2cs_game_get_pass(game)) continue;

		if (d2cs_try_joingame(c,game,"")!=D2CS_CLIENT_JOINGAMEREPLY_SUCCEED) continue;
		if ((rpacket=packet_create(packet_class_d2cs))) {
			packet_set_size(rpacket,sizeof(t_d2cs_client_gamelistreply));
			packet_set_type(rpacket,D2CS_CLIENT_GAMELISTREPLY);
			bn_short_set(&rpacket->u.d2cs_client_gamelistreply.seqno,seqno);
			bn_int_set(&rpacket->u.d2cs_client_gamelistreply.token,d2cs_game_get_id(game));
			bn_byte_set(&rpacket->u.d2cs_client_gamelistreply.currchar,game_get_currchar(game));
			bn_int_set(&rpacket->u.d2cs_client_gamelistreply.gameflag,game_get_gameflag(game));
			packet_append_string(rpacket,d2cs_game_get_name(game));
			packet_append_string(rpacket,game_get_desc(game));
			conn_push_outqueue(c,rpacket);
			packet_del_ref(rpacket);
			count++;
			if (prefs_get_maxgamelist() && count>=prefs_get_maxgamelist()) break;
		}
	}
	gamelist_set_curr_elem(elem);
	if (count) {
		if ((rpacket=packet_create(packet_class_d2cs))) {
			packet_set_size(rpacket,sizeof(t_d2cs_client_gamelistreply));
			packet_set_type(rpacket,D2CS_CLIENT_GAMELISTREPLY);
			bn_short_set(&rpacket->u.d2cs_client_gamelistreply.seqno,seqno);
			bn_int_set(&rpacket->u.d2cs_client_gamelistreply.token,0);
			bn_byte_set(&rpacket->u.d2cs_client_gamelistreply.currchar,0);
			bn_int_set(&rpacket->u.d2cs_client_gamelistreply.gameflag,0);
			packet_append_string(rpacket,"");
			packet_append_string(rpacket,"");
			packet_append_string(rpacket,"");
			conn_push_outqueue(c,rpacket);
			packet_del_ref(rpacket);
		}
	}
	return 0;
}

static int on_client_gameinforeq(t_connection * c, t_packet * packet)
{
	t_game_charinfo	* info;
	t_packet	* rpacket;
	char const	* gamename;
	t_game		* game;
	unsigned int	seqno, n;

	if (!(gamename=packet_get_str_const(packet,sizeof(t_client_d2cs_gameinforeq),MAX_GAMENAME_LEN))) {
		eventlog(eventlog_level_error,__FUNCTION__,"got bad game name");
		return -1;
	}
	if (!(game=d2cs_gamelist_find_game(gamename))) {
		eventlog(eventlog_level_error,__FUNCTION__,"game %s not found",gamename);
		return 0;
	}
	seqno=bn_short_get(packet->u.client_d2cs_gameinforeq.seqno);
	if ((rpacket=packet_create(packet_class_d2cs))) {
		packet_set_size(rpacket,sizeof(t_d2cs_client_gameinforeply));
		packet_set_type(rpacket,D2CS_CLIENT_GAMEINFOREPLY);
		bn_short_set(&rpacket->u.d2cs_client_gameinforeply.seqno,seqno);
		bn_int_set(&rpacket->u.d2cs_client_gameinforeply.gameflag,game_get_gameflag(game));
		bn_int_set(&rpacket->u.d2cs_client_gameinforeply.etime,std::time(NULL)-d2cs_game_get_create_time(game));
		bn_byte_set(&rpacket->u.d2cs_client_gameinforeply.charlevel,game_get_charlevel(game));
		bn_byte_set(&rpacket->u.d2cs_client_gameinforeply.leveldiff,game_get_leveldiff(game));
		bn_byte_set(&rpacket->u.d2cs_client_gameinforeply.maxchar,game_get_maxchar(game));
		packet_append_string(rpacket, game_get_desc(game) ? game_get_desc(game) : NULL);

		n=0;
		BEGIN_LIST_TRAVERSE_DATA_CONST(game_get_charlist(game),info,t_game_charinfo)
		{
			if (!info->charname) {
				eventlog(eventlog_level_error,__FUNCTION__,"got NULL charname in game %s char list",gamename);
				continue;
			}
			packet_append_string(rpacket,info->charname);
			bn_byte_set(&rpacket->u.d2cs_client_gameinforeply.chclass[n],info->chclass);
			/* GUI is limited to a max level of 255 */
			if (info->level < 255) {
			bn_byte_set(&rpacket->u.d2cs_client_gameinforeply.level[n],info->level);
			} else {
				bn_byte_set(&rpacket->u.d2cs_client_gameinforeply.level[n], 255);
			}
			n++;
		}
		END_LIST_TRAVERSE_DATA_CONST()

		bn_byte_set(&rpacket->u.d2cs_client_gameinforeply.currchar,n);
		if (n!=game_get_currchar(game)) {
			eventlog(eventlog_level_error,__FUNCTION__,"game %s character list corrupted",gamename);
		}
		conn_push_outqueue(c,rpacket);
		packet_del_ref(rpacket);
	}
	return 0;
}

static int on_client_charloginreq(t_connection * c, t_packet * packet)
{
	t_packet		* bnpacket;
	char const		* charname;
	char const		* account;
	t_sq			* sq;
	t_d2charinfo_file	data;
	unsigned int		expire_time;

	if (!(charname=packet_get_str_const(packet,sizeof(t_client_d2cs_charloginreq),MAX_CHARNAME_LEN))) {
		eventlog(eventlog_level_error,__FUNCTION__,"got bad character name");
		return -1;
	}
	if (!(account=d2cs_conn_get_account(c))) {
		eventlog(eventlog_level_error,__FUNCTION__,"missing account for connection");
		return -1;
	}
	if (d2charinfo_load(account,charname,&data)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error loading charinfo for character %s(*%s)",charname,account);
		return -1;
	} else if (!bnetd_conn()) {
		eventlog(eventlog_level_error,__FUNCTION__,"no bnetd connection available,character login rejected");
		return -1;
	}
	expire_time = prefs_get_char_expire_time();
	if (expire_time && (std::time(NULL) > bn_int_get(data.header.last_time) + expire_time)) {
		t_packet * rpacket;

		if ((rpacket=packet_create(packet_class_d2cs))) {
			packet_set_size(rpacket,sizeof(t_d2cs_client_charloginreply));
			packet_set_type(rpacket,D2CS_CLIENT_CHARLOGINREPLY);
			bn_int_set(&rpacket->u.d2cs_client_charloginreply.reply, D2CS_CLIENT_CHARLOGINREPLY_EXPIRED);
			conn_push_outqueue(c,rpacket);
			packet_del_ref(rpacket);
		}
		eventlog(eventlog_level_info,__FUNCTION__,"character %s(*%s) login rejected due to char expired",charname,account);
		return 0;
	}

	conn_set_charinfo(c,&data.summary);
	eventlog(eventlog_level_info,__FUNCTION__,"got character %s(*%s) login request",charname,account);
	if ((bnpacket=packet_create(packet_class_d2cs_bnetd))) {
		if ((sq=sq_create(d2cs_conn_get_sessionnum(c),packet,0))) {
			packet_set_size(bnpacket,sizeof(t_d2cs_bnetd_charloginreq));
			packet_set_type(bnpacket,D2CS_BNETD_CHARLOGINREQ);
			bn_int_set(&bnpacket->u.d2cs_bnetd_charloginreq.h.seqno,sq_get_seqno(sq));
			bn_int_set(&bnpacket->u.d2cs_bnetd_charloginreq.sessionnum,
				conn_get_bnetd_sessionnum(c));
			packet_append_string(bnpacket,charname);
			packet_append_string(bnpacket,(char const *)&data.portrait);
			conn_push_outqueue(bnetd_conn(),bnpacket);
		}
		packet_del_ref(bnpacket);
	}
	return 0;
}

static int on_client_deletecharreq(t_connection * c, t_packet * packet)
{
	t_packet	* rpacket;
	char const	* charname;
	char const	* account;
	unsigned int	reply;

	if (!(charname=packet_get_str_const(packet,sizeof(t_client_d2cs_deletecharreq),MAX_CHARNAME_LEN))) {
		eventlog(eventlog_level_error,__FUNCTION__,"got bad character name");
		return -1;
	}
	if (conn_check_multilogin(c,charname)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"character %s is already logged in",charname);
		return -1;
	}
	d2cs_conn_set_charname(c,NULL);
	account=d2cs_conn_get_account(c);
	if (d2char_delete(account,charname)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"failed to delete character %s(*%s)",charname,account);
		reply = D2CS_CLIENT_DELETECHARREPLY_FAILED;
	} else {
		reply = D2CS_CLIENT_DELETECHARREPLY_SUCCEED;
	}
	if ((rpacket=packet_create(packet_class_d2cs))) {
		packet_set_size(rpacket,sizeof(t_d2cs_client_deletecharreply));
		packet_set_type(rpacket,D2CS_CLIENT_DELETECHARREPLY);
		bn_short_set(&rpacket->u.d2cs_client_deletecharreply.u1,0);
		bn_int_set(&rpacket->u.d2cs_client_deletecharreply.reply,reply);
		conn_push_outqueue(c,rpacket);
		packet_del_ref(rpacket);
	}
	return 0;
}

static int on_client_ladderreq(t_connection * c, t_packet * packet)
{
	unsigned char		type;
	unsigned short		start_pos;

	type=bn_byte_get(packet->u.client_d2cs_ladderreq.type);
	start_pos=bn_short_get(packet->u.client_d2cs_ladderreq.start_pos);
	d2cs_send_client_ladder(c,type,start_pos);
	return 0;
}

static int d2cs_send_client_ladder(t_connection * c, unsigned char type, unsigned short from)
{
	t_packet			* rpacket;
	t_d2cs_client_ladderinfo const	* ladderinfo;
	unsigned int			curr_len, cont_len, total_len;
	t_d2cs_client_ladderheader	ladderheader;
	t_d2cs_client_ladderinfoheader	infoheader;
	unsigned int			start_pos, count, count_per_packet, npacket;
	unsigned int			i, n, curr_pos;

	start_pos=from;
	count=prefs_get_ladderlist_count();
	if (d2ladder_get_ladder(&start_pos,&count,type,&ladderinfo)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"error get ladder for type %d start_pos %d",type,from);
		return 0;
	}

	count_per_packet=14;
	npacket=count/count_per_packet;
	if (count % count_per_packet) npacket++;

	curr_len=0;
	cont_len=0;
	total_len = count * sizeof(*ladderinfo) + sizeof(ladderheader) + sizeof(infoheader) * npacket;
	total_len -= 4;
	bn_short_set(&ladderheader.start_pos,start_pos);
	bn_short_set(&ladderheader.u1,0);
	bn_int_set(&ladderheader.count1,count);

	for (i=0; i< npacket; i++) {
		curr_len=0;
		if ((rpacket=packet_create(packet_class_d2cs))) {
			packet_set_size(rpacket,sizeof(t_d2cs_client_ladderreply));
			packet_set_type(rpacket,D2CS_CLIENT_LADDERREPLY);
			bn_byte_set(&rpacket->u.d2cs_client_ladderreply.type, type);
			bn_short_set(&rpacket->u.d2cs_client_ladderreply.total_len, total_len);
			bn_short_set(&rpacket->u.d2cs_client_ladderreply.cont_len,cont_len);
			if (i==0) {
				bn_int_set(&infoheader.count2,count);
				packet_append_data(rpacket,&ladderheader,sizeof(ladderheader));
				curr_len += sizeof(ladderheader);
			} else {
				bn_int_set(&infoheader.count2,0);
			}
			packet_append_data(rpacket,&infoheader,sizeof(infoheader));
			curr_len += sizeof(infoheader);
			for (n=0; n< count_per_packet; n++) {
				curr_pos = n + i * count_per_packet;
				if (curr_pos >= count) break;
				packet_append_data(rpacket, ladderinfo+curr_pos, sizeof(*ladderinfo));
				curr_len += sizeof(*ladderinfo);
			}
			if (i==0) {
				packet_set_size(rpacket, packet_get_size(rpacket)-4);
				curr_len -= 4;
			}
			bn_short_set(&rpacket->u.d2cs_client_ladderreply.curr_len,curr_len);
			conn_push_outqueue(c,rpacket);
			packet_del_ref(rpacket);
		}
		cont_len += curr_len;
	}
	return 0;
}

#define MAX_MOTD_LENGTH 511
static int on_client_motdreq(t_connection * c, t_packet * packet)
{
	t_packet	* rpacket;
	char		* motd;
	int		motd_len;

	if (!packet)
	    return -1;

	/* client will crash if motd is too long */
	motd = xstrdup(prefs_get_motd());
	motd_len = std::strlen(motd);
	if (motd_len > MAX_MOTD_LENGTH) {
		WARN2("motd length (%i) exceeds maximun value (%i)",motd_len,MAX_MOTD_LENGTH);
		motd[MAX_MOTD_LENGTH]='\0';
	}
	if ((rpacket=packet_create(packet_class_d2cs))) {
		packet_set_size(rpacket,sizeof(t_d2cs_client_motdreply));
		packet_set_type(rpacket,D2CS_CLIENT_MOTDREPLY);
		bn_byte_set(&rpacket->u.d2cs_client_motdreply.u1,0);
		packet_append_string(rpacket,motd);
		conn_push_outqueue(c,rpacket);
		packet_del_ref(rpacket);
	}
	xfree(motd);
	return 0;
}

static int on_client_cancelcreategame(t_connection * c, t_packet * packet)
{
	t_gq	* gq;
	t_elem	* elem;

	if (!packet)
	    return -1;

	if (!(gq=conn_get_gamequeue(c))) {
		return 0;
	}
	conn_set_gamequeue(c,NULL);
	gq_destroy(gq,&elem);
	return 0;
}

static int on_client_charladderreq(t_connection * c, t_packet * packet)
{
	t_packet	* rpacket;
	char const	* charname;
	unsigned int	expansion, hardcore, type;
	int		pos;

	if (!(charname=packet_get_str_const(packet,sizeof(t_client_d2cs_charladderreq),MAX_CHARNAME_LEN))) {
		eventlog(eventlog_level_error,__FUNCTION__,"got bad character name");
		return -1;
	}
	expansion=bn_int_get(packet->u.client_d2cs_charladderreq.expansion);
	hardcore=bn_int_get(packet->u.client_d2cs_charladderreq.hardcore);
	type=0;					/* avoid warning */
	if (hardcore && expansion) {
		type=D2LADDER_EXP_HC_OVERALL;
	} else if (!hardcore && expansion) {
		type=D2LADDER_EXP_STD_OVERALL;
	} else if (hardcore && !expansion) {
		type=D2LADDER_HC_OVERALL;
	} else if (!hardcore && !expansion) {
		type=D2LADDER_STD_OVERALL;
	}
	if ((pos=d2ladder_find_character_pos(type,charname))<0) {
		if ((rpacket=packet_create(packet_class_d2cs))) {
			packet_set_size(rpacket,sizeof(t_d2cs_client_ladderreply));
			packet_set_type(rpacket,D2CS_CLIENT_LADDERREPLY);
			bn_byte_set(&rpacket->u.d2cs_client_ladderreply.type, type);
			bn_short_set(&rpacket->u.d2cs_client_ladderreply.total_len,0);
			bn_short_set(&rpacket->u.d2cs_client_ladderreply.curr_len,0);
			bn_short_set(&rpacket->u.d2cs_client_ladderreply.cont_len,0);
			conn_push_outqueue(c,rpacket);
			packet_del_ref(rpacket);
		}
		return 0;
	}
	pos -= prefs_get_ladderlist_count()/2;
	if (pos < 0) pos=0;
	d2cs_send_client_ladder(c,type,pos);
	return 0;
}

static int on_client_charlistreq(t_connection * c, t_packet * packet)
{
	t_packet		* rpacket;
	char const		* account;
	char const		* charname;
	char			* path;
	t_d2charinfo_file       * charinfo;
	unsigned int		n, maxchar;
	t_elist			charlist_head;
	char const		* charlist_sort_order;

	if (!packet)
	    return -1;

	if (!(account=d2cs_conn_get_account(c))) {
		eventlog(eventlog_level_error,__FUNCTION__,"missing account for connection");
		return -1;
	}
	path=(char*)xmalloc(std::strlen(prefs_get_charinfo_dir())+1+std::strlen(account)+1);
	charlist_sort_order = prefs_get_charlist_sort_order();

	elist_init(&charlist_head);

	d2char_get_infodir_name(path,account);
	maxchar=prefs_get_maxchar();
	if ((rpacket=packet_create(packet_class_d2cs))) {
		packet_set_size(rpacket,sizeof(t_d2cs_client_charlistreply));
		packet_set_type(rpacket,D2CS_CLIENT_CHARLISTREPLY);
		bn_short_set(&rpacket->u.d2cs_client_charlistreply.u1,0);
		n=0;
		try {
			Directory dir(path);
			while ((charname=dir.read())) {
				charinfo = (t_d2charinfo_file*)xmalloc(sizeof(t_d2charinfo_file));
				if (d2charinfo_load(account,charname,charinfo)<0) {
					eventlog(eventlog_level_error,__FUNCTION__,"error loading charinfo for %s(*%s)",charname,account);
					xfree((void *)charinfo);
					continue;
				}
				eventlog(eventlog_level_debug,__FUNCTION__,"adding char %s (*%s)", charname, account);
				d2charlist_add_char(&charlist_head,charinfo,0);
				n++;
				if (n>=maxchar) break;
			}
			if (prefs_allow_newchar() && (n<maxchar)) {
				bn_short_set(&rpacket->u.d2cs_client_charlistreply.maxchar,maxchar);
			} else {
				bn_short_set(&rpacket->u.d2cs_client_charlistreply.maxchar,0);
			}
			if (!std::strcmp(charlist_sort_order, "ASC"))
			{
			    t_elist * curr, * safe;
			    t_d2charlist * ccharlist;

			    elist_for_each_safe(curr,&charlist_head,safe)
			    {
				ccharlist = elist_entry(curr,t_d2charlist,list);
				packet_append_string(rpacket,(char*)ccharlist->charinfo->header.charname);
				packet_append_string(rpacket,(char *)&ccharlist->charinfo->portrait);
				xfree((void *)ccharlist->charinfo);
				xfree((void *)ccharlist);
			    }
			}
			else
			{
			    t_elist * curr, * safe;
			    t_d2charlist * ccharlist;

			    elist_for_each_safe_rev(curr,&charlist_head,safe)
			    {
				ccharlist = elist_entry(curr,t_d2charlist,list);
				packet_append_string(rpacket,(char*)ccharlist->charinfo->header.charname);
				packet_append_string(rpacket,(char *)&ccharlist->charinfo->portrait);
				xfree((void *)ccharlist->charinfo);
				xfree((void *)ccharlist);

			    }
			}
		} catch(const Directory::OpenError&) {
			INFO1("(*%s) charinfo directory do not exist, building it",account);
			p_mkdir(path,S_IRWXU);
		}
		bn_short_set(&rpacket->u.d2cs_client_charlistreply.currchar,n);
		bn_short_set(&rpacket->u.d2cs_client_charlistreply.currchar2,n);
		conn_push_outqueue(c,rpacket);
		packet_del_ref(rpacket);
	}
	xfree(path);
	return 0;
}

static int on_client_charlistreq_110(t_connection * c, t_packet * packet)
{
	t_packet		* rpacket;
	char const		* account;
	char const		* charname;
	char			* path;

	t_d2charinfo_file       * charinfo;
	unsigned int		n, maxchar;
	t_elist			charlist_head;

	unsigned int		exp_time;
	unsigned int		curr_exp_time;
	char const		* charlist_sort_order;

	if (!packet)
	    return -1;

	if (!(account=d2cs_conn_get_account(c))) {
		eventlog(eventlog_level_error,__FUNCTION__,"missing account for connection");
		return -1;
	}
	path=(char*)xmalloc(std::strlen(prefs_get_charinfo_dir())+1+std::strlen(account)+1);
	charlist_sort_order = prefs_get_charlist_sort_order();

	elist_init(&charlist_head);

	d2char_get_infodir_name(path,account);
	if (prefs_allow_newchar())
		maxchar=prefs_get_maxchar();
	else
		maxchar=0;

	if ((rpacket=packet_create(packet_class_d2cs))) {
		packet_set_size(rpacket,sizeof(t_d2cs_client_charlistreply_110));
		packet_set_type(rpacket,D2CS_CLIENT_CHARLISTREPLY_110);
		bn_short_set(&rpacket->u.d2cs_client_charlistreply_110.u1,0);
		n=0;
		try {
			Directory dir(path);

			exp_time = prefs_get_char_expire_time();
			while ((charname=dir.read())) {
				charinfo = (t_d2charinfo_file*)xmalloc(sizeof(t_d2charinfo_file));
				if (d2charinfo_load(account,charname,charinfo)<0) {
					eventlog(eventlog_level_error,__FUNCTION__,"error loading charinfo for %s(*%s)",charname,account);
					xfree(charinfo);
					continue;
				}
				if (exp_time) {
					curr_exp_time = bn_int_get(charinfo->header.last_time)+exp_time;
				} else {
					curr_exp_time = 0x7FFFFFFF;
				}
				eventlog(eventlog_level_debug,__FUNCTION__,"adding char %s (*%s)", charname, account);
				d2charlist_add_char(&charlist_head,charinfo,curr_exp_time);
				n++;
				if (n>=maxchar) break;
			}
			if (n>=maxchar)
				maxchar = 0;

			if (!std::strcmp(charlist_sort_order, "ASC"))
			{
			    t_elist * curr, *safe;
			    t_d2charlist * ccharlist;

			    elist_for_each_safe(curr,&charlist_head,safe)
			    {
			    	bn_int bn_exp_time;

				ccharlist = elist_entry(curr,t_d2charlist,list);
				bn_int_set(&bn_exp_time,ccharlist->expiration_time);
				packet_append_data(rpacket,bn_exp_time,sizeof(bn_exp_time));
				packet_append_string(rpacket,(char*)ccharlist->charinfo->header.charname);
				packet_append_string(rpacket,(char *)&ccharlist->charinfo->portrait);
				xfree((void *)ccharlist->charinfo);
				xfree((void *)ccharlist);
			    }
			}
			else
			{
			    t_elist * curr, *safe;
			    t_d2charlist * ccharlist;

			    elist_for_each_safe_rev(curr,&charlist_head,safe)
			    {
			    	bn_int bn_exp_time;

				ccharlist = elist_entry(curr,t_d2charlist,list);
				bn_int_set(&bn_exp_time,ccharlist->expiration_time);
				packet_append_data(rpacket,bn_exp_time,sizeof(bn_exp_time));
				packet_append_string(rpacket,(char*)ccharlist->charinfo->header.charname);
				packet_append_string(rpacket,(char *)&ccharlist->charinfo->portrait);
				xfree((void *)ccharlist->charinfo);
				xfree((void *)ccharlist);
			    }
			}
		} catch (const Directory::OpenError&) {
			INFO1("(*%s) charinfo directory do not exist, building it",account);
			p_mkdir(path,S_IRWXU);
		}
		bn_short_set(&rpacket->u.d2cs_client_charlistreply.currchar,n);
		bn_short_set(&rpacket->u.d2cs_client_charlistreply.currchar2,n);
		bn_short_set(&rpacket->u.d2cs_client_charlistreply.maxchar,maxchar);

		conn_push_outqueue(c,rpacket);
		packet_del_ref(rpacket);
	}
	xfree(path);
	return 0;
}

static int on_client_convertcharreq(t_connection * c, t_packet * packet)
{
	t_packet	* rpacket;
	char const	* charname;
	char const	* account;
	unsigned int	reply;

	if (!(charname=packet_get_str_const(packet,sizeof(t_client_d2cs_convertcharreq),MAX_CHARNAME_LEN))) {
		eventlog(eventlog_level_error,__FUNCTION__,"got bad character name");
		return -1;
	}
	if (conn_check_multilogin(c,charname)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"character %s is already logged in",charname);
		return -1;
	}
	account=d2cs_conn_get_account(c);
	if (d2char_convert(account,charname)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"failed to convert character %s(*%s)",charname,account);
		reply = D2CS_CLIENT_CONVERTCHARREPLY_FAILED;
	} else {
		reply = D2CS_CLIENT_CONVERTCHARREPLY_SUCCEED;
	}
	if ((rpacket=packet_create(packet_class_d2cs))) {
		packet_set_size(rpacket,sizeof(t_d2cs_client_convertcharreply));
		packet_set_type(rpacket,D2CS_CLIENT_CONVERTCHARREPLY);
		bn_int_set(&rpacket->u.d2cs_client_convertcharreply.reply,reply);
		conn_push_outqueue(c,rpacket);
		packet_del_ref(rpacket);
	}
	return 0;
}

extern int d2cs_send_client_creategamewait(t_connection * c, unsigned int position)
{
	t_packet	* packet;

	ASSERT(c,-1);
	if ((packet=packet_create(packet_class_d2cs))) {
		packet_set_size(packet,sizeof(t_d2cs_client_creategamewait));
		packet_set_type(packet,D2CS_CLIENT_CREATEGAMEWAIT);
		bn_int_set(&packet->u.d2cs_client_creategamewait.position,position);
		conn_push_outqueue(c,packet);
		packet_del_ref(packet);
	}
	return 0;
}

extern int d2cs_handle_client_creategame(t_connection * c, t_packet * packet)
{
	return on_client_creategamereq(c,packet);
}

static unsigned int d2cs_try_joingame(t_connection const * c, t_game const * game, char const * gamepass)
{
	unsigned int reply;

	ASSERT(c,D2CS_CLIENT_JOINGAMEREPLY_NOT_EXIST);
	ASSERT(game,D2CS_CLIENT_JOINGAMEREPLY_NOT_EXIST);
	if (!game_get_created(game)) {
		reply=D2CS_CLIENT_JOINGAMEREPLY_NOT_EXIST;
	} else if (!game_get_d2gs(game)) {
		reply=D2CS_CLIENT_JOINGAMEREPLY_NOT_EXIST;
	} else if (conn_get_charinfo_ladder(c) != game_get_gameflag_ladder(game)) {
		reply=D2CS_CLIENT_JOINGAMEREPLY_NORMAL_LADDER;
	} else if (conn_get_charinfo_expansion(c) != game_get_gameflag_expansion(game)) {
		reply=D2CS_CLIENT_JOINGAMEREPLY_CLASSIC_EXPANSION;
	} else if (conn_get_charinfo_hardcore(c) != game_get_gameflag_hardcore(game)) {
		reply=D2CS_CLIENT_JOINGAMEREPLY_HARDCORE_SOFTCORE;
	} else if (conn_get_charinfo_difficulty(c) < game_get_gameflag_difficulty(game))  {
		reply=D2CS_CLIENT_JOINGAMEREPLY_NORMAL_NIGHTMARE;
	} else if (prefs_allow_gamelimit()) {
		if (game_get_maxchar(game) <= game_get_currchar(game)) {
			reply=D2CS_CLIENT_JOINGAMEREPLY_GAME_FULL;
		} else if (conn_get_charinfo_level(c) > game_get_maxlevel(game)) {
			reply=D2CS_CLIENT_JOINGAMEREPLY_LEVEL_LIMIT;
		} else if (conn_get_charinfo_level(c) < game_get_minlevel(game)) {
			reply=D2CS_CLIENT_JOINGAMEREPLY_LEVEL_LIMIT;
		} else if (std::strcmp(d2cs_game_get_pass(game),gamepass)) {
			reply=D2CS_CLIENT_JOINGAMEREPLY_BAD_PASS;
		} else {
			reply=D2CS_CLIENT_JOINGAMEREPLY_SUCCEED;
		}
	} else {
		if (game_get_currchar(game) >= MAX_CHAR_PER_GAME) {
			reply=D2CS_CLIENT_JOINGAMEREPLY_GAME_FULL;
		} else {
			reply=D2CS_CLIENT_JOINGAMEREPLY_SUCCEED;
		}
	}
	return reply;
}

}
}
