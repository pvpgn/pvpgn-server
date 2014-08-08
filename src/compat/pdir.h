/*
 * Copyright (C) 2001,2006  Dizzy
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
#ifndef INCLUDED_PVPGN_PDIR
#define INCLUDED_PVPGN_PDIR

#include <string>
#include <stdexcept>

#ifdef HAVE_DIRENT_H
# include <dirent.h>
#else
# ifdef HAVE_SYS_NDIR_H
#  include <sys/ndir.h>
# endif
# if HAVE_SYS_DIR_H
#  include <sys/dir.h>
# endif
# if HAVE_NDIR_H
#  include <ndir.h>
# endif
# define dirent direct
#endif
#ifdef WIN32
# include <io.h> /* for _findfirst(), _findnext(), etc */
#endif

#include <vector>

namespace pvpgn
{

	class Directory
	{
	public:
		class OpenError : public std::runtime_error
		{
		public:
			OpenError(const std::string& path)
				:std::runtime_error("Error opening directory: " + path) {}
			~OpenError() throw() {}
		};

		explicit Directory(const std::string& fname, bool lazyread = false);
		~Directory() throw();
		void rewind();
		char const * read() const;
		void open(const std::string& fname, bool lazyread = false);
		operator bool() const;

	private:
		std::string        path;
		bool lazyread;
#ifdef WIN32
		long               lFindHandle;
		mutable struct _finddata_t fileinfo;
		mutable int        status; /* -1 == failure, 0 == freshly opened, 1 == opened and read, 2 == eof */
#else /* POSIX */
		DIR *              dir;
#endif

		/* do not allow copying for the time being */
		Directory(const Directory&);
		Directory& operator=(const Directory&);
		void close();

	};

	extern std::vector<std::string> dir_getfiles(const char * directory, const char* ext, bool recursive);

}

#endif

