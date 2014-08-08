/*
 * Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#include "runprog.h"

#include <cerrno>
#include <cstdlib>
#include <cstring>

#ifdef HAVE_SYS_TYPES_h
# include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#include "compat/stdfileno.h"
#include "common/eventlog.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{


#ifdef DO_SUBPROC
		static pid_t currpid = 0;
#endif


		extern std::FILE * runprog_open(char const * command)
		{
#ifndef DO_SUBPROC
			return NULL; /* always fail */
#else
			int    fds[2];
			std::FILE * pp;

			if (!command)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL command");
				return NULL;
			}

			if (pipe(fds) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not create pipe (pipe: %s)", std::strerror(errno));
				return NULL;
			}

			switch ((currpid = fork()))
			{
			case 0:
				close(fds[0]);

				close(STDINFD);
				close(STDOUTFD);
				close(STDERRFD);
				/* FIXME: we should close all other fds to make sure the program doesn't use them.
				   For now, leave it alone because we would either have to keep track of them all
				   or do a for for (fd=0; fd<BIGNUMBER; fd++) close(fd); loop :( */

				/* assume prog doesn't use stdin */
				if (fds[1] != STDOUTFD)
					dup2(fds[1], STDOUTFD);
				if (fds[1] != STDERRFD)
					dup2(fds[1], STDERRFD);
				if (fds[1] != STDOUTFD && fds[1] != STDERRFD)
					close(fds[1]);

				if (execlp(command, command, (char *)NULL) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not execute \"%s\" (execlp: %s)", command, std::strerror(errno));

				std::exit(127); /* popen exec failure code */

			case -1:
				eventlog(eventlog_level_error, __FUNCTION__, "could not fork (fork: %s)", std::strerror(errno));
				close(fds[0]);
				close(fds[1]);
				return NULL;

			default:
				close(fds[1]);

				if (!(pp = fdopen(fds[0], "r")))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not streamify output (fdopen: %s)", std::strerror(errno));
					close(fds[0]);
					return NULL;
				}
			}

			return pp;
#endif
		}


		extern int runprog_close(std::FILE * pp)
		{
#ifndef DO_SUBPROC
			return -1; /* always fail */
#else
			int   status;
			pid_t pid;

			if (!pp)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL pp");
				return -1;
			}

			if (std::fclose(pp) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not close process (std::fclose: %s)", std::strerror(errno));
				return -1;
			}

			for (;;)
			{
# ifdef HAVE_WAITPID
				pid = waitpid(currpid, &status, 0);
# else
#  ifdef HAVE_WAIT
				pid = wait(&status);
				if (pid != currpid)
					continue;
#  else
				return 0; /* assume program succeeded and hope that SIGCHLD handles the zombie */
#  endif
# endif
				if (pid != -1)
					return status;
				if (errno != EINTR)
					return 0;
			}
#endif
		}

	}

}
