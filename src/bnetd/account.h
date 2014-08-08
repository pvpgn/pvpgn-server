/*
 * Copyright (C) 1998,1999  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2000,2001  Marco Ziech (mmz@gmx.net)
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
#ifndef INCLUDED_ACCOUNT_TYPES
#define INCLUDED_ACCOUNT_TYPES

#ifndef JUST_NEED_TYPES
#define JUST_NEED_TYPES
#include "common/list.h"
#include "common/elist.h"
#include "team.h"
#include "attrgroup.h"
#undef JUST_NEED_TYPES
#else
#include "common/list.h"
#include "common/elist.h"
#include "team.h"
#include "attrgroup.h"
#endif

#define ACCOUNT_FLAG_NONE	0
#define ACCOUNT_FLAG_FLOADED	1	/* friends list loaded */

namespace pvpgn
{

	namespace bnetd
	{

		struct connection;
		struct _clanmember;
		struct clan;

		typedef struct account_struct
#ifdef ACCOUNT_INTERNAL_ACCESS
		{
			t_attrgroup	* attrgroup;
			char	* name;     /* profiling proved 99% of getstrattr its from get_name */
			unsigned int  namehash; /* cached from attrs */
			unsigned int  uid;      /* cached from attrs */
			unsigned int  flags;
			struct connection * conn;
			struct _clanmember   * clanmember;
			t_list * friends;
			t_list * teams;
		}
#endif
		t_account;

	}

}

#endif

/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_ACCOUNT_PROTOS
#define INCLUDED_ACCOUNT_PROTOS

#define JUST_NEED_TYPES
#include "common/hashtable.h"
#include "connection.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern unsigned int maxuserid;

		extern int accountlist_reload(void);
		extern int account_check_name(char const * name);
#define account_get_uid(A) account_get_uid_real(A,__FILE__,__LINE__)
		extern unsigned int account_get_uid_real(t_account const * account, char const * fn, unsigned int ln);
		extern int account_match(t_account * account, char const * username);
		extern int account_save(t_account *account, unsigned flags);
		extern char const * account_get_strattr_real(t_account * account, char const * key, char const * fn, unsigned int ln);
#define account_get_strattr(A,K) account_get_strattr_real(A,K,__FILE__,__LINE__)
		extern int account_set_strattr(t_account * account, char const * key, char const * val);

		extern int accountlist_create(void);
		extern int accountlist_destroy(void);
		extern t_hashtable * accountlist(void);
		extern t_hashtable * accountlist_uid(void);
		extern int accountlist_load_all(int flag);
		extern unsigned int accountlist_get_length(void);
		extern int accountlist_save(unsigned flags);
		extern int accountlist_flush(unsigned flags);
		extern t_account * accountlist_find_account(char const * username);
		extern t_account * accountlist_find_account_by_uid(unsigned int uid);
		extern char const *accountlist_find_vague_account(t_account * account, char const *vague_username);
		extern int accountlist_allow_add(void);
		extern t_account * accountlist_create_account(const char *username, const char *passhash1);
		extern void accounts_get_attr(char const *);
		/* names and passwords */
		extern char const * account_get_name_real(t_account * account, char const * fn, unsigned int ln);
# define account_get_name(A) account_get_name_real(A,__FILE__,__LINE__)


		extern int account_check_mutual(t_account * account, int myuserid);
		extern t_list * account_get_friends(t_account * account);

		extern int account_set_clanmember(t_account * account, _clanmember * clanmember);
		extern _clanmember * account_get_clanmember(t_account * account);
		extern _clanmember * account_get_clanmember_forced(t_account * account);
		extern clan * account_get_clan(t_account * account);
		extern clan * account_get_creating_clan(t_account * account);

		extern int account_set_conn(t_account * account, t_connection * conn);
		extern t_connection * account_get_conn(t_account * account);

		extern void account_add_team(t_account * account, t_team * team);
		extern t_team * account_find_team_by_accounts(t_account * account, t_account **accounts, t_clienttag clienttag);
		extern t_team * account_find_team_by_teamid(t_account * account, unsigned int teamid);
		extern t_list * account_get_teams(t_account * account);

	}

}

#endif
#endif
