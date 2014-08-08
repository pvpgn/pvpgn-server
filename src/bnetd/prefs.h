/*
 * Copyright (C) 1998,1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
 * Copyright (C) 1999  Mark Baysinger (mbaysing@ucsd.edu)
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
#ifndef INCLUDED_PREFS_TYPES
#define INCLUDED_PREFS_TYPES

#endif


#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_PREFS_PROTOS
#define INCLUDED_PREFS_PROTOS

namespace pvpgn
{

	namespace bnetd
	{

		extern int prefs_load(char const * filename);
		extern void prefs_unload(void);
		extern char const * prefs_get_storage_path(void);
		extern char const * prefs_get_filedir(void);
		extern char const * prefs_get_i18ndir(void);
		extern char const * prefs_get_logfile(void);
		extern char const * prefs_get_loglevels(void);
		extern char const * prefs_get_localizefile(void);
		extern char const * prefs_get_motdfile(void);
		extern char const * prefs_get_motdw3file(void);
		extern char const * prefs_get_newsfile(void);
		extern char const * prefs_get_adfile(void);
		extern char const * prefs_get_topicfile(void);
		extern char const * prefs_get_DBlayoutfile(void);
		extern unsigned int prefs_get_user_sync_timer(void);
		extern unsigned int prefs_get_user_flush_timer(void);
		extern unsigned int prefs_get_user_step(void);
		extern char const * prefs_get_hostname(void);
		extern char const * prefs_get_servername(void);
		extern unsigned int prefs_get_track(void);
		extern char const * prefs_get_location(void);
		extern char const * prefs_get_description(void);
		extern char const * prefs_get_url(void);
		extern char const * prefs_get_contact_name(void);
		extern char const * prefs_get_contact_email(void);
		extern unsigned int prefs_get_latency(void);
		extern unsigned int prefs_get_irc_latency(void);
		extern unsigned int prefs_get_shutdown_delay(void);
		extern unsigned int prefs_get_shutdown_decr(void);
		extern unsigned int prefs_get_allow_new_accounts(void);
		extern unsigned int prefs_get_max_accounts(void);
		extern unsigned int prefs_get_kick_old_login(void);
		extern char const * prefs_get_channelfile(void);
		extern unsigned int prefs_get_ask_new_channel(void);
		extern unsigned int prefs_get_hide_pass_games(void);
		extern unsigned int prefs_get_hide_started_games(void);
		extern unsigned int prefs_get_hide_temp_channels(void);
		extern unsigned int prefs_get_hide_addr(void);
		extern unsigned int prefs_get_enable_conn_all(void);
		extern unsigned int prefs_get_udptest_port(void);
		extern char const * prefs_get_reportdir(void);
		extern unsigned int prefs_get_report_all_games(void);
		extern unsigned int prefs_get_report_diablo_games(void);
		extern char const * prefs_get_pidfile(void);
		extern char const * prefs_get_iconfile(void);
		extern char const * prefs_get_war3_iconfile(void);
		extern char const * prefs_get_star_iconfile(void);
		extern char const * prefs_get_tosfile(void);
		extern char const * prefs_get_mpqauthfile(void);
		extern char const * prefs_get_mpqfile(void);
		extern char const * prefs_get_trackserv_addrs(void);
		extern char const * prefs_get_bnetdserv_addrs(void);
		extern char const * prefs_get_irc_addrs(void);
		extern char const * prefs_get_w3route_addr(void);
		extern unsigned int prefs_get_use_keepalive(void);
		extern char const * prefs_get_ipbanfile(void);
		extern unsigned int prefs_get_discisloss(void);
		extern char const * prefs_get_helpfile(void);
		extern char const * prefs_get_fortunecmd(void);
		extern char const * prefs_get_transfile(void);
		extern unsigned int prefs_get_chanlog(void);
		extern char const * prefs_get_chanlogdir(void);
		extern unsigned int prefs_get_quota(void);
		extern unsigned int prefs_get_quota_lines(void);
		extern unsigned int prefs_get_quota_time(void);
		extern unsigned int prefs_get_quota_wrapline(void);
		extern unsigned int prefs_get_quota_maxline(void);
		extern unsigned int prefs_get_ladder_init_rating(void);
		extern unsigned int prefs_get_quota_dobae(void);
		extern char const * prefs_get_realmfile(void);
		extern char const * prefs_get_issuefile(void);
		extern char const * prefs_get_effective_user(void);
		extern char const * prefs_get_effective_group(void);
		extern unsigned int prefs_get_nullmsg(void);
		extern unsigned int prefs_get_mail_support(void);
		extern unsigned int prefs_get_mail_quota(void);
		extern char const * prefs_get_maildir(void);
		extern char const * prefs_get_log_notice(void);
		extern unsigned int prefs_get_savebyname(void);
		extern unsigned int prefs_get_skip_versioncheck(void);
		extern unsigned int prefs_get_allow_bad_version(void);
		extern unsigned int prefs_get_allow_unknown_version(void);
		extern char const * prefs_get_versioncheck_file(void);
		extern unsigned int prefs_allow_d2cs_setname(void);
		extern unsigned int prefs_get_d2cs_version(void);
		extern unsigned int prefs_get_hashtable_size(void);
		extern char const * prefs_get_telnet_addrs(void);
		extern unsigned int prefs_get_ipban_check_int(void);
		extern char const * prefs_get_version_exeinfo_match(void);
		extern unsigned int prefs_get_version_exeinfo_maxdiff(void);

		extern unsigned int prefs_get_max_concurrent_logins(void);

		/* [zap-zero] 20020616 */
		extern char const * prefs_get_mysql_host(void);
		extern char const * prefs_get_mysql_account(void);
		extern char const * prefs_get_mysql_password(void);
		extern char const * prefs_get_mysql_sock(void);
		extern char const * prefs_get_mysql_dbname(void);
		extern unsigned int prefs_get_mysql_persistent(void);

		extern char const * prefs_get_mapsfile(void);
		extern char const * prefs_get_xplevel_file(void);
		extern char const * prefs_get_xpcalc_file(void);

		extern int prefs_get_initkill_timer(void);

		//aaron
		extern int prefs_get_war3_ladder_update_secs(void);
		extern int prefs_get_output_update_secs(void);
		extern char const * prefs_get_ladderdir(void);
		extern char const * prefs_get_outputdir(void);

		extern int prefs_get_XML_output_ladder(void);
		extern int prefs_get_XML_status_output(void);

		extern char const * prefs_get_account_allowed_symbols(void);
		extern unsigned int prefs_get_account_force_username(void);

		extern char const * prefs_get_command_groups_file(void);
		extern char const * prefs_get_tournament_file(void);
		extern char const * prefs_get_customicons_file(void);
		extern char const * prefs_get_scriptdir(void);
		extern char const * prefs_get_aliasfile(void);

		extern char const * prefs_get_anongame_infos_file(void);

		extern unsigned int prefs_get_max_conns_per_IP(void);

		extern int prefs_get_max_friends(void);

		extern unsigned int prefs_get_clan_newer_time(void);
		extern unsigned int prefs_get_clan_max_members(void);
		extern unsigned int prefs_get_clan_channel_default_private(void);
		extern unsigned int prefs_get_clan_min_invites(void);

		extern unsigned int prefs_get_passfail_count(void);
		extern unsigned int prefs_get_passfail_bantime(void);
		extern unsigned int prefs_get_maxusers_per_channel(void);
		extern char const * prefs_get_supportfile(void);
		extern char const * prefs_get_allowed_clients(void);
		extern char const * prefs_get_ladder_games(void);
		extern char const * prefs_get_ladder_prefix(void);
		extern unsigned int prefs_get_max_connections(void);
		extern unsigned int prefs_get_sync_on_logoff(void);
		extern char const * prefs_get_irc_network_name(void);
		extern unsigned int prefs_get_localize_by_country(void);

		/**
		*  Westwood Online Extensions
		*/
		extern char const * prefs_get_apireg_addrs(void);
		extern char const * prefs_get_wgameres_addrs(void);
		extern char const * prefs_get_wolv1_addrs(void);
		extern char const * prefs_get_wolv2_addrs(void);
		extern char const * prefs_get_wol_timezone(void);
		extern char const * prefs_get_wol_longitude(void);
		extern char const * prefs_get_wol_latitude(void);
		extern char const * prefs_get_wol_autoupdate_serverhost(void);
		extern char const * prefs_get_wol_autoupdate_username(void);
		extern char const * prefs_get_wol_autoupdate_password(void);
	}

}

#endif
#endif
