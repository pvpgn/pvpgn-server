/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
 * Copyright (C) 2005           Olaf Freyer (aaron@cs.tu-berlin.de)
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

	namespace d2dbs
	{

		static struct
		{
			char const      * logfile;
			char const	* logfile_gs;
			char const	* servaddrs;
			char const	* charsave_dir;
			char const	* charinfo_dir;
			char const	* charsave_bak_dir;
			char const	* charinfo_bak_dir;
			char const	* ladder_dir;
			char const	* d2gs_list;
			unsigned int	laddersave_interval;
			unsigned int	ladderinit_time;
			char const	* loglevels;
			char const	* pidfile;
			unsigned int	shutdown_delay;
			unsigned int	shutdown_decr;
			unsigned int	idletime;
			unsigned int	keepalive_interval;
			unsigned int	timeout_checkinterval;
			unsigned int	XML_output_ladder;
			unsigned int	ladderupdate_threshold;
			unsigned int	ladder_chars_only;
			unsigned int	difficulty_hack;

		} prefs_conf;

		static int conf_set_logfile(const char* valstr);
		static int conf_setdef_logfile(void);

		static int conf_set_logfile_gs(const char* valstr);
		static int conf_setdef_logfile_gs(void);

		static int conf_set_servaddrs(const char* valstr);
		static int conf_setdef_servaddrs(void);

		static int conf_set_charsave_dir(const char* valstr);
		static int conf_setdef_charsave_dir(void);

		static int conf_set_charinfo_dir(const char* valstr);
		static int conf_setdef_charinfo_dir(void);

		static int conf_set_charsave_bak_dir(const char* valstr);
		static int conf_setdef_charsave_bak_dir(void);

		static int conf_set_charinfo_bak_dir(const char* valstr);
		static int conf_setdef_charinfo_bak_dir(void);

		static int conf_set_ladder_dir(const char* valstr);
		static int conf_setdef_ladder_dir(void);

		static int conf_set_d2gs_list(const char* valstr);
		static int conf_setdef_d2gs_list(void);

		static int conf_set_laddersave_interval(const char* valstr);
		static int conf_setdef_laddersave_interval(void);

		static int conf_set_ladderinit_time(const char* valstr);
		static int conf_setdef_ladderinit_time(void);

		static int conf_set_loglevels(const char* valstr);
		static int conf_setdef_loglevels(void);

		static int conf_set_pidfile(const char* valstr);
		static int conf_setdef_pidfile(void);

		static int conf_set_shutdown_delay(const char* valstr);
		static int conf_setdef_shutdown_delay(void);

		static int conf_set_shutdown_decr(const char* valstr);
		static int conf_setdef_shutdown_decr(void);

		static int conf_set_idletime(const char* valstr);
		static int conf_setdef_idletime(void);

		static int conf_set_keepalive_interval(const char* valstr);
		static int conf_setdef_keepalive_interval(void);

		static int conf_set_timeout_checkinterval(const char* valstr);
		static int conf_setdef_timeout_checkinterval(void);

		static int conf_set_XML_output_ladder(const char* valstr);
		static int conf_setdef_XML_output_ladder(void);

		static int conf_set_ladderupdate_threshold(const char* valstr);
		static int conf_setdef_ladderupdate_threshold(void);

		static int conf_set_ladder_chars_only(const char* valstr);
		static int conf_setdef_ladder_chars_only(void);

		static int conf_set_difficulty_hack(const char* valstr);
		static int conf_setdef_difficulty_hack(void);


		static t_conf_entry prefs_conf_table[] = {
			{ "logfile", conf_set_logfile, NULL, conf_setdef_logfile },
			{ "logfile-gs", conf_set_logfile_gs, NULL, conf_setdef_logfile_gs },
			{ "servaddrs", conf_set_servaddrs, NULL, conf_setdef_servaddrs },
			{ "charsavedir", conf_set_charsave_dir, NULL, conf_setdef_charsave_dir },
			{ "charinfodir", conf_set_charinfo_dir, NULL, conf_setdef_charinfo_dir },
			{ "bak_charsavedir", conf_set_charsave_bak_dir, NULL, conf_setdef_charsave_bak_dir },
			{ "bak_charinfodir", conf_set_charinfo_bak_dir, NULL, conf_setdef_charinfo_bak_dir },
			{ "ladderdir", conf_set_ladder_dir, NULL, conf_setdef_ladder_dir },
			{ "gameservlist", conf_set_d2gs_list, NULL, conf_setdef_d2gs_list },
			{ "laddersave_interval", conf_set_laddersave_interval, NULL, conf_setdef_laddersave_interval },
			{ "ladderinit_time", conf_set_ladderinit_time, NULL, conf_setdef_ladderinit_time },
			{ "loglevels", conf_set_loglevels, NULL, conf_setdef_loglevels },
			{ "pidfile", conf_set_pidfile, NULL, conf_setdef_pidfile },
			{ "shutdown_delay", conf_set_shutdown_delay, NULL, conf_setdef_shutdown_delay },
			{ "shutdown_decr", conf_set_shutdown_decr, NULL, conf_setdef_shutdown_decr },
			{ "idletime", conf_set_idletime, NULL, conf_setdef_idletime },
			{ "keepalive_interval", conf_set_keepalive_interval, NULL, conf_setdef_keepalive_interval },
			{ "timeout_checkinterval", conf_set_timeout_checkinterval, NULL, conf_setdef_timeout_checkinterval },
			{ "XML_ladder_output", conf_set_XML_output_ladder, NULL, conf_setdef_XML_output_ladder },
			{ "ladderupdate_threshold", conf_set_ladderupdate_threshold, NULL, conf_setdef_ladderupdate_threshold },
			{ "ladder_chars_only", conf_set_ladder_chars_only, NULL, conf_setdef_ladder_chars_only },
			{ "difficulty_hack", conf_set_difficulty_hack, NULL, conf_setdef_difficulty_hack },
			{ NULL, NULL, NULL, NULL }
		};

		extern int d2dbs_prefs_load(char const * filename)
		{
			std::FILE *fd;

			if (!filename) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
				return -1;
			}

			fd = std::fopen(filename, "rt");
			if (!fd) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not open file '%s'", filename);
				return -1;
			}

			if (conf_load_file(fd, prefs_conf_table)) {
				eventlog(eventlog_level_error, __FUNCTION__, "error loading config file '%s'", filename);
				std::fclose(fd);
				return -1;
			}

			std::fclose(fd);

			return 0;
		}

		extern int d2dbs_prefs_reload(char const * filename)
		{
			d2dbs_prefs_unload();
			if (d2dbs_prefs_load(filename) < 0) return -1;
			return 0;
		}

		extern int d2dbs_prefs_unload(void)
		{
			conf_unload(prefs_conf_table);
			return 0;
		}

		extern char const * d2dbs_prefs_get_servaddrs(void)
		{
			return prefs_conf.servaddrs;
		}

		static int conf_set_servaddrs(const char* valstr)
		{
			return conf_set_str(&prefs_conf.servaddrs, valstr, NULL);
		}

		static int conf_setdef_servaddrs(void)
		{
			return conf_set_str(&prefs_conf.servaddrs, NULL, D2DBS_SERVER_ADDRS);
		}


		extern char const * d2dbs_prefs_get_logfile(void)
		{
			return prefs_conf.logfile;
		}

		static int conf_set_logfile(const char * valstr)
		{
			return conf_set_str(&prefs_conf.logfile, valstr, NULL);
		}

		static int conf_setdef_logfile(void)
		{
			return conf_set_str(&prefs_conf.logfile, NULL, DEFAULT_LOG_FILE);
		}


		extern char const * prefs_get_logfile_gs(void)
		{
			return prefs_conf.logfile_gs;
		}

		static int conf_set_logfile_gs(const char * valstr)
		{
			return conf_set_str(&prefs_conf.logfile_gs, valstr, NULL);
		}

		static int conf_setdef_logfile_gs(void)
		{
			return conf_set_str(&prefs_conf.logfile_gs, NULL, DEFAULT_LOG_FILE);
		}


		extern char const * d2dbs_prefs_get_charsave_dir(void)
		{
			return prefs_conf.charsave_dir;
		}

		static int conf_set_charsave_dir(const char * valstr)
		{
			return conf_set_str(&prefs_conf.charsave_dir, valstr, NULL);
		}

		static int conf_setdef_charsave_dir(void)
		{
			return conf_set_str(&prefs_conf.charsave_dir, NULL, D2DBS_CHARSAVE_DIR);
		}


		extern char const * d2dbs_prefs_get_charinfo_dir(void)
		{
			return prefs_conf.charinfo_dir;
		}

		static int conf_set_charinfo_dir(const char * valstr)
		{
			return conf_set_str(&prefs_conf.charinfo_dir, valstr, NULL);
		}

		static int conf_setdef_charinfo_dir(void)
		{
			return conf_set_str(&prefs_conf.charinfo_dir, NULL, D2DBS_CHARINFO_DIR);
		}


		extern char const * prefs_get_charsave_bak_dir(void)
		{
			return prefs_conf.charsave_bak_dir;
		}

		static int conf_set_charsave_bak_dir(const char * valstr)
		{
			return conf_set_str(&prefs_conf.charsave_bak_dir, valstr, NULL);
		}

		static int conf_setdef_charsave_bak_dir(void)
		{
			return conf_set_str(&prefs_conf.charsave_bak_dir, NULL, D2DBS_CHARSAVEBAK_DIR);
		}


		extern char const * prefs_get_charinfo_bak_dir(void)
		{
			return prefs_conf.charinfo_bak_dir;
		}

		static int conf_set_charinfo_bak_dir(const char * valstr)
		{
			return conf_set_str(&prefs_conf.charinfo_bak_dir, valstr, NULL);
		}

		static int conf_setdef_charinfo_bak_dir(void)
		{
			return conf_set_str(&prefs_conf.charinfo_bak_dir, NULL, D2DBS_CHARINFOBAK_DIR);
		}


		extern char const * d2dbs_prefs_get_ladder_dir(void)
		{
			return prefs_conf.ladder_dir;
		}

		static int conf_set_ladder_dir(const char * valstr)
		{
			return conf_set_str(&prefs_conf.ladder_dir, valstr, NULL);
		}

		static int conf_setdef_ladder_dir(void)
		{
			return conf_set_str(&prefs_conf.ladder_dir, NULL, D2DBS_LADDER_DIR);
		}


		extern char const * d2dbs_prefs_get_d2gs_list(void)
		{
			return prefs_conf.d2gs_list;
		}

		static int conf_set_d2gs_list(const char * valstr)
		{
			return conf_set_str(&prefs_conf.d2gs_list, valstr, NULL);
		}

		static int conf_setdef_d2gs_list(void)
		{
			return conf_set_str(&prefs_conf.d2gs_list, NULL, D2GS_SERVER_LIST);
		}


		extern unsigned int prefs_get_laddersave_interval(void)
		{
			return prefs_conf.laddersave_interval;
		}

		static int conf_set_laddersave_interval(char const * valstr)
		{
			return conf_set_int(&prefs_conf.laddersave_interval, valstr, 0);
		}

		static int conf_setdef_laddersave_interval(void)
		{
			return conf_set_int(&prefs_conf.laddersave_interval, NULL, 3600);
		}

		extern unsigned int prefs_get_ladderinit_time(void)
		{
			return prefs_conf.ladderinit_time;
		}

		static int conf_set_ladderinit_time(char const * valstr)
		{
			return conf_set_int(&prefs_conf.ladderinit_time, valstr, 0);
		}

		static int conf_setdef_ladderinit_time(void)
		{
			return conf_set_int(&prefs_conf.ladderinit_time, NULL, 0);
		}


		extern char const * d2dbs_prefs_get_loglevels(void)
		{
			return prefs_conf.loglevels;
		}

		static int conf_set_loglevels(const char* valstr)
		{
			return conf_set_str(&prefs_conf.loglevels, valstr, NULL);
		}

		static int conf_setdef_loglevels(void)
		{
			return conf_set_str(&prefs_conf.loglevels, NULL, DEFAULT_LOG_LEVELS);
		}


		extern char const * d2dbs_prefs_get_pidfile(void)
		{
			return prefs_conf.pidfile;
		}

		static int conf_set_pidfile(const char* valstr)
		{
			return conf_set_str(&prefs_conf.pidfile, valstr, NULL);
		}

		static int conf_setdef_pidfile(void)
		{
			return conf_set_str(&prefs_conf.pidfile, NULL, "");
		}


		extern unsigned int d2dbs_prefs_get_shutdown_delay(void)
		{
			return prefs_conf.shutdown_delay;
		}

		static int conf_set_shutdown_delay(const char * valstr)
		{
			return conf_set_int(&prefs_conf.shutdown_delay, valstr, 0);
		}

		static int conf_setdef_shutdown_delay(void)
		{
			return conf_set_int(&prefs_conf.shutdown_delay, NULL, DEFAULT_SHUTDOWN_DELAY);
		}


		extern unsigned int d2dbs_prefs_get_shutdown_decr(void)
		{
			return prefs_conf.shutdown_decr;
		}

		static int conf_set_shutdown_decr(const char * valstr)
		{
			return conf_set_int(&prefs_conf.shutdown_decr, valstr, 0);
		}

		static int conf_setdef_shutdown_decr(void)
		{
			return conf_set_int(&prefs_conf.shutdown_decr, NULL, DEFAULT_SHUTDOWN_DECR);
		}


		extern unsigned int d2dbs_prefs_get_idletime(void)
		{
			return prefs_conf.idletime;
		}

		static int conf_set_idletime(const char * valstr)
		{
			return conf_set_int(&prefs_conf.idletime, valstr, 0);
		}

		static int conf_setdef_idletime(void)
		{
			return conf_set_int(&prefs_conf.idletime, NULL, DEFAULT_IDLETIME);
		}


		extern unsigned int prefs_get_keepalive_interval(void)
		{
			return prefs_conf.keepalive_interval;
		}

		static int conf_set_keepalive_interval(const char * valstr)
		{
			return conf_set_int(&prefs_conf.keepalive_interval, valstr, 0);
		}

		static int conf_setdef_keepalive_interval(void)
		{
			return conf_set_int(&prefs_conf.keepalive_interval, NULL, DEFAULT_KEEPALIVE_INTERVAL);
		}


		extern unsigned int d2dbs_prefs_get_timeout_checkinterval(void)
		{
			return prefs_conf.timeout_checkinterval;
		}

		static int conf_set_timeout_checkinterval(const char * valstr)
		{
			return conf_set_int(&prefs_conf.timeout_checkinterval, valstr, 0);
		}

		static int conf_setdef_timeout_checkinterval(void)
		{
			return conf_set_int(&prefs_conf.timeout_checkinterval, NULL, DEFAULT_TIMEOUT_CHECKINTERVAL);
		}


		extern unsigned int d2dbs_prefs_get_XML_output_ladder(void)
		{
			return prefs_conf.XML_output_ladder;
		}

		static int conf_set_XML_output_ladder(const char * valstr)
		{
			return conf_set_bool(&prefs_conf.XML_output_ladder, valstr, 0);
		}

		static int conf_setdef_XML_output_ladder(void)
		{
			return conf_set_bool(&prefs_conf.XML_output_ladder, NULL, 0);
		}

		extern unsigned int prefs_get_ladderupdate_threshold(void)
		{
			return prefs_conf.ladderupdate_threshold;
		}

		static int conf_set_ladderupdate_threshold(const char * valstr)
		{
			return conf_set_int(&prefs_conf.ladderupdate_threshold, valstr, 0);
		}

		static int conf_setdef_ladderupdate_threshold(void)
		{
			return conf_set_int(&prefs_conf.ladderupdate_threshold, NULL, DEFAULT_LADDERUPDATE_THRESHOLD);
		}


		extern unsigned int prefs_get_ladder_chars_only(void)
		{
			return prefs_conf.ladder_chars_only;
		}

		static int conf_set_ladder_chars_only(const char * valstr)
		{
			return conf_set_bool(&prefs_conf.ladder_chars_only, valstr, 0);
		}

		static int conf_setdef_ladder_chars_only(void)
		{
			return conf_set_bool(&prefs_conf.ladder_chars_only, NULL, 0);
		}


		extern unsigned int prefs_get_difficulty_hack(void)
		{
			return prefs_conf.difficulty_hack;
		}

		static int conf_set_difficulty_hack(const char * valstr)
		{
			return conf_set_int(&prefs_conf.difficulty_hack, valstr, 0);
		}

		static int conf_setdef_difficulty_hack(void)
		{
			return conf_set_int(&prefs_conf.difficulty_hack, NULL, 0);
		}

	}

}
