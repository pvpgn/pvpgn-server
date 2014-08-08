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
#include "friends.h"

#include "compat/strcasecmp.h"
#include "common/list.h"
#include "common/eventlog.h"

#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		extern t_account * friend_get_account(t_friend * fr)
		{
			if (fr == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return NULL;
			}
			return fr->friendacc;
		}

		extern int friend_set_account(t_friend * fr, t_account * acc)
		{
			if (fr == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}
			fr->friendacc = acc;
			return 0;
		}

		extern char friend_get_mutual(t_friend * fr)
		{
			if (fr == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return 0;
			}
			return fr->mutual;
		}

		extern int friend_set_mutual(t_friend * fr, char mutual)
		{
			if (fr == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL account");
				return -1;
			}
			fr->mutual = mutual;
			return 0;
		}

		extern int friendlist_unload(t_list * flist)
		{
			t_elem  * curr;
			t_friend * fr;
			if (flist == NULL)
				return -1;
			LIST_TRAVERSE(flist, curr)
			{
				if (!(fr = (t_friend*)elem_get_data(curr)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
					continue;
				}
				fr->mutual = -1;
			}
			return 0;
		}

		extern int friendlist_close(t_list * flist)
		{
			t_elem * curr;
			t_friend * fr;

			if (flist == NULL)
				return -1;
			LIST_TRAVERSE(flist, curr)
			{
				if (!(fr = (t_friend*)elem_get_data(curr)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
					continue;
				}

				if (list_remove_elem(flist, &curr) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not remove elem from flist");
				xfree((void *)fr);
			}
			list_destroy(flist);
			return 0;
		}

		extern int friendlist_purge(t_list * flist)
		{
			t_elem  * curr;
			t_friend * fr;

			if (flist == NULL)
				return -1;
			LIST_TRAVERSE(flist, curr)
			{
				if (!(fr = (t_friend*)elem_get_data(curr)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
					continue;
				}
				if (fr->mutual < 0)
				{
					if (list_remove_elem(flist, &curr) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "could not remove item from list");
				}
			}
			return 0;
		}

		extern int friendlist_add_account(t_list * flist, t_account * acc, int mutual)
		{
			t_friend * fr;

			if (flist == NULL)
				return -1;

			fr = (t_friend*)xmalloc(sizeof(t_friend));
			fr->friendacc = acc;
			fr->mutual = mutual;
			list_append_data(flist, fr);
			return 0;
		}

		extern int friendlist_remove_friend(t_list * flist, t_friend * fr)
		{
			t_elem * elem;

			if (flist == NULL)
				return -1;

			if (fr != NULL)
			{
				if (list_remove_data(flist, fr, &elem) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not remove item from list");
					return -1;
				}

				xfree((void *)fr);
				return 0;
			}
			return -1;
		}

		extern int friendlist_remove_account(t_list * flist, t_account * acc)
		{
			t_elem * elem;
			t_friend * fr;

			if (flist == NULL)
				return -1;

			fr = friendlist_find_account(flist, acc);
			if (fr != NULL)
			{
				if (list_remove_data(flist, fr, &elem) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not remove item from list");
					return -1;
				}

				xfree((void *)fr);
				return 0;
			}
			return -1;
		}

		extern int friendlist_remove_username(t_list * flist, const char * accname)
		{
			t_elem * elem;
			t_friend * fr;

			if (flist == NULL)
				return -1;

			fr = friendlist_find_username(flist, accname);
			if (fr != NULL)
			{
				if (list_remove_data(flist, fr, &elem) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not remove item from list");
					return -1;
				}

				xfree((void *)fr);
				return 0;
			}
			return -1;
		}

		extern t_friend * friendlist_find_account(t_list * flist, t_account * acc)
		{
			t_elem  * curr;
			t_friend * fr;

			if (flist == NULL)
				return NULL;

			LIST_TRAVERSE(flist, curr)
			{
				if (!(fr = (t_friend*)elem_get_data(curr)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
					continue;
				}
				if (fr->friendacc == acc)
					return fr;
			}
			return NULL;
		}

		extern t_friend * friendlist_find_username(t_list * flist, const char * accname)
		{
			t_elem  * curr;
			t_friend * fr;

			if (flist == NULL)
				return NULL;

			LIST_TRAVERSE(flist, curr)
			{
				if (!(fr = (t_friend*)elem_get_data(curr)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
					continue;
				}
				if (strcasecmp(account_get_name(fr->friendacc), accname) == 0) return fr;
			}
			return NULL;
		}

		extern t_friend * friendlist_find_uid(t_list * flist, unsigned uid)
		{
			t_elem  * curr;
			t_friend * fr;

			if (flist == NULL)
				return NULL;

			LIST_TRAVERSE(flist, curr)
			{
				if (!(fr = (t_friend*)elem_get_data(curr)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
					continue;
				}
				if (account_get_uid(fr->friendacc) == uid) return fr;
			}
			return NULL;
		}

	}

}
