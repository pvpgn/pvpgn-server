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

#ifndef __GUI_PRINTF_H__
#define __GUI_PRINTF_H__

#ifdef WIN32_GUI 

#include <cstdarg>
#include "common/eventlog.h"

namespace pvpgn
{

	extern int gui_lvprintf(t_eventlog_level l, const char *format, va_list arglist);
	extern int gui_lprintf(t_eventlog_level l, const char *format, ...);
	extern int gui_printf(const char *format, ...);

}
#endif

#endif
