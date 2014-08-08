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
#ifndef INCLUDED_CONNECTION_H
#define INCLUDED_CONNECTION_H

#include "common/queue.h"
#include "common/hashtable.h"
#include "common/packet.h"
#include "common/fdwatch.h"
#include "d2charfile.h"
#include "gamequeue.h"

namespace pvpgn
{

namespace d2cs
{

typedef enum
{
	conn_state_none		=	0x00,
	conn_state_init		=	0x01,
	conn_state_connecting	=	0x02,
	conn_state_connected	=	0x04,
	conn_state_authed	=	0x08,
	conn_state_char_authed	=	0x10,
	conn_state_destroy	=	0x20,
	conn_state_destroying	=	0x40,
	conn_state_any		=	0xff
} t_conn_state;

typedef enum
{
	conn_class_none,
	conn_class_init,
	conn_class_d2cs,
	conn_class_d2gs,
	conn_class_bnetd
} t_conn_class;

typedef struct
{
	char const			* charname;
	char const			* account;
	int				sock;
	int				fdw_idx;
	unsigned int			socket_flag;
	unsigned int			addr;
	unsigned int			local_addr;
	unsigned short			port;
	unsigned short			local_port;
	unsigned int			last_active;
	t_conn_class			cclass;
	t_conn_state			state;
	unsigned int			sessionnum;
	t_queue				* outqueue;
	unsigned int			outsize;
	unsigned int			outsizep;
	t_packet			* inqueue;
	unsigned int			insize;
	t_d2charinfo_summary const	* charinfo;
	unsigned int			d2gs_id;
	t_gq				* gamequeue;
	unsigned int			bnetd_sessionnum;
	unsigned int			sessionnum_hash;
	unsigned int			charname_hash;
} t_connection;

typedef int ( * packet_handle_func) (t_connection * c, t_packet * packet);
typedef struct
{
	unsigned int		size;
	unsigned		state;
	packet_handle_func	handler;
} t_packet_handle_table;

#define SOCKET_FLAG_READ		0x1
#define SOCKET_FLAG_WRITE		0x2

extern t_hashtable * d2cs_connlist(void);
extern int d2cs_connlist_destroy(void);
extern int d2cs_connlist_reap(void);
extern int d2cs_connlist_create(void);
extern int conn_check_multilogin(t_connection const * c,char const * charname);
extern t_connection * d2cs_connlist_find_connection_by_sessionnum(unsigned int sessionnum);
extern t_connection * d2cs_connlist_find_connection_by_charname(char const * charname);
extern int conn_handle_socket(t_connection * c);
extern t_connection * d2cs_conn_create(int sock, unsigned int local_addr, unsigned short local_port,
				unsigned int addr, unsigned short port);
extern int d2cs_conn_destroy(t_connection * c, t_elem ** elem);
extern int d2cs_conn_get_socket(t_connection const * c);
extern unsigned int d2cs_conn_get_sessionnum(t_connection const * c);
extern t_conn_class d2cs_conn_get_class(t_connection const * c);
extern int d2cs_conn_set_class(t_connection * c, t_conn_class cclass);
extern t_conn_state d2cs_conn_get_state(t_connection const * c);
extern int d2cs_conn_set_state(t_connection * c, t_conn_state state);
extern t_queue * * d2cs_conn_get_out_queue(t_connection const * c);
extern t_packet * d2cs_conn_get_in_queue(t_connection const * c);
extern void d2cs_conn_set_in_queue(t_connection * c, t_packet * packet);
extern unsigned int d2cs_conn_get_in_size(t_connection const * c);
extern unsigned int d2cs_conn_get_out_size(t_connection const * c);
extern int conn_push_outqueue(t_connection * c, t_packet * packet);
extern t_packet * conn_peek_outqueue(t_connection * c);
extern t_packet * conn_pull_outqueue(t_connection * c);
extern int conn_add_socket_flag(t_connection * c, unsigned int flag);
extern int conn_process_packet(t_connection * c, t_packet * packet, t_packet_handle_table * table,
				unsigned int table_size);
extern int d2cs_conn_set_account(t_connection * c, char const * account);
extern int d2cs_conn_set_charname(t_connection * c, char const * charname);
extern char const * d2cs_conn_get_account(t_connection const * c);
extern char const * d2cs_conn_get_charname(t_connection const * c);
extern int conn_set_charinfo(t_connection * c, t_d2charinfo_summary const * charinfo);
extern unsigned int conn_get_charinfo_ladder(t_connection const * c);
extern unsigned int conn_get_charinfo_expansion(t_connection const * c);
extern unsigned int conn_get_charinfo_hardcore(t_connection const * c);
extern unsigned int conn_get_charinfo_dead(t_connection const * c);
extern unsigned int conn_get_charinfo_difficulty(t_connection const * c);
extern unsigned int conn_get_charinfo_level(t_connection const * c);
extern unsigned int conn_get_charinfo_class(t_connection const * c);
extern unsigned int conn_get_d2gs_id(t_connection const * c);
extern int conn_set_d2gs_id(t_connection * c, unsigned int d2gs_id);
extern unsigned int d2cs_conn_get_addr(t_connection const * c);
extern unsigned short d2cs_conn_get_port(t_connection const * c);
extern t_gq * conn_get_gamequeue(t_connection const * c);
extern int conn_set_gamequeue(t_connection * c, t_gq * gq);
extern int conn_set_bnetd_sessionnum(t_connection * c, unsigned int sessionnum);
extern unsigned int conn_get_bnetd_sessionnum(t_connection const * c);
extern int conn_add_fd(t_connection * c, t_fdwatch_type rw, fdwatch_handler handler);
extern int connlist_check_timeout(void);

}

}

#endif
