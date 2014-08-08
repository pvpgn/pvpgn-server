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
#ifndef INCLUDED_ADDR_TYPES
#define INCLUDED_ADDR_TYPES

#ifdef JUST_NEED_TYPES
# include "common/list.h"
#else
# define JUST_NEED_TYPES
# include "common/list.h"
# undef JUST_NEED_TYPES
#endif

namespace pvpgn
{

	typedef union
	{
		unsigned long n;
		int           i;
		void *        p;
	} t_addr_data;

	typedef struct addr_struct
#ifdef ADDR_INTERNAL_ACCESS
	{
		char const * str; /* hostname or ip */
		unsigned int ip;
		unsigned int port;
		t_addr_data  data;  /* data argument */
	}
#endif
	t_addr;

	typedef struct netaddr_struct
#ifdef ADDR_INTERNAL_ACCESS
	{
		unsigned int ip;
		unsigned int mask;
	}
#endif
	t_netaddr;

	typedef t_list t_addrlist;

}

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_ADDR_PROTOS
#define INCLUDED_ADDR_PROTOS

namespace pvpgn
{

	/* ipaddr and port are in host byte order */
	extern char const * addr_num_to_addr_str(unsigned int ipaddr, unsigned short port);
	/* ipaddr is in host byte order */
	extern char const * addr_num_to_ip_str(unsigned int ipaddr);

	extern char const * host_lookup(char const * hoststr, unsigned int * ipaddr);

	/* ipaddr and port are in host byte order */
	extern t_addr * addr_create_num(unsigned int ipaddr, unsigned short port);

	/* defipaddr and defport are in host byte order */
	extern t_addr * addr_create_str(char const * str, unsigned int defipaddr, unsigned short defport);

	extern int addr_destroy(t_addr const * addr);
	extern char * addr_get_host_str(t_addr const * addr, char * str, unsigned int len);
	extern char * addr_get_addr_str(t_addr const * addr, char * str, unsigned int len);
	/* returned in host byte order */
	extern unsigned int addr_get_ip(t_addr const * addr);
	/* returned in host byte order */
	extern unsigned short addr_get_port(t_addr const * addr);
	extern int addr_set_data(t_addr * addr, t_addr_data data);
	extern t_addr_data addr_get_data(t_addr const * addr);
	extern t_netaddr * netaddr_create_str(char const * str);
	extern int netaddr_destroy(t_netaddr const * netaddr);
	extern char * netaddr_get_addr_str(t_netaddr const * netaddr, char * str, unsigned int len);
	extern int netaddr_contains_addr_num(t_netaddr const * netaddr, unsigned int ipaddr);

	/* defipaddr and defport are in host byte order */
	extern int addrlist_append(t_addrlist * addrlist, char const * str, unsigned int defipaddr, unsigned short defport);
	extern t_addrlist * addrlist_create(char const * str, unsigned int defipaddr, unsigned short defport);
	extern int addrlist_destroy(t_addrlist * addrlist);
	extern int addrlist_get_length(t_addrlist const * addrlist);
}

#endif
#endif
