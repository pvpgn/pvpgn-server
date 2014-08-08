/*
 * Copyright (C) 1998,1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
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
#ifdef INCLUDED_SETUP_AFTER_H
# error "This file must be included before all other header files"
#endif
#ifndef INCLUDED_SETUP_BEFORE_H
#define INCLUDED_SETUP_BEFORE_H

/* get autoconf defines */
#ifdef HAVE_CONFIG_H
# include "config.h"
#else
# ifdef WIN32
#  include "win32/configwin.h"
# else
#  error "No config.h but not building on WIN32, how to configure the system?"
# endif
#endif

/* This file contains compile-time configuration parameters including
 * debugging options, default configuration values, and format strings.
 */

#include "version.h"

/***************************************************************/
/* Debugging options */

/* print bad calling locations of account functions */
#define DEBUG_ACCOUNT

/* make bnchat, etc. print debug messages */
#define CLIENTDEBUG

/* make the routines call xxx_check() and notice
   xxx_purge() calls during list traversal */
#undef LIST_DEBUG
#undef HASHTABLE_DEBUG

/* this will use GCC evensions to verify that all module arguments to
   eventlog() are correct. */
#undef DEBUGMODSTRINGS
/*
   After you compile, use this script:
   strings bnetd |
   egrep '@\([^@]*@@[a-z_]*\)@' |
   sed -e 's/@(\([^:]*\):\([a-z_]*\)@@\([a-z_]*\))@/\1 \2 \3/g' |
   awk '{ if ( $2 != $3 ) printf("%s: %s->%s\n",$1,$2,$3); }'
   */

/* this will test get/unget memory management in account.c */
#undef TESTUNGET

/* check ladders for impossible combinations when returning them */
#define LADDER_DEBUG

/***************************************************************/
/* compile-time options */

/* how long do we wait for the UDP test reply? */
#define CLIENT_MAX_UDPTEST_WAIT 5

/* length of listen socket queue */
const int LISTEN_QUEUE = 10;

/* the format for account numbers */
#define UID_FORMAT "#%08u"

/* the format for game ids */
#define GAMEID_FORMAT "#%06u"

/* the format of timestamps in the logfile */
#define EVENT_TIME_FORMAT "%b %d %H:%M:%S"
const int EVENT_TIME_MAXLEN = 32;

/* the format of the stat times in bnstat */
#define STAT_TIME_FORMAT "%Y %b %d %H:%M:%S"
const int STAT_TIME_MAXLEN = 32;

/* the format of the file modification time in bnftp */
#define FILE_TIME_FORMAT "%Y %b %d %H:%M:%S"
const int FILE_TIME_MAXLEN = 32;

/* the format of the dates in the game report and for /gameinfo */
#define GAME_TIME_FORMAT "%a %b %d %H:%M:%S %Z"
const int GAME_TIME_MAXLEN = 32;

/* the format of the timestamps for the start/end of channel log files */
#define CHANLOG_TIME_FORMAT "%Y %b %d %H:%M:%S %Z"
const int CHANLOG_TIME_MAXLEN = 32;

/* the format of the timestamps for lines in the channel log files */
#define CHANLOGLINE_TIME_FORMAT "%b %d %H:%M:%S"

/* adjustable constants */
#define BNETD_LADDER_DEFAULT_TIME "19764578 0" /* 0:00 1 Jan 1970 GMT */

/* for clients if ioctl(TIOCGWINSZ) fails and $LINES and $COLUMNS aren't set */
const int DEF_SCREEN_WIDTH = 80;
const int DEF_SCREEN_HEIGHT = 24;

/***************************************************************/
/* default values for bnetd.conf */

/* default Boolean setup values */
const bool BNETD_CHANLOG = false;

/* default path configuration values */
#ifndef BNETD_DEFAULT_CONF_FILE
# define BNETD_DEFAULT_CONF_FILE "conf/bnetd.conf"
#endif
const char * const BNETD_FILE_DIR = "files";
const char * const BNETD_SCRIPT_DIR = "lua";
const char * const BNETD_STORAGE_PATH = "";
const char * const BNETD_REPORT_DIR = "reports";
const char * const BNETD_I18N_DIR = "conf/i18n";
// ------ i18n files --------
const char * const BNETD_LOCALIZE_FILE = "common.xml";
const char * const BNETD_MOTD_FILE = "bnmotd.txt";
const char * const BNETD_MOTDW3_FILE = "w3motd.txt";
const char * const BNETD_NEWS_FILE = "news.txt";
const char * const BNETD_HELP_FILE = "bnhelp.conf";
const char * const BNETD_TOS_FILE = "newaccount.txt";
// --------------------------
const char * const BNETD_LOG_FILE = "logs/bnetd.log";
const char * const BNETD_AD_FILE = "conf/ad.conf";
const char * const BNETD_CHANNEL_FILE = "conf/channel.conf";
const char * const BNETD_PID_FILE = "";  /* this means "none" */
const char * const BNETD_ACCOUNT_TMP = ".bnetd_acct_temp";
const char * const BNETD_IPBAN_FILE = "conf/bnban.conf";
const char * const BNETD_FORTUNECMD = "/usr/games/fortune";
const char * const BNETD_TRANS_FILE = "conf/address_translation.conf";
const char * const BNETD_CHANLOG_DIR = "chanlogs";
const char * const BNETD_REALM_FILE = "conf/realm.conf";
const char * const BNETD_ISSUE_FILE = "conf/bnissue.txt";
const char * const BNETD_MAIL_DIR = "var/bnmail";
const char * const PVPGN_VERSIONCHECK = "conf/versioncheck.conf";
const char * const BNETD_LADDER_DIR = "var/ladders";
const char * const BNETD_STATUS_DIR = "var/status";
const char * const BNETD_TOPIC_FILE = "var/topics";
const char * const BNETD_DBLAYOUT_FILE = "conf/sql_DB_layout.conf";
const char * const BNETD_SUPPORT_FILE = "conf/supportfile.conf";

const char * const BNETD_COMMAND_GROUPS_FILE = "conf/command_groups.conf";
const char * const BNETD_TOURNAMENT_FILE = "conf/tournament.conf";
const char * const BNETD_CUSTOMICONS_FILE = "conf/icons.conf";
const char * const BNETD_ALIASFILE = "conf/bnalias.conf";
/* time limit for new member as newer(whom cannot be promoted) in clan, (hrs) */
const unsigned CLAN_NEWER_TIME = 168;
const unsigned CLAN_DEFAULT_MAX_MEMBERS = 50;
/* hardcoded limits in the client */
const unsigned CLAN_MIN_MEMBERS = 10;
const unsigned CLAN_MAX_MEMBERS = 100;
const unsigned CLAN_DEFAULT_MIN_INVITES = 2;

const unsigned MAX_FRIENDS = 20;

/* maximum ammount of bytes sent in a single server.c/sd_tcpoutput call */
const unsigned BNETD_MAX_OUTBURST = 16384;

/* default files relative to FILE_DIR */
const char * const BNETD_ICON_FILE = "icons.bni";
const char * const BNETD_WAR3_ICON_FILE = "icons-WAR3.bni";
const char * const BNETD_STAR_ICON_FILE = "icons_STAR.bni";
const char * const BNETD_MPQ_FILE = "autoupdate";

/* Westwood Online default configuration values */
const char * const BNETD_APIREG_ADDRS = "";
const int BNETD_APIREG_PORT = 5400;
const char * const BNETD_WOLV1_ADDRS = "";
const int BNETD_WOLV1_PORT = 4000;
const char * const BNETD_WOLV2_ADDRS = "";
const int BNETD_WOLV2_PORT = 4005;
const char * const BNETD_WGAMERES_ADDRS = "";
const int BNETD_WGAMERES_PORT = 4807;


/* other default configuration values */
const char * const BNETD_LOG_LEVELS = "warn,error";
const char * const BNETD_SERV_ADDRS = ""; /* this means none */
const int BNETD_SERV_PORT = 6112;
const char * const BNETD_W3ROUTE_ADDR = "0.0.0.0";
const int BNETD_W3ROUTE_PORT = 6200;
const char * const BNETD_SERVERNAME = "PvPGN Realm";
const char * const BNETD_IRC_ADDRS = ""; /* this means none */
const int BNETD_IRC_PORT = 6667; /* used if port not specified */
const char * const BNETD_IRC_NETWORK_NAME = "PvPGN";
const char * const BNETD_TRACK_ADDRS = "track.pvpgn.org";
const int BNETD_TRACK_PORT = 6114; /* use this port if not specified */
const int BNETD_DEF_TEST_PORT = 6112; /* default guess for UDP test port */
const int BNETD_MIN_TEST_PORT = 6112;
const int BNETD_MAX_TEST_PORT = 6500;
const unsigned BNETD_USERSYNC = 300; /* s */
const unsigned BNETD_USERFLUSH = 1000;
const unsigned BNETD_USERSTEP = 100; /* check 100 users per call in accountlist_save() */
const unsigned BNETD_LATENCY = 600; /* s */
const unsigned BNETD_IRC_LATENCY = 180; /* s */ /* Ping timeout for IRC connections */
const unsigned BNETD_DEF_NULLMSG = 120; /* s */
const unsigned BNETD_TRACK_TIME = 0;
const int BNETD_POLL_INTERVAL = 20; /* 20 ms */
const int BNETD_JIFFIES = 50; /* 50 ms jiffies time quantum */
const unsigned BNETD_SHUTDELAY = 300; /* s */
const unsigned BNETD_SHUTDECR = 60; /* s */
const char * const BNETD_DEFAULT_OWNER = "PvPGN";
const char * const BNETD_DEFAULT_KEY = "3310541526205";
const char * const BNETD_DEFAULT_HOST = "localhost";
const unsigned BNETD_QUOTA_DOBAE = 7; /* lines */
const unsigned BNETD_QUOTA_LINES = 5; /* lines */
const unsigned BNETD_QUOTA_TIME = 5; /* s */
const unsigned BNETD_QUOTA_WLINE = 40; /* chars */
const unsigned BNETD_QUOTA_MLINE = 200; /* chars */
const unsigned BNETD_LADDER_INIT_RAT = 1000;
const unsigned BNETD_MAIL_SUPPORT = 0;
const unsigned BNETD_MAIL_QUOTA = 5;
const char * const BNETD_LOG_NOTICE = "*** Please note this channel is logged! ***";
const unsigned BNETD_HASHTABLE_SIZE = 61;
const int BNETD_REALM_PORT = 6113;  /* where D2CS listens */
const char * const BNETD_TELNET_ADDRS = ""; /* this means none */
const int BNETD_TELNET_PORT = 23; /* used if port not specified */
const char * const BNETD_EXEINFO_MATCH = "exact";
const unsigned PVPGN_VERSION_TIMEDIV = 0; /* no timediff check by default */
const int PVPGN_CACHE_MEMLIMIT = 5000000;  /* bytes */
const char * const PVPGN_DEFAULT_SYMB = "-_[]";

/***************************************************************/
/* default values for the tracking server */

const int BNTRACKD_EXPIRE = 600;
const int BNTRACKD_UPDATE = 150;
const int BNTRACKD_GRANULARITY = 5;
const int BNTRACKD_SERVER_PORT = 6114;
const char * const BNTRACKD_PIDFILE = ""; /* this means "none" */
const char * const BNTRACKD_OUTFILE = "pvpgnlist.txt";
#ifdef WIN32
const char * const BNTRACKD_PROCESS = "process.pl";
const char * const BNTRACKD_LOGFILE = "bntrackd.log";
#else
const char * const BNTRACKD_PROCESS = "scripts/process.pl";
const char * const BNTRACKD_LOGFILE = "logs/bntrackd.log";
#endif

/***************************************************************/
/* default values for W3XP anongameinfo packet */

const char * const PVPGN_DEFAULT_URL = "www.pvpgn.org";

const char * const PVPGN_PG_1V1_DESC = "Solo Games";
const char * const PVPGN_AT_2V2_DESC = "2 player team";
const char * const PVPGN_AT_3V3_DESC = "3 player team";
const char * const PVPGN_AT_4V4_DESC = "4 player team";
const char * const PVPGN_PG_TEAM_DESC = "Team Games";
const char * const PVPGN_PG_FFA_DESC = "Free for All Games";
const char * const PVPGN_CLAN_1V1_DESC = "Solo Games";
const char * const PVPGN_CLAN_2V2_DESC = "2 player team";
const char * const PVPGN_CLAN_3V3_DESC = "3 player team";
const char * const PVPGN_CLAN_4V4_DESC = "4 player team";

const char * const PVPGN_1V1_GT_DESC = "One vs. One";
const char * const PVPGN_1V1_GT_LONG = "Two players fight to the death";

const char * const PVPGN_2V2_GT_DESC = "Two vs. Two";
const char * const PVPGN_2V2_GT_LONG = "Two teams of two vie for dominance";

const char * const PVPGN_3V3_GT_DESC = "Three vs. Three";
const char * const PVPGN_3V3_GT_LONG = "Two teams of three face off on the battlefield";

const char * const PVPGN_4V4_GT_DESC = "Four vs. Four";
const char * const PVPGN_4V4_GT_LONG = "Two teams of four head to battle";

const char * const PVPGN_5V5_GT_DESC = "Five vs. Five";
const char * const PVPGN_5V5_GT_LONG = "Two teams of five - who will prevail?";

const char * const PVPGN_6V6_GT_DESC = "Six vs. Six";
const char * const PVPGN_6V6_GT_LONG = "Two teams of six - get ready to rumble!";

const char * const PVPGN_SFFA_GT_DESC = "Small Free for All";
const char * const PVPGN_SFFA_GT_LONG = "Can you defeat 3-5 opponents alone?";

const char * const PVPGN_TFFA_GT_DESC = "Team Free for All";
const char * const PVPGN_TFFA_GT_LONG = "Can your team defeat 1-2 others?";

const char * const PVPGN_2V2V2_GT_DESC = "Two vs. Two vs. Two";
const char * const PVPGN_2V2V2_GT_LONG = "Three teams of two, can you handle it?";

const char * const PVPGN_3V3V3_GT_DESC = "Three vs. Three vs. Three";
const char * const PVPGN_3V3V3_GT_LONG = "Three teams of three battle each other ";

const char * const PVPGN_4V4V4_GT_DESC = "Four vs. Four vs. Four";
const char * const PVPGN_4V4V4_GT_LONG = "Three teams of four - things getting crowded?";

const char * const PVPGN_2V2V2V2_GT_DESC = "Two vs. Two vs. Two vs. Two";
const char * const PVPGN_2V2V2V2_GT_LONG = "Four teams of two, is this a challenge?";

const char * const PVPGN_3V3V3V3_GT_DESC = "Three vs. Three vs. Three vs. Three";
const char * const PVPGN_3V3V3V3_GT_LONG = "Four teams of three, the ultimate challenge!";

const char * const PVPGN_AINFO_FILE = "conf/anongame_infos.conf";

/* max number of players in an anongame match [Omega] */
const int ANONGAME_MAX_GAMECOUNT = 12;

/* max level of players*/
const int ANONGAME_MAX_LEVEL = 100;

/***************************************************************/
/* platform dependent features */

/* conditionally enabled features */

#if defined(HAVE_SIGACTION) && defined(HAVE_SIGPROCMASK) && defined(HAVE_SIGADDSET)
# define DO_POSIXSIG
#endif

#if defined(HAVE_FORK) && defined(HAVE_PIPE)
# define DO_SUBPROC
#endif

#if defined(HAVE_FORK) && defined(HAVE_CHDIR) && (defined(HAVE_SETPGID) || defined(HAVE_SETPGRP))
# define DO_DAEMONIZE
#endif


/* GCC attributes */

/* enable format mismatch warnings from gcc */
#if defined(__GNUC__) && ((__GNUC__ == 2 && __GNUC_MINOR__ >= 5) || __GNUC__ > 2)
# if __GNUC__ == 2 && __GNUC_MINOR__ < 7
/* namespace clean versions were available starting in 2.6.4 */
#  define __format__ format
#  define __printf__ printf
# endif
# define PRINTF_ATTR(FMTARG,VARG) __attribute__((__format__(printf,FMTARG,VARG)))
#else
# define PRINTF_ATTR(FMTARG,VARG)
#endif

/* type attributes */

/* set GCC machine storage mode */
#if defined(__GNUC__) && ((__GNUC__ == 2 && __GNUC_MINOR__ >= 7) || __GNUC__ > 2)
# define MODE_ATTR(M) __attribute__((__mode__(M)))
# define HAVE_MODE_ATTR
#else
# define MODE_ATTR(M)
#endif

/* avoid using padding on GCC, for other compilers you need alternate solutions */
#if defined(__GNUC__)
# define PACKED_ATTR() __attribute__((__packed__))
#else
# define PACKED_ATTR()
#endif

/* default maxim number of sockets in the fdwatch pool */
const int BNETD_MAX_SOCKETS = 1000;

/* Used for FDSETSIZE redefine (only on WIN32 so so far) */
const int BNETD_MAX_SOCKVAL = 8192;

/*
 * select() hackery... works most places, need to add autoconf checks
 * because some systems may redefine FD_SETSIZE, have it as a variable,
 * or not have the concept of such a value.
 * dizzy: this is a total hack. only WIN32 so far specifies this as beeing
 * "legal"; in UNIX in general it should be NOT because the kernel interface
 * of select will never notice your userland changes to the fd_sets
 */
/* Win32 defaults to 64, BSD and Linux default to 1024 */
/* FIXME: how big can this be before things break? */
#ifdef WIN32
# define FD_SETSIZE BNETD_MAX_SOCKVAL
#endif

#ifdef HAVE_EPOLL_CREATE
# define HAVE_EPOLL	1
#endif

#if defined(WITH_LUA)
#define WITH_LUA	1
#endif

#if defined(WITH_SQL_MYSQL) || defined(WITH_SQL_PGSQL) || defined(WITH_SQL_SQLITE3) || defined(WITH_SQL_ODBC)
#define WITH_SQL	1
#endif

#endif
