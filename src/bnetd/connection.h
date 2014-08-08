/*
 * Copyright (C) 1998,1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2000,2001  Marco Ziech (mmz@gmx.net)
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
#ifndef INCLUDED_CONNECTION_TYPES
#define INCLUDED_CONNECTION_TYPES

#ifdef CONNECTION_INTERNAL_ACCESS

#include <ctime>

#ifdef JUST_NEED_TYPES
# include "game.h"
# include "channel.h"
# include "account.h"
# include "quota.h"
# include "character.h"
# include "versioncheck.h"
# include "anongame.h"
# include "anongame_wol.h"
# include "realm.h"
# include "common/queue.h"
# include "common/tag.h"
# include "common/elist.h"
# include "common/packet.h"
# include "common/rcm.h"
#else
# define JUST_NEED_TYPES
# include "game.h"
# include "channel.h"
# include "account.h"
# include "quota.h"
# include "character.h"
# include "versioncheck.h"
# include "anongame.h"
# include "anongame_wol.h"
# include "realm.h"
# include "common/queue.h"
# include "common/tag.h"
# include "common/elist.h"
# include "common/packet.h"
# include "common/rcm.h"
# undef JUST_NEED_TYPES
#endif

#endif

namespace pvpgn
{

	namespace bnetd
	{

		typedef enum
		{
			conn_class_init,
			conn_class_bnet,
			conn_class_file,
			conn_class_bot,
			conn_class_telnet,
			conn_class_ircinit,    /* IRC based protocol INIT*/
			conn_class_irc,        /* Internet Relay Chat */
			conn_class_wol,        /* Westwood Chat and Game protocol (IRC based) */
			conn_class_wserv,      /* Westwood servserv (IRC based) */
			conn_class_wgameres,   /* Westwood Gameresolution */
			conn_class_wladder,    /* Westwood Ladder server */
			conn_class_apireg,     /* Westwood API Register */
			conn_class_d2cs_bnetd,
			conn_class_w3route,
			conn_class_none
		} t_conn_class;

		typedef enum
		{
			conn_state_empty,
			conn_state_initial,
			conn_state_connected,
			conn_state_loggedin,
			conn_state_destroy,
			conn_state_bot_username,
			conn_state_bot_password,
			conn_state_untrusted,
			conn_state_pending_raw
		} t_conn_state;

#ifdef CONNECTION_INTERNAL_ACCESS
		typedef enum
		{
			conn_flags_welcomed = 0x01,
			conn_flags_udpok = 0x02,
			conn_flags_joingamewhisper = 0x04,
			conn_flags_leavegamewhisper = 0x08,
			conn_flags_echoback = 0x10

		} t_conn_flags;

#endif

		typedef struct connection
#ifdef CONNECTION_INTERNAL_ACCESS
		{
			struct {
				int			tcp_sock;
				unsigned int		tcp_addr;
				unsigned short		tcp_port;
				int			udp_sock;
				unsigned int		udp_addr;
				unsigned short		udp_port;
				unsigned int		local_addr;
				unsigned short		local_port;
				unsigned int		real_local_addr;
				unsigned short		real_local_port;
				int			fdw_idx;
			} socket; /* IP and socket specific data */
			struct {
				t_conn_class		cclass;
				t_conn_state		state;
				unsigned int		sessionkey;
				unsigned int		sessionnum;
				unsigned int		secret; /* random number... never sent over net unencrypted */
				unsigned int		flags;
				unsigned int		latency;
				t_account *		account;
				struct {
					t_tag			archtag;
					t_tag			gamelang;
					t_clienttag			clienttag;
					char const *		clientver;
					unsigned long		versionid; /* AKA bnversion */
					unsigned long		gameversion;
					unsigned long		checksum;
					char const *		country;
					int				tzbias;
					char const *		host;
					char const *		user;
					char const *		clientexe;
					char const *		owner;
					char const *		cdkey;
					t_versioncheck *		versioncheck; /* equation and MPQ file used to validate game checksum */
				} client; /* client program specific data */
				struct {
					t_queue *		outqueue;  /* packets waiting to be sent */
					unsigned int	outsize;   /* amount sent from the current output packet */
					unsigned int	outsizep;
					t_packet *		inqueue;   /* packet waiting to be processed */
					unsigned int	insize;    /* amount received into the current input packet */
				} queues; /* network queues and related data */
				struct {
					t_channel *		channel;
					char const *	tmpOP_channel;
					char const *	tmpVOICE_channel;
					char const *	away;
					char const * 	dnd;
					t_account * *	ignore_list;
					unsigned int	ignore_count;
					t_quota		quota;
					std::time_t		last_message;
					char const *	lastsender; /* last person to whisper to this connection */
					struct {
						char const *		ircline; /* line cache for IRC connections */
						unsigned int		ircping; /* value of last ping */
						char const *		ircpass; /* hashed password for PASS authentication */
					} irc; /* irc chat specific data */
				} chat; /* chat and messages specific data */
				t_game *		game;
				const char *		loggeduser;   /* username as logged in or given (not taken from account) */
				struct connection *	bound; /* matching Diablo II auth connection */
				t_elist			timers; /* cached list of timers for cleaning */
				/* FIXME: this d2/w3 specific data could be unified into an union */
				struct {
					t_realm *			realm;
					t_rcm_regref		realm_regref;
					t_character *		character;
					char const *		realminfo;
					char const *		charname;
				} d2;
				struct {
					char const *		w3_playerinfo; /* ADDED BY UNDYING SOULZZ 4/7/02 */
					std::time_t			anongame_search_starttime;
					/* [zap-zero] 20020527 - matching w3route connection for game connection /
					 matching game connection for w3route connection */
					/* FIXME: this "optimization" is so confusing leading to many possible bugs */
					struct connection *	routeconn;
					t_anongame *	anongame;
					/* those will be filled when recieving 0x53ff and wiped out after 54ff */
					char const * client_proof;
					char const * server_proof;
				} w3;
				struct {
					int ingame;				        /* Are we in a game channel? */
					int codepage;
					int findme;                     /* Allow others to find me? */
					int pageme;                     /* Allow others to page me? */
					char const * apgar;			    /* WOL User Password (encrypted) */
					t_anongame_wol_player * anongame_player;
				} wol;
				int			cr_time;
				/* Pass fail count for bruteforce protection */
				unsigned int		passfail_count;
				/* connection flag substituting some other values */
				unsigned int		cflags;
			} protocol;
		}
#endif
		t_connection;

	}

}

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_CONNECTION_PROTOS
#define INCLUDED_CONNECTION_PROTOS

#include <ctime>

#define JUST_NEED_TYPES
#include "common/packet.h"
#include "common/queue.h"
#include "channel.h"
#include "game.h"
#include "account.h"
#include "common/list.h"
#include "character.h"
#include "versioncheck.h"
#include "timer.h"
#include "anongame.h"
#include "anongame_wol.h"
#include "realm.h"
#include "message.h"
#include "common/tag.h"
#include "common/fdwatch.h"
#undef JUST_NEED_TYPES

#define DESTROY_FROM_CONNLIST 0
#define DESTROY_FROM_DEADLIST 1

namespace pvpgn
{

	namespace bnetd
	{

		extern t_anongame * conn_create_anongame(t_connection * c);
		extern void conn_destroy_anongame(t_connection * c);

		extern t_anongame * conn_get_anongame(t_connection *c);


		extern void conn_shutdown(t_connection * c, std::time_t now, t_timer_data foo);
		extern void conn_test_latency(t_connection * c, std::time_t now, t_timer_data delta);
		extern char const * conn_class_get_str(t_conn_class cclass);
		extern char const * conn_state_get_str(t_conn_state state);

		extern t_connection * conn_create(int tsock, int usock, unsigned int real_local_addr, unsigned short real_local_port, unsigned int local_addr, unsigned short local_port, unsigned int addr, unsigned short port);
		extern void conn_destroy(t_connection * c, t_elem ** elem, int conn_or_dead_list);
		extern int conn_match(t_connection const * c, char const * user);
		extern t_conn_class conn_get_class(t_connection const * c);
		extern void conn_set_class(t_connection * c, t_conn_class cclass);
		extern t_conn_state conn_get_state(t_connection const * c);
		extern void conn_set_state(t_connection * c, t_conn_state state);
		extern unsigned int conn_get_sessionkey(t_connection const * c);
		extern unsigned int conn_get_sessionnum(t_connection const * c);
		extern unsigned int conn_get_secret(t_connection const * c);
		extern unsigned int conn_get_addr(t_connection const * c);
		extern unsigned short conn_get_port(t_connection const * c);
		extern unsigned int conn_get_local_addr(t_connection const * c);
		extern unsigned short conn_get_local_port(t_connection const * c);
		extern unsigned int conn_get_real_local_addr(t_connection const * c);
		extern unsigned short conn_get_real_local_port(t_connection const * c);
		extern unsigned int conn_get_game_addr(t_connection const * c);
		extern int conn_set_game_addr(t_connection * c, unsigned int game_addr);
		extern unsigned short conn_get_game_port(t_connection const * c);
		extern int conn_set_game_port(t_connection * c, unsigned short game_port);
		extern void conn_set_host(t_connection * c, char const * host);
		extern void conn_set_user(t_connection * c, char const * user);
		extern const char * conn_get_user(t_connection const * c);
		extern void conn_set_owner(t_connection * c, char const * owner);
		extern const char * conn_get_owner(t_connection const * c);
		extern void conn_set_cdkey(t_connection * c, char const * cdkey);
		extern char const * conn_get_clientexe(t_connection const * c);
		extern void conn_set_clientexe(t_connection * c, char const * clientexe);
		extern t_tag		conn_get_archtag(t_connection const * c);
		extern void		conn_set_archtag(t_connection * c, t_tag archtag);
		extern t_tag		conn_get_gamelang(t_connection const * c);
		extern void		conn_set_gamelang(t_connection * c, t_tag gamelang);
		extern t_clienttag	conn_get_clienttag(t_connection const * c);
		extern t_clienttag	conn_get_fake_clienttag(t_connection const * c);
		extern void conn_set_clienttag(t_connection * c, t_clienttag clienttag);
		extern unsigned long conn_get_versionid(t_connection const * c);
		extern int conn_set_versionid(t_connection * c, unsigned long versionid);
		extern unsigned long conn_get_gameversion(t_connection const * c);
		extern int conn_set_gameversion(t_connection * c, unsigned long gameversion);
		extern unsigned long conn_get_checksum(t_connection const * c);
		extern int conn_set_checksum(t_connection * c, unsigned long checksum);
		extern char const * conn_get_clientver(t_connection const * c);
		extern void conn_set_clientver(t_connection * c, char const * clientver);
		extern int conn_get_tzbias(t_connection const * c);
		extern void conn_set_tzbias(t_connection * c, int tzbias);
		extern int conn_set_loggeduser(t_connection * c, char const * username);
		extern char const * conn_get_loggeduser(t_connection const * c);
		extern unsigned int conn_get_flags(t_connection const * c);
		extern int conn_set_flags(t_connection * c, unsigned int flags);
		extern void conn_add_flags(t_connection * c, unsigned int flags);
		extern void conn_del_flags(t_connection * c, unsigned int flags);
		extern unsigned int conn_get_latency(t_connection const * c);
		extern void conn_set_latency(t_connection * c, unsigned int ms);
		extern char const * conn_get_awaystr(t_connection const * c);
		extern int conn_set_awaystr(t_connection * c, char const * away);
		extern char const * conn_get_dndstr(t_connection const * c);
		extern int conn_set_dndstr(t_connection * c, char const * dnd);
		extern int conn_add_ignore(t_connection * c, t_account * account);
		extern int conn_del_ignore(t_connection * c, t_account const * account);
		extern int conn_add_watch(t_connection * c, t_account * account, t_clienttag clienttag);
		extern int conn_del_watch(t_connection * c, t_account * account, t_clienttag clienttag);
		extern t_channel * conn_get_channel(t_connection const * c);
		extern int conn_set_channel_var(t_connection * c, t_channel * channel);
		extern int conn_set_channel(t_connection * c, char const * channelname);
		extern int conn_part_channel(t_connection * c);
		extern int conn_kick_channel(t_connection * c, char const * text);
		extern int conn_quit_channel(t_connection * c, char const * text);
		extern t_game * conn_get_game(t_connection const * c);
		extern int conn_set_game(t_connection * c, char const * gamename, char const * gamepass, char const * gameinfo, t_game_type type, int version);
		extern unsigned int conn_get_tcpaddr(t_connection * c);
		extern t_packet * conn_get_in_queue(t_connection * c);
		extern void conn_put_in_queue(t_connection * c, t_packet *packet);
		extern unsigned int conn_get_in_size(t_connection const * c);
		extern void conn_set_in_size(t_connection * c, unsigned int size);
		extern unsigned int conn_get_out_size(t_connection const * c);
		extern void conn_set_out_size(t_connection * c, unsigned int size);
		extern int conn_push_outqueue(t_connection * c, t_packet * packet);
		extern t_packet * conn_peek_outqueue(t_connection * c);
		extern t_packet * conn_pull_outqueue(t_connection * c);
		extern int conn_clear_outqueue(t_connection * c);
		extern void conn_close_read(t_connection * c);
		extern int conn_check_ignoring(t_connection const * c, char const * me);
		extern t_account * conn_get_account(t_connection const * c);
		extern void conn_login(t_connection * c, t_account * account, const char *loggeduser);
		extern int conn_get_socket(t_connection const * c);
		extern int conn_get_game_socket(t_connection const * c);
		extern int conn_set_game_socket(t_connection * c, int usock);
		extern char const * conn_get_username_real(t_connection const * c, char const * fn, unsigned int ln);
#define conn_get_username(C) conn_get_username_real(C,__FILE__,__LINE__)
		extern char const * conn_get_chatname(t_connection const * c);
		extern int conn_unget_chatname(t_connection const * c, char const * name);
		extern char const * conn_get_chatcharname(t_connection const * c, t_connection const * dst);
		extern int conn_unget_chatcharname(t_connection const * c, char const * name);
		extern t_message_class conn_get_message_class(t_connection const * c, t_connection const * dst);
		extern unsigned int conn_get_userid(t_connection const * c);
		extern char const * conn_get_playerinfo(t_connection const * c);
		extern int conn_set_playerinfo(t_connection const * c, char const * playerinfo);
		extern char const * conn_get_realminfo(t_connection const * c);
		extern int conn_set_realminfo(t_connection * c, char const * realminfo);
		extern char const * conn_get_charname(t_connection const * c);
		extern int conn_set_charname(t_connection * c, char const * charname);
		extern int conn_set_idletime(t_connection * c);
		extern unsigned int conn_get_idletime(t_connection const * c);
		extern t_realm * conn_get_realm(t_connection const * c);
		extern int conn_set_realm(t_connection * c, t_realm * realm);
		extern int conn_set_character(t_connection * c, t_character * ch);
		extern int conn_bind(t_connection * c1, t_connection * c2);
		extern void conn_set_country(t_connection * c, char const * country);
		extern char const * conn_get_country(t_connection const * c);
		extern int conn_quota_exceeded(t_connection * c, char const * message);
		extern int conn_set_lastsender(t_connection * c, char const * sender);
		extern char const * conn_get_lastsender(t_connection const * c);
		extern t_versioncheck * conn_get_versioncheck(t_connection * c);
		extern int conn_set_versioncheck(t_connection * c, t_versioncheck * versioncheck);
		extern int conn_get_echoback(t_connection * c);
		extern void conn_set_echoback(t_connection * c, int echoback);
		extern int conn_set_ircline(t_connection * c, char const * line);
		extern char const * conn_get_ircline(t_connection const * c);
		extern int conn_set_ircpass(t_connection * c, char const * pass);
		extern char const * conn_get_ircpass(t_connection const * c);
		extern int conn_set_ircping(t_connection * c, unsigned int ping);
		extern unsigned int conn_get_ircping(t_connection const * c);
		extern int conn_set_udpok(t_connection * c);
		extern int conn_get_welcomed(t_connection const * c);
		extern void conn_set_welcomed(t_connection * c, int welcomed);

		extern int conn_set_w3_playerinfo(t_connection * c, char const * w3_playerinfo);
		extern const char * conn_get_w3_playerinfo(t_connection * c);

		extern int conn_get_crtime(t_connection *c);

		extern int conn_set_w3_loginreq(t_connection * c, char const * loginreq);
		extern char const * conn_get_w3_loginreq(t_connection * c);

		extern int conn_set_routeconn(t_connection * c, t_connection * rc);
		extern t_connection * conn_get_routeconn(t_connection const * c);
		extern int connlist_create(void);
		extern void connlist_reap(void);
		extern int connlist_destroy(void);
		extern t_list * connlist(void);
		extern t_connection * connlist_find_connection_by_sessionkey(unsigned int sessionkey);
		extern t_connection * connlist_find_connection_by_socket(int socket);
		extern t_connection * connlist_find_connection_by_sessionnum(unsigned int sessionnum);
		extern t_connection * connlist_find_connection_by_name(char const * name, t_realm * realm); /* any chat name format */
		extern t_connection * connlist_find_connection_by_accountname(char const * username);
		extern t_connection * connlist_find_connection_by_charname(char const * charname, char const * realmname);
		extern t_connection * connlist_find_connection_by_account(t_account * account);
		extern t_connection * connlist_find_connection_by_uid(unsigned int uid);
		extern int connlist_get_length(void);
		extern unsigned int connlist_login_get_length(void);
		extern int connlist_total_logins(void);
		extern int conn_set_joingamewhisper_ack(t_connection * c, unsigned int value);
		extern int conn_get_joingamewhisper_ack(t_connection * c);
		extern int conn_set_leavegamewhisper_ack(t_connection * c, unsigned int value);
		extern int conn_get_leavegamewhisper_ack(t_connection * c);
		extern int conn_set_anongame_search_starttime(t_connection * c, std::time_t t);
		extern std::time_t conn_get_anongame_search_starttime(t_connection * c);

		extern int conn_get_user_count_by_clienttag(t_clienttag ct);

		extern unsigned int connlist_count_connections(unsigned int addr);

		extern int conn_update_w3_playerinfo(t_connection * c);

		extern int conn_get_passfail_count(t_connection * c);
		extern int conn_set_passfail_count(t_connection * c, unsigned int failcount);
		extern int conn_increment_passfail_count(t_connection * c);

		extern char const * conn_get_client_proof(t_connection * c);
		extern int conn_set_client_proof(t_connection * c, char const * client_proof);

		extern char const * conn_get_server_proof(t_connection * c);
		extern int conn_set_server_proof(t_connection * c, char const * server_proof);

		extern int conn_set_tmpOP_channel(t_connection * c, char const * tmpOP_channel);
		extern char const * conn_get_tmpOP_channel(t_connection * c);
		extern int conn_set_tmpVOICE_channel(t_connection * c, char const * tmpVOICE_channel);
		extern char const * conn_get_tmpVOICE_channel(t_connection * c);
		extern t_elist *conn_get_timer(t_connection * c);
		extern int conn_add_fdwatch(t_connection *c, fdwatch_handler handle);
		extern int conn_is_irc_variant(t_connection * c);

		/* Westwood Online Extensions */
		extern int conn_get_wol(t_connection * c);
		extern void conn_wol_set_apgar(t_connection * c, char const * apgar);
		extern char const * conn_wol_get_apgar(t_connection * c);
		extern void conn_wol_set_codepage(t_connection * c, int codepage);
		extern int conn_wol_get_codepage(t_connection * c);
		extern void conn_wol_set_findme(t_connection * c, bool findme);
		extern bool conn_wol_get_findme(t_connection * c);
		extern void conn_wol_set_pageme(t_connection * c, bool pageme);
		extern bool conn_wol_get_pageme(t_connection * c);
		extern void conn_wol_set_anongame_player(t_connection * c, t_anongame_wol_player * anongame_player);
		extern t_anongame_wol_player * conn_wol_get_anongame_player(t_connection * c);

		extern int conn_client_readmemory(t_connection * c, unsigned int request_id, unsigned int offset, unsigned int length);
	}

}

#endif
#endif
