/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
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
#ifndef INCLUDED_HANDLE_SIGNAL_H
#define INCLUDED_HANDLE_SIGNAL_H

namespace pvpgn
{

	namespace d2cs
	{

#ifndef WIN32
		extern int handle_signal_init(void);
#else
		extern void signal_quit_wrapper(void);
		extern void signal_reload_config_wrapper(void);
		extern void signal_load_ladder_wrapper(void);
		extern void signal_exit_wrapper(void);
		extern void signal_restart_d2gs_wrapper(void);
#endif

		extern int handle_signal(void);

	}

}

#endif
