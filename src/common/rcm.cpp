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

#include "common/setup_before.h"
#include "common/rcm.h"

#include <cassert>

#include "common/xalloc.h"
#include "common/elist.h"
#include "common/setup_after.h"

namespace pvpgn
{

	extern void rcm_init(t_rcm *rcm)
	{
		assert(rcm);

		elist_init(&rcm->refs);
		rcm->count = 0;
	}

	extern void rcm_regref_init(t_rcm_regref *regref, t_chref_cb cb, void *data)
	{
		assert(regref);

		elist_init(&regref->refs_link);
		regref->chref = cb;
		regref->data = data;
	}

	extern void rcm_get(t_rcm *rcm, t_rcm_regref *regref)
	{
		assert(rcm);
		assert(regref);

		rcm->count++;
		elist_add_tail(&rcm->refs, &regref->refs_link);
	}

	extern void rcm_put(t_rcm *rcm, t_rcm_regref *regref)
	{
		assert(rcm);
		assert(regref);
		assert(rcm->count);	/* might use eventlog but I want this stopped fast */

		rcm->count--;
		elist_del(&regref->refs_link);
	}

	extern void rcm_chref(t_rcm *rcm, void *newref)
	{
		t_elist *curr, *save;
		t_rcm_regref *regref;

		assert(rcm);

		elist_for_each_safe(curr, &rcm->refs, save) {
			regref = elist_entry(curr, t_rcm_regref, refs_link);
			if (regref->chref) regref->chref(regref->data, newref);
		}
	}

}
