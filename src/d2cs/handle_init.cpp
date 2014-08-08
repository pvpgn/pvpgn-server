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
#include "handle_init.h"

#include "common/eventlog.h"
#include "common/addr.h"
#include "common/init_protocol.h"
#include "d2gs.h"
#include "handle_d2gs.h"
#include "prefs.h"
#include "common/setup_after.h"

namespace pvpgn
{

namespace d2cs
{

static int on_d2gs_initconn(t_connection * c);
static int on_d2cs_initconn(t_connection * c);

extern int d2cs_handle_init_packet(t_connection * c, t_packet * packet)
{
	int	cclass;
	int	retval;

	ASSERT(c,-1);
	ASSERT(packet,-1);
	cclass=bn_byte_get(packet->u.client_initconn.cclass);
	switch (cclass) {
		case CLIENT_INITCONN_CLASS_D2CS:
			retval=on_d2cs_initconn(c);
			break;
		case CLIENT_INITCONN_CLASS_D2GS:
			retval=on_d2gs_initconn(c);
			break;
		default:
			eventlog(eventlog_level_error,__FUNCTION__,"got bad connection class %d",cclass);
			retval=-1;
			break;
	}
	return retval;
}

static int on_d2gs_initconn(t_connection * c)
{
	t_d2gs * gs;

	eventlog(eventlog_level_info,__FUNCTION__,"[%d] client initiated d2gs connection",d2cs_conn_get_socket(c));
	if (!(gs=d2gslist_find_gs_by_ip(d2cs_conn_get_addr(c)))) {
		// reload list and see if any dns addy's has changed
		if (d2gslist_reload(prefs_get_d2gs_list())<0) {
			eventlog(eventlog_level_error,__FUNCTION__,"error reloading game server list,exitting");
			return -1;
		}
		//recheck
		if (!(gs=d2gslist_find_gs_by_ip(d2cs_conn_get_addr(c)))) {
			eventlog(eventlog_level_error,__FUNCTION__,"d2gs connection from invalid ip address %s",addr_num_to_ip_str(d2cs_conn_get_addr(c)));
			return -1;
		}
	}
	d2cs_conn_set_class(c,conn_class_d2gs);
	d2cs_conn_set_state(c,conn_state_connected);
	conn_set_d2gs_id(c,d2gs_get_id(gs));
	if (handle_d2gs_init(c)<0) {
		eventlog(eventlog_level_error,__FUNCTION__,"failed to init d2gs connection");
		return -1;
	}
	return 0;
}

static int on_d2cs_initconn(t_connection * c)
{
	eventlog(eventlog_level_info,__FUNCTION__,"[%d] client initiated d2cs connection",d2cs_conn_get_socket(c));
	d2cs_conn_set_class(c,conn_class_d2cs);
	d2cs_conn_set_state(c,conn_state_connected);
	return 0;
}

}

}
