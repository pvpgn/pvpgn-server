/*
 * Copyright (C) 2004 CreepLord (creeplord@pvpgn.org)
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
#ifndef INCLUDED_RUNTIME_LIBS_H
#define INCLUDED_RUNTIME_LIBS_H

#ifdef WIN32

# include <windows.h>		/* for GetProcAddress() & LoadLibrary() & FreeLibrary() */
# define OpenLibrary(l)		LoadLibrary(l)
# define GetFunction(h,f)	GetProcAddress((HINSTANCE)h,f)
# define CloseLibrary(h)	FreeLibrary((HINSTANCE)h)

# define MYSQL_LIB		"libmysql.dll"
# define PGSQL_LIB		"libpq.dll"
# define SQLITE3_LIB		"sqlite3.dll"
# define ODBC_LIB		"odbc32.dll"

#else /* FIXME: will this work on all nix like systems? */

# include <dlfcn.h>		/* for dlopen() & dlsym() & dlclose() */
/* link to 'libdl.so' */
# define OpenLibrary(l)		dlopen(l, RTLD_LOCAL | RTLD_LAZY) /* is this the correct mode? */
# define GetFunction(h,f)	dlsym(h,f)
# define CloseLibrary(h)	dlclose(h)

# define MYSQL_LIB		"libmysqlclient.so"
# define PGSQL_LIB		"libpq.so"
# define ODBC_LIB		"libodbc.so"
# define SQLITE3_LIB		"libsqlite3.so"

#endif

#endif
