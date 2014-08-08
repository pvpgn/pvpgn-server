/*
 * Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_UNAME_TYPES
#define INCLUDED_UNAME_TYPES

#ifndef HAVE_UNAME

#ifndef SYS_NMLN
# define SYS_NMLN 64
#endif

namespace pvpgn
{

	struct utsname
	{
		char sysname[SYS_NMLN];
		char nodename[SYS_NMLN];
		char release[SYS_NMLN];
		char version[SYS_NMLN];
		char machine[SYS_NMLN];
		char domainname[SYS_NMLN];
	};

}

#endif

#endif


#ifndef INCLUDED_UNAME_PROTOS
#define INCLUDED_UNAME_PROTOS

#ifndef HAVE_UNAME
namespace pvpgn
{

	extern int uname(struct utsname * buf);

}
#endif

#endif
