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
#ifndef INCLUDED_HANDLE_D2CS_H
#define INCLUDED_HANDLE_D2CS_H

#include "common/packet.h"
#include "connection.h"

namespace pvpgn
{

namespace d2cs
{

extern int d2cs_handle_d2cs_packet(t_connection * c, t_packet * packet);
extern int d2cs_handle_client_creategame(t_connection * c, t_packet * packet);
extern int d2cs_send_client_creategamewait(t_connection * c, unsigned int position);

}

}

#endif
