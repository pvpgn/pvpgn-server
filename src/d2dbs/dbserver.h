/*
 * Copyright (C) 2001		sousou	(liupeng.cs@263.net)
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
#ifndef INCLUDED_DBSERVER_H
#define INCLUDED_DBSERVER_H

#include "common/list.h"

namespace pvpgn
{

	namespace d2dbs
	{

		typedef struct {
			int		sd;
			unsigned int	ipaddr;
			unsigned char	major;
			unsigned char	minor;
			unsigned char	type;
			unsigned char	stats;
			unsigned int	serverid;
			unsigned int	verified;
			unsigned char	serverip[16];
			int		last_active;
			int nCharsInReadBuffer;
			int nCharsInWriteBuffer;
			char ReadBuf[kBufferSize];
			char WriteBuf[kBufferSize];
		} t_d2dbs_connection;

		typedef struct raw_preset_d2gsid {
			unsigned int	ipaddr;
			unsigned int	d2gsid;
			struct raw_preset_d2gsid	*next;
		} t_preset_d2gsid;

		int dbs_server_main(void);
		int dbs_server_shutdown_connection(t_d2dbs_connection* conn);

		extern t_list * dbs_server_connection_list;

	}

}

#endif
