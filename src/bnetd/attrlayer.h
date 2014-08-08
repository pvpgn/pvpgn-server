/*
 * Copyright (C) 2004 Dizzy
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

#ifndef __ATTRLAYER_H_INCLUDED__
#define __ATTRLAYER_H_INCLUDED__

#include "common/elist.h"
#include "attrgroup.h"

/* flags controlling flush/save operations */
#define	FS_NONE		0
#define FS_FORCE	1	/* force save/flush no matter of time */
#define FS_ALL		2	/* save/flush all, not in steps */

namespace pvpgn
{

	namespace bnetd
	{

		extern int attrlayer_init(void);
		extern int attrlayer_cleanup(void);
		extern int attrlayer_load_default(void);
		extern int attrlayer_save(int flags);
		extern int attrlayer_flush(int flags);
		extern t_attrgroup * attrlayer_get_defattrgroup(void);
		extern void attrlayer_add_loadedlist(t_elist *what);
		extern void attrlayer_del_loadedlist(t_elist *what);
		extern void attrlayer_add_dirtylist(t_elist *what);
		extern void attrlayer_del_dirtylist(t_elist *what);
		extern void attrlayer_accessed(t_attrgroup* attrgroup);

	}

}

#endif /* __ATTRLAYER_H_INCLUDED__ */
