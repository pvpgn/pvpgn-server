/*
 * Copyright (C) 1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#include "handle_init.h"

#include "common/init_protocol.h"
#include "common/eventlog.h"
#include "common/packet.h"
#include "common/bn_type.h"
#include "common/addr.h"

#include "prefs.h"
#include "connection.h"
#include "realm.h"
#include "handle_d2cs.h"
#include "common/setup_after.h"


namespace pvpgn
{

	namespace bnetd
	{

		extern int handle_init_packet(t_connection * c, t_packet const * const packet)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] got NULL connection", conn_get_socket(c));
				return -1;
			}
			if (!packet)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] got NULL packet", conn_get_socket(c));
				return -1;
			}
			if (packet_get_class(packet) != packet_class_init)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] got bad packet (class %d)", conn_get_socket(c), (int)packet_get_class(packet));
				return -1;
			}
			if ((prefs_get_max_conns_per_IP() != 0) &&
				bn_byte_get(packet->u.client_initconn.cclass) != CLIENT_INITCONN_CLASS_D2CS_BNETD &&
				(connlist_count_connections(conn_get_addr(c)) > prefs_get_max_conns_per_IP()))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] too many connections from address %s (closing connection)", conn_get_socket(c), addr_num_to_addr_str(conn_get_addr(c), conn_get_port(c)));
				return -1;
			}

			switch (packet_get_type(packet))
			{
			case CLIENT_INITCONN:
				switch (bn_byte_get(packet->u.client_initconn.cclass))
				{
				case CLIENT_INITCONN_CLASS_BNET:
					eventlog(eventlog_level_info, __FUNCTION__, "[%d] client initiated bnet connection", conn_get_socket(c));
					conn_set_state(c, conn_state_connected);
					conn_set_class(c, conn_class_bnet);

					break;

				case CLIENT_INITCONN_CLASS_FILE:
					eventlog(eventlog_level_info, __FUNCTION__, "[%d] client initiated file download connection", conn_get_socket(c));
					conn_set_state(c, conn_state_connected);
					conn_set_class(c, conn_class_file);

					break;

				case CLIENT_INITCONN_CLASS_BOT:
					eventlog(eventlog_level_info, __FUNCTION__, "[%d] client initiated chat bot connection", conn_get_socket(c));
					conn_set_state(c, conn_state_connected);
					conn_set_class(c, conn_class_bot);

					break;

				case CLIENT_INITCONN_CLASS_TELNET:
					eventlog(eventlog_level_info, __FUNCTION__, "[%d] client initiated telnet connection", conn_get_socket(c));
					conn_set_state(c, conn_state_connected);
					conn_set_class(c, conn_class_telnet);

					break;

				case CLIENT_INITCONN_CLASS_D2CS_BNETD:
				{
					eventlog(eventlog_level_info, __FUNCTION__, "[%d] client initiated d2cs_bnetd connection", conn_get_socket(c));
					
					if (!(realmlist_find_realm_by_ip(conn_get_addr(c))))
					{
						eventlog(eventlog_level_info, __FUNCTION__, "[%d] d2cs connection from unknown ip address %s", conn_get_socket(c), addr_num_to_addr_str(conn_get_addr(c), conn_get_port(c)));
						return -1;
					}
					
					conn_set_state(c, conn_state_connected);
					conn_set_class(c, conn_class_d2cs_bnetd);
					if (handle_d2cs_init(c) < 0)
					{
						eventlog(eventlog_level_info, __FUNCTION__, "faild to init d2cs connection");
						return -1;
					}
				}
					break;

				case CLIENT_INITCONN_CLASS_ENC:
					eventlog(eventlog_level_info, __FUNCTION__, "[%d] client initiated encrypted connection (not supported)", conn_get_socket(c));
					return -1;
					break;
				
				case CLIENT_INITCONN_CLASS_LOCALMACHINE:
					eventlog(eventlog_level_info, __FUNCTION__, "[%d] client initiated connection from local computer to 127.0.0.1", conn_get_socket(c));
					/*
					conn_set_state(c, conn_state_connected);
					conn_set_class(c, conn_class_localmachine;
					*/
					return -1;
					break;

				default:
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] client requested unknown class 0x%02x (length %d) (closing connection)", conn_get_socket(c), (unsigned int)bn_byte_get(packet->u.client_initconn.cclass), packet_get_size(packet));
					return -1;
				}
				break;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] unknown init packet type 0x%04x, len %u", conn_get_socket(c), packet_get_type(packet), packet_get_size(packet));
				return -1;
			}

			return 0;
		}

	}

}
