/*
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
#include "setup.h"
#include "pgsid.h"

#include "common/xalloc.h"

#include "common/setup_after.h"


namespace pvpgn
{

	namespace d2dbs
	{

		static int dbs_packet_gs_id = 0;
		static t_preset_d2gsid* preset_d2gsid_head = nullptr;


		unsigned int pgsid_get_id(unsigned int ipaddr)
		{
			t_preset_d2gsid* pgsid;

			pgsid = preset_d2gsid_head;
			while (pgsid)
			{
				if (pgsid->ipaddr == ipaddr)
					return pgsid->d2gsid;
				pgsid = pgsid->next;
			}

			// not found, build a new item
			pgsid = (t_preset_d2gsid*)xmalloc(sizeof(t_preset_d2gsid));
			pgsid->ipaddr = ipaddr;
			pgsid->d2gsid = ++dbs_packet_gs_id;

			// add to list
			pgsid->next = preset_d2gsid_head;
			preset_d2gsid_head = pgsid;
			return preset_d2gsid_head->d2gsid;
		}

		void pgsid_destroy()
		{
			if (preset_d2gsid_head)
			{
				t_preset_d2gsid* curr;
				t_preset_d2gsid* next;

				for (curr = preset_d2gsid_head; curr; curr = next)
				{
					next = curr->next;
					xfree(curr);
				}
			}
		}

	}

}