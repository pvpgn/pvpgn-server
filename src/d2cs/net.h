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
#ifndef INCLUDED_NET_H
#define INCLUDED_NET_H

namespace pvpgn
{

	namespace d2cs
	{

		extern int net_socket(int type);
		extern unsigned long int net_inet_addr(char const * host);
		extern int net_check_connected(int sock);
		extern int net_listen(unsigned int ip, unsigned int port, int type);
		extern int net_send_data(int sock, char * buff, int buffsize, int * pos, int * currsize);
		extern int net_recv_data(int sock, char * buff, int buffsize, int * pos, int * currsize);

	}

}

#endif
