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
#ifndef INCLUDED_STDFILENO_PROTOS
#define INCLUDED_STDFILENO_PROTOS

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifdef STDIN_FILENO
# define STDINFD STDIN_FILENO
#else
# define STDINFD 0
#endif

#ifdef STDOUT_FILENO
# define STDOUTFD STDOUT_FILENO
#else
# define STDOUTFD 1
#endif

#ifdef STDERR_FILENO
# define STDERRFD STDERR_FILENO
#else
# define STDERRFD 2
#endif

#endif
