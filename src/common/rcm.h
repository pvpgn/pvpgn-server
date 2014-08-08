/*
 * Copyright (C) 2004  Dizzy
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

/*
 * Reference Change Mechanism
 * an abstract interface to implement safe management of references of objects
 * which do not share any specific dependencies
 */

#ifndef __RCM_H_TYPES__
#define __RCM_H_TYPES__

#include "common/elist.h"

namespace pvpgn
{

	/* reference change mechanism main object
	 * an object which wishes to be referenced using RCM has to include this */
	typedef struct {
		unsigned count;
		t_elist refs;	/* list of registered references */
	} t_rcm;

	/* callback called when the object referenced is moved/deleted */
	typedef int(*t_chref_cb)(void *data, void *newref);

	/* registered reference object
	 * an object which wants to register it's references to an rcm enabled object
	 * will have to include one of this for every rcm enabled referenced object
	 */
	typedef struct {
		t_chref_cb chref;
		void *data;
		t_elist refs_link;
	} t_rcm_regref;

}

#endif /* __RCM_H_TYPES__ */

#ifndef __RCM_H_PROTOS__
#define __RCM_H_PROTOS__

namespace pvpgn
{

	extern void rcm_init(t_rcm *rcm);
	extern void rcm_regref_init(t_rcm_regref *regref, t_chref_cb cb, void *data);
	extern void rcm_get(t_rcm *rcm, t_rcm_regref *regref);
	extern void rcm_put(t_rcm *rcm, t_rcm_regref *regref);
	/* the main function, cycles through the registered references and calls the
	 * registered callback with the new reference */
	extern void rcm_chref(t_rcm *rcm, void *newref);

}

#endif /* __RCM_H_PROTOS__ */
