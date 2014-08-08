/*
* Copyright (C) 2014  HarpyWar (harpywar@gmail.com)
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
#ifndef INCLUDED_LOCALIZATION_TYPES
#define INCLUDED_LOCALIZATION_TYPES

namespace pvpgn
{
	namespace bnetd
	{
		extern const t_gamelang languages[12];

	}
}

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_LOCALIZATION_PROTOS
#define INCLUDED_LOCALIZATION_PROTOS

#define JUST_NEED_TYPES
# include <string>
# include "connection.h"
# include "common/format.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{
		extern int i18n_load(void);
		extern int i18n_reload(void);

		extern const char * i18n_filename(const char * filename, t_tag gamelang);
		extern t_gamelang lang_find_by_country(const char * code);
		extern t_gamelang conn_get_gamelang_localized(t_connection * c);

		extern int handle_language_command(t_connection * c, char const *text);
		extern char * i18n_convert(t_connection * c, char * buf);


		extern std::string _localize(t_connection * c, const char * func, const char *fmt, const fmt::ArgList &args);
		FMT_VARIADIC(std::string, _localize, t_connection *, const char *, const char *)

		#define localize(c, ...) _localize(c, __FUNCTION__, __VA_ARGS__)
	}

}

/*****/
#endif
#endif
