/*
 * Copyright (C) 1999  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1999  Ross Combs (rocombs@cs.nmsu.edu)
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
#define TRACKER_INTERNAL_ACCESS
#define SERVER_INTERNAL_ACCESS
#include "tracker.h"

#include <cstring>
#include <cerrno>

#ifdef HAVE_SYS_UTSNAME_H
# include <sys/utsname.h>
#endif

#include "compat/psock.h"
#include "compat/strerror.h"
#include "compat/uname.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "common/addr.h"
#include "common/tracker.h"
#include "common/bn_type.h"

#include "prefs.h"
#include "connection.h"
#include "channel.h"
#include "game.h"
#include "server.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static t_addrlist * track_servers = NULL;


		extern int tracker_set_servers(char const * servers)
		{
			t_addr const * addr;
			t_elem const * curr;
			char           temp[32];

			if (track_servers && addrlist_destroy(track_servers) < 0)
				eventlog(eventlog_level_error, __FUNCTION__, "unable to destroy tracker list");

			if (!servers)
			{
				track_servers = NULL;
				return 0;
			}

			if (!(track_servers = addrlist_create(servers, INADDR_LOOPBACK, BNETD_TRACK_PORT)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not create tracking server list");
				return -1;
			}

			LIST_TRAVERSE_CONST(track_servers, curr)
			{
				addr = (t_addr*)elem_get_data(curr);
				if (!addr_get_addr_str(addr, temp, sizeof(temp)))
					std::strcpy(temp, "x.x.x.x:x");
				eventlog(eventlog_level_info, __FUNCTION__, "tracking packets will be sent to %s", temp);
			}

			return 0;
		}


		extern int tracker_send_report(t_addrlist const * laddrs)
		{
			t_addr const *     addrl;
			t_elem const *     currl;
			t_addr const *     addrt;
			t_elem const *     currt;
			t_trackpacket      packet;
			struct utsname     utsbuf;
			struct sockaddr_in tempaddr;
			t_laddr_info *     laddr_info;
			char               tempa[64];
			char               tempb[64];

			if (addrlist_get_length(track_servers) > 0)
			{
				std::memset(&packet, 0, sizeof(packet));
				bn_short_nset(&packet.packet_version, (unsigned short)TRACK_VERSION);
				/* packet.port is set below */
				bn_int_nset(&packet.flags, 0);
				std::strncpy((char *)packet.server_location,
					prefs_get_location(),
					sizeof(packet.server_location));
				bn_byte_set(&packet.server_location[sizeof(packet.server_location) - 1], '\0');
				std::strncpy((char *)packet.software,
					PVPGN_SOFTWARE,
					sizeof(packet.software));
				bn_byte_set(&packet.software[sizeof(packet.software) - 1], '\0');
				std::strncpy((char *)packet.version,
					PVPGN_VERSION,
					sizeof(packet.version));
				bn_byte_set(&packet.version[sizeof(packet.version) - 1], '\0');
				std::strncpy((char *)packet.server_desc,
					prefs_get_description(),
					sizeof(packet.server_desc));
				bn_byte_set(&packet.server_desc[sizeof(packet.server_desc) - 1], '\0');
				std::strncpy((char *)packet.server_url,
					prefs_get_url(),
					sizeof(packet.server_url));
				bn_byte_set(&packet.server_url[sizeof(packet.server_url) - 1], '\0');
				std::strncpy((char *)packet.contact_name,
					prefs_get_contact_name(),
					sizeof(packet.contact_name));
				bn_byte_set(&packet.contact_name[sizeof(packet.contact_name) - 1], '\0');
				std::strncpy((char *)packet.contact_email,
					prefs_get_contact_email(),
					sizeof(packet.contact_email));
				bn_byte_set(&packet.contact_email[sizeof(packet.contact_email) - 1], '\0');
				bn_int_nset(&packet.users, connlist_login_get_length());
				bn_int_nset(&packet.channels, channellist_get_length());
				bn_int_nset(&packet.games, gamelist_get_length());
				bn_int_nset(&packet.uptime, server_get_uptime());
				bn_int_nset(&packet.total_logins, connlist_total_logins());
				bn_int_nset(&packet.total_games, gamelist_total_games());

				if (uname(&utsbuf) < 0)
				{
					eventlog(eventlog_level_warn, __FUNCTION__, "could not get platform info (uname: %s)", pstrerror(errno));
					std::strncpy((char *)packet.platform, "", sizeof(packet.platform));
				}
				else
				{
					std::strncpy((char *)packet.platform,
						utsbuf.sysname,
						sizeof(packet.platform));
					bn_byte_set(&packet.platform[sizeof(packet.platform) - 1], '\0');
				}

				LIST_TRAVERSE_CONST(laddrs, currl)
				{
					addrl = (t_addr*)elem_get_data(currl);

					if (!(laddr_info = (t_laddr_info*)addr_get_data(addrl).p))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "address data is NULL");
						continue;
					}
					if (laddr_info->type != laddr_type_bnet)
						continue; /* don't report IRC, telnet, and other non-game ports */

					bn_short_nset(&packet.port, addr_get_port(addrl));

					LIST_TRAVERSE_CONST(track_servers, currt)
					{
						addrt = (t_addr*)elem_get_data(currt);

						std::memset(&tempaddr, 0, sizeof(tempaddr));
						tempaddr.sin_family = PSOCK_AF_INET;
						tempaddr.sin_port = htons(addr_get_port(addrt));
						tempaddr.sin_addr.s_addr = htonl(addr_get_ip(addrt));

						if (!addr_get_addr_str(addrl, tempa, sizeof(tempa)))
							std::strcpy(tempa, "x.x.x.x:x");
						if (!addr_get_addr_str(addrt, tempb, sizeof(tempb)))
							std::strcpy(tempa, "x.x.x.x:x");
						/* eventlog(eventlog_level_debug,__FUNCTION__,"sending tracking info from %s to %s",tempa,tempb); */

						if (psock_sendto(laddr_info->usocket, &packet, sizeof(packet), 0, (struct sockaddr *)&tempaddr, (psock_t_socklen)sizeof(tempaddr)) < 0)
							eventlog(eventlog_level_warn, __FUNCTION__, "could not send tracking information from %s to %s (psock_sendto: %s)", tempa, tempb, pstrerror(errno));
					}
				}
			}

			return 0;
		}

	}

}
