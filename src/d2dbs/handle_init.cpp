/*
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

#include "common/addr.h"
#include "common/bn_type.h"
#include "common/eventlog.h"
#include "common/init_protocol.h"
#include "common/packet.h"
#include "common/xalloc.h"

#include "compat/strsep.h"

#include "connection.h"
#include "prefs.h"

#include "common/setup_after.h"


namespace pvpgn
{

	namespace d2dbs
	{

		static bool dbs_verify_ipaddr(t_d2dbs_connection* c);


		int handle_init_packet(t_d2dbs_connection* c, const t_packet* const packet)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (!packet)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got NULL packet", c->sd);
				return -1;
			}

			if (packet_get_class(packet) != packet_class_init)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] got bad packet (class {})", c->sd, (int)packet_get_class(packet));
				return -1;
			}

			switch (packet_get_type(packet))
			{
			case CLIENT_INITCONN:
				switch (bn_byte_get(packet->u.client_initconn.cclass))
				{
				case CLIENT_INITCONN_CLASS_D2GS_D2DBS:
					eventlog(eventlog_level_info, __FUNCTION__, "[{}] client initiated d2gs connection", c->sd);

					if (!dbs_verify_ipaddr(c))
					{
						eventlog(eventlog_level_info, __FUNCTION__, "[{}] d2gs connection from unknown ip address {}", c->sd, addr_num_to_ip_str(c->ipaddr));
						return -1;
					}

					conn_set_class(c, conn_class_d2gs);
					conn_set_state(c, conn_state_loggedin);

					break;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "[{}] client requested unknown class 0x{:02x} (length {}) (closing connection)", c->sd, bn_byte_get(packet->u.client_initconn.cclass), packet_get_size(packet));
					return -1;
				}
				break;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] unknown init packet type 0x{:04x}, len {}", c->sd, packet_get_type(packet), packet_get_size(packet));
				return -1;
			}

			return 0;
		}

		static bool dbs_verify_ipaddr(t_d2dbs_connection* c)
		{
			bool c_ipaddr_is_valid = false;

			{
				char* adlist = xstrdup(d2dbs_prefs_get_d2gs_list());
				char* temp = adlist;
				char* s;
				while ((s = strsep(&temp, ",")))
				{
					unsigned int resolveipaddr = 0;
					host_lookup(s, &resolveipaddr);
					if (resolveipaddr == 0) continue;

					if (c->ipaddr == resolveipaddr)
					{
						c_ipaddr_is_valid = true;
						break;
					}
				}
				xfree(adlist);
			}

			if (c_ipaddr_is_valid)
			{
				eventlog(eventlog_level_info, __FUNCTION__, "[{}] ip address {} is valid", c->sd, addr_num_to_ip_str(c->ipaddr));

				for (auto tempc : connlist())
				{
					if (!tempc)
					{
						continue;
					}

					if (tempc != c && tempc->ipaddr == c->ipaddr)
					{
						eventlog(eventlog_level_info, __FUNCTION__, "[{}] destroying previous connection {} from same ip address", c->sd, tempc->serverid);
						conn_set_state(tempc, conn_state_destroy);
					}
				}

				c->verified = 1;

				return true;
			}
			else
			{
				eventlog(eventlog_level_info, __FUNCTION__, "[{}] ip address {} is invalid", c->sd, addr_num_to_ip_str(c->ipaddr));
			}

			return false;
		}

	}

}
