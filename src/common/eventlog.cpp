/*
 * Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
 * Copyright (C) 1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#include "common/setup_before.h"
#include "common/eventlog.h"
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <cstdarg>

#include "compat/strcasecmp.h"
#include "common/hexdump.h"
#ifdef WIN32_GUI
# include "common/gui_printf.h"
#endif
#include "common/setup_after.h"

namespace pvpgn
{

	static std::FILE *           eventstrm = NULL;
	static unsigned currlevel = eventlog_level_debug |
		eventlog_level_info |
		eventlog_level_warn |
		eventlog_level_error |
		eventlog_level_fatal
#ifdef WIN32GUI
		| eventlog_level_gui
#endif
		;
	/* FIXME: maybe this should be default for win32 */
	static int eventlog_debugmode = 0;

	extern void eventlog_set_debugmode(int debugmode)
	{
		eventlog_debugmode = debugmode;
	}

	extern void eventlog_set(std::FILE * fp)
	{
		eventstrm = fp;
	}

	extern std::FILE * eventlog_get(void)
	{
		return eventstrm;
	}

	extern int eventlog_close(void)
	{
		std::fclose(eventstrm);
		return 0;
	}

	extern int eventlog_open(char const * filename)
	{
		std::FILE * temp;

		if (!filename)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
			return -1;
		}

		if (!(temp = std::fopen(filename, "a")))
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for appending (std::fopen: %s)", filename, std::strerror(errno));
			return -1;
		}

		if (eventstrm && eventstrm != stderr) /* close old one */
		if (std::fclose(eventstrm) < 0)
		{
			eventstrm = temp;
			eventlog(eventlog_level_error, __FUNCTION__, "could not close previous logfile after writing (std::fclose: %s)", std::strerror(errno));
			return 0;
		}
		eventstrm = temp;

		return 0;
	}


	extern void eventlog_clear_level(void)
	{
		currlevel = eventlog_level_none;
	}


	extern int eventlog_add_level(char const * levelname)
	{
		if (!levelname)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL levelname");
			return -1;
		}

		if (strcasecmp(levelname, "trace") == 0)
		{
			currlevel |= eventlog_level_trace;
			return 0;
		}
		if (strcasecmp(levelname, "debug") == 0)
		{
			currlevel |= eventlog_level_debug;
			return 0;
		}
		if (strcasecmp(levelname, "info") == 0)
		{
			currlevel |= eventlog_level_info;
			return 0;
		}
		if (strcasecmp(levelname, "warn") == 0)
		{
			currlevel |= eventlog_level_warn;
			return 0;
		}
		if (strcasecmp(levelname, "error") == 0)
		{
			currlevel |= eventlog_level_error;
			return 0;
		}
		if (strcasecmp(levelname, "fatal") == 0)
		{
			currlevel |= eventlog_level_fatal;
			return 0;
		}
#ifdef WIN32_GUI
		if (strcasecmp(levelname, "gui") == 0)
		{
			currlevel |= eventlog_level_gui;
			return 0;
		}
#endif

		eventlog(eventlog_level_error, __FUNCTION__, "got bad levelname \"%s\"", levelname);
		return -1;
	}


	extern int eventlog_del_level(char const * levelname)
	{
		if (!levelname)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL levelname");
			return -1;
		}

		if (strcasecmp(levelname, "trace") == 0)
		{
			currlevel &= ~eventlog_level_trace;
			return 0;
		}
		if (strcasecmp(levelname, "debug") == 0)
		{
			currlevel &= ~eventlog_level_debug;
			return 0;
		}
		if (strcasecmp(levelname, "info") == 0)
		{
			currlevel &= ~eventlog_level_info;
			return 0;
		}
		if (strcasecmp(levelname, "warn") == 0)
		{
			currlevel &= ~eventlog_level_warn;
			return 0;
		}
		if (strcasecmp(levelname, "error") == 0)
		{
			currlevel &= ~eventlog_level_error;
			return 0;
		}
		if (strcasecmp(levelname, "fatal") == 0)
		{
			currlevel &= ~eventlog_level_fatal;
			return 0;
		}
#ifdef WIN32_GUI
		if (strcasecmp(levelname, "gui") == 0)
		{
			currlevel &= ~eventlog_level_gui;
			return 0;
		}
#endif

		eventlog(eventlog_level_error, __FUNCTION__, "got bad levelname \"%s\"", levelname);
		return -1;
	}

	extern char const * eventlog_get_levelname_str(t_eventlog_level level)
	{
		switch (level)
		{
		case eventlog_level_trace:
			return "trace";
		case eventlog_level_debug:
			return "debug";
		case eventlog_level_info:
			return "info ";
		case eventlog_level_warn:
			return "warn ";
		case eventlog_level_error:
			return "error";
		case eventlog_level_fatal:
			return "fatal";
#ifdef WIN32_GUI
		case eventlog_level_gui:
			return "gui";
#endif
		default:
			return "unknown";
		}
	}

	extern void eventlog_hexdump_data(void const * data, unsigned int len)
	{
		unsigned int i;
		char dst[100];
		unsigned char * datac;

		if (!data) {
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL data");
			return;
		}

		for (i = 0, datac = (unsigned char*)data; i < len; i += 16, datac += 16)
		{
			hexdump_string(datac, (len - i < 16) ? (len - i) : 16, dst, i);
			std::fprintf(eventstrm, "%s\n", dst);
#ifdef WIN32_GUI
			if (eventlog_level_gui&currlevel)
				gui_lprintf(eventlog_level_info, "%s\n", dst);
#endif
			if (eventlog_debugmode)
			{
				std::printf("%s\n", dst);
			}

		}
		if (eventlog_debugmode) std::fflush(stdout);
		std::fflush(eventstrm);
	}

	extern void eventlog(t_eventlog_level level, char const * module, char const * fmt, ...)
	{
		std::va_list     args;
		char        time_string[EVENT_TIME_MAXLEN];
		struct std::tm * tmnow;
		std::time_t      now;

		if (!(level&currlevel))
			return;
		if (!eventstrm)
			return;

		/* get the time before parsing args */
		std::time(&now);
		if (!(tmnow = std::localtime(&now)))
			std::strcpy(time_string, "?");
		else
			std::strftime(time_string, EVENT_TIME_MAXLEN, EVENT_TIME_FORMAT, tmnow);

		if (!module)
		{
			std::fprintf(eventstrm, "%s [error] eventlog: got NULL module\n", time_string);
#ifdef WIN32_GUI
			if (eventlog_level_gui&currlevel)
				gui_lprintf(eventlog_level_error, "%s [error] eventlog: got NULL module\n", time_string);
#endif
			std::fflush(eventstrm);
			return;
		}

		if (!fmt)
		{
			std::fprintf(eventstrm, "%s [error] eventlog: got NULL fmt\n", time_string);
#ifdef WIN32_GUI
			if (eventlog_level_gui&currlevel)
				gui_lprintf(eventlog_level_error, "%s [error] eventlog: got NULL fmt\n", time_string);
#endif
			std::fflush(eventstrm);
			return;
		}

		std::fprintf(eventstrm, "%s [%s] %s: ", time_string, eventlog_get_levelname_str(level), module);
#ifdef WIN32_GUI
		if (eventlog_level_gui&currlevel)
			gui_lprintf(level, "%s [%s] %s: ", time_string, eventlog_get_levelname_str(level), module);
#endif

		va_start(args, fmt);

		std::vfprintf(eventstrm, fmt, args);
#ifdef WIN32_GUI
		if (eventlog_level_gui&currlevel)
			gui_lvprintf(level, fmt, args);
#endif
		va_end(args);
		std::fprintf(eventstrm, "\n");
#ifdef WIN32_GUI
		if (eventlog_level_gui&currlevel)
			gui_lprintf(level, "\n");
#endif

		if (eventlog_debugmode) {
			std::printf("%s [%s] %s: ", time_string, eventlog_get_levelname_str(level), module);
			va_start(args, fmt);
			std::vprintf(fmt, args);
			va_end(args);
			std::printf("\n");
			std::fflush(stdout);
		}
		std::fflush(eventstrm);
	}


	extern void eventlog_step(char const * filename, t_eventlog_level level, char const * module, char const * fmt, ...)
	{
		std::va_list args;
		char        time_string[EVENT_TIME_MAXLEN];
		struct std::tm * tmnow;
		std::time_t      now;
		std::FILE *      fp;

		if (!(level&currlevel))
			return;
		if (!eventstrm)
			return;

		if (!(fp = std::fopen(filename, "a")))
			return;

		/* get the time before parsing args */
		std::time(&now);
		if (!(tmnow = std::localtime(&now)))
			std::strcpy(time_string, "?");
		else
			std::strftime(time_string, EVENT_TIME_MAXLEN, EVENT_TIME_FORMAT, tmnow);

		if (!module)
		{
			std::fprintf(fp, "%s [error] eventlog_step: got NULL module\n", time_string);
			std::fclose(fp);
			return;
		}
		if (!fmt)
		{
			std::fprintf(fp, "%s [error] eventlog_step: got NULL fmt\n", time_string);
			std::fclose(fp);
			return;
		}

		std::fprintf(fp, "%s [%s] %s: ", time_string, eventlog_get_levelname_str(level), module);
		va_start(args, fmt);
		std::vfprintf(fp, fmt, args);
		va_end(args);
		std::fprintf(fp, "\n");
		std::fclose(fp);
	}

}
