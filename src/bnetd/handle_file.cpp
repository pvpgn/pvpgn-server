/*
 * Copyright (C) 1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 1999,2000  Rob Crittenden (rcrit@greyoak.com)
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
#include "handle_file.h"

#include "compat/psock.h"
#include "common/eventlog.h"
#include "common/packet.h"
#include "common/bn_type.h"

#include "connection.h"
#include "file.h"
#include "common/setup_after.h"


namespace pvpgn
{

	namespace bnetd
	{

		extern int handle_file_packet(t_connection * c, t_packet const * const packet)
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
			/* REMOVED BY UNDYING SOULZZ 4/3/02 */
			/*
				if (packet_get_class(packet)!=packet_class_file)
				{
				eventlog(eventlog_level_error,__FUNCTION__,"[%d] got bad packet (class %d)",conn_get_socket(c),(int)packet_get_class(packet));
				return -1;
				}
				*/
			switch (conn_get_state(c))
			{
			case conn_state_connected:
				switch (packet_get_type(packet))
				{
				case CLIENT_FILE_REQ:
				{
										char const * rawname;

										if (!(rawname = packet_get_str_const(packet, sizeof(t_client_file_req), MAX_FILENAME_STR)))
										{
											eventlog(eventlog_level_error, __FUNCTION__, "[%d] got bad FILE_REQ (missing or too long filename)", conn_get_socket(c));

											return -1;
										}

										file_send(c, rawname,
											bn_int_get(packet->u.client_file_req.adid),
											bn_int_get(packet->u.client_file_req.extensiontag),
											bn_int_get(packet->u.client_file_req.startoffset),
											1);
				}
					break;

				case CLIENT_FILE_REQ2:
				{
										 t_packet * rpacket = NULL;
										 if ((rpacket = packet_create(packet_class_raw))) {
											 packet_set_size(rpacket, sizeof(t_server_file_unknown1));
											 bn_int_set(&rpacket->u.server_file_unknown1.unknown, 0xdeadbeef);
											 conn_push_outqueue(c, rpacket);
											 packet_del_ref(rpacket);
										 }
										 conn_set_state(c, conn_state_pending_raw);
										 break;
				}

				default:
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] unknown file packet type 0x%04x, len %u", conn_get_socket(c), packet_get_type(packet), packet_get_size(packet));

					break;
				}
				break;

			case conn_state_pending_raw:
				switch (packet_get_type(packet))
				{
				case CLIENT_FILE_REQ3:
				{
										 char rawname[MAX_FILENAME_STR];

										 psock_recv(conn_get_socket(c), rawname, MAX_FILENAME_STR, 0);
										 file_send(c, rawname, 0, 0, 0, 1);
				}
					break;

				default:
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] unknown file packet type 0x%04x, len %u", conn_get_socket(c), packet_get_type(packet), packet_get_size(packet));

					break;
				}
				break;

			default:
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] unknown file connection state %d", conn_get_socket(c), (int)conn_get_state(c));
			}

			return 0;
		}

	}

}
