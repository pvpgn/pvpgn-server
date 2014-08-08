/*
 * Copyright (C) 2001  Hakan Tandogan (hakan@gurkensalat.com)
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
#include "common/give_up_root_privileges.h"
#include <cerrno>
#include <cstring>
#include <cstdlib>

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_PWD_H
# include <pwd.h>
#endif
#ifdef HAVE_GRP_H
# include <grp.h>
#endif

#include "common/eventlog.h"
#include "common/setup_after.h"


#define ILLEGAL_ID -1


namespace pvpgn
{

	static int gurp_gname2id(const char *name);
	static int gurp_uname2id(const char *name);


	extern int give_up_root_privileges(char const * user_name, char const * group_name)
	{
		int user_id = 0;
		int group_id = 0;

		eventlog(eventlog_level_debug, __FUNCTION__, "about to give up root privileges");

		if (user_name)
		{
			if ((user_id = gurp_uname2id(user_name)) == ILLEGAL_ID)
			{
				return -1;
			}
			else
			{
				eventlog(eventlog_level_debug, __FUNCTION__, "should change to user = '%s' (%d)", user_name, user_id);
			}
		}
		if (group_name)
		{
			if ((group_id = gurp_gname2id(group_name)) == ILLEGAL_ID)
			{
				return -1;
			}
			else
			{
				eventlog(eventlog_level_debug, __FUNCTION__, "should change to group = '%s' (%d)", group_name, group_id);
			}
		}

		/*  Change first the group ID, later we might not be able to anymore
		 *  We can use setgid safely because we don't want to return to root
		 *  privileges anymore
		 */

#ifdef HAVE_SETGID
		if (group_name)
		{
			if (-1 == setgid(group_id))
			{
				eventlog(eventlog_level_fatal, __FUNCTION__, "could not set gid to %d (setgid: %s)", group_id, std::strerror(errno));
				return -1;
			}
# ifdef HAVE_GETUID
			eventlog(eventlog_level_info, __FUNCTION__, "Changed privileges to gid = %d", getgid());
# endif
		}
#endif

#ifdef HAVE_SETUID
		if (user_name)
		{
			if (-1 == setuid(user_id))
			{
				eventlog(eventlog_level_fatal, __FUNCTION__, "could not set uid to %d (setuid: %s)", user_id, std::strerror(errno));
				return -1;
			}
# ifdef HAVE_GETGID
			eventlog(eventlog_level_info, __FUNCTION__, "Changed privileges to uid = %d", getuid());
# endif
		}
#endif

		return 0;
	}


	static int gurp_uname2id(const char *name)
	{
		int id = ILLEGAL_ID;

		if (name != NULL)
		{
			if (name[0] == '#')
			{
				id = std::atoi(&name[1]);
			}
			else
			{
#ifdef HAVE_GETPWNAM
				struct passwd * ent;

				eventlog(eventlog_level_debug, __FUNCTION__, "about to getpwnam(%s)", name);

				if (!(ent = getpwnam(name)))
				{
					eventlog(eventlog_level_fatal, __FUNCTION__, "cannot get password file entry for '%s' (getpwnam: %s)", name, std::strerror(errno));
					return id;
				}
				id = ent->pw_uid;
#else
				return id;
#endif
			}
		}

		return id;
	}


	static int gurp_gname2id(const char *name)
	{
		int id = ILLEGAL_ID;

		if (name != NULL)
		{
			if (name[0] == '#')
			{
				id = std::atoi(&name[1]);
			}
			else
			{
#ifdef HAVE_GETGRNAM
				struct group * ent;

				eventlog(eventlog_level_debug, __FUNCTION__, "about to getgrnam(%s)", name);

				if (!(ent = getgrnam(name)))
				{
					eventlog(eventlog_level_fatal, __FUNCTION__, "cannot get group file entry for '%s' (getgrnam: %s)", name, std::strerror(errno));
					return id;
				}
				id = ent->gr_gid;
#else
				return id;
#endif
			}
		}

		return id;
	}

}
