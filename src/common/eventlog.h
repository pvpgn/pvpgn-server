/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_EVENTLOG_TYPES
#define INCLUDED_EVENTLOG_TYPES

namespace pvpgn
{

	typedef enum
	{
		eventlog_level_none = 0,
		eventlog_level_trace = 1,
		eventlog_level_debug = 2,
		eventlog_level_info = 4,
		eventlog_level_warn = 8,
		eventlog_level_error = 16,
		eventlog_level_fatal = 32
#ifdef WIN32_GUI
		, eventlog_level_gui = 64
#endif
	} t_eventlog_level;

}

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_EVENTLOG_PROTOS
#define INCLUDED_EVENTLOG_PROTOS

#include <cstdio>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <utility>

#include "compat/localtime_s.h"
#ifdef WIN32_GUI
# include "common/gui_printf.h"
#endif

#include <fmt/format.h>

namespace pvpgn
{

	extern void eventlog_set_debugmode(int debugmode);
	extern void eventlog_set(std::FILE * fp);
	extern std::FILE * eventlog_get(void);
	extern int eventlog_open(char const * filename);
	extern int eventlog_close(void);
	extern void eventlog_clear_level(void);
	extern int eventlog_add_level(char const * levelname);
	extern int eventlog_del_level(char const * levelname);
	extern char const * eventlog_get_levelname_str(t_eventlog_level level);
	extern void eventlog_hexdump_data(void const * data, unsigned int len);

	extern std::FILE *eventstrm;
	extern unsigned currlevel;
	extern int eventlog_debugmode;

	template <typename... Args>
	void eventlog(t_eventlog_level level, const char* module, fmt::string_view format_str, const Args& ... args)
	{
		if (!(level & currlevel))
		{
			return;
		}

		if (!eventstrm)
		{
			return;
		}

		std::time_t now = std::time(nullptr);
		std::tm tmnow = {};
		std::string time;
		if (now == (std::time_t)(-1) || pvpgn::localtime_s(&now, &tmnow) == nullptr)
		{
			time = "?";
		}
		else
		{
			std::stringstream temp;
			temp << std::put_time(&tmnow, EVENT_TIME_FORMAT);
			time = fmt::format("{}", temp.str());
		}

		if (!module)
		{
			fmt::print(eventstrm, "{} [error] eventlog: got NULL module\n", time);
#ifdef WIN32_GUI
			if (eventlog_level_gui & currlevel)
				gui_lvprintf(eventlog_level_error, "{} [error] eventlog: got NULL module\n", time);
#endif
			std::fflush(eventstrm);
			return;
		}

		if (format_str.size() == 0)
		{
			fmt::print(eventstrm, "{} [error] eventlog: got NULL fmt\n", time);
#ifdef WIN32_GUI
			if (eventlog_level_gui&currlevel)
				gui_lvprintf(eventlog_level_error, "{} [error] eventlog: got NULL fmt\n", time);
#endif
			std::fflush(eventstrm);
			return;
		}

		/****************************************************************/

		try
		{
			fmt::print(eventstrm, "{} [{}] {}: ", time, eventlog_get_levelname_str(level), module);
#ifdef WIN32_GUI
			if (eventlog_level_gui&currlevel)
			{
				gui_lvprintf(level, "{} [{}] {}: ", time, eventlog_get_levelname_str(level), module);
			}
#endif

			fmt::print(eventstrm, format_str, args...);
#ifdef WIN32_GUI
			if (eventlog_level_gui & currlevel)
			{
				gui_lvprintf(level, format_str.data(), args...);
			}
#endif

			fmt::print(eventstrm, "\n");
#ifdef WIN32_GUI
			if (eventlog_level_gui & currlevel)
			{
				gui_lvprintf(level, "\n");
			}
#endif

			if (eventlog_debugmode)
			{
				fmt::print("{} [{}] {}: {}\n", time, eventlog_get_levelname_str(level), module, fmt::format(format_str, args...));
				std::fflush(stdout);
			}
		}
		catch (const fmt::format_error& e)
		{
			fmt::print(eventstrm, "Failed to format string ({})\n", e.what());
#ifdef WIN32_GUI
			if (eventlog_level_gui & currlevel)
				gui_lvprintf(eventlog_level_error, "Failed to format string ({})\n", e.what());
#endif
		}

		std::fflush(eventstrm);
	}

	extern void eventlog_step(char const * filename, t_eventlog_level level, char const * module, char const * fmt, ...) PRINTF_ATTR(4, 5);

#define ERROR0(fmt) eventlog(eventlog_level_error,__FUNCTION__,fmt)
#define ERROR1(fmt,arg1) eventlog(eventlog_level_error,__FUNCTION__,fmt,arg1)
#define ERROR2(fmt,arg1,arg2) eventlog(eventlog_level_error,__FUNCTION__,fmt,arg1,arg2)
#define ERROR3(fmt,arg1,arg2,arg3) eventlog(eventlog_level_error,__FUNCTION__,fmt,arg1,arg2,arg3)

#define WARN0(fmt) eventlog(eventlog_level_warn,__FUNCTION__,fmt)
#define WARN1(fmt,arg1) eventlog(eventlog_level_warn,__FUNCTION__,fmt,arg1)
#define WARN2(fmt,arg1,arg2) eventlog(eventlog_level_warn,__FUNCTION__,fmt,arg1,arg2)
#define WARN3(fmt,arg1,arg2,arg3) eventlog(eventlog_level_warn,__FUNCTION__,fmt,arg1,arg2,arg3)

#define INFO0(fmt) eventlog(eventlog_level_info,__FUNCTION__,fmt)
#define INFO1(fmt,arg1) eventlog(eventlog_level_info,__FUNCTION__,fmt,arg1)
#define INFO2(fmt,arg1,arg2) eventlog(eventlog_level_info,__FUNCTION__,fmt,arg1,arg2)
#define INFO3(fmt,arg1,arg2,arg3) eventlog(eventlog_level_info,__FUNCTION__,fmt,arg1,arg2,arg3)

#define DEBUG0(fmt) eventlog(eventlog_level_debug,__FUNCTION__,fmt)
#define DEBUG1(fmt,arg1) eventlog(eventlog_level_debug,__FUNCTION__,fmt,arg1)
#define DEBUG2(fmt,arg1,arg2) eventlog(eventlog_level_debug,__FUNCTION__,fmt,arg1,arg2)
#define DEBUG3(fmt,arg1,arg2,arg3) eventlog(eventlog_level_debug,__FUNCTION__,fmt,arg1,arg2,arg3)

#define TRACE0(fmt) eventlog(eventlog_level_trace,__FUNCTION__,fmt)
#define TRACE1(fmt,arg1) eventlog(eventlog_level_trace,__FUNCTION__,fmt,arg1)
#define TRACE2(fmt,arg1,arg2) eventlog(eventlog_level_trace,__FUNCTION__,fmt,arg1,arg2)
#define TRACE3(fmt,arg1,arg2,arg3) eventlog(eventlog_level_trace,__FUNCTION__,fmt,arg1,arg2,arg3)

}

#endif
#endif
