/*
 * Copyright (C) 2003 Dizzy
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

#ifndef __HANDLERS_H__
#define __HANDLERS_H__

#include "common/packet.h"
#include "connection.h"

namespace pvpgn
{

	namespace bnetd
	{

		typedef int(*t_handler)(t_connection *, t_packet const * const);

		typedef struct {
			int type;
			t_handler handler;
		} t_htable_row;

	}

}

#endif /* __HANDLERS_H__ */
