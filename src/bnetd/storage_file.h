/*
 * Copyright (C) 2002,2003,2004 Dizzy
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

#ifndef INClUDED_STORAGE_FILE_TYPES
#define INClUDED_STORAGE_FILE_TYPES

#include "storage.h"

namespace pvpgn
{

	namespace bnetd
	{

		typedef const char t_file_info;

		typedef struct {
			t_attr * (*read_attr)(const char *filename, const char *key);
			int(*read_attrs)(const char *filename, t_read_attr_func cb, void *data);
			int(*write_attrs)(const char *filename, const t_hlist *attributes);
		} t_file_engine;

	}

}

#endif /* INClUDED_STORAGE_FILE_TYPES */

#ifndef JUST_NEED_TYPES
#ifndef INClUDED_STORAGE_FILE_PROTOS
#define INClUDED_STORAGE_FILE_PROTOS

namespace pvpgn
{

	namespace bnetd
	{

		extern t_storage storage_file;

	}

}

#endif /* INClUDED_STORAGE_FILE_PROTOS */
#endif /* JUST_NEED_TYPES */
