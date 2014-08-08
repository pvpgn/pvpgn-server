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
#ifndef INCLUDED_D2GS_H
#define INCLUDED_D2GS_H

#include "common/list.h"
#include "connection.h"

namespace pvpgn
{

namespace d2cs
{

typedef enum
{
    d2gs_state_none,
    d2gs_state_connected,
    d2gs_state_authed
} t_d2gs_state;

typedef struct d2gs
{
	unsigned int		ip;
	unsigned int      	id;
	unsigned int		flag;
	unsigned int		active;
	unsigned int		token;
	t_d2gs_state	   	state;
	unsigned int      	maxgame;
	unsigned int      	gamenum;
	t_connection *		connection;
} t_d2gs;

#define D2GS_FLAG_VALID		0x01

extern t_list *	d2gslist(void);
extern int d2gslist_create(void);
extern int d2gslist_destroy(void);
extern int d2gslist_reload(char const * gslist);
extern t_d2gs * d2gs_create(char const * ip);
extern int d2gs_destroy(t_d2gs * gs,t_elem ** curr);
extern t_d2gs * d2gslist_get_server_by_id(unsigned int id);
extern t_d2gs * d2gslist_choose_server(void);
extern t_d2gs * d2gslist_find_gs(unsigned int id);
extern t_d2gs * d2gslist_find_gs_by_ip(unsigned int ip);
extern unsigned int d2gs_get_id(t_d2gs const * gs);
extern unsigned int d2gs_get_ip(t_d2gs const * gs);
extern t_d2gs_state d2gs_get_state(t_d2gs const * gs);
extern int d2gs_set_state(t_d2gs * gs, t_d2gs_state state);
extern unsigned int d2gs_get_maxgame(t_d2gs const * gs);
extern int d2gs_set_maxgame(t_d2gs * gs,unsigned int maxgame);
extern unsigned int d2gs_get_gamenum(t_d2gs const * gs);
extern int d2gs_add_gamenum(t_d2gs * gs,int number);
extern unsigned int d2gs_get_token(t_d2gs const * gs);
extern unsigned int d2gs_make_token(t_d2gs * gs);
extern t_connection * d2gs_get_connection(t_d2gs const * gs);
extern int d2gs_active(t_d2gs * gs, t_connection * c);
extern int d2gs_deactive(t_d2gs * gs, t_connection * c);
extern unsigned int d2gs_calc_checksum(t_connection * c);
extern int d2gs_keepalive(void);
extern int d2gs_restart_all_gs(void);

}

}

#define D2GS_MAJOR_VERSION_MASK			0xffffff00

#endif
