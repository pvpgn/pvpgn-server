/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
 * Copyright (C) 2001		sousou	(liupeng.cs@263.net)
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
#ifndef INCLUDED_PREFS_H
#define INCLUDED_PREFS_H

namespace pvpgn
{

	namespace d2dbs
	{

		extern int d2dbs_prefs_load(char const * filename);
		extern int d2dbs_prefs_reload(char const * filename);
		extern int d2dbs_prefs_unload(void);

		extern char const * d2dbs_prefs_get_logfile(void);
		extern char const * prefs_get_logfile_gs(void);
		extern char const * d2dbs_prefs_get_servaddrs(void);
		extern char const * d2dbs_prefs_get_charsave_dir(void);
		extern char const * d2dbs_prefs_get_charinfo_dir(void);
		extern char const * prefs_get_charsave_bak_dir(void);
		extern char const * prefs_get_charinfo_bak_dir(void);
		extern char const * d2dbs_prefs_get_ladder_dir(void);
		extern char const * d2dbs_prefs_get_d2gs_list(void);
		extern unsigned int prefs_get_laddersave_interval(void);
		extern unsigned int prefs_get_ladderinit_time(void);
		extern char const * d2dbs_prefs_get_loglevels(void);
		extern unsigned int d2dbs_prefs_get_shutdown_delay(void);
		extern unsigned int d2dbs_prefs_get_shutdown_decr(void);
		extern unsigned int d2dbs_prefs_get_idletime(void);
		extern unsigned int prefs_get_keepalive_interval(void);
		extern unsigned int d2dbs_prefs_get_timeout_checkinterval(void);
		extern unsigned int d2dbs_prefs_get_XML_output_ladder(void);
		extern unsigned int prefs_get_ladderupdate_threshold(void);
		extern unsigned int prefs_get_ladder_chars_only(void);
		extern unsigned int prefs_get_difficulty_hack(void);
		extern char const * d2dbs_prefs_get_pidfile(void);

	}

}

#endif
