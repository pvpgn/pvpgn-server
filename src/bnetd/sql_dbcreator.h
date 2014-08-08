/*
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

#ifndef INCLUDED_SQL_DBCREATOR_TYPES
#define INCLUDED_SQL_DBCRETAOR_TYPES

#include "common/list.h"

namespace pvpgn
{

	namespace bnetd
	{

		typedef struct sqlcommand
#ifdef SQL_DBCREATOR_INTERNAL_ACCESS
		{
			char * sql_command;
			char * mode;
			char * extra_cmd;
		}
#endif
		t_sqlcommand;

		typedef struct column
#ifdef SQL_DBCREATOR_INTERNAL_ACCESS
		{
			char * name;
			char * value;
			char * mode;
			char * extra_cmd;
		}
#endif
		t_column;

		typedef struct table
#ifdef SQL_DBCREATOR_INTERNAL_ACCESS
		{
			char   * name;
			t_list * columns;
			t_list * sql_commands;
		}
#endif
		t_table;

		typedef struct db_layout
#ifdef SQL_DBCREATOR_INTERNAL_ACCESS
		{
			t_list * tables;
		}
#endif
		t_db_layout;

	}

}

#endif /* INCLUDED_SQL_DBCREATOR_TYPES */

#ifndef JUST_NEED_TYPES

#ifndef INCLUDED_SQL_DBCREATOR_PROTOS
#define INCLUDED_SQL_DBCRETAOR_PROTOS

#ifndef SQL_INTERNAL
#define SQL_INTERNAL 1
#endif
# include "sql_common.h"
#undef SQL_INTERNAL

namespace pvpgn
{

	namespace bnetd
	{

		int sql_dbcreator(t_sql_engine * sql);

	}

}

#endif /* INCLUDED_SQL_DBCREATOR_PROTOS */
#endif /* JUST_NEED_TYPES */
