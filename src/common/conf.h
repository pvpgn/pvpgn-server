/*
 * Copyright (C) 2004,2005  Dizzy
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

#ifndef __CONF_H_TYPES__
#define __CONF_H_TYPES__

namespace pvpgn
{

	/* a conf entry object with a set and a get method */

	typedef struct {
		const char *name;
		int(*set)(const char *valstr);	/* called with NULL for cleanup */
		const char * (*get)(void);
		int(*setdef)(void);
	} t_conf_entry;

}

#endif /* __CONF_H_TYPES__ */

#ifndef __CONF_H_PROTOS__
#define __CONF_H_PROTOS__

#include <cstdio>
#include <ctime>

namespace pvpgn
{

	/* helpfull utility functions for common conf types like bool, int and str */
	extern int conf_set_bool(unsigned *pbool, const char *valstr, unsigned def);
	extern int conf_set_int(unsigned *pint, const char *valstr, unsigned def);
	extern int conf_set_str(const char **pstr, const char *valstr, const char *def);
	extern int conf_set_timestr(std::time_t* ptime, const char *valstr, std::time_t def);
	extern const char* conf_get_int(unsigned ival);
	extern const char* conf_get_bool(unsigned ival);

	/* loading/unloading functions */
	extern int conf_load_file(std::FILE *fd, t_conf_entry *conftab);
	extern int conf_load_cmdline(int argc, char **argv, t_conf_entry *conftab);
	extern void conf_unload(t_conf_entry *conftab);

}

#endif /* __CONF_H_PROTOS__ */
