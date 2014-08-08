/*
 * Copyright (C) 1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
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
#define ADDR_INTERNAL_ACCESS
#include "common/setup_before.h"
#include "common/addr.h"

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <cassert>

#include "compat/psock.h"
#include "compat/inet_aton.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "common/util.h"
#include "common/xalloc.h"
#include "common/setup_after.h"

namespace pvpgn
{

	static char const * netaddr_num_to_addr_str(unsigned int netipaddr, unsigned int netmask);


#define HACK_SIZE 4

	/* both arguments are in host byte order */
	extern char const * addr_num_to_addr_str(unsigned int ipaddr, unsigned short port)
	{
		static unsigned int curr = 0;
		static char         temp[HACK_SIZE][64];
		struct sockaddr_in  tsa;

		curr = (curr + 1) % HACK_SIZE;

		std::memset(&tsa, 0, sizeof(tsa));
		tsa.sin_family = PSOCK_AF_INET;
		tsa.sin_port = htons((unsigned short)0);
		tsa.sin_addr.s_addr = htonl(ipaddr);
		std::sprintf(temp[curr], "%.32s:%hu", inet_ntoa(tsa.sin_addr), port);

		return temp[curr];
	}


	/* ipaddr is in host byte order */
	extern char const * addr_num_to_ip_str(unsigned int ipaddr)
	{
		static unsigned int curr = 0;
		static char         temp[HACK_SIZE][64];
		struct sockaddr_in  tsa;

		curr = (curr + 1) % HACK_SIZE;

		std::memset(&tsa, 0, sizeof(tsa));
		tsa.sin_family = PSOCK_AF_INET;
		tsa.sin_port = htons((unsigned short)0);
		tsa.sin_addr.s_addr = htonl(ipaddr);
		std::sprintf(temp[curr], "%.32s", inet_ntoa(tsa.sin_addr));

		return temp[curr];
	}


	static char const * netaddr_num_to_addr_str(unsigned int netipaddr, unsigned int netmask)
	{
		static unsigned int curr = 0;
		static char         temp[HACK_SIZE][64];
		struct sockaddr_in  tsa;

		curr = (curr + 1) % HACK_SIZE;

		std::memset(&tsa, 0, sizeof(tsa));
		tsa.sin_family = PSOCK_AF_INET;
		tsa.sin_port = htons((unsigned short)0);
		tsa.sin_addr.s_addr = htonl(netipaddr);
		std::sprintf(temp[curr], "%.32s/0x%08x", inet_ntoa(tsa.sin_addr), netmask);

		return temp[curr];
	}



	extern char const * host_lookup(char const * hoststr, unsigned int * ipaddr)
	{
		struct sockaddr_in tsa;
#ifdef HAVE_GETHOSTBYNAME
		struct hostent *   hp;
#endif

		if (!hoststr)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL hoststr");
			return NULL;
		}
		if (!ipaddr)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL ipaddr");
			return NULL;
		}

		std::memset(&tsa, 0, sizeof(tsa));
		tsa.sin_family = PSOCK_AF_INET;
		tsa.sin_port = htons(0);

#ifdef HAVE_GETHOSTBYNAME
#ifdef WIN32
		psock_init();
#endif
		hp = gethostbyname(hoststr);
		if (!hp || !hp->h_addr_list)
#endif
		{
			if (inet_aton(hoststr, &tsa.sin_addr))
			{
				*ipaddr = ntohl(tsa.sin_addr.s_addr);
				return hoststr; /* We could call gethostbyaddr() on tsa to try and get the
						   official hostname but most systems would have already found
						   it when sending a dotted-quad to gethostbyname().  This is
						   good enough when that fails. */
			}
			eventlog(eventlog_level_error, __FUNCTION__, "could not lookup host \"%s\"", hoststr);
			return NULL;
		}

#ifdef HAVE_GETHOSTBYNAME
		std::memcpy(&tsa.sin_addr, (void *)hp->h_addr_list[0], sizeof(struct in_addr)); /* avoid warning */
		*ipaddr = ntohl(tsa.sin_addr.s_addr);
		if (hp->h_name)
			return hp->h_name;
		return hoststr;
#endif
	}


	extern t_addr * addr_create_num(unsigned int ipaddr, unsigned short port)
	{
		t_addr * temp;

		temp = (t_addr*)xmalloc(sizeof(t_addr));
		temp->str = xstrdup(addr_num_to_addr_str(ipaddr, port));
		temp->str = NULL;
		temp->ip = ipaddr;
		temp->port = port;
		temp->data.p = NULL;

		return temp;
	}


	extern t_addr * addr_create_str(char const * str, unsigned int defipaddr, unsigned short defport)
	{
		char *             tstr;
		t_addr *           temp;
		unsigned int       ipaddr;
		unsigned short     port;
		char const *       hoststr;
		char *             portstr;
		char const *       hostname;

		if (!str)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL str");
			return NULL;
		}

		tstr = xstrdup(str);

		if ((portstr = std::strrchr(tstr, ':')))
		{
			char * protstr;

			*portstr = '\0';
			portstr++;

			if ((protstr = std::strrchr(portstr, '/')))
			{
				*protstr = '\0';
				protstr++;
			}

			if (portstr[0] != '\0')
			{
				if (str_to_ushort(portstr, &port) < 0)
				{
#ifdef HAVE_GETSERVBYNAME
					struct servent * sp;

					if (!(sp = getservbyname(portstr, protstr ? protstr : "tcp")))
#endif
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not convert \"%s\" to a port number", portstr);
						xfree(tstr);
						return NULL;
					}
#ifdef HAVE_GETSERVBYNAME
					port = ntohs(sp->s_port);
#endif
				}
			}
			else
				port = defport;
		}
		else
			port = defport;

		if (tstr[0] != '\0')
			hoststr = tstr;
		else
		{
			struct sockaddr_in tsa;

			tsa.sin_addr.s_addr = htonl(defipaddr);
			hoststr = inet_ntoa(tsa.sin_addr);
		}

		if (!(hostname = host_lookup(hoststr, &ipaddr)))
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not lookup host \"%s\"", hoststr);
			xfree(tstr);
			return NULL;
		}

		temp = (t_addr*)xmalloc(sizeof(t_addr));
		temp->str = xstrdup(hostname);
		xfree(tstr);

		temp->ip = ipaddr;
		temp->port = port;
		temp->data.p = NULL;

		return temp;
	}


	extern int addr_destroy(t_addr const * addr)
	{
		if (!addr)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL addr");
			return -1;
		}

		if (addr->str)
			xfree((void *)addr->str); /* avoid warning */
		xfree((void *)addr); /* avoid warning */

		return 0;
	}


	/* hostname or IP */
	extern char * addr_get_host_str(t_addr const * addr, char * str, unsigned int len)
	{
		if (!addr)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL addr");
			return NULL;
		}
		if (!str)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL str");
			return NULL;
		}
		if (len < 2)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "str too short");
			return NULL;
		}

		if (!addr->str)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "addr has NULL str");
			return NULL;
		}

		std::strncpy(str, addr->str, len - 1);
		str[len - 1] = '\0';

		return str;
	}


	/* IP:port */
	extern char * addr_get_addr_str(t_addr const * addr, char * str, unsigned int len)
	{
		if (!addr)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL addr");
			return NULL;
		}
		if (!str)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL str");
			return NULL;
		}
		if (len < 2)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "str too short");
			return NULL;
		}

		std::strncpy(str, addr_num_to_addr_str(addr->ip, addr->port), len - 1);
		str[len - 1] = '\0';

		return str;
	}


	extern unsigned int addr_get_ip(t_addr const * addr)
	{
		if (!addr)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL addr");
			return 0;
		}

		return addr->ip;
	}


	extern unsigned short addr_get_port(t_addr const * addr)
	{
		if (!addr)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL addr");
			return 0;
		}

		return addr->port;
	}


	extern int addr_set_data(t_addr * addr, t_addr_data data)
	{
		if (!addr)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL addr");
			return -1;
		}

		addr->data = data;
		return 0;
	}


	extern t_addr_data addr_get_data(t_addr const * addr)
	{
		t_addr_data tdata;

		if (!addr)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL addr");
			tdata.p = NULL;
			return tdata;
		}

		return addr->data;
	}


	extern t_netaddr * netaddr_create_str(char const * netstr)
	{
		t_netaddr *  netaddr;
		char *       temp;
		char const * netipstr;
		char const * netmaskstr;
		unsigned int netip;
		unsigned int netmask;

		if (!netstr)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "unable to allocate memory for netaddr");
			return NULL;
		}

		temp = xstrdup(netstr);
		if (!(netipstr = std::strtok(temp, "/")))
		{
			xfree(temp);
			return NULL;
		}
		if (!(netmaskstr = std::strtok(NULL, "/")))
		{
			xfree(temp);
			return NULL;
		}

		netaddr = (t_netaddr*)xmalloc(sizeof(t_netaddr));

		/* FIXME: call getnetbyname() first, then host_lookup() */
		if (!host_lookup(netipstr, &netip))
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not lookup net");
			xfree(netaddr);
			xfree(temp);
			return NULL;
		}
		netaddr->ip = netip;

		if (str_to_uint(netmaskstr, &netmask) < 0)
		{
			struct sockaddr_in tsa;

			if (inet_aton(netmaskstr, &tsa.sin_addr))
				netmask = ntohl(tsa.sin_addr.s_addr);
			else
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not convert mask");
				xfree(netaddr);
				xfree(temp);
				return NULL;
			}
		}
		else
		{
			if (netmask > 32)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "network bits must be less than or equal to 32 (%u)", netmask);
				xfree(netaddr);
				xfree(temp);
				return NULL;
			}
			/* for example, 8 -> 11111111000000000000000000000000 */
			if (netmask != 0)
				netmask = ~((1 << (32 - netmask)) - 1);
		}
		netaddr->mask = netmask;

		xfree(temp);

		return netaddr;
	}


	extern int netaddr_destroy(t_netaddr const * netaddr)
	{
		if (!netaddr)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL netaddr");
			return -1;
		}

		xfree((void *)netaddr); /* avoid warning */

		return 0;
	}


	extern char * netaddr_get_addr_str(t_netaddr const * netaddr, char * str, unsigned int len)
	{
		if (!netaddr)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL netaddr");
			return NULL;
		}
		if (!str)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL str");
			return NULL;
		}
		if (len < 2)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "str too short");
			return NULL;
		}

		std::strncpy(str, netaddr_num_to_addr_str(netaddr->ip, netaddr->mask), len - 1); /* FIXME: format nicely with x.x.x.x/bitcount */
		str[len - 1] = '\0';

		return str;
	}


	extern int netaddr_contains_addr_num(t_netaddr const * netaddr, unsigned int ipaddr)
	{
		if (!netaddr)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL netaddr");
			return -1;
		}

		return (ipaddr&netaddr->mask) == netaddr->ip;
	}


	extern int addrlist_append(t_addrlist * addrlist, char const * str, unsigned int defipaddr, unsigned short defport)
	{
		t_addr *     addr;
		char *       tstr;
		char *       tok;

		assert(addrlist != NULL);

		if (!str)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL str");
			return -1;
		}

		tstr = xstrdup(str);
		for (tok = std::strtok(tstr, ","); tok; tok = std::strtok(NULL, ",")) /* std::strtok modifies the string it is passed */
		{
			if (!(addr = addr_create_str(tok, defipaddr, defport)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not create addr");
				xfree(tstr);
				return -1;
			}
			list_append_data(addrlist, addr);
		}

		xfree(tstr);

		return 0;
	}

	extern t_addrlist * addrlist_create(char const * str, unsigned int defipaddr, unsigned short defport)
	{
		t_addrlist * addrlist;

		if (!str)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL str");
			return NULL;
		}

		addrlist = list_create();

		if (addrlist_append(addrlist, str, defipaddr, defport) < 0) {
			eventlog(eventlog_level_error, __FUNCTION__, "could not append to newly created addrlist");
			list_destroy(addrlist);
			return NULL;
		}

		return addrlist;
	}

	extern int addrlist_destroy(t_addrlist * addrlist)
	{
		t_elem * curr;
		t_addr * addr;

		if (!addrlist)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL addrlist");
			return -1;
		}

		LIST_TRAVERSE(addrlist, curr)
		{
			if (!(addr = (t_addr*)elem_get_data(curr)))
				eventlog(eventlog_level_error, __FUNCTION__, "found NULL addr in list");
			else
				addr_destroy(addr);
			list_remove_elem(addrlist, &curr);
		}

		return list_destroy(addrlist);
	}


	extern int addrlist_get_length(t_addrlist const * addrlist)
	{
		return list_get_length(addrlist);
	}

}
