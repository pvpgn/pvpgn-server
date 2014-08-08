/*
 * Copyright (C) 2001		faster	(lqx@cic.tsinghua.edu.cn)
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
#include "setup.h"
#include "charlock.h"

#include <limits>
#include <cstring>
#include <cctype>

#include "compat/strcasecmp.h"
#include "common/xalloc.h"
#include "common/introtate.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace d2dbs
	{

		/* FIXME: for simplification, no multiple realm support now */

		/* local functions */
		static int cl_insert_to_gsq_list(unsigned int gsid, t_charlockinfo *pcl);
		static int cl_delete_from_gsq_list(t_charlockinfo *pcl);
		static unsigned int string_hash(char const *string);


		/* variables */
		static unsigned int		clitbl_len = 0;
		static unsigned int		gsqtbl_len = 0;
		static t_charlockinfo		* * clitbl = NULL;
		static t_charlockinfo		* * gsqtbl = NULL;


		int cl_init(unsigned int tbllen, unsigned int maxgs)
		{
			if (!tbllen || !maxgs) return -1;
			cl_destroy();

			clitbl = (t_charlockinfo**)xmalloc(tbllen*sizeof(t_charlockinfo**));
			gsqtbl = (t_charlockinfo**)xmalloc(maxgs*sizeof(t_charlockinfo**));
			std::memset(clitbl, 0, tbllen*sizeof(t_charlockinfo**));
			std::memset(gsqtbl, 0, maxgs*sizeof(t_charlockinfo**));
			clitbl_len = tbllen;
			gsqtbl_len = maxgs;
			return 0;
		}


		int cl_destroy(void)
		{
			unsigned int	i;
			t_charlockinfo	* ptl, *ptmp;

			if (clitbl) {
				for (i = 0; i < clitbl_len; i++) {
					ptl = clitbl[i];
					while (ptl) {
						ptmp = ptl;
						ptl = ptl->next;
						xfree(ptmp);
					}
				}
				xfree(clitbl);
			}
			if (gsqtbl) xfree(gsqtbl);
			clitbl = gsqtbl = NULL;
			clitbl_len = gsqtbl_len = 0;
			return 0;
		}


		int cl_query_charlock_status(unsigned char *charname,
			unsigned char *realmname, unsigned int *gsid)
		{
			t_charlockinfo	*pcl;
			unsigned int	hashval;

			if (!charname || !realmname) return -1;
			if (!clitbl_len || !gsqtbl) return -1;
			if (std::strlen((char*)charname) >= MAX_CHARNAME_LEN) return -1;

			hashval = string_hash((char*)charname) % clitbl_len;
			pcl = clitbl[hashval];
			while (pcl)
			{
				if (strcasecmp((char*)pcl->charname, (char*)charname) == 0) {
					*gsid = pcl->gsid;
					return 1;	/* found the char, it is locked */
				}
				pcl = pcl->next;
			}
			return 0;	/* not found, it is unlocked */
		}


		int cl_lock_char(unsigned char *charname,
			unsigned char *realmname, unsigned int gsid)
		{
			t_charlockinfo	*pcl, *ptmp;
			unsigned int	hashval;

			if (!charname || !realmname) return -1;
			if (!clitbl_len || !gsqtbl) return -1;
			if (std::strlen((char*)charname) >= MAX_CHARNAME_LEN) return -1;

			hashval = string_hash((char*)charname) % clitbl_len;
			pcl = clitbl[hashval];
			ptmp = NULL;
			while (pcl)
			{
				if (strcasecmp((char*)pcl->charname, (char*)charname) == 0)
					return 0;	/* found the char is already locked */
				ptmp = pcl;
				pcl = pcl->next;
			}

			/* not found, locked it */
			pcl = (t_charlockinfo*)xmalloc(sizeof(t_charlockinfo));
			std::memset(pcl, 0, sizeof(t_charlockinfo));
			std::strncpy((char*)pcl->charname, (char*)charname, MAX_CHARNAME_LEN - 1);
			std::strncpy((char*)pcl->realmname, (char*)realmname, MAX_REALMNAME_LEN - 1);
			pcl->gsid = gsid;

			/* add to hash table link list */
			if (ptmp) ptmp->next = pcl;
			else clitbl[hashval] = pcl;
			/* add to gs specified link list */
			cl_insert_to_gsq_list(gsid, pcl);

			return 0;
		}


		int cl_unlock_char(unsigned char *charname, unsigned char *realmname, unsigned int gsid)
		{
			t_charlockinfo	*pcl, *ptmp;
			unsigned int	hashval;

			if (!charname || !realmname) return -1;
			if (!clitbl_len || !gsqtbl) return 0;
			if (std::strlen((char*)charname) >= MAX_CHARNAME_LEN) return -1;

			hashval = string_hash((char*)charname) % clitbl_len;
			pcl = clitbl[hashval];
			ptmp = NULL;
			while (pcl)
			{
				if ((strcasecmp((char*)pcl->charname, (char*)charname) == 0) && (pcl->gsid == gsid)) {
					cl_delete_from_gsq_list(pcl);
					if (ptmp) ptmp->next = pcl->next;
					else clitbl[hashval] = pcl->next;
					xfree(pcl);
					return 0;
				}
				ptmp = pcl;
				pcl = pcl->next;
			}
			return 0;
		}


		int cl_unlock_all_char_by_gsid(unsigned int gsid)
		{
			unsigned int	index_pos;
			t_charlockinfo	*pcl, *pnext;

			index_pos = gsid % gsqtbl_len;
			pcl = gsqtbl[index_pos];
			while (pcl)
			{
				pnext = pcl->gsqnext;
				cl_unlock_char(pcl->charname, pcl->realmname, gsid);
				pcl = pnext;
			}
			return 0;
		}


		static int cl_insert_to_gsq_list(unsigned int gsid, t_charlockinfo *pcl)
		{
			unsigned int	index_pos;
			t_charlockinfo	*ptmp;

			if (!pcl) return -1;

			index_pos = gsid % gsqtbl_len;
			ptmp = gsqtbl[index_pos];
			gsqtbl[index_pos] = pcl;
			if (ptmp) {
				pcl->gsqnext = ptmp;
				ptmp->gsqprev = pcl;
			}
			return 0;
		}


		static int cl_delete_from_gsq_list(t_charlockinfo *pcl)
		{
			unsigned int	index_pos;
			t_charlockinfo	*pprev, *pnext;

			if (!pcl) return -1;

			index_pos = (pcl->gsid) % gsqtbl_len;
			pprev = pcl->gsqprev;
			pnext = pcl->gsqnext;
			if (pprev) pprev->gsqnext = pnext;
			else gsqtbl[index_pos] = pnext;
			if (pnext) pnext->gsqprev = pprev;

			return 0;
		}


		static unsigned int string_hash(char const *string)
		{
			unsigned int	i;
			unsigned int	pos;
			unsigned int	hash;
			unsigned int	ch;

			if (!string) return 0;

			for (hash = 0, pos = 0, i = 0; i < std::strlen(string); i++)
			{
				if (isascii((int)string[i]))
					ch = (unsigned int)(unsigned char)std::tolower((int)string[i]);
				else
					ch = (unsigned int)(unsigned char)string[i];
				hash ^= ROTL(ch, pos, (std::numeric_limits<unsigned int>::digits));
				pos += (std::numeric_limits<unsigned char>::digits) - 1;
			}

			return hash;
		}

	}

}
