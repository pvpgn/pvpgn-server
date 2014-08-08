/*
 * Copyright (C) 2001  Ross Combs (rocombs@cs.nmsu.edu)
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
#include "channel_conv.h"
#include "common/bnet_protocol.h"
#include "channel.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		extern unsigned int cflags_to_bncflags(unsigned flags)
		{
			unsigned int res;

			res = 0;
			if (flags&channel_flags_public)
				res |= CF_PUBLIC;
			if (flags&channel_flags_moderated)
				res |= CF_MODERATED;
			if (flags&channel_flags_restricted)
				res |= CF_RESTRICTED;
			if (flags&channel_flags_thevoid)
				res |= CF_THEVOID;
			if (flags&channel_flags_system)
				res |= CF_SYSTEM;
			if (flags&channel_flags_official)
				res |= CF_OFFICIAL;

			return res;
		}

	}

}
