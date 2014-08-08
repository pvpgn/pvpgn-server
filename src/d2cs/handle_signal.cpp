/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
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
#include "handle_signal.h"

#include <ctime>
#include <cstring>
#include <csignal>

#include "common/eventlog.h"
#include "common/trans.h"
#include "common/xalloc.h"
#include "prefs.h"
#include "cmdline.h"
#include "d2gs.h"
#include "d2ladder.h"
#include "common/setup_after.h"

namespace pvpgn
{

namespace d2cs
{

static void on_signal(int s);

static volatile struct
{
	unsigned char	do_quit;
	unsigned char	cancel_quit;
	unsigned char	reload_config;
	unsigned char	reload_ladder;
	unsigned char	restart_d2gs;
	time_t			exit_time;
} signal_data ={ 0, 0, 0, 0, 0, 0 };

extern int handle_signal(void)
{
	std::time_t		now;
    char const * levels;
    char *       temp;
    char const * tok;


	if (signal_data.cancel_quit) {
		signal_data.cancel_quit=0;
		if (!signal_data.exit_time) {
			eventlog(eventlog_level_info,__FUNCTION__,"there is no previous shutdown to be canceled");
		} else {
			signal_data.exit_time=0;
			eventlog(eventlog_level_info,__FUNCTION__,"shutdown was canceled due to std::signal");
		}
	}
	if (signal_data.do_quit) {
		signal_data.do_quit=0;
		now=std::time(NULL);
		if (!signal_data.exit_time) {
			signal_data.exit_time=now+d2cs_prefs_get_shutdown_delay();
		} else {
			signal_data.exit_time-=d2cs_prefs_get_shutdown_decr();
		}
		eventlog(eventlog_level_info,__FUNCTION__,"the server is going to shutdown in %lu minutes",(signal_data.exit_time-now)/60);
	}
	if (signal_data.exit_time) {
		now=std::time(NULL);
		if (now >= (signed)signal_data.exit_time) {
			signal_data.exit_time=0;
			eventlog(eventlog_level_info,__FUNCTION__,"shutdown server due to std::signal");
			return -1;
		}
	}
	if (signal_data.reload_config) {
		signal_data.reload_config=0;
		eventlog(eventlog_level_info,__FUNCTION__,"reloading configuartion file due to std::signal");
		if (prefs_reload(cmdline_get_preffile())<0) {
			eventlog(eventlog_level_error,__FUNCTION__,"error reload configuration file,exitting");
			return -1;
		}
		if (d2gslist_reload(prefs_get_d2gs_list())<0) {
			eventlog(eventlog_level_error,__FUNCTION__,"error reloading game server list,exitting");
			return -1;
		}
		if (trans_reload(d2cs_prefs_get_transfile(),TRANS_D2CS)<0) {
	    		eventlog(eventlog_level_error,__FUNCTION__,"could not reload trans list");
		}

        eventlog_clear_level();
        if ((levels = d2cs_prefs_get_loglevels()))
        {
            temp = xstrdup(levels);
            tok = std::strtok(temp,","); /* std::strtok modifies the string it is passed */

            while (tok)
            {
              if (eventlog_add_level(tok)<0)
              eventlog(eventlog_level_error,__FUNCTION__,"could not add std::log level \"%s\"",tok);
              tok = std::strtok(NULL,",");
            }

            xfree(temp);
        }
#ifdef DO_DAEMONIZE
		if (!cmdline_get_foreground())
#endif
			eventlog_open(d2cs_prefs_get_logfile());
	}
	if (signal_data.reload_ladder) {
		signal_data.reload_ladder=0;
		eventlog(eventlog_level_info,__FUNCTION__,"reloading ladder data due to std::signal");
		d2ladder_refresh();
	}

	if (signal_data.restart_d2gs) {
		signal_data.restart_d2gs=0;
		eventlog(eventlog_level_info,__FUNCTION__,"restarting all game servers due to std::signal");
		d2gs_restart_all_gs();
	}

	return 0;
}
#ifdef WIN32
extern void signal_quit_wrapper(void)
{
  signal_data.do_quit=1;
}

extern void signal_reload_config_wrapper(void)
{
    signal_data.reload_config = 1;
}

extern void signal_load_ladder_wrapper(void)
{
    signal_data.reload_ladder = 1;
}

extern void signal_exit_wrapper(void)
{
    signal_data.exit_time = 1;
}

extern void signal_restart_d2gs_wrapper(void)
{
    signal_data.restart_d2gs = 1;
}
#else
extern int handle_signal_init(void)
{
	std::signal(SIGINT,on_signal);
	std::signal(SIGTERM,on_signal);
	std::signal(SIGABRT,on_signal);
	std::signal(SIGHUP,on_signal);
	std::signal(SIGUSR1,on_signal);
	std::signal(SIGUSR2,on_signal);
	std::signal(SIGPIPE,on_signal);
	return 0;
}

static void on_signal(int s)
{
	switch (s) {
		case SIGINT:
			eventlog(eventlog_level_debug,__FUNCTION__,"sigint received");
			signal_data.do_quit=1;
			break;
		case SIGTERM:
			eventlog(eventlog_level_debug,__FUNCTION__,"sigint received");
			signal_data.do_quit=1;
			break;
		case SIGABRT:
			eventlog(eventlog_level_debug,__FUNCTION__,"sigabrt received");
			signal_data.cancel_quit=1;
			break;
		case SIGHUP:
			eventlog(eventlog_level_debug,__FUNCTION__,"sighup received");
			signal_data.reload_config=1;
			break;
		case SIGUSR1:
			eventlog(eventlog_level_debug,__FUNCTION__,"sigusr1 received");
			signal_data.reload_ladder=1;
			break;
		case SIGUSR2:
			eventlog(eventlog_level_debug,__FUNCTION__,"sigusr2 received");
			signal_data.restart_d2gs=1;
			break;
		case SIGPIPE:
			eventlog(eventlog_level_debug,__FUNCTION__,"sigpipe received");
			break;
	}
	std::signal(s,on_signal);
}
#endif

}

}
