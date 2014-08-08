/*
 * Copyright (C) 1999  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_ACCESS_PROTOS
#define INCLUDED_ACCESS_PROTOS

#ifdef WIN32
# include <io.h>
/* Values for the second argument to access. These may be OR'd together. */
# define R_OK	4	/* Test for read permission.	*/
# define W_OK	2	/* Test for write permission.	*/
# define X_OK	1	/* Test for execute permission.	*/
# define F_OK	0	/* Test for existence.		*/
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#endif
