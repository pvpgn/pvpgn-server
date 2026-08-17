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

#define STATUS_D2LADDER_FAILURE		20
#define STATUS_CHARLOCK_FAILURE		30
#define STATUS_FDWATCH_FAILURE		90


namespace pvpgn
{

	namespace d2dbs
	{

#ifdef SERVER_INTERNAL_ACCESS

		enum t_laddr_type
		{
			laddr_type_d2gs, // d2gs (port 4000)
		};

		// listen address structure
		struct t_laddr_info
		{
			int ssocket; // TCP listen socket
			t_laddr_type type;
		};

#endif

		int pre_server_startup();
		bool server_process();
		void post_server_shutdown(int status);

	}

}

#endif
