/*
 * Copyright (C) 2004  CreepLord (creeplord@pvpgn.org)
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
#ifndef INCLUDED_TRANS_TYPES
#define INCLUDED_TRANS_TYPES

#ifdef TRANS_INTERNAL_ACCESS

#ifdef JUST_NEED_TYPES
# include "common/addr.h"
#else
# define JUST_NEED_TYPES
# include "common/addr.h"
# undef JUST_NEED_TYPES
#endif

namespace pvpgn
{

	typedef struct
	{
		t_addr	*input;
		t_addr	*output;
		t_netaddr	*network;
	} t_trans;

}

#endif

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_TRANS_PROTOS
#define INCLUDED_TRANS_PROTOS

#define TRANS_BNETD 1
#define TRANS_D2CS  2

namespace pvpgn
{

	extern int trans_load(char const * filename, int program);
	extern int trans_unload(void);
	extern int trans_reload(char const * filename, int program);
	extern int trans_net(unsigned int clientaddr, unsigned int *addr, unsigned short *port);

}

#endif
#endif
