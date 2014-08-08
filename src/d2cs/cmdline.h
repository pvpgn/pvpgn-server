/*
 * Copyright (C) 2005           Dizzy
 * Copyright (C) 2005           Olaf Freyer (aaron@cs.tu-berlin.de)
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

#ifndef __D2CS_CMDLINE_H_PROTOS__
#define __D2CS_CMDLINE_H_PROTOS__
namespace pvpgn
{

namespace d2cs
{


extern int cmdline_load(int argc, char * * argv);
extern void cmdline_unload(void);

/* options exported by cmdline */
#ifdef DO_DAEMONIZE
extern int cmdline_get_foreground(void);
#endif
extern const char* cmdline_get_preffile(void);
extern const char* cmdline_get_logfile(void);
#ifdef WIN32_GUI
extern unsigned cmdline_get_console(void);
extern unsigned cmdline_get_gui(void);
#endif

}

}

#endif /* __D2CS_CMDLINE_H_PROTOS__ */
