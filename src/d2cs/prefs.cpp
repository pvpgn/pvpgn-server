/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
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
#include "setup.h"
#include "prefs.h"

#include <cstdio>

#include "common/conf.h"
#include "common/eventlog.h"
#include "common/setup_after.h"

namespace pvpgn
{

namespace d2cs
{

static struct
{
        char const      * logfile;
        char const      * loglevels;
        char const      * servaddrs;
        char const      * gameservlist;
        char const      * bnetdaddr;
        char const      * charsavedir;
        char const      * charinfodir;
        char const      * bak_charsavedir;
        char const      * bak_charinfodir;
        char const      * ladderdir;
		char const      * newbiefile_amazon;
		char const      * newbiefile_sorceress;
		char const      * newbiefile_necromancer;
		char const      * newbiefile_paladin;
		char const      * newbiefile_barbarian;
		char const      * newbiefile_druid;
		char const      * newbiefile_assasin;
        char const      * motd;
        char const      * realmname;
        char const      * d2gs_password;
        char const      * transfile;
        char const      * account_allowed_symbols;
        char const      * d2gsconffile;
	char const	* pidfile;
        unsigned int    ladder_refresh_interval;
        unsigned int    maxchar;
        unsigned int    listpurgeinterval;
        unsigned int    gqcheckinterval;
        unsigned int    s2s_retryinterval;
        unsigned int    s2s_timeout;
        unsigned int    s2s_idletime;
        unsigned int    sq_checkinterval;
        unsigned int    sq_timeout;
        unsigned int    maxgamelist;
        unsigned int    max_game_idletime;
        unsigned int    gamelist_showall;
	unsigned int    hide_pass_games;
        unsigned int    game_maxlifetime;
		unsigned int	game_maxlevel;
        unsigned int    allow_gamelimit;
        unsigned int    allow_newchar;
        unsigned int    idletime;
        unsigned int    shutdown_delay;
        unsigned int    shutdown_decr;
        unsigned int    d2gs_checksum;
        unsigned int    d2gs_version;
        unsigned int    check_multilogin;
        unsigned int    timeout_checkinterval;
        unsigned int    s2s_keepalive_interval;
        unsigned int    lod_realm;
        unsigned int    allow_convert;
        unsigned int    d2gs_restart_delay;
        std::time_t          ladder_start_time;
        unsigned int    char_expire_day;
        char const      * charlist_sort;
        char const      * charlist_sort_order;
        unsigned int    max_connections;
} prefs_conf;

static int conf_set_logfile(const char* valstr);
static int conf_setdef_logfile(void);

static int conf_set_loglevels(const char* valstr);
static int conf_setdef_loglevels(void);

static int conf_set_servaddrs(const char* valstr);
static int conf_setdef_servaddrs(void);

static int conf_set_gameservlist(const char* valstr);
static int conf_setdef_gameservlist(void);

static int conf_set_bnetdaddr(const char* valstr);
static int conf_setdef_bnetaddr(void);

static int conf_set_charsavedir(const char* valstr);
static int conf_setdef_charsavedir(void);

static int conf_set_charinfodir(const char* valstr);
static int conf_setdef_charinfodir(void);

static int conf_set_bakcharsavedir(const char* valstr);
static int conf_setdef_backcharsavedir(void);

static int conf_set_bakcharinfodir(const char* valstr);
static int conf_setdef_backcharinfodir(void);

static int conf_set_ladderdir(const char* valstr);
static int conf_setdef_ladderdir(void);

static int conf_set_ladder_start_time(const char* valstr);
static int conf_setdef_ladder_start_time(void);

static int conf_set_ladder_refresh_interval(const char* valstr);
static int conf_setdef_ladder_refresh_interval(void);

static int conf_set_newbiefile_amazon(const char* valstr);
static int conf_setdef_newbiefile_amazon(void);
static int conf_set_newbiefile_sorceress(const char* valstr);
static int conf_setdef_newbiefile_sorceress(void);
static int conf_set_newbiefile_necromancer(const char* valstr);
static int conf_setdef_newbiefile_necromancer(void);
static int conf_set_newbiefile_paladin(const char* valstr);
static int conf_setdef_newbiefile_paladin(void);
static int conf_set_newbiefile_barbarian(const char* valstr);
static int conf_setdef_newbiefile_barbarian(void);
static int conf_set_newbiefile_druid(const char* valstr);
static int conf_setdef_newbiefile_druid(void);
static int conf_set_newbiefile_assasin(const char* valstr);
static int conf_setdef_newbiefile_assasin(void);

static int conf_set_transfile(const char* valstr);
static int conf_setdef_transfile(void);

static int conf_set_motd(const char* valstr);
static int conf_setdef_motd(void);

static int conf_set_realmname(const char* valstr);
static int conf_setdef_realmname(void);

static int conf_set_maxchar(const char* valstr);
static int conf_setdef_maxchar(void);

static int conf_set_listpurgeinterval(const char* valstr);
static int conf_setdef_listpurgeinterval(void);

static int conf_set_gqcheckinterval(const char* valstr);
static int conf_setdef_gqcheckinterval(void);

static int conf_set_maxgamelist(const char* valstr);
static int conf_setdef_maxgamelist(void);

static int conf_set_max_game_idletime(const char* valstr);
static int conf_setdef_max_game_idletime(void);

static int conf_set_gamelist_showall(const char* valstr);
static int conf_setdef_gamelist_showall(void);

static int conf_set_hide_pass_games(const char* valstr);
static int conf_setdef_hide_pass_games(void);

static int conf_set_game_maxlifetime(const char* valstr);
static int conf_setdef_game_maxlifetime(void);

static int conf_set_game_maxlevel(const char* valstr);
static int conf_setdef_game_maxlevel(void);

static int conf_set_allow_gamelimit(const char* valstr);
static int conf_setdef_allow_gamelimit(void);

static int conf_set_allow_newchar(const char* valstr);
static int conf_setdef_allow_newchar(void);

static int conf_set_idletime(const char* valstr);
static int conf_setdef_idletime(void);

static int conf_set_shutdown_delay(const char* valstr);
static int conf_setdef_shutdown_delay(void);

static int conf_set_shutdown_decr(const char* valstr);
static int conf_setdef_shutdown_decr(void);

static int conf_set_s2s_retryinterval(const char* valstr);
static int conf_setdef_s2s_retryinterval(void);

static int conf_set_s2s_timeout(const char* valstr);
static int conf_setdef_s2s_timeout(void);

static int conf_set_sq_checkinterval(const char* valstr);
static int conf_setdef_sq_checkinterval(void);

static int conf_set_sq_timeout(const char* valstr);
static int conf_setdef_sq_timeout(void);

static int conf_set_d2gs_checksum(const char* valstr);
static int conf_setdef_d2gs_checksum(void);

static int conf_set_d2gs_version(const char* valstr);
static int conf_setdef_d2gs_version(void);

static int conf_set_d2gs_password(const char* valstr);
static int conf_setdef_d2gs_password(void);

static int conf_set_check_multilogin(const char* valstr);
static int conf_setdef_check_multilogin(void);

static int conf_set_s2s_idletime(const char* valstr);
static int conf_setdef_s2s_idletime(void);

static int conf_set_s2s_keepalive_interval(const char* valstr);
static int conf_setdef_s2s_keepalive_interval(void);

static int conf_set_timeout_checkinterval(const char* valstr);
static int conf_setdef_timeout_checkinterval(void);

static int conf_set_lod_realm(const char* valstr);
static int conf_setdef_lod_realm(void);

static int conf_set_allow_convert(const char* valstr);
static int conf_setdef_allow_convert(void);

static int conf_set_account_allowed_symbols(const char* valstr);
static int conf_setdef_account_allowed_symbols(void);

static int conf_set_d2gs_restart_delay(const char* valstr);
static int conf_setdef_d2gs_restart_delay(void);

static int conf_set_char_expire_day(const char* valstr);
static int conf_setdef_char_expire_day(void);

static int conf_set_d2gsconffile(const char* valstr);
static int conf_setdef_d2gsconffile(void);

static int conf_set_charlist_sort(const char* valstr);
static int conf_setdef_charlist_sort(void);

static int conf_set_charlist_sort_order(const char* valstr);
static int conf_setdef_charlist_sort_order(void);

static int conf_set_max_connections(const char* valstr);
static int conf_setdef_max_connections(void);

static int conf_set_pidfile(const char* valstr);
static int conf_setdef_pidfile(void);

static t_conf_entry prefs_conf_table[]={
    { "logfile",                conf_set_logfile,                NULL,    conf_setdef_logfile },
    { "loglevels",              conf_set_loglevels,              NULL,    conf_setdef_loglevels},
    { "servaddrs",              conf_set_servaddrs,              NULL,    conf_setdef_servaddrs},
    { "gameservlist",           conf_set_gameservlist,           NULL,    conf_setdef_gameservlist},
    { "bnetdaddr",              conf_set_bnetdaddr,              NULL,    conf_setdef_bnetaddr},
    { "charsavedir",            conf_set_charsavedir,            NULL,    conf_setdef_charsavedir},
    { "charinfodir",            conf_set_charinfodir,            NULL,    conf_setdef_charinfodir},
    { "bak_charsavedir",	conf_set_bakcharsavedir,         NULL,    conf_setdef_backcharsavedir},
    { "bak_charinfodir",	conf_set_bakcharinfodir,	 NULL,    conf_setdef_backcharinfodir},
    { "ladderdir",              conf_set_ladderdir,              NULL,    conf_setdef_ladderdir},
    { "ladder_start_time",	conf_set_ladder_start_time,      NULL,    conf_setdef_ladder_start_time},
    { "ladder_refresh_interval",conf_set_ladder_refresh_interval,NULL,   conf_setdef_ladder_refresh_interval},
	{ "newbiefile_amazon", conf_set_newbiefile_amazon, NULL, conf_setdef_newbiefile_amazon },
	{ "newbiefile_sorceress", conf_set_newbiefile_sorceress, NULL, conf_setdef_newbiefile_sorceress },
	{ "newbiefile_necromancer", conf_set_newbiefile_necromancer, NULL, conf_setdef_newbiefile_necromancer },
	{ "newbiefile_paladin", conf_set_newbiefile_paladin, NULL, conf_setdef_newbiefile_paladin },
	{ "newbiefile_barbarian", conf_set_newbiefile_barbarian, NULL, conf_setdef_newbiefile_barbarian },
	{ "newbiefile_druid", conf_set_newbiefile_druid, NULL, conf_setdef_newbiefile_druid },
	{ "newbiefile_assasin", conf_set_newbiefile_assasin, NULL, conf_setdef_newbiefile_assasin },
    { "transfile",		conf_set_transfile,	         NULL,    conf_setdef_transfile},
    { "pidfile",		conf_set_pidfile,	         NULL,    conf_setdef_pidfile},
    { "motd",                   conf_set_motd,                   NULL,    conf_setdef_motd},
    { "realmname",              conf_set_realmname,              NULL,    conf_setdef_realmname},
    { "maxchar",                conf_set_maxchar,                NULL,    conf_setdef_maxchar},
    { "listpurgeinterval",      conf_set_listpurgeinterval,      NULL,    conf_setdef_listpurgeinterval},
    { "gqcheckinterval",        conf_set_gqcheckinterval,        NULL,    conf_setdef_gqcheckinterval},
    { "maxgamelist",            conf_set_maxgamelist,            NULL,    conf_setdef_maxgamelist},
    { "max_game_idletime",      conf_set_max_game_idletime,      NULL,    conf_setdef_max_game_idletime},
    { "gamelist_showall",       conf_set_gamelist_showall,       NULL,    conf_setdef_gamelist_showall},
    { "hide_pass_games",        conf_set_hide_pass_games,        NULL,    conf_setdef_hide_pass_games},
    { "game_maxlifetime",       conf_set_game_maxlifetime,       NULL,    conf_setdef_game_maxlifetime},
    { "game_maxlevel",			conf_set_game_maxlevel,          NULL,    conf_setdef_game_maxlevel},
    { "allow_gamelimit",        conf_set_allow_gamelimit,        NULL,    conf_setdef_allow_gamelimit},
    { "allow_newchar",          conf_set_allow_newchar,          NULL,    conf_setdef_allow_newchar},
    { "idletime",               conf_set_idletime,               NULL,    conf_setdef_idletime},
    { "shutdown_delay",         conf_set_shutdown_delay,         NULL,    conf_setdef_shutdown_delay},
    { "shutdown_decr",          conf_set_shutdown_decr,          NULL,    conf_setdef_shutdown_decr},
    { "s2s_retryinterval",      conf_set_s2s_retryinterval,      NULL,    conf_setdef_s2s_retryinterval},
    { "s2s_timeout",            conf_set_s2s_timeout,            NULL,    conf_setdef_s2s_timeout},
    { "sq_checkinterval",       conf_set_sq_checkinterval,       NULL,    conf_setdef_sq_checkinterval},
    { "sq_timeout",             conf_set_sq_timeout,             NULL,    conf_setdef_sq_timeout},
    { "d2gs_checksum",          conf_set_d2gs_checksum,          NULL,    conf_setdef_d2gs_checksum},
    { "d2gs_version",           conf_set_d2gs_version,           NULL,    conf_setdef_d2gs_version},
    { "d2gs_password",          conf_set_d2gs_password,          NULL,    conf_setdef_d2gs_password},
    { "check_multilogin",       conf_set_check_multilogin,       NULL,    conf_setdef_check_multilogin},
    { "s2s_idletime",           conf_set_s2s_idletime,           NULL,    conf_setdef_s2s_idletime},
    { "s2s_keepalive_interval", conf_set_s2s_keepalive_interval, NULL,    conf_setdef_s2s_keepalive_interval},
    { "timeout_checkinterval",  conf_set_timeout_checkinterval,  NULL,    conf_setdef_timeout_checkinterval},
    { "lod_realm",		conf_set_lod_realm,              NULL,    conf_setdef_lod_realm},
    { "allow_convert",		conf_set_allow_convert,          NULL,    conf_setdef_allow_convert},
    { "account_allowed_symbols",conf_set_account_allowed_symbols,NULL,    conf_setdef_account_allowed_symbols},
    { "d2gs_restart_delay",	conf_set_d2gs_restart_delay,     NULL,    conf_setdef_d2gs_restart_delay},
    { "char_expire_day",	conf_set_char_expire_day,        NULL,    conf_setdef_char_expire_day},
    { "d2gsconffile",           conf_set_d2gsconffile,           NULL,    conf_setdef_d2gsconffile},
    { "charlist_sort",          conf_set_charlist_sort,          NULL,    conf_setdef_charlist_sort},
    { "charlist_sort_order",    conf_set_charlist_sort_order,    NULL,    conf_setdef_charlist_sort_order},
    { "max_connections",    	conf_set_max_connections,    	 NULL,    conf_setdef_max_connections},
    { NULL,                     NULL,                            NULL,    NULL }
};

extern int d2cs_prefs_load(char const * filename)
{
    std::FILE *fd;

    if (!filename) {
        eventlog(eventlog_level_error,__FUNCTION__,"got NULL filename");
        return -1;
    }

    fd = std::fopen(filename,"rt");
    if (!fd) {
        eventlog(eventlog_level_error,__FUNCTION__,"could not open file '%s'",filename);
        return -1;
    }

    if (conf_load_file(fd,prefs_conf_table)) {
        eventlog(eventlog_level_error,__FUNCTION__,"error loading config file '%s'",filename);
        std::fclose(fd);
        return -1;
    }

    std::fclose(fd);

    return 0;
}

extern int prefs_reload(char const * filename)
{
	d2cs_prefs_unload();
	if (d2cs_prefs_load(filename)<0) return -1;
	return 0;
}

extern int d2cs_prefs_unload(void)
{
	conf_unload(prefs_conf_table);
	return 0;
}

extern char const * prefs_get_servaddrs(void)
{
	return prefs_conf.servaddrs;
}

static int conf_set_servaddrs(const char* valstr)
{
    return conf_set_str(&prefs_conf.servaddrs,valstr,NULL);
}

static int conf_setdef_servaddrs(void)
{
    return conf_set_str(&prefs_conf.servaddrs,NULL,D2CS_SERVER_ADDRS);
}


extern char const * prefs_get_charsave_dir(void)
{
	return prefs_conf.charsavedir;
}

static int conf_set_charsavedir(const char* valstr)
{
    return conf_set_str(&prefs_conf.charsavedir,valstr,NULL);
}

static int conf_setdef_charsavedir(void)
{
    return conf_set_str(&prefs_conf.charsavedir,NULL,D2CS_CHARSAVE_DIR);
}


extern char const * prefs_get_charinfo_dir(void)
{
	return prefs_conf.charinfodir;
}

static int conf_set_charinfodir(const char* valstr)
{
    return conf_set_str(&prefs_conf.charinfodir,valstr,NULL);
}

static int conf_setdef_charinfodir(void)
{
    return conf_set_str(&prefs_conf.charinfodir,NULL,D2CS_CHARINFO_DIR);
}


extern char const * prefs_get_bak_charsave_dir(void)
{
	return prefs_conf.bak_charsavedir;
}

static int conf_set_bakcharsavedir(const char* valstr)
{
    return conf_set_str(&prefs_conf.bak_charsavedir,valstr,NULL);
}

static int conf_setdef_backcharsavedir(void)
{
    return conf_set_str(&prefs_conf.bak_charsavedir,NULL,D2CS_BAK_CHARSAVE_DIR);
}


extern char const * prefs_get_bak_charinfo_dir(void)
{
	return prefs_conf.bak_charinfodir;
}

static int conf_set_bakcharinfodir(const char* valstr)
{
    return conf_set_str(&prefs_conf.bak_charinfodir,valstr,NULL);
}

static int conf_setdef_backcharinfodir(void)
{
    return conf_set_str(&prefs_conf.bak_charinfodir,NULL,D2CS_BAK_CHARINFO_DIR);
}


/* newbiefile */

extern char const * prefs_get_charsave_newbie_amazon(void)
{
	return prefs_conf.newbiefile_amazon;
}
static int conf_set_newbiefile_amazon(const char* valstr)
{
	return conf_set_str(&prefs_conf.newbiefile_amazon, valstr, NULL);
}
static int conf_setdef_newbiefile_amazon(void)
{
	return conf_set_str(&prefs_conf.newbiefile_amazon, NULL, D2CS_CHARSAVE_NEWBIE);
}

extern char const * prefs_get_charsave_newbie_sorceress(void)
{
	return prefs_conf.newbiefile_sorceress;
}
static int conf_set_newbiefile_sorceress(const char* valstr)
{
	return conf_set_str(&prefs_conf.newbiefile_sorceress, valstr, NULL);
}
static int conf_setdef_newbiefile_sorceress(void)
{
	return conf_set_str(&prefs_conf.newbiefile_sorceress, NULL, D2CS_CHARSAVE_NEWBIE);
}

extern char const * prefs_get_charsave_newbie_necromancer(void)
{
	return prefs_conf.newbiefile_necromancer;
}
static int conf_set_newbiefile_necromancer(const char* valstr)
{
	return conf_set_str(&prefs_conf.newbiefile_necromancer, valstr, NULL);
}
static int conf_setdef_newbiefile_necromancer(void)
{
	return conf_set_str(&prefs_conf.newbiefile_necromancer, NULL, D2CS_CHARSAVE_NEWBIE);
}

extern char const * prefs_get_charsave_newbie_paladin(void)
{
	return prefs_conf.newbiefile_paladin;
}
static int conf_set_newbiefile_paladin(const char* valstr)
{
	return conf_set_str(&prefs_conf.newbiefile_paladin, valstr, NULL);
}
static int conf_setdef_newbiefile_paladin(void)
{
	return conf_set_str(&prefs_conf.newbiefile_paladin, NULL, D2CS_CHARSAVE_NEWBIE);
}

extern char const * prefs_get_charsave_newbie_barbarian(void)
{
	return prefs_conf.newbiefile_barbarian;
}
static int conf_set_newbiefile_barbarian(const char* valstr)
{
	return conf_set_str(&prefs_conf.newbiefile_barbarian, valstr, NULL);
}
static int conf_setdef_newbiefile_barbarian(void)
{
	return conf_set_str(&prefs_conf.newbiefile_barbarian, NULL, D2CS_CHARSAVE_NEWBIE);
}

extern char const * prefs_get_charsave_newbie_druid(void)
{
	return prefs_conf.newbiefile_druid;
}
static int conf_set_newbiefile_druid(const char* valstr)
{
	return conf_set_str(&prefs_conf.newbiefile_druid, valstr, NULL);
}
static int conf_setdef_newbiefile_druid(void)
{
	return conf_set_str(&prefs_conf.newbiefile_druid, NULL, D2CS_CHARSAVE_NEWBIE);
}

extern char const * prefs_get_charsave_newbie_assasin(void)
{
	return prefs_conf.newbiefile_assasin;
}
static int conf_set_newbiefile_assasin(const char* valstr)
{
	return conf_set_str(&prefs_conf.newbiefile_assasin, valstr, NULL);
}
static int conf_setdef_newbiefile_assasin(void)
{
	return conf_set_str(&prefs_conf.newbiefile_assasin, NULL, D2CS_CHARSAVE_NEWBIE);
}



extern char const * prefs_get_motd(void)
{
	return prefs_conf.motd;
}

static int conf_set_motd(const char* valstr)
{
	return conf_set_str(&prefs_conf.motd,valstr,NULL);
}

static int conf_setdef_motd(void)
{
	return conf_set_str(&prefs_conf.motd,NULL,D2CS_MOTD);
}


extern char const * prefs_get_d2gs_list(void)
{
	return prefs_conf.gameservlist;
}

static int conf_set_gameservlist(const char* valstr)
{
	return conf_set_str(&prefs_conf.gameservlist,valstr,NULL);
}

static int conf_setdef_gameservlist(void)
{
	return conf_set_str(&prefs_conf.gameservlist,NULL,D2GS_SERVER_LIST);
}


extern unsigned int prefs_get_maxchar(void)
{
	return (prefs_conf.maxchar>MAX_MAX_CHAR_PER_ACCT)?MAX_MAX_CHAR_PER_ACCT:prefs_conf.maxchar;
}

static int conf_set_maxchar(const char* valstr)
{
	return conf_set_int(&prefs_conf.maxchar,valstr,0);
}


static int conf_setdef_maxchar(void)
{
	return conf_set_int(&prefs_conf.maxchar,NULL,MAX_CHAR_PER_ACCT);
}


extern unsigned int prefs_get_list_purgeinterval(void)
{
	return prefs_conf.listpurgeinterval;
}

static int conf_set_listpurgeinterval(const char* valstr)
{
	return conf_set_int(&prefs_conf.listpurgeinterval,valstr,0);
}

static int conf_setdef_listpurgeinterval(void)
{
	return conf_set_int(&prefs_conf.listpurgeinterval,NULL,LIST_PURGE_INTERVAL);
}


extern unsigned int prefs_get_gamequeue_checkinterval(void)
{
	return prefs_conf.gqcheckinterval;
}

static int conf_set_gqcheckinterval(const char* valstr)
{
	return conf_set_int(&prefs_conf.gqcheckinterval,valstr,0);
}

static int conf_setdef_gqcheckinterval(void)
{
	return conf_set_int(&prefs_conf.gqcheckinterval,NULL,GAMEQUEUE_CHECK_INTERVAL);
}


extern unsigned int prefs_get_maxgamelist(void)
{
	return prefs_conf.maxgamelist;
}

static int conf_set_maxgamelist(const char* valstr)
{
	return conf_set_int(&prefs_conf.maxgamelist,valstr,0);
}

static int conf_setdef_maxgamelist(void)
{
	return conf_set_int(&prefs_conf.maxgamelist,NULL,MAX_GAME_LIST);
}


extern unsigned int prefs_allow_newchar(void)
{
	return prefs_conf.allow_newchar;
}

static int conf_set_allow_newchar(const char* valstr)
{
	return conf_set_bool(&prefs_conf.allow_newchar,valstr,0);
}


static int conf_setdef_allow_newchar(void)
{
	return conf_set_bool(&prefs_conf.allow_newchar,NULL,1);
}


extern unsigned int prefs_get_idletime(void)
{
	return prefs_conf.idletime;
}

static int conf_set_idletime(const char* valstr)
{
	return conf_set_int(&prefs_conf.idletime,valstr,0);
}

static int conf_setdef_idletime(void)
{
	return conf_set_int(&prefs_conf.idletime,NULL,MAX_CLIENT_IDLETIME);
}


extern char const * d2cs_prefs_get_logfile(void)
{
	return prefs_conf.logfile;
}

static int conf_set_logfile(const char* valstr)
{
	return conf_set_str(&prefs_conf.logfile,valstr,NULL);
}

static int conf_setdef_logfile(void)
{
	return conf_set_str(&prefs_conf.logfile,NULL,DEFAULT_LOG_FILE);
}


extern unsigned int d2cs_prefs_get_shutdown_delay(void)
{
	return prefs_conf.shutdown_delay;
}

static int conf_set_shutdown_delay(const char* valstr)
{
	return conf_set_int(&prefs_conf.shutdown_delay,valstr,0);
}

static int conf_setdef_shutdown_delay(void)
{
	return conf_set_int(&prefs_conf.shutdown_delay,NULL,DEFAULT_SHUTDOWN_DELAY);
}


extern unsigned int d2cs_prefs_get_shutdown_decr(void)
{
	return prefs_conf.shutdown_decr;
}

static int conf_set_shutdown_decr(const char* valstr)
{
	return conf_set_int(&prefs_conf.shutdown_decr,valstr,0);
}

static int conf_setdef_shutdown_decr(void)
{
	return conf_set_int(&prefs_conf.shutdown_decr,NULL,DEFAULT_SHUTDOWN_DECR);
}


extern char const * prefs_get_bnetdaddr(void)
{
	return prefs_conf.bnetdaddr;
}

static int conf_set_bnetdaddr(const char* valstr)
{
	return conf_set_str(&prefs_conf.bnetdaddr,valstr,NULL);
}

static int conf_setdef_bnetaddr(void)
{
	return conf_set_str(&prefs_conf.bnetdaddr,NULL,BNETD_SERVER_LIST);
}


extern char const * prefs_get_realmname(void)
{
	return prefs_conf.realmname;
}

static int conf_set_realmname(const char* valstr)
{
	return conf_set_str(&prefs_conf.realmname,valstr,NULL);
}

static int conf_setdef_realmname(void)
{
	return conf_set_str(&prefs_conf.realmname,NULL,DEFAULT_REALM_NAME);
}


extern unsigned int prefs_get_s2s_retryinterval(void)
{
	return prefs_conf.s2s_retryinterval;
}

static int conf_set_s2s_retryinterval(const char* valstr)
{
	return conf_set_int(&prefs_conf.s2s_retryinterval,valstr,0);
}

static int conf_setdef_s2s_retryinterval(void)
{
	return conf_set_int(&prefs_conf.s2s_retryinterval,NULL,DEFAULT_S2S_RETRYINTERVAL);
}


extern unsigned int prefs_get_s2s_timeout(void)
{
	return prefs_conf.s2s_timeout;
}

static int conf_set_s2s_timeout(const char* valstr)
{
	return conf_set_int(&prefs_conf.s2s_timeout,valstr,0);
}

static int conf_setdef_s2s_timeout(void)
{
	return conf_set_int(&prefs_conf.s2s_timeout,NULL,DEFAULT_S2S_TIMEOUT);
}


extern unsigned int prefs_get_sq_timeout(void)
{
	return prefs_conf.sq_timeout;
}

static int conf_set_sq_timeout(const char* valstr)
{
	return conf_set_int(&prefs_conf.sq_timeout,valstr,0);
}

static int conf_setdef_sq_timeout(void)
{
	return conf_set_int(&prefs_conf.sq_timeout,NULL,DEFAULT_SQ_TIMEOUT);
}


extern unsigned int prefs_get_sq_checkinterval(void)
{
	return prefs_conf.sq_checkinterval;
}

static int conf_set_sq_checkinterval(const char* valstr)
{
	return conf_set_int(&prefs_conf.sq_checkinterval,valstr,0);
}

static int conf_setdef_sq_checkinterval(void)
{
	return conf_set_int(&prefs_conf.sq_checkinterval,NULL,DEFAULT_SQ_CHECKINTERVAL);
}


extern unsigned int prefs_get_d2gs_checksum(void)
{
	return prefs_conf.d2gs_checksum;
}

static int conf_set_d2gs_checksum(const char* valstr)
{
	return conf_set_int(&prefs_conf.d2gs_checksum,valstr,0);
}

static int conf_setdef_d2gs_checksum(void)
{
	return conf_set_int(&prefs_conf.d2gs_checksum,NULL,0);
}


extern unsigned int prefs_get_d2gs_version(void)
{
	return prefs_conf.d2gs_version;
}

static int conf_set_d2gs_version(const char* valstr)
{
	return conf_set_int(&prefs_conf.d2gs_version,valstr,0);
}

static int conf_setdef_d2gs_version(void)
{
	return conf_set_int(&prefs_conf.d2gs_version,NULL,0);
}


extern unsigned int prefs_get_ladderlist_count(void)
{
	return 0x10;
}

extern unsigned int prefs_get_d2ladder_refresh_interval(void)
{
	return prefs_conf.ladder_refresh_interval;
}

static int conf_set_ladder_refresh_interval(const char* valstr)
{
	return conf_set_int(&prefs_conf.ladder_refresh_interval,valstr,0);
}

static int conf_setdef_ladder_refresh_interval(void)
{
	return conf_set_int(&prefs_conf.ladder_refresh_interval,NULL,3600);
}


extern unsigned int prefs_get_game_maxlifetime(void)
{
	return prefs_conf.game_maxlifetime;
}

static int conf_set_game_maxlifetime(const char* valstr)
{
	return conf_set_int(&prefs_conf.game_maxlifetime,valstr,0);
}

static int conf_setdef_game_maxlifetime(void)
{
	return conf_set_int(&prefs_conf.game_maxlifetime,NULL,0);
}


extern unsigned int prefs_get_game_maxlevel(void)
{
	return prefs_conf.game_maxlevel;
}

static int conf_set_game_maxlevel(const char* valstr)
{
	return conf_set_int(&prefs_conf.game_maxlevel,valstr,0);
}

static int conf_setdef_game_maxlevel(void)
{
	return conf_set_int(&prefs_conf.game_maxlevel,NULL,0xff);
}


extern char const * prefs_get_ladder_dir(void)
{
	return prefs_conf.ladderdir;
}

static int conf_set_ladderdir(const char* valstr)
{
	return conf_set_str(&prefs_conf.ladderdir,valstr,NULL);
}

static int conf_setdef_ladderdir(void)
{
	return conf_set_str(&prefs_conf.ladderdir,NULL,D2CS_LADDER_DIR);
}


extern char const * d2cs_prefs_get_loglevels(void)
{
	return prefs_conf.loglevels;
}

static int conf_set_loglevels(const char* valstr)
{
	return conf_set_str(&prefs_conf.loglevels,valstr,NULL);
}

static int conf_setdef_loglevels(void)
{
	return conf_set_str(&prefs_conf.loglevels,NULL,DEFAULT_LOG_LEVELS);
}


extern unsigned int prefs_allow_gamelist_showall(void)
{
	return prefs_conf.gamelist_showall;
}

static int conf_set_gamelist_showall(const char* valstr)
{
	return conf_set_bool(&prefs_conf.gamelist_showall,valstr,0);
}

static int conf_setdef_gamelist_showall(void)
{
	return conf_set_bool(&prefs_conf.gamelist_showall,NULL,0);
}


extern unsigned int prefs_hide_pass_games(void)
{
	return prefs_conf.hide_pass_games;
}

static int conf_set_hide_pass_games(const char* valstr)
{
	return conf_set_bool(&prefs_conf.hide_pass_games,valstr,0);
}

static int conf_setdef_hide_pass_games(void)
{
	return conf_set_bool(&prefs_conf.hide_pass_games,NULL,0);
}


extern unsigned int prefs_allow_gamelimit(void)
{
	return prefs_conf.allow_gamelimit;
}

static int conf_set_allow_gamelimit(const char* valstr)
{
	return conf_set_bool(&prefs_conf.allow_gamelimit,valstr,0);
}

static int conf_setdef_allow_gamelimit(void)
{
	return conf_set_bool(&prefs_conf.allow_gamelimit,NULL,1);
}


extern unsigned int prefs_check_multilogin(void)
{
	return prefs_conf.check_multilogin;
}

static int conf_set_check_multilogin(const char* valstr)
{
	return conf_set_bool(&prefs_conf.check_multilogin,valstr,0);
}

static int conf_setdef_check_multilogin(void)
{
	return conf_set_bool(&prefs_conf.check_multilogin,NULL,1);
}


extern char const * prefs_get_d2gs_password(void)
{
	return prefs_conf.d2gs_password;
}

static int conf_set_d2gs_password(const char* valstr)
{
	return conf_set_str(&prefs_conf.d2gs_password,valstr,NULL);
}

static int conf_setdef_d2gs_password(void)
{
	return conf_set_str(&prefs_conf.d2gs_password,NULL,"");
}


extern unsigned int prefs_get_s2s_idletime(void)
{
	return prefs_conf.s2s_idletime;
}

static int conf_set_s2s_idletime(const char* valstr)
{
	return conf_set_int(&prefs_conf.s2s_idletime,valstr,0);
}

static int conf_setdef_s2s_idletime(void)
{
	return conf_set_int(&prefs_conf.s2s_idletime,NULL,DEFAULT_S2S_IDLETIME);
}


extern unsigned int prefs_get_s2s_keepalive_interval(void)
{
	return prefs_conf.s2s_keepalive_interval;
}

static int conf_set_s2s_keepalive_interval(const char* valstr)
{
	return conf_set_int(&prefs_conf.s2s_keepalive_interval,valstr,0);
}

static int conf_setdef_s2s_keepalive_interval(void)
{
	return conf_set_int(&prefs_conf.s2s_keepalive_interval,NULL,DEFAULT_S2S_KEEPALIVE_INTERVAL);
}


extern unsigned int prefs_get_timeout_checkinterval(void)
{
	return prefs_conf.timeout_checkinterval;
}

static int conf_set_timeout_checkinterval(const char* valstr)
{
	return conf_set_int(&prefs_conf.timeout_checkinterval,valstr,0);
}

static int conf_setdef_timeout_checkinterval(void)
{
	return conf_set_int(&prefs_conf.timeout_checkinterval,NULL,DEFAULT_TIMEOUT_CHECKINTERVAL);
}


extern unsigned int prefs_get_max_game_idletime(void)
{
	return prefs_conf.max_game_idletime;
}

static int conf_set_max_game_idletime(const char* valstr)
{
	return conf_set_int(&prefs_conf.max_game_idletime,valstr,0);
}

static int conf_setdef_max_game_idletime(void)
{
	return conf_set_int(&prefs_conf.max_game_idletime,NULL,MAX_GAME_IDLE_TIME);
}


extern unsigned int prefs_get_lod_realm(void)
{
	return prefs_conf.lod_realm;
}

static int conf_set_lod_realm(const char* valstr)
{
	return conf_set_int(&prefs_conf.lod_realm,valstr,0);
}

static int conf_setdef_lod_realm(void)
{
	return conf_set_int(&prefs_conf.lod_realm,NULL,2);
}


extern unsigned int prefs_get_allow_convert(void)
{
	return prefs_conf.allow_convert;
}

static int conf_set_allow_convert(const char* valstr)
{
	return conf_set_bool(&prefs_conf.allow_convert,valstr,0);
}

static int conf_setdef_allow_convert(void)
{
	return conf_set_bool(&prefs_conf.allow_convert,NULL,0);
}


extern char const * d2cs_prefs_get_transfile(void)
{
	return prefs_conf.transfile;
}

static int conf_set_transfile(const char* valstr)
{
	return conf_set_str(&prefs_conf.transfile,valstr,NULL);
}

static int conf_setdef_transfile(void)
{
	return conf_set_str(&prefs_conf.transfile,NULL,D2CS_TRANS_FILE);
}


extern char const * prefs_get_d2cs_account_allowed_symbols(void)
{
	return prefs_conf.account_allowed_symbols;
}

static int conf_set_account_allowed_symbols(const char* valstr)
{
	return conf_set_str(&prefs_conf.account_allowed_symbols,valstr,NULL);
}

static int conf_setdef_account_allowed_symbols(void)
{
	return conf_set_str(&prefs_conf.account_allowed_symbols,NULL,DEFAULT_ACC_ALLOWED_SYMBOLS);
}


extern unsigned int prefs_get_ladder_start_time(void)
{
	return prefs_conf.ladder_start_time;
}

static int conf_set_ladder_start_time(const char* valstr)
{
	return conf_set_timestr(&prefs_conf.ladder_start_time,valstr,0);
}

static int conf_setdef_ladder_start_time(void)
{
	return conf_set_timestr(&prefs_conf.ladder_start_time,NULL,0);
}


extern unsigned int prefs_get_d2gs_restart_delay(void)
{
	return prefs_conf.d2gs_restart_delay;
}

static int conf_set_d2gs_restart_delay(const char* valstr)
{
	return conf_set_int(&prefs_conf.d2gs_restart_delay,valstr,0);
}

static int conf_setdef_d2gs_restart_delay(void)
{
	return conf_set_int(&prefs_conf.d2gs_restart_delay,NULL,DEFAULT_D2GS_RESTART_DELAY);
}


extern unsigned int prefs_get_char_expire_time(void)
{
	return prefs_conf.char_expire_day * 3600 * 24;
}

static int conf_set_char_expire_day(const char* valstr)
{
	return conf_set_int(&prefs_conf.char_expire_day,valstr,0);
}

static int conf_setdef_char_expire_day(void)
{
	return conf_set_int(&prefs_conf.char_expire_day,NULL,0);
}


extern char const * prefs_get_d2gsconffile(void)
{
	return prefs_conf.d2gsconffile;
}

static int conf_set_d2gsconffile(const char* valstr)
{
	return conf_set_str(&prefs_conf.d2gsconffile,valstr,NULL);
}

static int conf_setdef_d2gsconffile(void)
{
	return conf_set_str(&prefs_conf.d2gsconffile,NULL,"");
}


extern char const * prefs_get_charlist_sort(void)
{
	return prefs_conf.charlist_sort;
}

static int conf_set_charlist_sort(const char* valstr)
{
	return conf_set_str(&prefs_conf.charlist_sort,valstr,NULL);
}

static int conf_setdef_charlist_sort(void)
{
	return conf_set_str(&prefs_conf.charlist_sort,NULL,"none");
}


extern char const * prefs_get_charlist_sort_order(void)
{
	return prefs_conf.charlist_sort_order;
}

static int conf_set_charlist_sort_order(const char* valstr)
{
	return conf_set_str(&prefs_conf.charlist_sort_order,valstr,NULL);
}

static int conf_setdef_charlist_sort_order(void)
{
	return conf_set_str(&prefs_conf.charlist_sort_order,NULL,"ASC");
}


extern unsigned int prefs_get_max_connections(void)
{
	return prefs_conf.max_connections;
}

static int conf_set_max_connections(const char* valstr)
{
	return conf_set_int(&prefs_conf.max_connections,valstr,0);
}

static int conf_setdef_max_connections(void)
{
	return conf_set_int(&prefs_conf.max_connections,NULL,BNETD_MAX_SOCKETS);
}


extern char const * prefs_get_pidfile(void)
{
	return prefs_conf.pidfile;
}

static int conf_set_pidfile(const char* valstr)
{
	return conf_set_str(&prefs_conf.pidfile,valstr,NULL);
}

static int conf_setdef_pidfile(void)
{
	return conf_set_str(&prefs_conf.pidfile,NULL,"");
}

}

}
