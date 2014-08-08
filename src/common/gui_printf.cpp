/*
 * Copyright (C) 2004   CreepLord (creeplord@pvpgn.org)
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

#ifdef WIN32_GUI 

#include "common/setup_before.h"
#include "gui_printf.h"

#include <windows.h>
#include <richedit.h>
#include <cstdarg>
#include "common/eventlog.h"
#include "common/setup_after.h"

namespace pvpgn
{

	HWND		ghwndConsole;

	static void guiAddText(const char *str, COLORREF clr)
	{
		int start_lines, text_length, end_lines;
		CHARRANGE cr;
		CHARRANGE ds;
		CHARFORMAT fmt;

		text_length = SendMessage(ghwndConsole, WM_GETTEXTLENGTH, 0, 0);

		if (text_length > 30000)
		{
			ds.cpMin = 0;
			ds.cpMax = text_length - 30000;
			SendMessage(ghwndConsole, EM_EXSETSEL, 0, (LPARAM)&ds);
			SendMessage(ghwndConsole, EM_REPLACESEL, FALSE, 0);
		}

		cr.cpMin = text_length;
		cr.cpMax = text_length;
		SendMessage(ghwndConsole, EM_EXSETSEL, 0, (LPARAM)&cr);

		fmt.cbSize = sizeof(CHARFORMAT);
		fmt.dwMask = CFM_COLOR | CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_ITALIC | CFM_STRIKEOUT | CFM_UNDERLINE;
		fmt.yHeight = 160;
		fmt.dwEffects = 0;
		fmt.crTextColor = clr;
		strcpy(fmt.szFaceName, "Courier New");

		SendMessage(ghwndConsole, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&fmt);
		SendMessage(ghwndConsole, EM_REPLACESEL, FALSE, (LPARAM)str);
	}

	extern int gui_lvprintf(t_eventlog_level l, const char *format, va_list arglist)
	{
		char buff[4096];
		int result;
		COLORREF clr;

		result = vsprintf(buff, format, arglist);

		switch (l)
		{
		case eventlog_level_none:
			clr = RGB(0, 0, 0);
			break;
		case eventlog_level_trace:
			clr = RGB(255, 0, 255);
			break;
		case eventlog_level_debug:
			clr = RGB(0, 0, 255);
			break;
		case eventlog_level_info:
			clr = RGB(0, 0, 0);
			break;
		case eventlog_level_warn:
			clr = RGB(255, 128, 64);
			break;
		case eventlog_level_error:
			clr = RGB(255, 0, 0);
			break;
		case eventlog_level_fatal:
			clr = RGB(255, 0, 0);
			break;
		default:
			clr = RGB(0, 0, 0);
		}
		guiAddText(buff, clr);
		return result;
	}

	extern int gui_lprintf(t_eventlog_level l, const char *format, ...)
	{
		va_list arglist;
		va_start(arglist, format);

		return gui_lvprintf(l, format, arglist);
	}

	extern int gui_printf(const char *format, ...)
	{
		va_list arglist;
		va_start(arglist, format);
		return gui_lvprintf(eventlog_level_error, format, arglist);
	}

}
#endif
