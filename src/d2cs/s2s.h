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
#ifndef INCLUDED_S2S_H
#define INCLUDED_S2S_H

#include "connection.h"

namespace pvpgn
{

	namespace d2cs
	{

		extern int s2s_init(void);
		extern int s2s_check(void);
		extern t_connection * s2s_create(char const * server, unsigned short def_port, t_conn_class cclass);
		extern int s2s_destroy(t_connection * s2s);

	}

}

#endif
