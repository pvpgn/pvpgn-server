/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2005 Dizzy
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

#include <exception>
#include <iostream>
#include <stdexcept>

#include <cerrno>
#include <cstring>

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef WIN32
# include "win32/service.h"
#endif
#ifdef WIN32_GUI
# include "common/gui_printf.h"
#endif

#include "compat/stdfileno.h"
#include "compat/psock.h"
#include "compat/uname.h"
#include "compat/pgetpid.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/fdwatch.h"
#include "common/trans.h"
#include "common/give_up_root_privileges.h"
#include "common/version.h"
#include "common/hexdump.h"

#include "server.h"
#include "prefs.h"
#include "cmdline.h"
#include "storage.h"
#include "support.h"
#include "anongame_maplists.h"
#include "anongame.h"
#include "connection.h"
#include "game.h"
#include "timer.h"
#include "channel.h"
#include "helpfile.h"
#include "ipban.h"
#include "adbanner.h"
#include "autoupdate.h"
#include "versioncheck.h"
#include "news.h"
#include "watch.h"
#include "output.h"
#include "attrlayer.h"
#include "account.h"
#include "ladder.h"
#include "character.h"
#include "tracker.h"
#include "command_groups.h"
#include "alias_command.h"
#include "tournament.h"
#include "icons.h"
#include "anongame_infos.h"
#include "anongame_wol.h"
#include "clan.h"
#include "team.h"
#include "realm.h"
#include "topic.h"
#include "handle_apireg.h"
#include "i18n.h"
#include "userlog.h"
#include "account_email_verification.h"
#include "smtp.h"
#ifdef WIN32
#include "win32/windump.h"
#endif
#include "common/setup_after.h"

#ifdef WITH_LUA
#include "luainterface.h"
#endif


/* out of memory safety */
#define OOM_SAFE_MEM	1000000		/* 1 Mbyte of safety memory */

using namespace pvpgn::bnetd;
using namespace pvpgn;

static int * emergency_mem = new int[1048576]; // 1 MiB
void new_oom_handler()
{
	delete[] emergency_mem;
	eventlog(eventlog_level_fatal, __FUNCTION__, "out of memory, forcing immediate shutdown");

	std::abort();
}

// FIXME: Use new instead of malloc everywhere

void *oom_buffer = NULL;

static int bnetd_oom_handler(void)
{
	/* no safety buffer, sorry :( */
	if (!oom_buffer) return 0;

	/* free the safety buffer hoping next allocs will succeed */
	free(oom_buffer);
	oom_buffer = NULL;

	eventlog(eventlog_level_fatal, __FUNCTION__, "out of memory, forcing immediate shutdown");

	/* shutdown immediately */
	server_quit_delay(-1);

	return 1;	/* ask xalloc codes to retry the allocation request */
}

static int oom_setup(void)
{
	/* use calloc so it initilizez the memory so it will make some lazy
	 * allocators really allocate it (ex. the linux kernel)
	 */
	oom_buffer = calloc(1, OOM_SAFE_MEM);
	if (!oom_buffer) return -1;

	xalloc_setcb(bnetd_oom_handler);
	return 0;
}

static void oom_free(void)
{
	free(oom_buffer);
	oom_buffer = NULL;
}

FILE	*hexstrm = NULL;

char serviceLongName[] = PVPGN_SOFTWARE " service";
char serviceName[] = "pvpgn";
char serviceDescription[] = "Player vs. Player Gaming Network - Server";

/*
 * by quetzal. indicates service status
 * -1 - not in service mode
 *  0 - stopped
 *  1 - running
 *  2 - paused
 */
int g_ServiceStatus = -1;

/* added some more std::exit status --> put in "compat/exitstatus.h" ??? */
#define STATUS_OOM_FAILURE		20
#define STATUS_STORAGE_FAILURE		30
#define STATUS_PSOCK_FAILURE		35
#define STATUS_MAPLISTS_FAILURE		40
#define STATUS_MATCHLISTS_FAILURE	50
#define STATUS_LADDERLIST_FAILURE	60
#define STATUS_WAR3XPTABLES_FAILURE	70
#define STATUS_SUPPORT_FAILURE          80
#define STATUS_FDWATCH_FAILURE          90

int pre_server_startup(void);
void post_server_shutdown(int status);
int eventlog_startup(void);
int fork_bnetd(int foreground);
char * write_to_pidfile(void);
void pvpgn_greeting();

int eventlog_startup(void)
{
	char const * levels;
	char *       temp;
	char const * tok;

	eventlog_clear_level();
	if ((levels = prefs_get_loglevels())) {
		temp = xstrdup(levels);
		tok = std::strtok(temp, ","); /* std::strtok modifies the string it is passed */
		while (tok) {
			if (eventlog_add_level(tok) < 0)
				eventlog(eventlog_level_error, __FUNCTION__, "could not add std::log level \"{}\"", tok);
			tok = std::strtok(NULL, ",");
		}
		xfree(temp);
	}

#ifdef WIN32_GUI
	if (cmdline_get_gui()){
		eventlog_add_level(eventlog_get_levelname_str(eventlog_level_gui));
	}
#endif

	if (eventlog_open(prefs_get_logfile()) < 0) {
		if (prefs_get_logfile()) {
			eventlog(eventlog_level_fatal, __FUNCTION__, "could not use file \"{}\" for the eventlog (exiting)", prefs_get_logfile());
		}
		else {
			eventlog(eventlog_level_fatal, __FUNCTION__, "no logfile specified in configuration file \"{}\" (exiting)", cmdline_get_preffile());
		}
		return -1;
	}
	eventlog(eventlog_level_info, __FUNCTION__, "logging event levels: {}", prefs_get_loglevels());
	return 0;
}

int fork_bnetd(int foreground)
{
#ifdef DO_DAEMONIZE
	int		pid;

	if (!foreground) {
		if (chdir("/") < 0) {
			eventlog(eventlog_level_error, __FUNCTION__, "could not change working directory to / (chdir: {})", std::strerror(errno));
			return -1;
		}

		switch ((pid = fork())) {
		case -1:
			eventlog(eventlog_level_error, __FUNCTION__, "could not fork (fork: {})", std::strerror(errno));
			return -1;
		case 0: /* child */
			break;
		default: /* parent */
			return pid;
		}
#ifndef WITH_D2
		close(STDINFD);
		close(STDOUTFD);
		close(STDERRFD);
#endif
# ifdef HAVE_SETPGID
		if (setpgid(0, 0) < 0) {
			eventlog(eventlog_level_error, __FUNCTION__, "could not create new process group (setpgid: {})", std::strerror(errno));
			return -1;
		}
# else
#  ifdef HAVE_SETPGRP
#   ifdef SETPGRP_VOID
		if (setpgrp() < 0) {
			eventlog(eventlog_level_error, __FUNCTION__, "could not create new process group (setpgrp: {})", std::strerror(errno));
			return -1;
		}
#   else
		if (setpgrp(0, 0) < 0) {
			eventlog(eventlog_level_error, __FUNCTION__, "could not create new process group (setpgrp: {})", std::strerror(errno));
			return -1;
		}
#   endif
#  else
#   ifdef HAVE_SETSID
		if (setsid() < 0) {
			eventlog(eventlog_level_error, __FUNCTION__, "could not create new process group (setsid: {})", std::strerror(errno));
			return -1;
		}
#   else
#    error "One of setpgid(), setpgrp(), or setsid() is required"
#   endif
#  endif
# endif
	}
	return 0;
#endif
	return 0;
}

char * write_to_pidfile(void)
{
	char *pidfile = xstrdup(prefs_get_pidfile());

	if (pidfile)
	{
		if (pidfile[0] == '\0') {
			xfree((void *)pidfile); /* avoid warning */
			return NULL;
		}

#ifdef HAVE_GETPID
		std::FILE * fp;

		if (!(fp = std::fopen(pidfile, "w"))) {
			eventlog(eventlog_level_error, __FUNCTION__, "unable to open pid file \"{}\" for writing (std::fopen: {})", pidfile, std::strerror(errno));
			xfree((void *)pidfile); /* avoid warning */
			return NULL;
		}
		else {
			std::fprintf(fp, "%u", (unsigned int)getpid());
			if (std::fclose(fp) < 0)
				eventlog(eventlog_level_error, __FUNCTION__, "could not close pid file \"{}\" after writing (std::fclose: {})", pidfile, std::strerror(errno));
		}
#else
		eventlog(eventlog_level_warn, __FUNCTION__, "no getpid() std::system call, disable pid file in bnetd.conf");
		xfree((void *)pidfile); /* avoid warning */
		return NULL;
#endif
	}
	return pidfile;
}


/* Initialize config files */
int pre_server_startup(void)
{
	pvpgn_greeting();
	if (oom_setup() < 0) {
		eventlog(eventlog_level_error, __FUNCTION__, "OOM init failed");
		return STATUS_OOM_FAILURE;
	}

	std::set_new_handler(new_oom_handler);

	if (storage_init(prefs_get_storage_path()) < 0) {
		eventlog(eventlog_level_error, "pre_server_startup", "storage init failed");
		return STATUS_STORAGE_FAILURE;
	}
	if (psock_init() < 0) {
		eventlog(eventlog_level_error, __FUNCTION__, "could not initialize socket functions");
		return STATUS_PSOCK_FAILURE;
	}
	if (support_check_files(prefs_get_supportfile()) < 0) {
		eventlog(eventlog_level_error, "pre_server_startup", "some needed files are missing");
		eventlog(eventlog_level_error, "pre_server_startup", "please make sure you installed the supportfiles in {}", prefs_get_filedir());
		return STATUS_SUPPORT_FAILURE;
	}
	if (anongame_maplists_create() < 0) {
		eventlog(eventlog_level_error, "pre_server_startup", "could not load maps");
		return STATUS_MAPLISTS_FAILURE;
	}
	if (anongame_matchlists_create() < 0) {
		eventlog(eventlog_level_error, "pre_server_startup", "could not create matchlists");
		return STATUS_MATCHLISTS_FAILURE;
	}
	if (fdwatch_init(prefs_get_max_connections())) {
		eventlog(eventlog_level_error, __FUNCTION__, "error initializing fdwatch");
		return STATUS_FDWATCH_FAILURE;
	}

	i18n_load();

	connlist_create();
	gamelist_create();
	timerlist_create();
	server_set_hostname();
	channellist_create();
	apireglist_create();
	if (helpfile_init(prefs_get_helpfile()) < 0)
		eventlog(eventlog_level_error, __FUNCTION__, "could not load helpfile");
	ipbanlist_create();
	if (ipbanlist_load(prefs_get_ipbanfile()) < 0)
		eventlog(eventlog_level_error, __FUNCTION__, "could not load IP ban list");

	try
	{
		if (AdBannerList.is_loaded())
		{
			AdBannerList.unload();
		}

		AdBannerList.load(prefs_get_adfile());
	}
	catch (const std::exception & e)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "{}", e.what());
	}

	if (autoupdate_load(prefs_get_mpqfile()) < 0)
		eventlog(eventlog_level_error, __FUNCTION__, "could not load autoupdate list");

	if (!load_versioncheck_conf(prefs_get_versioncheck_file()))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "could not load versioncheck list");
	}

	if (news_load(prefs_get_newsfile()) < 0)
		eventlog(eventlog_level_error, __FUNCTION__, "could not load news list");
	watchlist.reset(new WatchComponent());
	output_init();
	attrlayer_init();
	accountlist_create();
	if (ladder_createxptable(prefs_get_xplevel_file(), prefs_get_xpcalc_file()) < 0) {
		eventlog(eventlog_level_error, "pre_server_startup", "could not load WAR3 xp calc tables");
		return STATUS_WAR3XPTABLES_FAILURE;
	}
	ladders.load();
	ladders.update();
	if (characterlist_create("") < 0)
		eventlog(eventlog_level_error, __FUNCTION__, "could not load character list");
	if (prefs_get_track()) /* setup the tracking mechanism */
		tracker_set_servers(prefs_get_trackserv_addrs());
	if (command_groups_load(prefs_get_command_groups_file()) < 0)
		eventlog(eventlog_level_error, __FUNCTION__, "could not load command_groups list");
	aliasfile_load(prefs_get_aliasfile());
	if (trans_load(prefs_get_transfile(), TRANS_BNETD) < 0)
		eventlog(eventlog_level_error, __FUNCTION__, "could not load trans list");
	tournament_init(prefs_get_tournament_file());
	customicons_load(prefs_get_customicons_file());
	anongame_infos_load(prefs_get_anongame_infos_file());
	anongame_wol_matchlist_create();
	clanlist_load();
	teamlist_load();
	if (realmlist_create(prefs_get_realmfile()) < 0)
		eventlog(eventlog_level_error, __FUNCTION__, "could not load realm list");
	load_topic_conf(prefs_get_topicfile());
	userlog_init();
	if (prefs_get_verify_account_email() == 1)
	{
		if (smtp_init(prefs_get_smtp_ca_cert_store_file(), prefs_get_smtp_server_url(), prefs_get_smtp_port(), prefs_get_smtp_username(), prefs_get_smtp_password()))
		{
			eventlog(eventlog_level_info, __FUNCTION__, "Successfully initialized SMTP client");
		}
		else
		{
			eventlog(eventlog_level_error, __FUNCTION__, "Failed to initialize SMTP client");
			eventlog(eventlog_level_error, __FUNCTION__, "Disabling account email verification");
			prefs_set_verify_account_email(false);
		}

		if (!account_email_verification_load(prefs_get_email_verification_file(), prefs_get_servername(), prefs_get_verify_account_email_from_address(), prefs_get_verify_account_email_from_name()))
		{
			eventlog(eventlog_level_error, __FUNCTION__, "Failed to load email verification message");
			eventlog(eventlog_level_error, __FUNCTION__, "Disabling account email verification");
			prefs_set_verify_account_email(false);
		}
	}
	else
	{
		eventlog(eventlog_level_debug, __FUNCTION__, "Config option 'verify_account_email' is false");
	}

#ifdef WITH_LUA
	lua_load(prefs_get_scriptdir());
#endif


	return 0;
}

void post_server_shutdown(int status)
{
	switch (status)
	{
	case 0:
		unload_topic_conf();
		account_email_verification_unload();
		smtp_cleanup();
		account_email_verification_unload();
		realmlist_destroy();
		teamlist_unload();
		clanlist_unload();
		tournament_destroy();
		customicons_unload();
		anongame_infos_unload();
		anongame_wol_matchlist_destroy();
		trans_unload();
		aliasfile_unload();
		command_groups_unload();
		tracker_set_servers(NULL);
		characterlist_destroy();
		ladder_destroyxptable();
	case STATUS_WAR3XPTABLES_FAILURE:

	case STATUS_LADDERLIST_FAILURE:
		ladders.save();
		accountlist_destroy();
		attrlayer_cleanup();
		watchlist.reset();
		news_unload();
		unload_versioncheck_conf();
		autoupdate_unload();
		ipbanlist_save(prefs_get_ipbanfile());
		ipbanlist_destroy();
		helpfile_unload();
		apireglist_destroy();
		channellist_destroy();
		server_clear_hostname();
		timerlist_destroy();
		gamelist_destroy();
		connlist_destroy();
		fdwatch_close();
	case STATUS_FDWATCH_FAILURE:
		anongame_matchlists_destroy();
	case STATUS_MATCHLISTS_FAILURE:
		anongame_maplists_destroy();
	case STATUS_MAPLISTS_FAILURE:
	case STATUS_SUPPORT_FAILURE:
		if (psock_deinit())
			eventlog(eventlog_level_error, __FUNCTION__, "got error from psock_deinit()");
	case STATUS_PSOCK_FAILURE:
		storage_close();
	case STATUS_STORAGE_FAILURE:
		oom_free();
		delete[] emergency_mem;
	case STATUS_OOM_FAILURE:
	case -1:
		break;
	default:
		eventlog(eventlog_level_error, __FUNCTION__, "got bad status \"{}\" during shutdown", status);
	}
	return;
}

void pvpgn_greeting()
{
#ifdef HAVE_GETPID
	eventlog(eventlog_level_info, __FUNCTION__, PVPGN_SOFTWARE " " PVPGN_VERSION " process {}", getpid());
#else
	eventlog(eventlog_level_info, __FUNCTION__, PVPGN_SOFTWARE" version "PVPGN_VERSION);
#endif

	struct utsname utsbuf = {};
	if (uname(&utsbuf) == 0)
	{
		eventlog(eventlog_level_info, __FUNCTION__, "running on {} ({} {}, {})", utsbuf.sysname, utsbuf.version, utsbuf.release, utsbuf.machine);
	}
	else
	{
		eventlog(eventlog_level_info, __FUNCTION__, "uname() failed");
	}

#ifdef WIN32_GUI
	gui_lvprintf(eventlog_level_info, "You are currently running " PVPGN_SOFTWARE " " PVPGN_VERSION "\n");
	gui_lvprintf(eventlog_level_info, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
	gui_lvprintf(eventlog_level_info, "If you need support:\n");
	gui_lvprintf(eventlog_level_info, " * Create an issue at https://github.com/pvpgn/pvpgn-server\n");
	gui_lvprintf(eventlog_level_info, "\nServer is now running.\n");
	gui_lvprintf(eventlog_level_info, "=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
#else
	std::printf("You are currently running " PVPGN_SOFTWARE " " PVPGN_VERSION "\n");
	std::printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
	std::printf("If you need support:\n");
	std::printf(" * Create an issue at https://github.com/pvpgn/pvpgn-server\n");
	std::printf("\nServer is now running.\n");
	std::printf("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=\n");
#endif

	return;
}

#ifdef WIN32_GUI
extern int app_main(int argc, char ** argv)
#else
extern int main(int argc, char ** argv)
#endif
{
	#ifdef WIN32
	// create a dump file whenever the gateway crashes
    SetUnhandledExceptionFilter(unhandled_handler);
	#endif
	
	try {
		int a;
		char *pidfile;

		if ((a = cmdline_load(argc, argv)) != 1)
			return a;

#ifdef DO_DAEMONIZE
		if ((a = fork_bnetd(cmdline_get_foreground())) != 0)
			return a < 0 ? a : 0; /* dont return code != 0 when things are OK! */
#endif

		eventlog_set(stderr);
		/* errors to eventlog from here on... */

		if (prefs_load(cmdline_get_preffile()) < 0) {
			eventlog(eventlog_level_fatal, __FUNCTION__, "could not parse configuration file (exiting)");
			return -1;
		}

		/* Start logging to std::log file */
		if (eventlog_startup() == -1)
			return -1;
		/* eventlog goes to std::log file from here on... */

		/* Give up root privileges */
		/* Hakan: That's way too late to give up root privileges... Have to look for a better place */
		if (give_up_root_privileges(prefs_get_effective_user(), prefs_get_effective_group()) < 0) {
			eventlog(eventlog_level_fatal, __FUNCTION__, "could not give up privileges (exiting)");
			return -1;
		}

		/* Write the pidfile */
		pidfile = write_to_pidfile();

		if (cmdline_get_hexfile()) {
			if (!(hexstrm = std::fopen(cmdline_get_hexfile(), "w")))
				eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"{}\" for writing the hexdump (std::fopen: {})", cmdline_get_hexfile(), std::strerror(errno));
			else
				std::fprintf(hexstrm, "# dump generated by " PVPGN_SOFTWARE " version " PVPGN_VERSION "\n");
		}

		/* Run the pre server stuff */
		a = pre_server_startup();

		/* now process connections and network traffic */
		if (a == 0) {
			if (server_process() < 0)
				eventlog(eventlog_level_fatal, __FUNCTION__, "failed to initialize network (exiting)");
		}

		// run post server stuff and exit
		post_server_shutdown(a);

		// Close hexfile
		if (hexstrm) {
			std::fprintf(hexstrm, "# end of dump\n");
			if (std::fclose(hexstrm) < 0)
				eventlog(eventlog_level_error, __FUNCTION__, "could not close hexdump file \"{}\" after writing (std::fclose: {})", cmdline_get_hexfile(), std::strerror(errno));
		}

		// Delete pidfile
		if (pidfile) {
			if (std::remove(pidfile) < 0)
				eventlog(eventlog_level_error, __FUNCTION__, "could not remove pid file \"{}\" (std::remove: {})", pidfile, std::strerror(errno));
			xfree((void *)pidfile); /* avoid warning */
		}

		if (a == 0)
			eventlog(eventlog_level_info, __FUNCTION__, "server has shut down");
		prefs_unload();
		cmdline_unload();
		//guiOnClose
#ifndef WIN32_GUI
		eventlog_close();
#endif

		if (a == 0)
			return 0;

		return -1;
	}
	catch (const std::exception& ex) {
		std::cerr << "FATAL ERROR: " << ex.what() << std::endl;
	}
	catch (...) {
		std::cerr << "UNKNOWN EXCEPTION TYPE" << std::endl;
	}
	return -1;
}

