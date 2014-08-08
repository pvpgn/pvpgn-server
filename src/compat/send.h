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
#ifndef INCLUDED_SEND_PROTOS
#define INCLUDED_SEND_PROTOS

#ifndef HAVE_SEND

/* some sendto()s don't handle NULL, but send is called depreciated on BSD */
#ifdef HAVE_SENDTO
# include <cstddef>
# define send(s, b, l, f) sendto(s, b, l, f, NULL, NULL)
#else
# ifdef HAVE_WINSOCK2_H
#  include <winsock2.h>
#  define send(s, b, l, f) sendto(s, b, l, f, NULL, NULL)
# else
#   error "This program requires sendto()"
# endif
#endif

#endif

#endif
