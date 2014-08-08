/*
 * Copyright (C) 1998,1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
 * Copyright (C) 1999  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 2004,2005 Dizzy
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
#define PREFS_INTERNAL_ACCESS
#include "prefs.h"

#include <cstdio>

#include "common/conf.h"
#include "common/eventlog.h"
#include "common/setup_after.h"

#define NONE 0

namespace pvpgn
{

	namespace bnetd
	{

		static struct {
			/* files and paths */
			char const * filedir;
			char const * i18ndir;
			char const * storage_path;
			char const * logfile;
			char const * loglevels;
			char const * localizefile;
			char const * motdfile;
			char const * motdw3file;
			char const * newsfile;
			char const * channelfile;
			char const * pidfile;
			char const * adfile;
			char const * topicfile;
			char const * DBlayoutfile;

			unsigned int usersync;
			unsigned int userflush;
			unsigned int userstep;

			char const * servername;
			char const * hostname;

			unsigned int track;
			char const * location;
			char const * description;
			char const * url;
			char const * contact_name;
			char const * contact_email;
			unsigned int latency;
			unsigned int irc_latency;
			unsigned int shutdown_delay;
			unsigned int shutdown_decr;
			unsigned int new_accounts;
			unsigned int max_accounts;
			unsigned int kick_old_login;
			unsigned int ask_new_channel;
			unsigned int hide_pass_games;
			unsigned int hide_started_games;
			unsigned int hide_temp_channels;
			unsigned int hide_addr;
			unsigned int enable_conn_all;
			char const * reportdir;
			unsigned int report_all_games;
			unsigned int report_diablo_games;
			char const * iconfile;
			char const * war3_iconfile;
			char const * star_iconfile;
			char const * tosfile;
			char const * mpqfile;
			char const * trackaddrs;
			char const * servaddrs;
			char const * w3routeaddr;
			char const * ircaddrs;
			unsigned int use_keepalive;
			unsigned int udptest_port;
			char const * ipbanfile;
			unsigned int disc_is_loss;
			char const * helpfile;
			char const * fortunecmd;
			char const * transfile;
			unsigned int chanlog;
			char const * chanlogdir;
			unsigned int quota;
			unsigned int quota_lines;
			unsigned int quota_time;
			unsigned int quota_wrapline;
			unsigned int quota_maxline;
			unsigned int ladder_init_rating;
			unsigned int quota_dobae;
			char const * realmfile;
			char const * issuefile;
			char const * effective_user;
			char const * effective_group;
			unsigned int nullmsg;
			unsigned int mail_support;
			unsigned int mail_quota;
			char const * maildir;
			char const * log_notice;
			unsigned int savebyname;
			unsigned int skip_versioncheck;
			unsigned int allow_bad_version;
			unsigned int allow_unknown_version;
			char const * versioncheck_file;
			unsigned int d2cs_version;
			unsigned int allow_d2cs_setname;
			unsigned int hashtable_size;
			char const * telnetaddrs;
			unsigned int ipban_check_int;
			char const * version_exeinfo_match;
			unsigned int version_exeinfo_maxdiff;
			unsigned int max_concurrent_logins;
			char const * mapsfile;
			char const * xplevelfile;
			char const * xpcalcfile;
			unsigned int initkill_timer;
			unsigned int war3_ladder_update_secs;
			unsigned int output_update_secs;
			char const * ladderdir;
			char const * statusdir;
			unsigned int XML_output_ladder;
			unsigned int XML_status_output;
			char const * account_allowed_symbols;
			unsigned int account_force_username;
			char const * command_groups_file;
			char const * tournament_file;
			char const * customicons_file;
			char const * scriptdir;
			char const * aliasfile;
			char const * anongame_infos_file;
			unsigned int max_conns_per_IP;
			unsigned int max_friends;
			unsigned int clan_newer_time;
			unsigned int clan_max_members;
			unsigned int clan_channel_default_private;
			unsigned int clan_min_invites;
			unsigned int passfail_count;
			unsigned int passfail_bantime;
			unsigned int maxusers_per_channel;
			char const * supportfile;
			char const * allowed_clients;
			char const * ladder_games;
			char const * ladder_prefix;
			unsigned int max_connections;
			unsigned int sync_on_logoff;
			char const * irc_network_name;
			unsigned int localize_by_country;

			char const * apiregaddrs;
			char const * wolv1addrs;
			char const * wolv2addrs;
			char const * wgameresaddrs;
			char const * woltimezone;
			char const * wollongitude;
			char const * wollatitude;
			char const * wol_autoupdate_serverhost;
			char const * wol_autoupdate_username;
			char const * wol_autoupdate_password;
		} prefs_runtime_config;

		static int conf_set_filedir(const char *valstr);
		static const char *conf_get_filedir(void);
		static int conf_setdef_filedir(void);

		static int conf_set_i18ndir(const char *valstr);
		static const char *conf_get_i18ndir(void);
		static int conf_setdef_i18ndir(void);

		static int conf_set_storage_path(const char *valstr);
		static const char *conf_get_storage_path(void);
		static int conf_setdef_storage_path(void);

		static int conf_set_logfile(const char *valstr);
		static const char *conf_get_logfile(void);
		static int conf_setdef_logfile(void);

		static int conf_set_loglevels(const char *valstr);
		static const char *conf_get_loglevels(void);
		static int conf_setdef_loglevels(void);

		static int conf_set_localizefile(const char *valstr);
		static const char *conf_get_localizefile(void);
		static int conf_setdef_localizefile(void);

		static int conf_set_motdfile(const char *valstr);
		static const char *conf_get_motdfile(void);
		static int conf_setdef_motdfile(void);

		static int conf_set_motdw3file(const char *valstr);
		static const char *conf_get_motdw3file(void);
		static int conf_setdef_motdw3file(void);

		static int conf_set_newsfile(const char *valstr);
		static const char *conf_get_newsfile(void);
		static int conf_setdef_newsfile(void);

		static int conf_set_channelfile(const char *valstr);
		static const char *conf_get_channelfile(void);
		static int conf_setdef_channelfile(void);

		static int conf_set_pidfile(const char *valstr);
		static const char *conf_get_pidfile(void);
		static int conf_setdef_pidfile(void);

		static int conf_set_adfile(const char *valstr);
		static const char *conf_get_adfile(void);
		static int conf_setdef_adfile(void);

		static int conf_set_topicfile(const char *valstr);
		static const char *conf_get_topicfile(void);
		static int conf_setdef_topicfile(void);

		static int conf_set_DBlayoutfile(const char *valstr);
		static const char *conf_get_DBlayoutfile(void);
		static int conf_setdef_DBlayoutfile(void);

		static int conf_set_supportfile(const char *valstr);
		static const char *conf_get_supportfile(void);
		static int conf_setdef_supportfile(void);

		static int conf_set_usersync(const char *valstr);
		static const char *conf_get_usersync(void);
		static int conf_setdef_usersync(void);

		static int conf_set_userflush(const char *valstr);
		static const char *conf_get_userflush(void);
		static int conf_setdef_userflush(void);

		static int conf_set_userstep(const char *valstr);
		static const char *conf_get_userstep(void);
		static int conf_setdef_userstep(void);

		static int conf_set_servername(const char *valstr);
		static const char *conf_get_servername(void);
		static int conf_setdef_servername(void);

		static int conf_set_hostname(const char *valstr);
		static const char *conf_get_hostname(void);
		static int conf_setdef_hostname(void);

		static int conf_set_track(const char *valstr);
		static const char *conf_get_track(void);
		static int conf_setdef_track(void);

		static int conf_set_location(const char *valstr);
		static const char *conf_get_location(void);
		static int conf_setdef_location(void);

		static int conf_set_description(const char *valstr);
		static const char *conf_get_description(void);
		static int conf_setdef_description(void);

		static int conf_set_url(const char *valstr);
		static const char *conf_get_url(void);
		static int conf_setdef_url(void);

		static int conf_set_contact_name(const char *valstr);
		static const char *conf_get_contact_name(void);
		static int conf_setdef_contact_name(void);

		static int conf_set_contact_email(const char *valstr);
		static const char *conf_get_contact_email(void);
		static int conf_setdef_contact_email(void);

		static int conf_set_latency(const char *valstr);
		static const char *conf_get_latency(void);
		static int conf_setdef_latency(void);

		static int conf_set_irc_latency(const char *valstr);
		static const char *conf_get_irc_latency(void);
		static int conf_setdef_irc_latency(void);

		static int conf_set_shutdown_delay(const char *valstr);
		static const char *conf_get_shutdown_delay(void);
		static int conf_setdef_shutdown_delay(void);

		static int conf_set_shutdown_decr(const char *valstr);
		static const char *conf_get_shutdown_decr(void);
		static int conf_setdef_shutdown_decr(void);

		static int conf_set_new_accounts(const char *valstr);
		static const char *conf_get_new_accounts(void);
		static int conf_setdef_new_accounts(void);

		static int conf_set_max_accounts(const char *valstr);
		static const char *conf_get_max_accounts(void);
		static int conf_setdef_max_accounts(void);

		static int conf_set_kick_old_login(const char *valstr);
		static const char *conf_get_kick_old_login(void);
		static int conf_setdef_kick_old_login(void);

		static int conf_set_ask_new_channel(const char *valstr);
		static const char *conf_get_ask_new_channel(void);
		static int conf_setdef_ask_new_channel(void);

		static int conf_set_hide_pass_games(const char *valstr);
		static const char *conf_get_hide_pass_games(void);
		static int conf_setdef_hide_pass_games(void);

		static int conf_set_hide_started_games(const char *valstr);
		static const char *conf_get_hide_started_games(void);
		static int conf_setdef_hide_started_games(void);

		static int conf_set_hide_temp_channels(const char *valstr);
		static const char *conf_get_hide_temp_channels(void);
		static int conf_setdef_hide_temp_channels(void);

		static int conf_set_hide_addr(const char *valstr);
		static const char *conf_get_hide_addr(void);
		static int conf_setdef_hide_addr(void);

		static int conf_set_enable_conn_all(const char *valstr);
		static const char *conf_get_enable_conn_all(void);
		static int conf_setdef_enable_conn_all(void);

		static int conf_set_reportdir(const char *valstr);
		static const char *conf_get_reportdir(void);
		static int conf_setdef_reportdir(void);

		static int conf_set_report_all_games(const char *valstr);
		static const char *conf_get_report_all_games(void);
		static int conf_setdef_report_all_games(void);

		static int conf_set_report_diablo_games(const char *valstr);
		static const char *conf_get_report_diablo_games(void);
		static int conf_setdef_report_diablo_games(void);

		static int conf_set_iconfile(const char *valstr);
		static const char *conf_get_iconfile(void);
		static int conf_setdef_iconfile(void);

		static int conf_set_war3_iconfile(const char *valstr);
		static const char *conf_get_war3_iconfile(void);
		static int conf_setdef_war3_iconfile(void);

		static int conf_set_star_iconfile(const char *valstr);
		static const char *conf_get_star_iconfile(void);
		static int conf_setdef_star_iconfile(void);

		static int conf_set_tosfile(const char *valstr);
		static const char *conf_get_tosfile(void);
		static int conf_setdef_tosfile(void);

		static int conf_set_mpqfile(const char *valstr);
		static const char *conf_get_mpqfile(void);
		static int conf_setdef_mpqfile(void);

		static int conf_set_trackaddrs(const char *valstr);
		static const char *conf_get_trackaddrs(void);
		static int conf_setdef_trackaddrs(void);

		static int conf_set_servaddrs(const char *valstr);
		static const char *conf_get_servaddrs(void);
		static int conf_setdef_servaddrs(void);

		static int conf_set_w3routeaddr(const char *valstr);
		static const char *conf_get_w3routeaddr(void);
		static int conf_setdef_w3routeaddr(void);

		static int conf_set_ircaddrs(const char *valstr);
		static const char *conf_get_ircaddrs(void);
		static int conf_setdef_ircaddrs(void);

		static int conf_set_use_keepalive(const char *valstr);
		static const char *conf_get_use_keepalive(void);
		static int conf_setdef_use_keepalive(void);

		static int conf_set_udptest_port(const char *valstr);
		static const char *conf_get_udptest_port(void);
		static int conf_setdef_udptest_port(void);

		static int conf_set_ipbanfile(const char *valstr);
		static const char *conf_get_ipbanfile(void);
		static int conf_setdef_ipbanfile(void);

		static int conf_set_disc_is_loss(const char *valstr);
		static const char *conf_get_disc_is_loss(void);
		static int conf_setdef_disc_is_loss(void);

		static int conf_set_helpfile(const char *valstr);
		static const char *conf_get_helpfile(void);
		static int conf_setdef_helpfile(void);

		static int conf_set_fortunecmd(const char *valstr);
		static const char *conf_get_fortunecmd(void);
		static int conf_setdef_fortunecmd(void);

		static int conf_set_transfile(const char *valstr);
		static const char *conf_get_transfile(void);
		static int conf_setdef_transfile(void);

		static int conf_set_chanlog(const char *valstr);
		static const char *conf_get_chanlog(void);
		static int conf_setdef_chanlog(void);

		static int conf_set_chanlogdir(const char *valstr);
		static const char *conf_get_chanlogdir(void);
		static int conf_setdef_chanlogdir(void);

		static int conf_set_quota(const char *valstr);
		static const char *conf_get_quota(void);
		static int conf_setdef_quota(void);

		static int conf_set_quota_lines(const char *valstr);
		static const char *conf_get_quota_lines(void);
		static int conf_setdef_quota_lines(void);

		static int conf_set_quota_time(const char *valstr);
		static const char *conf_get_quota_time(void);
		static int conf_setdef_quota_time(void);

		static int conf_set_quota_wrapline(const char *valstr);
		static const char *conf_get_quota_wrapline(void);
		static int conf_setdef_quota_wrapline(void);

		static int conf_set_quota_maxline(const char *valstr);
		static const char *conf_get_quota_maxline(void);
		static int conf_setdef_quota_maxline(void);

		static int conf_set_ladder_init_rating(const char *valstr);
		static const char *conf_get_ladder_init_rating(void);
		static int conf_setdef_ladder_init_rating(void);

		static int conf_set_quota_dobae(const char *valstr);
		static const char *conf_get_quota_dobae(void);
		static int conf_setdef_quota_dobae(void);

		static int conf_set_realmfile(const char *valstr);
		static const char *conf_get_realmfile(void);
		static int conf_setdef_realmfile(void);

		static int conf_set_issuefile(const char *valstr);
		static const char *conf_get_issuefile(void);
		static int conf_setdef_issuefile(void);

		static int conf_set_effective_user(const char *valstr);
		static const char *conf_get_effective_user(void);
		static int conf_setdef_effective_user(void);

		static int conf_set_effective_group(const char *valstr);
		static const char *conf_get_effective_group(void);
		static int conf_setdef_effective_group(void);

		static int conf_set_nullmsg(const char *valstr);
		static const char *conf_get_nullmsg(void);
		static int conf_setdef_nullmsg(void);

		static int conf_set_mail_support(const char *valstr);
		static const char *conf_get_mail_support(void);
		static int conf_setdef_mail_support(void);

		static int conf_set_mail_quota(const char *valstr);
		static const char *conf_get_mail_quota(void);
		static int conf_setdef_mail_quota(void);

		static int conf_set_maildir(const char *valstr);
		static const char *conf_get_maildir(void);
		static int conf_setdef_maildir(void);

		static int conf_set_log_notice(const char *valstr);
		static const char *conf_get_log_notice(void);
		static int conf_setdef_log_notice(void);

		static int conf_set_savebyname(const char *valstr);
		static const char *conf_get_savebyname(void);
		static int conf_setdef_savebyname(void);

		static int conf_set_skip_versioncheck(const char *valstr);
		static const char *conf_get_skip_versioncheck(void);
		static int conf_setdef_skip_versioncheck(void);

		static int conf_set_allow_bad_version(const char *valstr);
		static const char *conf_get_allow_bad_version(void);
		static int conf_setdef_allow_bad_version(void);

		static int conf_set_allow_unknown_version(const char *valstr);
		static const char *conf_get_allow_unknown_version(void);
		static int conf_setdef_allow_unknown_version(void);

		static int conf_set_versioncheck_file(const char *valstr);
		static const char *conf_get_versioncheck_file(void);
		static int conf_setdef_versioncheck_file(void);

		static int conf_set_d2cs_version(const char *valstr);
		static const char *conf_get_d2cs_version(void);
		static int conf_setdef_d2cs_version(void);

		static int conf_set_allow_d2cs_setname(const char *valstr);
		static const char *conf_get_allow_d2cs_setname(void);
		static int conf_setdef_allow_d2cs_setname(void);

		static int conf_set_hashtable_size(const char *valstr);
		static const char *conf_get_hashtable_size(void);
		static int conf_setdef_hashtable_size(void);

		static int conf_set_telnetaddrs(const char *valstr);
		static const char *conf_get_telnetaddrs(void);
		static int conf_setdef_telnetaddrs(void);

		static int conf_set_ipban_check_int(const char *valstr);
		static const char *conf_get_ipban_check_int(void);
		static int conf_setdef_ipban_check_int(void);

		static int conf_set_version_exeinfo_match(const char *valstr);
		static const char *conf_get_version_exeinfo_match(void);
		static int conf_setdef_version_exeinfo_match(void);

		static int conf_set_version_exeinfo_maxdiff(const char *valstr);
		static const char *conf_get_version_exeinfo_maxdiff(void);
		static int conf_setdef_version_exeinfo_maxdiff(void);

		static int conf_set_max_concurrent_logins(const char *valstr);
		static const char *conf_get_max_concurrent_logins(void);
		static int conf_setdef_max_concurrent_logins(void);

		static int conf_set_mapsfile(const char *valstr);
		static const char *conf_get_mapsfile(void);
		static int conf_setdef_mapsfile(void);

		static int conf_set_xplevelfile(const char *valstr);
		static const char *conf_get_xplevelfile(void);
		static int conf_setdef_xplevelfile(void);

		static int conf_set_xpcalcfile(const char *valstr);
		static const char *conf_get_xpcalcfile(void);
		static int conf_setdef_xpcalcfile(void);

		static int conf_set_initkill_timer(const char *valstr);
		static const char *conf_get_initkill_timer(void);
		static int conf_setdef_initkill_timer(void);

		static int conf_set_war3_ladder_update_secs(const char *valstr);
		static const char *conf_get_war3_ladder_update_secs(void);
		static int conf_setdef_war3_ladder_update_secs(void);

		static int conf_set_output_update_secs(const char *valstr);
		static const char *conf_get_output_update_secs(void);
		static int conf_setdef_output_update_secs(void);

		static int conf_set_ladderdir(const char *valstr);
		static const char *conf_get_ladderdir(void);
		static int conf_setdef_ladderdir(void);

		static int conf_set_statusdir(const char *valstr);
		static const char *conf_get_statusdir(void);
		static int conf_setdef_statusdir(void);

		static int conf_set_XML_output_ladder(const char *valstr);
		static const char *conf_get_XML_output_ladder(void);
		static int conf_setdef_XML_output_ladder(void);

		static int conf_set_XML_status_output(const char *valstr);
		static const char *conf_get_XML_status_output(void);
		static int conf_setdef_XML_status_output(void);

		static int conf_set_account_allowed_symbols(const char *valstr);
		static const char *conf_get_account_allowed_symbols(void);
		static int conf_setdef_account_allowed_symbols(void);

		static int conf_set_account_force_username(const char *valstr);
		static const char *conf_get_account_force_username(void);
		static int conf_setdef_account_force_username(void);

		static int conf_set_command_groups_file(const char *valstr);
		static const char *conf_get_command_groups_file(void);
		static int conf_setdef_command_groups_file(void);

		static int conf_set_tournament_file(const char *valstr);
		static const char *conf_get_tournament_file(void);
		static int conf_setdef_tournament_file(void);

		static int conf_set_customicons_file(const char *valstr);
		static const char *conf_get_customicons_file(void);
		static int conf_setdef_customicons_file(void);

		static int conf_set_scriptdir(const char *valstr);
		static const char *conf_get_scriptdir(void);
		static int conf_setdef_scriptdir(void);

		static int conf_set_aliasfile(const char *valstr);
		static const char *conf_get_aliasfile(void);
		static int conf_setdef_aliasfile(void);

		static int conf_set_anongame_infos_file(const char *valstr);
		static const char *conf_get_anongame_infos_file(void);
		static int conf_setdef_anongame_infos_file(void);

		static int conf_set_max_conns_per_IP(const char *valstr);
		static const char *conf_get_max_conns_per_IP(void);
		static int conf_setdef_max_conns_per_IP(void);

		static int conf_set_max_friends(const char *valstr);
		static const char *conf_get_max_friends(void);
		static int conf_setdef_max_friends(void);

		static int conf_set_clan_newer_time(const char *valstr);
		static const char *conf_get_clan_newer_time(void);
		static int conf_setdef_clan_newer_time(void);

		static int conf_set_clan_max_members(const char *valstr);
		static const char *conf_get_clan_max_members(void);
		static int conf_setdef_clan_max_members(void);

		static int conf_set_clan_channel_default_private(const char *valstr);
		static const char *conf_get_clan_channel_default_private(void);
		static int conf_setdef_clan_channel_default_private(void);

		static int conf_set_clan_min_invites(const char *valstr);
		static const char *conf_get_clan_min_invites(void);
		static int conf_setdef_clan_min_invites(void);

		static int conf_set_passfail_count(const char *valstr);
		static const char *conf_get_passfail_count(void);
		static int conf_setdef_passfail_count(void);

		static int conf_set_passfail_bantime(const char *valstr);
		static const char *conf_get_passfail_bantime(void);
		static int conf_setdef_passfail_bantime(void);

		static int conf_set_maxusers_per_channel(const char *valstr);
		static const char *conf_get_maxusers_per_channel(void);
		static int conf_setdef_maxusers_per_channel(void);

		static int conf_set_allowed_clients(const char *valstr);
		static const char *conf_get_allowed_clients(void);
		static int conf_setdef_allowed_clients(void);

		static int conf_set_ladder_games(const char *valstr);
		static const char *conf_get_ladder_games(void);
		static int conf_setdef_ladder_games(void);

		static int conf_set_max_connections(const char *valstr);
		static const char *conf_get_max_connections(void);
		static int conf_setdef_max_connections(void);

		static int conf_set_sync_on_logoff(const char *valstr);
		static const char *conf_get_sync_on_logoff(void);
		static int conf_setdef_sync_on_logoff(void);

		static int conf_set_ladder_prefix(const char *valstr);
		static const char *conf_get_ladder_prefix(void);
		static int conf_setdef_ladder_prefix(void);

		static int conf_setdef_irc_network_name(void);
		static int conf_set_irc_network_name(const char *valstr);
		static const char *conf_get_irc_network_name(void);

		static int conf_set_localize_by_country(const char *valstr);
		static const char *conf_get_localize_by_country(void);
		static int conf_setdef_localize_by_country(void);


		static int conf_setdef_apireg_addrs(void);
		static int conf_set_apireg_addrs(const char *valstr);
		static const char *conf_get_apireg_addrs(void);

		static int conf_setdef_wgameres_addrs(void);
		static int conf_set_wgameres_addrs(const char *valstr);
		static const char *conf_get_wgameres_addrs(void);

		static int conf_setdef_wolv1_addrs(void);
		static int conf_set_wolv1_addrs(const char *valstr);
		static const char *conf_get_wolv1_addrs(void);

		static int conf_setdef_wolv2_addrs(void);
		static int conf_set_wolv2_addrs(const char *valstr);
		static const char *conf_get_wolv2_addrs(void);

		static int conf_set_wol_timezone(const char *valstr);
		static const char *conf_get_wol_timezone(void);
		static int conf_setdef_wol_timezone(void);

		static int conf_set_wol_longitude(const char *valstr);
		static const char *conf_get_wol_longitude(void);
		static int conf_setdef_wol_longitude(void);

		static int conf_set_wol_latitude(const char *valstr);
		static const char *conf_get_wol_latitude(void);
		static int conf_setdef_wol_latitude(void);

		static int conf_set_wol_autoupdate_serverhost(const char *valstr);
		static const char *conf_get_wol_autoupdate_serverhost(void);
		static int conf_setdef_wol_autoupdate_serverhost(void);

		static int conf_set_wol_autoupdate_username(const char *valstr);
		static const char *conf_get_wol_autoupdate_username(void);
		static int conf_setdef_wol_autoupdate_username(void);

		static int conf_set_wol_autoupdate_password(const char *valstr);
		static const char *conf_get_wol_autoupdate_password(void);
		static int conf_setdef_wol_autoupdate_password(void);

		/*    directive                 set method                     get method         */
		static t_conf_entry conf_table[] =
		{
			{ "filedir", conf_set_filedir, conf_get_filedir, conf_setdef_filedir },
			{ "i18ndir", conf_set_i18ndir, conf_get_i18ndir, conf_setdef_i18ndir },
			{ "storage_path", conf_set_storage_path, conf_get_storage_path, conf_setdef_storage_path },
			{ "logfile", conf_set_logfile, conf_get_logfile, conf_setdef_logfile },
			{ "loglevels", conf_set_loglevels, conf_get_loglevels, conf_setdef_loglevels },
			{ "localizefile", conf_set_localizefile, conf_get_localizefile, conf_setdef_localizefile },
			{ "motdfile", conf_set_motdfile, conf_get_motdfile, conf_setdef_motdfile },
			{ "motdw3file", conf_set_motdw3file, conf_get_motdw3file, conf_setdef_motdw3file },
			{ "newsfile", conf_set_newsfile, conf_get_newsfile, conf_setdef_newsfile },
			{ "channelfile", conf_set_channelfile, conf_get_channelfile, conf_setdef_channelfile },
			{ "pidfile", conf_set_pidfile, conf_get_pidfile, conf_setdef_pidfile },
			{ "adfile", conf_set_adfile, conf_get_adfile, conf_setdef_adfile },
			{ "topicfile", conf_set_topicfile, conf_get_topicfile, conf_setdef_topicfile },
			{ "DBlayoutfile", conf_set_DBlayoutfile, conf_get_DBlayoutfile, conf_setdef_DBlayoutfile },
			{ "supportfile", conf_set_supportfile, conf_get_supportfile, conf_setdef_supportfile },
			{ "usersync", conf_set_usersync, conf_get_usersync, conf_setdef_usersync },
			{ "userflush", conf_set_userflush, conf_get_userflush, conf_setdef_userflush },
			{ "userstep", conf_set_userstep, conf_get_userstep, conf_setdef_userstep },
			{ "servername", conf_set_servername, conf_get_servername, conf_setdef_servername },
			{ "hostname", conf_set_hostname, conf_get_hostname, conf_setdef_hostname },
			{ "track", conf_set_track, conf_get_track, conf_setdef_track },
			{ "location", conf_set_location, conf_get_location, conf_setdef_location },
			{ "description", conf_set_description, conf_get_description, conf_setdef_description },
			{ "url", conf_set_url, conf_get_url, conf_setdef_url },
			{ "contact_name", conf_set_contact_name, conf_get_contact_name, conf_setdef_contact_name },
			{ "contact_email", conf_set_contact_email, conf_get_contact_email, conf_setdef_contact_email },
			{ "latency", conf_set_latency, conf_get_latency, conf_setdef_latency },
			{ "irc_latency", conf_set_irc_latency, conf_get_irc_latency, conf_setdef_irc_latency },
			{ "shutdown_delay", conf_set_shutdown_delay, conf_get_shutdown_delay, conf_setdef_shutdown_delay },
			{ "shutdown_decr", conf_set_shutdown_decr, conf_get_shutdown_decr, conf_setdef_shutdown_decr },
			{ "new_accounts", conf_set_new_accounts, conf_get_new_accounts, conf_setdef_new_accounts },
			{ "max_accounts", conf_set_max_accounts, conf_get_max_accounts, conf_setdef_max_accounts },
			{ "kick_old_login", conf_set_kick_old_login, conf_get_kick_old_login, conf_setdef_kick_old_login },
			{ "ask_new_channel", conf_set_ask_new_channel, conf_get_ask_new_channel, conf_setdef_ask_new_channel },
			{ "hide_pass_games", conf_set_hide_pass_games, conf_get_hide_pass_games, conf_setdef_hide_pass_games },
			{ "hide_started_games", conf_set_hide_started_games, conf_get_hide_started_games, conf_setdef_hide_started_games },
			{ "hide_temp_channels", conf_set_hide_temp_channels, conf_get_hide_temp_channels, conf_setdef_hide_temp_channels },
			{ "hide_addr", conf_set_hide_addr, conf_get_hide_addr, conf_setdef_hide_addr },
			{ "enable_conn_all", conf_set_enable_conn_all, conf_get_enable_conn_all, conf_setdef_enable_conn_all },
			{ "reportdir", conf_set_reportdir, conf_get_reportdir, conf_setdef_reportdir },
			{ "report_all_games", conf_set_report_all_games, conf_get_report_all_games, conf_setdef_report_all_games },
			{ "report_diablo_games", conf_set_report_diablo_games, conf_get_report_diablo_games, conf_setdef_report_diablo_games },
			{ "iconfile", conf_set_iconfile, conf_get_iconfile, conf_setdef_iconfile },
			{ "war3_iconfile", conf_set_war3_iconfile, conf_get_war3_iconfile, conf_setdef_war3_iconfile },
			{ "star_iconfile", conf_set_star_iconfile, conf_get_star_iconfile, conf_setdef_star_iconfile },
			{ "tosfile", conf_set_tosfile, conf_get_tosfile, conf_setdef_tosfile },
			{ "mpqfile", conf_set_mpqfile, conf_get_mpqfile, conf_setdef_mpqfile },
			{ "trackaddrs", conf_set_trackaddrs, conf_get_trackaddrs, conf_setdef_trackaddrs },
			{ "servaddrs", conf_set_servaddrs, conf_get_servaddrs, conf_setdef_servaddrs },
			{ "w3routeaddr", conf_set_w3routeaddr, conf_get_w3routeaddr, conf_setdef_w3routeaddr },
			{ "ircaddrs", conf_set_ircaddrs, conf_get_ircaddrs, conf_setdef_ircaddrs },
			{ "use_keepalive", conf_set_use_keepalive, conf_get_use_keepalive, conf_setdef_use_keepalive },
			{ "udptest_port", conf_set_udptest_port, conf_get_udptest_port, conf_setdef_udptest_port },
			{ "ipbanfile", conf_set_ipbanfile, conf_get_ipbanfile, conf_setdef_ipbanfile },
			{ "disc_is_loss", conf_set_disc_is_loss, conf_get_disc_is_loss, conf_setdef_disc_is_loss },
			{ "helpfile", conf_set_helpfile, conf_get_helpfile, conf_setdef_helpfile },
			{ "fortunecmd", conf_set_fortunecmd, conf_get_fortunecmd, conf_setdef_fortunecmd },
			{ "transfile", conf_set_transfile, conf_get_transfile, conf_setdef_transfile },
			{ "chanlog", conf_set_chanlog, conf_get_chanlog, conf_setdef_chanlog },
			{ "chanlogdir", conf_set_chanlogdir, conf_get_chanlogdir, conf_setdef_chanlogdir },
			{ "quota", conf_set_quota, conf_get_quota, conf_setdef_quota },
			{ "quota_lines", conf_set_quota_lines, conf_get_quota_lines, conf_setdef_quota_lines },
			{ "quota_time", conf_set_quota_time, conf_get_quota_time, conf_setdef_quota_time },
			{ "quota_wrapline", conf_set_quota_wrapline, conf_get_quota_wrapline, conf_setdef_quota_wrapline },
			{ "quota_maxline", conf_set_quota_maxline, conf_get_quota_maxline, conf_setdef_quota_maxline },
			{ "ladder_init_rating", conf_set_ladder_init_rating, conf_get_ladder_init_rating, conf_setdef_ladder_init_rating },
			{ "quota_dobae", conf_set_quota_dobae, conf_get_quota_dobae, conf_setdef_quota_dobae },
			{ "realmfile", conf_set_realmfile, conf_get_realmfile, conf_setdef_realmfile },
			{ "issuefile", conf_set_issuefile, conf_get_issuefile, conf_setdef_issuefile },
			{ "effective_user", conf_set_effective_user, conf_get_effective_user, conf_setdef_effective_user },
			{ "effective_group", conf_set_effective_group, conf_get_effective_group, conf_setdef_effective_group },
			{ "nullmsg", conf_set_nullmsg, conf_get_nullmsg, conf_setdef_nullmsg },
			{ "mail_support", conf_set_mail_support, conf_get_mail_support, conf_setdef_mail_support },
			{ "mail_quota", conf_set_mail_quota, conf_get_mail_quota, conf_setdef_mail_quota },
			{ "maildir", conf_set_maildir, conf_get_maildir, conf_setdef_maildir },
			{ "log_notice", conf_set_log_notice, conf_get_log_notice, conf_setdef_log_notice },
			{ "savebyname", conf_set_savebyname, conf_get_savebyname, conf_setdef_savebyname },
			{ "skip_versioncheck", conf_set_skip_versioncheck, conf_get_skip_versioncheck, conf_setdef_skip_versioncheck },
			{ "allow_bad_version", conf_set_allow_bad_version, conf_get_allow_bad_version, conf_setdef_allow_bad_version },
			{ "allow_unknown_version", conf_set_allow_unknown_version, conf_get_allow_unknown_version, conf_setdef_allow_unknown_version },
			{ "versioncheck_file", conf_set_versioncheck_file, conf_get_versioncheck_file, conf_setdef_versioncheck_file },
			{ "d2cs_version", conf_set_d2cs_version, conf_get_d2cs_version, conf_setdef_d2cs_version },
			{ "allow_d2cs_setname", conf_set_allow_d2cs_setname, conf_get_allow_d2cs_setname, conf_setdef_allow_d2cs_setname },
			{ "hashtable_size", conf_set_hashtable_size, conf_get_hashtable_size, conf_setdef_hashtable_size },
			{ "telnetaddrs", conf_set_telnetaddrs, conf_get_telnetaddrs, conf_setdef_telnetaddrs },
			{ "ipban_check_int", conf_set_ipban_check_int, conf_get_ipban_check_int, conf_setdef_ipban_check_int },
			{ "version_exeinfo_match", conf_set_version_exeinfo_match, conf_get_version_exeinfo_match, conf_setdef_version_exeinfo_match },
			{ "version_exeinfo_maxdiff", conf_set_version_exeinfo_maxdiff, conf_get_version_exeinfo_maxdiff, conf_setdef_version_exeinfo_maxdiff },
			{ "max_concurrent_logins", conf_set_max_concurrent_logins, conf_get_max_concurrent_logins, conf_setdef_max_concurrent_logins },
			{ "mapsfile", conf_set_mapsfile, conf_get_mapsfile, conf_setdef_mapsfile },
			{ "xplevelfile", conf_set_xplevelfile, conf_get_xplevelfile, conf_setdef_xplevelfile },
			{ "xpcalcfile", conf_set_xpcalcfile, conf_get_xpcalcfile, conf_setdef_xpcalcfile },
			{ "initkill_timer", conf_set_initkill_timer, conf_get_initkill_timer, conf_setdef_initkill_timer },
			{ "war3_ladder_update_secs", conf_set_war3_ladder_update_secs, conf_get_war3_ladder_update_secs, conf_setdef_war3_ladder_update_secs },
			{ "output_update_secs", conf_set_output_update_secs, conf_get_output_update_secs, conf_setdef_output_update_secs },
			{ "ladderdir", conf_set_ladderdir, conf_get_ladderdir, conf_setdef_ladderdir },
			{ "statusdir", conf_set_statusdir, conf_get_statusdir, conf_setdef_statusdir },
			{ "XML_output_ladder", conf_set_XML_output_ladder, conf_get_XML_output_ladder, conf_setdef_XML_output_ladder },
			{ "XML_status_output", conf_set_XML_status_output, conf_get_XML_status_output, conf_setdef_XML_status_output },
			{ "account_allowed_symbols", conf_set_account_allowed_symbols, conf_get_account_allowed_symbols, conf_setdef_account_allowed_symbols },
			{ "account_force_username", conf_set_account_force_username, conf_get_account_force_username, conf_setdef_account_force_username },
			{ "command_groups_file", conf_set_command_groups_file, conf_get_command_groups_file, conf_setdef_command_groups_file },
			{ "tournament_file", conf_set_tournament_file, conf_get_tournament_file, conf_setdef_tournament_file },
			{ "customicons_file", conf_set_customicons_file, conf_get_customicons_file, conf_setdef_customicons_file },
			{ "scriptdir", conf_set_scriptdir, conf_get_scriptdir, conf_setdef_scriptdir },
			{ "aliasfile", conf_set_aliasfile, conf_get_aliasfile, conf_setdef_aliasfile },
			{ "anongame_infos_file", conf_set_anongame_infos_file, conf_get_anongame_infos_file, conf_setdef_anongame_infos_file },
			{ "max_conns_per_IP", conf_set_max_conns_per_IP, conf_get_max_conns_per_IP, conf_setdef_max_conns_per_IP },
			{ "max_friends", conf_set_max_friends, conf_get_max_friends, conf_setdef_max_friends },
			{ "clan_newer_time", conf_set_clan_newer_time, conf_get_clan_newer_time, conf_setdef_clan_newer_time },
			{ "clan_max_members", conf_set_clan_max_members, conf_get_clan_max_members, conf_setdef_clan_max_members },
			{ "clan_channel_default_private", conf_set_clan_channel_default_private, conf_get_clan_channel_default_private, conf_setdef_clan_channel_default_private },
			{ "clan_min_invites", conf_set_clan_min_invites, conf_get_clan_min_invites, conf_setdef_clan_min_invites },
			{ "passfail_count", conf_set_passfail_count, conf_get_passfail_count, conf_setdef_passfail_count },
			{ "passfail_bantime", conf_set_passfail_bantime, conf_get_passfail_bantime, conf_setdef_passfail_bantime },
			{ "maxusers_per_channel", conf_set_maxusers_per_channel, conf_get_maxusers_per_channel, conf_setdef_maxusers_per_channel },
			{ "allowed_clients", conf_set_allowed_clients, conf_get_allowed_clients, conf_setdef_allowed_clients },
			{ "ladder_games", conf_set_ladder_games, conf_get_ladder_games, conf_setdef_ladder_games },
			{ "max_connections", conf_set_max_connections, conf_get_max_connections, conf_setdef_max_connections },
			{ "sync_on_logoff", conf_set_sync_on_logoff, conf_get_sync_on_logoff, conf_setdef_sync_on_logoff },
			{ "ladder_prefix", conf_set_ladder_prefix, conf_get_ladder_prefix, conf_setdef_ladder_prefix },
			{ "irc_network_name", conf_set_irc_network_name, conf_get_irc_network_name, conf_setdef_irc_network_name },
			{ "localize_by_country", conf_set_localize_by_country, conf_get_localize_by_country, conf_setdef_localize_by_country },

			{ "apiregaddrs", conf_set_apireg_addrs, conf_get_apireg_addrs, conf_setdef_apireg_addrs },
			{ "wgameresaddrs", conf_set_wgameres_addrs, conf_get_wgameres_addrs, conf_setdef_wgameres_addrs },
			{ "wolv1addrs", conf_set_wolv1_addrs, conf_get_wolv1_addrs, conf_setdef_wolv1_addrs },
			{ "wolv2addrs", conf_set_wolv2_addrs, conf_get_wolv2_addrs, conf_setdef_wolv2_addrs },
			{ "woltimezone", conf_set_wol_timezone, conf_get_wol_timezone, conf_setdef_wol_timezone },
			{ "wollongitude", conf_set_wol_longitude, conf_get_wol_longitude, conf_setdef_wol_longitude },
			{ "wollatitude", conf_set_wol_latitude, conf_get_wol_latitude, conf_setdef_wol_latitude },
			{ "wol_autoupdate_serverhost", conf_set_wol_autoupdate_serverhost, conf_get_wol_autoupdate_serverhost, conf_setdef_wol_autoupdate_serverhost },
			{ "wol_autoupdate_username", conf_set_wol_autoupdate_username, conf_get_wol_autoupdate_username, conf_setdef_wol_autoupdate_username },
			{ "wol_autoupdate_password", conf_set_wol_autoupdate_password, conf_get_wol_autoupdate_password, conf_setdef_wol_autoupdate_password },

			{ NULL, NULL, NULL, NONE },
		};

		extern int prefs_load(char const * filename)
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

			if (conf_load_file(fd, conf_table)) {
				eventlog(eventlog_level_error, __FUNCTION__, "error loading config file '%s'", filename);
				std::fclose(fd);
				return -1;
			}

			std::fclose(fd);

			return 0;
		}


		extern void prefs_unload(void)
		{
			conf_unload(conf_table);
		}

		extern char const * prefs_get_storage_path(void)
		{
			return prefs_runtime_config.storage_path;
		}

		static int conf_set_storage_path(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.storage_path, valstr, NULL);
		}

		static int conf_setdef_storage_path(void)
		{
			return conf_set_str(&prefs_runtime_config.storage_path, NULL, BNETD_STORAGE_PATH);
		}

		static const char* conf_get_storage_path(void)
		{
			return prefs_runtime_config.storage_path;
		}


		extern char const * prefs_get_filedir(void)
		{
			return prefs_runtime_config.filedir;
		}

		static int conf_set_filedir(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.filedir, valstr, NULL);
		}

		static int conf_setdef_filedir(void)
		{
			return conf_set_str(&prefs_runtime_config.filedir, NULL, BNETD_FILE_DIR);
		}

		static const char* conf_get_filedir(void)
		{
			return prefs_runtime_config.filedir;
		}


		extern char const * prefs_get_i18ndir(void)
		{
			return prefs_runtime_config.i18ndir;
		}

		static int conf_set_i18ndir(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.i18ndir, valstr, NULL);
		}

		static int conf_setdef_i18ndir(void)
		{
			return conf_set_str(&prefs_runtime_config.i18ndir, NULL, BNETD_I18N_DIR);
		}

		static const char* conf_get_i18ndir(void)
		{
			return prefs_runtime_config.i18ndir;
		}


		extern char const * prefs_get_logfile(void)
		{
			return prefs_runtime_config.logfile;
		}

		static int conf_set_logfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.logfile, valstr, NULL);
		}

		static int conf_setdef_logfile(void)
		{
			return conf_set_str(&prefs_runtime_config.logfile, NULL, BNETD_LOG_FILE);
		}

		static const char* conf_get_logfile(void)
		{
			return prefs_runtime_config.logfile;
		}


		extern char const * prefs_get_loglevels(void)
		{
			return prefs_runtime_config.loglevels;
		}

		static int conf_set_loglevels(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.loglevels, valstr, NULL);
		}

		static int conf_setdef_loglevels(void)
		{
			return conf_set_str(&prefs_runtime_config.loglevels, NULL, BNETD_LOG_LEVELS);
		}

		static const char* conf_get_loglevels(void)
		{
			return prefs_runtime_config.loglevels;
		}


		extern char const * prefs_get_localizefile(void)
		{
			return prefs_runtime_config.localizefile;
		}

		static int conf_set_localizefile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.localizefile, valstr, NULL);
		}

		static int conf_setdef_localizefile(void)
		{
			return conf_set_str(&prefs_runtime_config.localizefile, NULL, BNETD_LOCALIZE_FILE);
		}

		static const char* conf_get_localizefile(void)
		{
			return prefs_runtime_config.localizefile;
		}


		extern char const * prefs_get_motdfile(void)
		{
			return prefs_runtime_config.motdfile;
		}

		static int conf_set_motdfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.motdfile, valstr, NULL);
		}

		static int conf_setdef_motdfile(void)
		{
			return conf_set_str(&prefs_runtime_config.motdfile, NULL, BNETD_MOTD_FILE);
		}

		static const char* conf_get_motdfile(void)
		{
			return prefs_runtime_config.motdfile;
		}


		extern char const * prefs_get_motdw3file(void)
		{
			return prefs_runtime_config.motdw3file;
		}

		static int conf_set_motdw3file(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.motdw3file, valstr, NULL);
		}

		static int conf_setdef_motdw3file(void)
		{
			return conf_set_str(&prefs_runtime_config.motdw3file, NULL, BNETD_MOTDW3_FILE);
		}

		static const char* conf_get_motdw3file(void)
		{
			return prefs_runtime_config.motdw3file;
		}


		extern char const * prefs_get_newsfile(void)
		{
			return prefs_runtime_config.newsfile;
		}

		static int conf_set_newsfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.newsfile, valstr, NULL);
		}

		static int conf_setdef_newsfile(void)
		{
			return conf_set_str(&prefs_runtime_config.newsfile, NULL, BNETD_NEWS_FILE);
		}

		static const char* conf_get_newsfile(void)
		{
			return prefs_runtime_config.newsfile;
		}


		extern char const * prefs_get_adfile(void)
		{
			return prefs_runtime_config.adfile;
		}

		static int conf_set_adfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.adfile, valstr, NULL);
		}

		static int conf_setdef_adfile(void)
		{
			return conf_set_str(&prefs_runtime_config.adfile, NULL, BNETD_AD_FILE);
		}

		static const char* conf_get_adfile(void)
		{
			return prefs_runtime_config.adfile;
		}


		extern char const * prefs_get_topicfile(void)
		{
			return prefs_runtime_config.topicfile;
		}

		static int conf_set_topicfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.topicfile, valstr, NULL);
		}

		static int conf_setdef_topicfile(void)
		{
			return conf_set_str(&prefs_runtime_config.topicfile, NULL, BNETD_TOPIC_FILE);
		}

		static const char* conf_get_topicfile(void)
		{
			return prefs_runtime_config.topicfile;
		}


		extern char const * prefs_get_DBlayoutfile(void)
		{
			return prefs_runtime_config.DBlayoutfile;
		}

		static int conf_set_DBlayoutfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.DBlayoutfile, valstr, NULL);
		}

		static int conf_setdef_DBlayoutfile(void)
		{
			return conf_set_str(&prefs_runtime_config.DBlayoutfile, NULL, BNETD_DBLAYOUT_FILE);
		}

		static const char* conf_get_DBlayoutfile(void)
		{
			return prefs_runtime_config.DBlayoutfile;
		}


		extern unsigned int prefs_get_user_sync_timer(void)
		{
			return prefs_runtime_config.usersync;
		}

		static int conf_set_usersync(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.usersync, valstr, 0);
		}

		static int conf_setdef_usersync(void)
		{
			return conf_set_int(&prefs_runtime_config.usersync, NULL, BNETD_USERSYNC);
		}

		static const char* conf_get_usersync(void)
		{
			return conf_get_int(prefs_runtime_config.usersync);
		}


		extern unsigned int prefs_get_user_flush_timer(void)
		{
			return prefs_runtime_config.userflush;
		}

		static int conf_set_userflush(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.userflush, valstr, 0);
		}

		static int conf_setdef_userflush(void)
		{
			return conf_set_int(&prefs_runtime_config.userflush, NULL, BNETD_USERFLUSH);
		}

		static const char* conf_get_userflush(void)
		{
			return conf_get_int(prefs_runtime_config.userflush);
		}


		extern unsigned int prefs_get_user_step(void)
		{
			return prefs_runtime_config.userstep;
		}

		static int conf_set_userstep(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.userstep, valstr, 0);
		}

		static int conf_setdef_userstep(void)
		{
			return conf_set_int(&prefs_runtime_config.userstep, NULL, BNETD_USERSTEP);
		}

		static const char* conf_get_userstep(void)
		{
			return conf_get_int(prefs_runtime_config.userstep);
		}


		extern char const * prefs_get_servername(void)
		{
			return prefs_runtime_config.servername;
		}

		static int conf_set_servername(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.servername, valstr, NULL);
		}

		static int conf_setdef_servername(void)
		{
			return conf_set_str(&prefs_runtime_config.servername, NULL, BNETD_SERVERNAME);
		}

		static const char* conf_get_servername(void)
		{
			return prefs_runtime_config.servername;
		}


		extern char const * prefs_get_hostname(void)
		{
			return prefs_runtime_config.hostname;
		}

		static int conf_set_hostname(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.hostname, valstr, NULL);
		}

		static int conf_setdef_hostname(void)
		{
			return conf_set_str(&prefs_runtime_config.hostname, NULL, "");
		}

		static const char* conf_get_hostname(void)
		{
			return prefs_runtime_config.hostname;
		}


		extern unsigned int prefs_get_track(void)
		{
			return prefs_runtime_config.track;
		}

		static int conf_set_track(const char *valstr)
		{
			unsigned int rez;

			conf_set_int(&prefs_runtime_config.track, valstr, 0);
			rez = prefs_runtime_config.track;
			if (rez > 0 && rez < 60) rez = 60;
			return 0;
		}

		static int conf_setdef_track(void)
		{
			return conf_set_int(&prefs_runtime_config.track, NULL, BNETD_TRACK_TIME);
		}

		static const char* conf_get_track(void)
		{
			return conf_get_int(prefs_runtime_config.track);
		}


		extern char const * prefs_get_location(void)
		{
			return prefs_runtime_config.location;
		}

		static int conf_set_location(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.location, valstr, NULL);
		}

		static int conf_setdef_location(void)
		{
			return conf_set_str(&prefs_runtime_config.location, NULL, "");
		}

		static const char* conf_get_location(void)
		{
			return prefs_runtime_config.location;
		}


		extern char const * prefs_get_description(void)
		{
			return prefs_runtime_config.description;
		}

		static int conf_set_description(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.description, valstr, NULL);
		}

		static int conf_setdef_description(void)
		{
			return conf_set_str(&prefs_runtime_config.description, NULL, "");
		}

		static const char* conf_get_description(void)
		{
			return prefs_runtime_config.description;
		}


		extern char const * prefs_get_url(void)
		{
			return prefs_runtime_config.url;
		}

		static int conf_set_url(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.url, valstr, NULL);
		}

		static int conf_setdef_url(void)
		{
			return conf_set_str(&prefs_runtime_config.url, NULL, "");
		}

		static const char* conf_get_url(void)
		{
			return prefs_runtime_config.url;
		}


		extern char const * prefs_get_contact_name(void)
		{
			return prefs_runtime_config.contact_name;
		}

		static int conf_set_contact_name(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.contact_name, valstr, NULL);
		}

		static int conf_setdef_contact_name(void)
		{
			return conf_set_str(&prefs_runtime_config.contact_name, NULL, "");
		}

		static const char* conf_get_contact_name(void)
		{
			return prefs_runtime_config.contact_name;
		}


		extern char const * prefs_get_contact_email(void)
		{
			return prefs_runtime_config.contact_email;
		}

		static int conf_set_contact_email(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.contact_email, valstr, NULL);
		}

		static int conf_setdef_contact_email(void)
		{
			return conf_set_str(&prefs_runtime_config.contact_email, NULL, "");
		}

		static const char* conf_get_contact_email(void)
		{
			return prefs_runtime_config.contact_email;
		}


		extern unsigned int prefs_get_latency(void)
		{
			return prefs_runtime_config.latency;
		}

		static int conf_set_latency(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.latency, valstr, 0);
		}

		static int conf_setdef_latency(void)
		{
			return conf_set_int(&prefs_runtime_config.latency, NULL, BNETD_LATENCY);
		}

		static const char* conf_get_latency(void)
		{
			return conf_get_int(prefs_runtime_config.latency);
		}


		extern unsigned int prefs_get_irc_latency(void)
		{
			return prefs_runtime_config.irc_latency;
		}

		static int conf_set_irc_latency(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.irc_latency, valstr, 0);
		}

		static int conf_setdef_irc_latency(void)
		{
			return conf_set_int(&prefs_runtime_config.irc_latency, NULL, BNETD_IRC_LATENCY);
		}

		static const char* conf_get_irc_latency(void)
		{
			return conf_get_int(prefs_runtime_config.irc_latency);
		}


		extern unsigned int prefs_get_shutdown_delay(void)
		{
			return prefs_runtime_config.shutdown_delay;
		}

		static int conf_set_shutdown_delay(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.shutdown_delay, valstr, 0);
		}

		static int conf_setdef_shutdown_delay(void)
		{
			return conf_set_int(&prefs_runtime_config.shutdown_delay, NULL, BNETD_SHUTDELAY);
		}

		static const char* conf_get_shutdown_delay(void)
		{
			return conf_get_int(prefs_runtime_config.shutdown_delay);
		}


		extern unsigned int prefs_get_shutdown_decr(void)
		{
			return prefs_runtime_config.shutdown_decr;
		}

		static int conf_set_shutdown_decr(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.shutdown_decr, valstr, 0);
		}

		static int conf_setdef_shutdown_decr(void)
		{
			return conf_set_int(&prefs_runtime_config.shutdown_decr, NULL, BNETD_SHUTDECR);
		}

		static const char* conf_get_shutdown_decr(void)
		{
			return conf_get_int(prefs_runtime_config.shutdown_decr);
		}


		extern unsigned int prefs_get_allow_new_accounts(void)
		{
			return prefs_runtime_config.new_accounts;
		}

		static int conf_set_new_accounts(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.new_accounts, valstr, 0);
		}

		static int conf_setdef_new_accounts(void)
		{
			return conf_set_bool(&prefs_runtime_config.new_accounts, NULL, 1);
		}

		static const char* conf_get_new_accounts(void)
		{
			return conf_get_bool(prefs_runtime_config.new_accounts);
		}


		extern unsigned int prefs_get_max_accounts(void)
		{
			return prefs_runtime_config.max_accounts;
		}

		static int conf_set_max_accounts(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.max_accounts, valstr, 0);
		}

		static int conf_setdef_max_accounts(void)
		{
			return conf_set_int(&prefs_runtime_config.max_accounts, NULL, 0);
		}

		static const char* conf_get_max_accounts(void)
		{
			return conf_get_int(prefs_runtime_config.max_accounts);
		}


		extern unsigned int prefs_get_kick_old_login(void)
		{
			return prefs_runtime_config.kick_old_login;
		}

		static int conf_set_kick_old_login(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.kick_old_login, valstr, 0);
		}

		static int conf_setdef_kick_old_login(void)
		{
			return conf_set_bool(&prefs_runtime_config.kick_old_login, NULL, 1);
		}

		static const char* conf_get_kick_old_login(void)
		{
			return conf_get_bool(prefs_runtime_config.kick_old_login);
		}


		extern char const * prefs_get_channelfile(void)
		{
			return prefs_runtime_config.channelfile;
		}

		static int conf_set_channelfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.channelfile, valstr, NULL);
		}

		static int conf_setdef_channelfile(void)
		{
			return conf_set_str(&prefs_runtime_config.channelfile, NULL, BNETD_CHANNEL_FILE);
		}

		static const char* conf_get_channelfile(void)
		{
			return prefs_runtime_config.channelfile;
		}


		extern unsigned int prefs_get_ask_new_channel(void)
		{
			return prefs_runtime_config.ask_new_channel;
		}

		static int conf_set_ask_new_channel(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.ask_new_channel, valstr, 0);
		}

		static int conf_setdef_ask_new_channel(void)
		{
			return conf_set_bool(&prefs_runtime_config.ask_new_channel, NULL, 1);
		}

		static const char* conf_get_ask_new_channel(void)
		{
			return conf_get_bool(prefs_runtime_config.ask_new_channel);
		}


		extern unsigned int prefs_get_hide_pass_games(void)
		{
			return prefs_runtime_config.hide_pass_games;
		}

		static int conf_set_hide_pass_games(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.hide_pass_games, valstr, 0);
		}

		static int conf_setdef_hide_pass_games(void)
		{
			return conf_set_bool(&prefs_runtime_config.hide_pass_games, NULL, 1);
		}

		static const char* conf_get_hide_pass_games(void)
		{
			return conf_get_bool(prefs_runtime_config.hide_pass_games);
		}


		extern unsigned int prefs_get_hide_started_games(void)
		{
			return prefs_runtime_config.hide_started_games;
		}

		static int conf_set_hide_started_games(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.hide_started_games, valstr, 0);
		}

		static int conf_setdef_hide_started_games(void)
		{
			return conf_set_bool(&prefs_runtime_config.hide_started_games, NULL, 1);
		}

		static const char* conf_get_hide_started_games(void)
		{
			return conf_get_bool(prefs_runtime_config.hide_started_games);
		}


		extern unsigned int prefs_get_hide_temp_channels(void)
		{
			return prefs_runtime_config.hide_temp_channels;
		}

		static int conf_set_hide_temp_channels(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.hide_temp_channels, valstr, 0);
		}

		static int conf_setdef_hide_temp_channels(void)
		{
			return conf_set_bool(&prefs_runtime_config.hide_temp_channels, NULL, 1);
		}

		static const char* conf_get_hide_temp_channels(void)
		{
			return conf_get_bool(prefs_runtime_config.hide_temp_channels);
		}

		extern unsigned prefs_get_hide_addr(void)
		{
			return prefs_runtime_config.hide_addr;
		}

		static int conf_set_hide_addr(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.hide_addr, valstr, 0);
		}

		static int conf_setdef_hide_addr(void)
		{
			return conf_set_bool(&prefs_runtime_config.hide_addr, NULL, 1);
		}

		static const char* conf_get_hide_addr(void)
		{
			return conf_get_bool(prefs_runtime_config.hide_addr);
		}


		extern unsigned int prefs_get_enable_conn_all(void)
		{
			return prefs_runtime_config.enable_conn_all;
		}

		static int conf_set_enable_conn_all(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.enable_conn_all, valstr, 0);
		}

		static int conf_setdef_enable_conn_all(void)
		{
			return conf_set_bool(&prefs_runtime_config.enable_conn_all, NULL, 0);
		}

		static const char* conf_get_enable_conn_all(void)
		{
			return conf_get_bool(prefs_runtime_config.enable_conn_all);
		}


		extern char const * prefs_get_reportdir(void)
		{
			return prefs_runtime_config.reportdir;
		}

		static int conf_set_reportdir(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.reportdir, valstr, NULL);
		}

		static int conf_setdef_reportdir(void)
		{
			return conf_set_str(&prefs_runtime_config.reportdir, NULL, BNETD_REPORT_DIR);
		}

		static const char* conf_get_reportdir(void)
		{
			return prefs_runtime_config.reportdir;
		}


		extern unsigned int prefs_get_report_all_games(void)
		{
			return prefs_runtime_config.report_all_games;
		}

		static int conf_set_report_all_games(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.report_all_games, valstr, 0);
		}

		static int conf_setdef_report_all_games(void)
		{
			return conf_set_bool(&prefs_runtime_config.report_all_games, NULL, 0);
		}

		static const char* conf_get_report_all_games(void)
		{
			return conf_get_bool(prefs_runtime_config.report_all_games);
		}


		extern unsigned int prefs_get_report_diablo_games(void)
		{
			return prefs_runtime_config.report_diablo_games;
		}

		static int conf_set_report_diablo_games(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.report_diablo_games, valstr, 0);
		}

		static int conf_setdef_report_diablo_games(void)
		{
			return conf_set_bool(&prefs_runtime_config.report_diablo_games, NULL, 0);
		}

		static const char* conf_get_report_diablo_games(void)
		{
			return conf_get_bool(prefs_runtime_config.report_diablo_games);
		}


		extern char const * prefs_get_pidfile(void)
		{
			return prefs_runtime_config.pidfile;
		}

		static int conf_set_pidfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.pidfile, valstr, NULL);
		}

		static int conf_setdef_pidfile(void)
		{
			return conf_set_str(&prefs_runtime_config.pidfile, NULL, BNETD_PID_FILE);
		}

		static const char* conf_get_pidfile(void)
		{
			return prefs_runtime_config.pidfile;
		}


		extern char const * prefs_get_iconfile(void)
		{
			return prefs_runtime_config.iconfile;
		}

		static int conf_set_iconfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.iconfile, valstr, NULL);
		}

		static int conf_setdef_iconfile(void)
		{
			return conf_set_str(&prefs_runtime_config.iconfile, NULL, BNETD_ICON_FILE);
		}

		static const char* conf_get_iconfile(void)
		{
			return prefs_runtime_config.iconfile;
		}


		extern char const * prefs_get_war3_iconfile(void)
		{
			return prefs_runtime_config.war3_iconfile;
		}

		static int conf_set_war3_iconfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.war3_iconfile, valstr, NULL);
		}

		static int conf_setdef_war3_iconfile(void)
		{
			return conf_set_str(&prefs_runtime_config.war3_iconfile, NULL, BNETD_WAR3_ICON_FILE);
		}

		static const char* conf_get_war3_iconfile(void)
		{
			return prefs_runtime_config.war3_iconfile;
		}


		extern char const * prefs_get_star_iconfile(void)
		{
			return prefs_runtime_config.star_iconfile;
		}

		static int conf_set_star_iconfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.star_iconfile, valstr, NULL);
		}

		static int conf_setdef_star_iconfile(void)
		{
			return conf_set_str(&prefs_runtime_config.star_iconfile, NULL, BNETD_STAR_ICON_FILE);
		}

		static const char* conf_get_star_iconfile(void)
		{
			return prefs_runtime_config.star_iconfile;
		}


		extern char const * prefs_get_tosfile(void)
		{
			return prefs_runtime_config.tosfile;
		}

		static int conf_set_tosfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.tosfile, valstr, NULL);
		}

		static int conf_setdef_tosfile(void)
		{
			return conf_set_str(&prefs_runtime_config.tosfile, NULL, BNETD_TOS_FILE);
		}

		static const char* conf_get_tosfile(void)
		{
			return prefs_runtime_config.tosfile;
		}


		extern char const * prefs_get_mpqfile(void)
		{
			return prefs_runtime_config.mpqfile;
		}

		static int conf_set_mpqfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.mpqfile, valstr, NULL);
		}

		static int conf_setdef_mpqfile(void)
		{
			return conf_set_str(&prefs_runtime_config.mpqfile, NULL, BNETD_MPQ_FILE);
		}

		static const char* conf_get_mpqfile(void)
		{
			return prefs_runtime_config.mpqfile;
		}


		extern char const * prefs_get_trackserv_addrs(void)
		{
			return prefs_runtime_config.trackaddrs;
		}

		static int conf_set_trackaddrs(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.trackaddrs, valstr, NULL);
		}

		static int conf_setdef_trackaddrs(void)
		{
			return conf_set_str(&prefs_runtime_config.trackaddrs, NULL, BNETD_TRACK_ADDRS);
		}

		static const char* conf_get_trackaddrs(void)
		{
			return prefs_runtime_config.trackaddrs;
		}


		extern char const * prefs_get_bnetdserv_addrs(void)
		{
			return prefs_runtime_config.servaddrs;
		}

		static int conf_set_servaddrs(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.servaddrs, valstr, NULL);
		}

		static int conf_setdef_servaddrs(void)
		{
			return conf_set_str(&prefs_runtime_config.servaddrs, NULL, BNETD_SERV_ADDRS);
		}

		static const char* conf_get_servaddrs(void)
		{
			return prefs_runtime_config.servaddrs;
		}


		extern char const * prefs_get_w3route_addr(void)
		{
			return prefs_runtime_config.w3routeaddr;
		}

		static int conf_set_w3routeaddr(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.w3routeaddr, valstr, NULL);
		}

		static int conf_setdef_w3routeaddr(void)
		{
			return conf_set_str(&prefs_runtime_config.w3routeaddr, NULL, BNETD_W3ROUTE_ADDR);
		}

		static const char* conf_get_w3routeaddr(void)
		{
			return prefs_runtime_config.w3routeaddr;
		}


		extern char const * prefs_get_irc_addrs(void)
		{
			return prefs_runtime_config.ircaddrs;
		}

		static int conf_set_ircaddrs(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.ircaddrs, valstr, NULL);
		}

		static int conf_setdef_ircaddrs(void)
		{
			return conf_set_str(&prefs_runtime_config.ircaddrs, NULL, BNETD_IRC_ADDRS);
		}

		static const char* conf_get_ircaddrs(void)
		{
			return prefs_runtime_config.ircaddrs;
		}


		extern unsigned int prefs_get_use_keepalive(void)
		{
			return prefs_runtime_config.use_keepalive;
		}

		static int conf_set_use_keepalive(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.use_keepalive, valstr, 0);
		}

		static int conf_setdef_use_keepalive(void)
		{
			return conf_set_bool(&prefs_runtime_config.use_keepalive, NULL, 0);
		}

		static const char* conf_get_use_keepalive(void)
		{
			return conf_get_bool(prefs_runtime_config.use_keepalive);
		}


		extern unsigned int prefs_get_udptest_port(void)
		{
			return prefs_runtime_config.udptest_port;
		}

		static int conf_set_udptest_port(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.udptest_port, valstr, 0);
		}

		static int conf_setdef_udptest_port(void)
		{
			return conf_set_int(&prefs_runtime_config.udptest_port, NULL, BNETD_DEF_TEST_PORT);
		}

		static const char* conf_get_udptest_port(void)
		{
			return conf_get_int(prefs_runtime_config.udptest_port);
		}


		extern char const * prefs_get_ipbanfile(void)
		{
			return prefs_runtime_config.ipbanfile;
		}

		static int conf_set_ipbanfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.ipbanfile, valstr, NULL);
		}

		static int conf_setdef_ipbanfile(void)
		{
			return conf_set_str(&prefs_runtime_config.ipbanfile, NULL, BNETD_IPBAN_FILE);
		}

		static const char* conf_get_ipbanfile(void)
		{
			return prefs_runtime_config.ipbanfile;
		}


		extern unsigned int prefs_get_discisloss(void)
		{
			return prefs_runtime_config.disc_is_loss;
		}

		static int conf_set_disc_is_loss(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.disc_is_loss, valstr, 0);
		}

		static int conf_setdef_disc_is_loss(void)
		{
			return conf_set_bool(&prefs_runtime_config.disc_is_loss, NULL, 0);
		}

		static const char* conf_get_disc_is_loss(void)
		{
			return conf_get_bool(prefs_runtime_config.disc_is_loss);
		}


		extern char const * prefs_get_helpfile(void)
		{
			return prefs_runtime_config.helpfile;
		}

		static int conf_set_helpfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.helpfile, valstr, NULL);
		}

		static int conf_setdef_helpfile(void)
		{
			return conf_set_str(&prefs_runtime_config.helpfile, NULL, BNETD_HELP_FILE);
		}

		static const char* conf_get_helpfile(void)
		{
			return prefs_runtime_config.helpfile;
		}


		extern char const * prefs_get_fortunecmd(void)
		{
			return prefs_runtime_config.fortunecmd;
		}

		static int conf_set_fortunecmd(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.fortunecmd, valstr, NULL);
		}

		static int conf_setdef_fortunecmd(void)
		{
			return conf_set_str(&prefs_runtime_config.fortunecmd, NULL, BNETD_FORTUNECMD);
		}

		static const char* conf_get_fortunecmd(void)
		{
			return prefs_runtime_config.fortunecmd;
		}


		extern char const * prefs_get_transfile(void)
		{
			return prefs_runtime_config.transfile;
		}

		static int conf_set_transfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.transfile, valstr, NULL);
		}

		static int conf_setdef_transfile(void)
		{
			return conf_set_str(&prefs_runtime_config.transfile, NULL, BNETD_TRANS_FILE);
		}

		static const char* conf_get_transfile(void)
		{
			return prefs_runtime_config.transfile;
		}


		extern unsigned int prefs_get_chanlog(void)
		{
			return prefs_runtime_config.chanlog;
		}

		static int conf_set_chanlog(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.chanlog, valstr, 0);
		}

		static int conf_setdef_chanlog(void)
		{
			return conf_set_bool(&prefs_runtime_config.chanlog, NULL, BNETD_CHANLOG);
		}

		static const char* conf_get_chanlog(void)
		{
			return conf_get_bool(prefs_runtime_config.chanlog);
		}


		extern char const * prefs_get_chanlogdir(void)
		{
			return prefs_runtime_config.chanlogdir;
		}

		static int conf_set_chanlogdir(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.chanlogdir, valstr, NULL);
		}

		static int conf_setdef_chanlogdir(void)
		{
			return conf_set_str(&prefs_runtime_config.chanlogdir, NULL, BNETD_CHANLOG_DIR);
		}

		static const char* conf_get_chanlogdir(void)
		{
			return prefs_runtime_config.chanlogdir;
		}


		extern unsigned int prefs_get_quota(void)
		{
			return prefs_runtime_config.quota;
		}

		static int conf_set_quota(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.quota, valstr, 0);
		}

		static int conf_setdef_quota(void)
		{
			return conf_set_bool(&prefs_runtime_config.quota, NULL, 0);
		}

		static const char* conf_get_quota(void)
		{
			return conf_get_bool(prefs_runtime_config.quota);
		}


		extern unsigned int prefs_get_quota_lines(void)
		{
			unsigned int rez;

			rez = prefs_runtime_config.quota_lines;
			if (rez<1) rez = 1;
			if (rez>100) rez = 100;
			return rez;
		}

		static int conf_set_quota_lines(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.quota_lines, valstr, 0);
		}

		static int conf_setdef_quota_lines(void)
		{
			return conf_set_int(&prefs_runtime_config.quota_lines, NULL, BNETD_QUOTA_LINES);
		}

		static const char* conf_get_quota_lines(void)
		{
			return conf_get_int(prefs_runtime_config.quota_lines);
		}


		extern unsigned int prefs_get_quota_time(void)
		{
			unsigned int rez;

			rez = prefs_runtime_config.quota_time;
			if (rez<1) rez = 1;
			if (rez>10) rez = 60;
			return rez;
		}

		static int conf_set_quota_time(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.quota_time, valstr, 0);
		}

		static int conf_setdef_quota_time(void)
		{
			return conf_set_int(&prefs_runtime_config.quota_time, NULL, BNETD_QUOTA_TIME);
		}

		static const char* conf_get_quota_time(void)
		{
			return conf_get_int(prefs_runtime_config.quota_time);
		}


		extern unsigned int prefs_get_quota_wrapline(void)
		{
			unsigned int rez;

			rez = prefs_runtime_config.quota_wrapline;
			if (rez<1) rez = 1;
			if (rez>256) rez = 256;
			return rez;
		}

		static int conf_set_quota_wrapline(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.quota_wrapline, valstr, 0);
		}

		static int conf_setdef_quota_wrapline(void)
		{
			return conf_set_int(&prefs_runtime_config.quota_wrapline, NULL, BNETD_QUOTA_WLINE);
		}

		static const char* conf_get_quota_wrapline(void)
		{
			return conf_get_int(prefs_runtime_config.quota_wrapline);
		}


		extern unsigned int prefs_get_quota_maxline(void)
		{
			unsigned int rez;

			rez = prefs_runtime_config.quota_maxline;
			if (rez<1) rez = 1;
			if (rez>256) rez = 256;
			return rez;
		}

		static int conf_set_quota_maxline(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.quota_maxline, valstr, 0);
		}

		static int conf_setdef_quota_maxline(void)
		{
			return conf_set_int(&prefs_runtime_config.quota_maxline, NULL, BNETD_QUOTA_MLINE);
		}

		static const char* conf_get_quota_maxline(void)
		{
			return conf_get_int(prefs_runtime_config.quota_maxline);
		}


		extern unsigned int prefs_get_ladder_init_rating(void)
		{
			return prefs_runtime_config.ladder_init_rating;
		}

		static int conf_set_ladder_init_rating(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.ladder_init_rating, valstr, 0);
		}

		static int conf_setdef_ladder_init_rating(void)
		{
			return conf_set_int(&prefs_runtime_config.ladder_init_rating, NULL, BNETD_LADDER_INIT_RAT);
		}

		static const char* conf_get_ladder_init_rating(void)
		{
			return conf_get_int(prefs_runtime_config.ladder_init_rating);
		}


		extern unsigned int prefs_get_quota_dobae(void)
		{
			unsigned int rez;

			rez = prefs_runtime_config.quota_dobae;
			if (rez<1) rez = 1;
			if (rez>100) rez = 100;
			return rez;
		}

		static int conf_set_quota_dobae(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.quota_dobae, valstr, 0);
		}

		static int conf_setdef_quota_dobae(void)
		{
			return conf_set_int(&prefs_runtime_config.quota_dobae, NULL, BNETD_QUOTA_DOBAE);
		}

		static const char* conf_get_quota_dobae(void)
		{
			return conf_get_int(prefs_runtime_config.quota_dobae);
		}


		extern char const * prefs_get_realmfile(void)
		{
			return prefs_runtime_config.realmfile;
		}

		static int conf_set_realmfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.realmfile, valstr, NULL);
		}

		static int conf_setdef_realmfile(void)
		{
			return conf_set_str(&prefs_runtime_config.realmfile, NULL, BNETD_REALM_FILE);
		}

		static const char* conf_get_realmfile(void)
		{
			return prefs_runtime_config.realmfile;
		}


		extern char const * prefs_get_issuefile(void)
		{
			return prefs_runtime_config.issuefile;
		}

		static int conf_set_issuefile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.issuefile, valstr, NULL);
		}

		static int conf_setdef_issuefile(void)
		{
			return conf_set_str(&prefs_runtime_config.issuefile, NULL, BNETD_ISSUE_FILE);
		}

		static const char* conf_get_issuefile(void)
		{
			return prefs_runtime_config.issuefile;
		}


		extern char const * prefs_get_effective_user(void)
		{
			return prefs_runtime_config.effective_user;
		}

		static int conf_set_effective_user(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.effective_user, valstr, NULL);
		}

		static int conf_setdef_effective_user(void)
		{
			return conf_set_str(&prefs_runtime_config.effective_user, NULL, NULL);
		}

		static const char* conf_get_effective_user(void)
		{
			return prefs_runtime_config.effective_user;
		}


		extern char const * prefs_get_effective_group(void)
		{
			return prefs_runtime_config.effective_group;
		}

		static int conf_set_effective_group(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.effective_group, valstr, NULL);
		}

		static int conf_setdef_effective_group(void)
		{
			return conf_set_str(&prefs_runtime_config.effective_group, NULL, NULL);
		}

		static const char* conf_get_effective_group(void)
		{
			return prefs_runtime_config.effective_group;
		}


		extern unsigned int prefs_get_nullmsg(void)
		{
			return prefs_runtime_config.nullmsg;
		}

		static int conf_set_nullmsg(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.nullmsg, valstr, 0);
		}

		static int conf_setdef_nullmsg(void)
		{
			return conf_set_int(&prefs_runtime_config.nullmsg, NULL, BNETD_DEF_NULLMSG);
		}

		static const char* conf_get_nullmsg(void)
		{
			return conf_get_int(prefs_runtime_config.nullmsg);
		}


		extern unsigned int prefs_get_mail_support(void)
		{
			return prefs_runtime_config.mail_support;
		}

		static int conf_set_mail_support(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.mail_support, valstr, 0);
		}

		static int conf_setdef_mail_support(void)
		{
			return conf_set_bool(&prefs_runtime_config.mail_support, NULL, BNETD_MAIL_SUPPORT);
		}

		static const char* conf_get_mail_support(void)
		{
			return conf_get_bool(prefs_runtime_config.mail_support);
		}


		extern unsigned int prefs_get_mail_quota(void)
		{
			unsigned int rez;

			rez = prefs_runtime_config.mail_quota;
			if (rez<1) rez = 1;
			if (rez>30) rez = 30;
			return rez;
		}

		static int conf_set_mail_quota(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.mail_quota, valstr, 0);
		}

		static int conf_setdef_mail_quota(void)
		{
			return conf_set_int(&prefs_runtime_config.mail_quota, NULL, BNETD_MAIL_QUOTA);
		}

		static const char* conf_get_mail_quota(void)
		{
			return conf_get_int(prefs_runtime_config.mail_quota);
		}


		extern char const * prefs_get_maildir(void)
		{
			return prefs_runtime_config.maildir;
		}

		static int conf_set_maildir(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.maildir, valstr, NULL);
		}

		static int conf_setdef_maildir(void)
		{
			return conf_set_str(&prefs_runtime_config.maildir, NULL, BNETD_MAIL_DIR);
		}

		static const char* conf_get_maildir(void)
		{
			return prefs_runtime_config.maildir;
		}


		extern char const * prefs_get_log_notice(void)
		{
			return prefs_runtime_config.log_notice;
		}

		static int conf_set_log_notice(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.log_notice, valstr, NULL);
		}

		static int conf_setdef_log_notice(void)
		{
			return conf_set_str(&prefs_runtime_config.log_notice, NULL, BNETD_LOG_NOTICE);
		}

		static const char* conf_get_log_notice(void)
		{
			return prefs_runtime_config.log_notice;
		}


		extern unsigned int prefs_get_savebyname(void)
		{
			return prefs_runtime_config.savebyname;
		}

		static int conf_set_savebyname(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.savebyname, valstr, 0);
		}

		static int conf_setdef_savebyname(void)
		{
			return conf_set_bool(&prefs_runtime_config.savebyname, NULL, 1);
		}

		static const char* conf_get_savebyname(void)
		{
			return conf_get_bool(prefs_runtime_config.savebyname);
		}


		extern unsigned int prefs_get_skip_versioncheck(void)
		{
			return prefs_runtime_config.skip_versioncheck;
		}

		static int conf_set_skip_versioncheck(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.skip_versioncheck, valstr, 0);
		}

		static int conf_setdef_skip_versioncheck(void)
		{
			return conf_set_bool(&prefs_runtime_config.skip_versioncheck, NULL, 0);
		}

		static const char* conf_get_skip_versioncheck(void)
		{
			return conf_get_bool(prefs_runtime_config.skip_versioncheck);
		}


		extern unsigned int prefs_get_allow_bad_version(void)
		{
			return prefs_runtime_config.allow_bad_version;
		}

		static int conf_set_allow_bad_version(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.allow_bad_version, valstr, 0);
		}

		static int conf_setdef_allow_bad_version(void)
		{
			return conf_set_bool(&prefs_runtime_config.allow_bad_version, NULL, 0);
		}

		static const char* conf_get_allow_bad_version(void)
		{
			return conf_get_bool(prefs_runtime_config.allow_bad_version);
		}


		extern unsigned int prefs_get_allow_unknown_version(void)
		{
			return prefs_runtime_config.allow_unknown_version;
		}

		static int conf_set_allow_unknown_version(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.allow_unknown_version, valstr, 0);
		}

		static int conf_setdef_allow_unknown_version(void)
		{
			return conf_set_bool(&prefs_runtime_config.allow_unknown_version, NULL, 0);
		}

		static const char* conf_get_allow_unknown_version(void)
		{
			return conf_get_bool(prefs_runtime_config.allow_unknown_version);
		}


		extern char const * prefs_get_versioncheck_file(void)
		{
			return prefs_runtime_config.versioncheck_file;
		}

		static int conf_set_versioncheck_file(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.versioncheck_file, valstr, NULL);
		}

		static int conf_setdef_versioncheck_file(void)
		{
			return conf_set_str(&prefs_runtime_config.versioncheck_file, NULL, PVPGN_VERSIONCHECK);
		}

		static const char* conf_get_versioncheck_file(void)
		{
			return prefs_runtime_config.versioncheck_file;
		}


		extern unsigned int prefs_allow_d2cs_setname(void)
		{
			return prefs_runtime_config.allow_d2cs_setname;
		}

		static int conf_set_allow_d2cs_setname(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.allow_d2cs_setname, valstr, 0);
		}

		static int conf_setdef_allow_d2cs_setname(void)
		{
			return conf_set_bool(&prefs_runtime_config.allow_d2cs_setname, NULL, 1);
		}

		static const char* conf_get_allow_d2cs_setname(void)
		{
			return conf_get_bool(prefs_runtime_config.allow_d2cs_setname);
		}


		extern unsigned int prefs_get_d2cs_version(void)
		{
			return prefs_runtime_config.d2cs_version;
		}

		static int conf_set_d2cs_version(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.d2cs_version, valstr, 0);
		}

		static int conf_setdef_d2cs_version(void)
		{
			return conf_set_int(&prefs_runtime_config.d2cs_version, NULL, 0);
		}

		static const char* conf_get_d2cs_version(void)
		{
			return conf_get_int(prefs_runtime_config.d2cs_version);
		}


		extern unsigned int prefs_get_hashtable_size(void)
		{
			return prefs_runtime_config.hashtable_size;
		}

		static int conf_set_hashtable_size(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.hashtable_size, valstr, 0);
		}

		static int conf_setdef_hashtable_size(void)
		{
			return conf_set_int(&prefs_runtime_config.hashtable_size, NULL, BNETD_HASHTABLE_SIZE);
		}

		static const char* conf_get_hashtable_size(void)
		{
			return conf_get_int(prefs_runtime_config.hashtable_size);
		}


		extern char const * prefs_get_telnet_addrs(void)
		{
			return prefs_runtime_config.telnetaddrs;
		}

		static int conf_set_telnetaddrs(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.telnetaddrs, valstr, NULL);
		}

		static int conf_setdef_telnetaddrs(void)
		{
			return conf_set_str(&prefs_runtime_config.telnetaddrs, NULL, BNETD_TELNET_ADDRS);
		}

		static const char* conf_get_telnetaddrs(void)
		{
			return prefs_runtime_config.telnetaddrs;
		}


		extern unsigned int prefs_get_ipban_check_int(void)
		{
			return prefs_runtime_config.ipban_check_int;
		}

		static int conf_set_ipban_check_int(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.ipban_check_int, valstr, 0);
		}

		static int conf_setdef_ipban_check_int(void)
		{
			return conf_set_int(&prefs_runtime_config.ipban_check_int, NULL, 30);
		}

		static const char* conf_get_ipban_check_int(void)
		{
			return conf_get_int(prefs_runtime_config.ipban_check_int);
		}


		extern char const * prefs_get_version_exeinfo_match(void)
		{
			return prefs_runtime_config.version_exeinfo_match;
		}

		static int conf_set_version_exeinfo_match(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.version_exeinfo_match, valstr, NULL);
		}

		static int conf_setdef_version_exeinfo_match(void)
		{
			return conf_set_str(&prefs_runtime_config.version_exeinfo_match, NULL, BNETD_EXEINFO_MATCH);
		}

		static const char* conf_get_version_exeinfo_match(void)
		{
			return prefs_runtime_config.version_exeinfo_match;
		}


		extern unsigned int prefs_get_version_exeinfo_maxdiff(void)
		{
			return prefs_runtime_config.version_exeinfo_maxdiff;
		}

		static int conf_set_version_exeinfo_maxdiff(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.version_exeinfo_maxdiff, valstr, 0);
		}

		static int conf_setdef_version_exeinfo_maxdiff(void)
		{
			return conf_set_int(&prefs_runtime_config.version_exeinfo_maxdiff, NULL, PVPGN_VERSION_TIMEDIV);
		}

		static const char* conf_get_version_exeinfo_maxdiff(void)
		{
			return conf_get_int(prefs_runtime_config.version_exeinfo_maxdiff);
		}


		extern unsigned int prefs_get_max_concurrent_logins(void)
		{
			return prefs_runtime_config.max_concurrent_logins;
		}

		static int conf_set_max_concurrent_logins(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.max_concurrent_logins, valstr, 0);
		}

		static int conf_setdef_max_concurrent_logins(void)
		{
			return conf_set_int(&prefs_runtime_config.max_concurrent_logins, NULL, 0);
		}

		static const char* conf_get_max_concurrent_logins(void)
		{
			return conf_get_int(prefs_runtime_config.max_concurrent_logins);
		}


		extern char const * prefs_get_mapsfile(void)
		{
			return prefs_runtime_config.mapsfile;
		}

		static int conf_set_mapsfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.mapsfile, valstr, NULL);
		}

		static int conf_setdef_mapsfile(void)
		{
			return conf_set_str(&prefs_runtime_config.mapsfile, NULL, NULL);
		}

		static const char* conf_get_mapsfile(void)
		{
			return prefs_runtime_config.mapsfile;
		}


		extern char const * prefs_get_xplevel_file(void)
		{
			return prefs_runtime_config.xplevelfile;
		}

		static int conf_set_xplevelfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.xplevelfile, valstr, NULL);
		}

		static int conf_setdef_xplevelfile(void)
		{
			return conf_set_str(&prefs_runtime_config.xplevelfile, NULL, NULL);
		}

		static const char* conf_get_xplevelfile(void)
		{
			return prefs_runtime_config.xplevelfile;
		}


		extern char const * prefs_get_xpcalc_file(void)
		{
			return prefs_runtime_config.xpcalcfile;
		}

		static int conf_set_xpcalcfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.xpcalcfile, valstr, NULL);
		}

		static int conf_setdef_xpcalcfile(void)
		{
			return conf_set_str(&prefs_runtime_config.xpcalcfile, NULL, NULL);
		}

		static const char* conf_get_xpcalcfile(void)
		{
			return prefs_runtime_config.xpcalcfile;
		}


		extern int prefs_get_initkill_timer(void)
		{
			return prefs_runtime_config.initkill_timer;
		}

		static int conf_set_initkill_timer(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.initkill_timer, valstr, 0);
		}

		static int conf_setdef_initkill_timer(void)
		{
			return conf_set_int(&prefs_runtime_config.initkill_timer, NULL, 0);
		}

		static const char* conf_get_initkill_timer(void)
		{
			return conf_get_int(prefs_runtime_config.initkill_timer);
		}


		extern int prefs_get_war3_ladder_update_secs(void)
		{
			return prefs_runtime_config.war3_ladder_update_secs;
		}

		static int conf_set_war3_ladder_update_secs(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.war3_ladder_update_secs, valstr, 0);
		}

		static int conf_setdef_war3_ladder_update_secs(void)
		{
			return conf_set_int(&prefs_runtime_config.war3_ladder_update_secs, NULL, 0);
		}

		static const char* conf_get_war3_ladder_update_secs(void)
		{
			return conf_get_int(prefs_runtime_config.war3_ladder_update_secs);
		}


		extern int prefs_get_output_update_secs(void)
		{
			return prefs_runtime_config.output_update_secs;
		}

		static int conf_set_output_update_secs(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.output_update_secs, valstr, 0);
		}

		static int conf_setdef_output_update_secs(void)
		{
			return conf_set_int(&prefs_runtime_config.output_update_secs, NULL, 0);
		}

		static const char* conf_get_output_update_secs(void)
		{
			return conf_get_int(prefs_runtime_config.output_update_secs);
		}


		extern char const * prefs_get_ladderdir(void)
		{
			return prefs_runtime_config.ladderdir;
		}

		static int conf_set_ladderdir(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.ladderdir, valstr, NULL);
		}

		static int conf_setdef_ladderdir(void)
		{
			return conf_set_str(&prefs_runtime_config.ladderdir, NULL, BNETD_LADDER_DIR);
		}

		static const char* conf_get_ladderdir(void)
		{
			return prefs_runtime_config.ladderdir;
		}


		extern char const * prefs_get_outputdir(void)
		{
			return prefs_runtime_config.statusdir;
		}

		static int conf_set_statusdir(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.statusdir, valstr, NULL);
		}

		static int conf_setdef_statusdir(void)
		{
			return conf_set_str(&prefs_runtime_config.statusdir, NULL, BNETD_STATUS_DIR);
		}

		static const char* conf_get_statusdir(void)
		{
			return prefs_runtime_config.statusdir;
		}


		extern int prefs_get_XML_output_ladder(void)
		{
			return prefs_runtime_config.XML_output_ladder;
		}

		static int conf_set_XML_output_ladder(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.XML_output_ladder, valstr, 0);
		}

		static int conf_setdef_XML_output_ladder(void)
		{
			return conf_set_bool(&prefs_runtime_config.XML_output_ladder, NULL, 0);
		}

		static const char* conf_get_XML_output_ladder(void)
		{
			return conf_get_bool(prefs_runtime_config.XML_output_ladder);
		}


		extern int prefs_get_XML_status_output(void)
		{
			return prefs_runtime_config.XML_status_output;
		}

		static int conf_set_XML_status_output(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.XML_status_output, valstr, 0);
		}

		static int conf_setdef_XML_status_output(void)
		{
			return conf_set_bool(&prefs_runtime_config.XML_status_output, NULL, 0);
		}

		static const char* conf_get_XML_status_output(void)
		{
			return conf_get_bool(prefs_runtime_config.XML_status_output);
		}


		extern char const * prefs_get_account_allowed_symbols(void)
		{
			return prefs_runtime_config.account_allowed_symbols;
		}

		static int conf_set_account_allowed_symbols(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.account_allowed_symbols, valstr, NULL);
		}

		static int conf_setdef_account_allowed_symbols(void)
		{
			return conf_set_str(&prefs_runtime_config.account_allowed_symbols, NULL, PVPGN_DEFAULT_SYMB);
		}

		static const char* conf_get_account_allowed_symbols(void)
		{
			return prefs_runtime_config.account_allowed_symbols;
		}


		extern unsigned int prefs_get_account_force_username(void)
		{
			return prefs_runtime_config.account_force_username;
		}

		static int conf_set_account_force_username(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.account_force_username, valstr, 0);
		}

		static int conf_setdef_account_force_username(void)
		{
			return conf_set_bool(&prefs_runtime_config.account_force_username, NULL, 0);
		}

		static const char* conf_get_account_force_username(void)
		{
			return conf_get_bool(prefs_runtime_config.account_force_username);
		}

		extern char const * prefs_get_command_groups_file(void)
		{
			return prefs_runtime_config.command_groups_file;
		}

		static int conf_set_command_groups_file(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.command_groups_file, valstr, NULL);
		}

		static int conf_setdef_command_groups_file(void)
		{
			return conf_set_str(&prefs_runtime_config.command_groups_file, NULL, BNETD_COMMAND_GROUPS_FILE);
		}

		static const char* conf_get_command_groups_file(void)
		{
			return prefs_runtime_config.command_groups_file;
		}


		extern char const * prefs_get_tournament_file(void)
		{
			return prefs_runtime_config.tournament_file;
		}

		static int conf_set_tournament_file(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.tournament_file, valstr, NULL);
		}

		static int conf_setdef_tournament_file(void)
		{
			return conf_set_str(&prefs_runtime_config.tournament_file, NULL, BNETD_TOURNAMENT_FILE);
		}

		static const char* conf_get_tournament_file(void)
		{
			return prefs_runtime_config.tournament_file;
		}


		extern char const * prefs_get_customicons_file(void)
		{
			return prefs_runtime_config.customicons_file;
		}

		static int conf_set_customicons_file(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.customicons_file, valstr, NULL);
		}

		static int conf_setdef_customicons_file(void)
		{
			return conf_set_str(&prefs_runtime_config.customicons_file, NULL, BNETD_CUSTOMICONS_FILE);
		}

		static const char* conf_get_customicons_file(void)
		{
			return prefs_runtime_config.customicons_file;
		}

		extern char const * prefs_get_scriptdir(void)
		{
			return prefs_runtime_config.scriptdir;
		}

		static int conf_set_scriptdir(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.scriptdir, valstr, NULL);
		}

		static int conf_setdef_scriptdir(void)
		{
			return conf_set_str(&prefs_runtime_config.scriptdir, NULL, BNETD_SCRIPT_DIR);
		}

		static const char* conf_get_scriptdir(void)
		{
			return prefs_runtime_config.scriptdir;
		}

		extern char const * prefs_get_aliasfile(void)
		{
			return prefs_runtime_config.aliasfile;
		}

		static int conf_set_aliasfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.aliasfile, valstr, NULL);
		}

		static int conf_setdef_aliasfile(void)
		{
			return conf_set_str(&prefs_runtime_config.aliasfile, NULL, BNETD_ALIASFILE);
		}

		static const char* conf_get_aliasfile(void)
		{
			return prefs_runtime_config.aliasfile;
		}


		extern char const * prefs_get_anongame_infos_file(void)
		{
			return prefs_runtime_config.anongame_infos_file;
		}

		static int conf_set_anongame_infos_file(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.anongame_infos_file, valstr, NULL);
		}

		static int conf_setdef_anongame_infos_file(void)
		{
			return conf_set_str(&prefs_runtime_config.anongame_infos_file, NULL, PVPGN_AINFO_FILE);
		}

		static const char* conf_get_anongame_infos_file(void)
		{
			return prefs_runtime_config.anongame_infos_file;
		}


		extern unsigned int prefs_get_max_conns_per_IP(void)
		{
			return prefs_runtime_config.max_conns_per_IP;
		}

		static int conf_set_max_conns_per_IP(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.max_conns_per_IP, valstr, 0);
		}

		static int conf_setdef_max_conns_per_IP(void)
		{
			return conf_set_int(&prefs_runtime_config.max_conns_per_IP, NULL, 0);
		}

		static const char* conf_get_max_conns_per_IP(void)
		{
			return conf_get_int(prefs_runtime_config.max_conns_per_IP);
		}


		extern int prefs_get_max_friends(void)
		{
			return prefs_runtime_config.max_friends;
		}

		static int conf_set_max_friends(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.max_friends, valstr, 0);
		}

		static int conf_setdef_max_friends(void)
		{
			return conf_set_int(&prefs_runtime_config.max_friends, NULL, MAX_FRIENDS);
		}

		static const char* conf_get_max_friends(void)
		{
			return conf_get_int(prefs_runtime_config.max_friends);
		}


		extern unsigned int prefs_get_clan_newer_time(void)
		{
			return prefs_runtime_config.clan_newer_time;
		}

		static int conf_set_clan_newer_time(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.clan_newer_time, valstr, 0);
		}

		static int conf_setdef_clan_newer_time(void)
		{
			return conf_set_int(&prefs_runtime_config.clan_newer_time, NULL, CLAN_NEWER_TIME);
		}

		static const char* conf_get_clan_newer_time(void)
		{
			return conf_get_int(prefs_runtime_config.clan_newer_time);
		}


		extern unsigned int prefs_get_clan_max_members(void)
		{
			return prefs_runtime_config.clan_max_members;
		}

		static int conf_set_clan_max_members(const char *valstr)
		{
			int rez = conf_set_int(&prefs_runtime_config.clan_max_members, valstr, 0);

			if (!rez && valstr) {
				if (prefs_runtime_config.clan_max_members < CLAN_MIN_MEMBERS) {
					WARN1("Cannot set clan max members to %u lower than 10, setting to 10.", prefs_runtime_config.clan_max_members);
					prefs_runtime_config.clan_max_members = CLAN_MIN_MEMBERS;
				}
				else if (prefs_runtime_config.clan_max_members > CLAN_MAX_MEMBERS) {
					WARN1("Cannot set clan max members to %u higher than 100, setting to 100.", prefs_runtime_config.clan_max_members);
					prefs_runtime_config.clan_max_members = CLAN_MAX_MEMBERS;
				}
			}

			return rez;
		}

		static int conf_setdef_clan_max_members(void)
		{
			return conf_set_int(&prefs_runtime_config.clan_max_members, NULL, CLAN_DEFAULT_MAX_MEMBERS);
		}

		static const char* conf_get_clan_max_members(void)
		{
			return conf_get_int(prefs_runtime_config.clan_max_members);
		}


		extern unsigned int prefs_get_clan_channel_default_private(void)
		{
			return prefs_runtime_config.clan_channel_default_private;
		}

		static int conf_set_clan_channel_default_private(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.clan_channel_default_private, valstr, 0);
		}

		static int conf_setdef_clan_channel_default_private(void)
		{
			return conf_set_bool(&prefs_runtime_config.clan_channel_default_private, NULL, 0);
		}

		static const char* conf_get_clan_channel_default_private(void)
		{
			return conf_get_bool(prefs_runtime_config.clan_channel_default_private);
		}

		extern unsigned int prefs_get_clan_min_invites(void)
		{
			return prefs_runtime_config.clan_min_invites;
		}

		static int conf_set_clan_min_invites(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.clan_min_invites, valstr, 0);
		}

		static int conf_setdef_clan_min_invites(void)
		{
			return conf_set_int(&prefs_runtime_config.clan_min_invites, NULL, CLAN_DEFAULT_MIN_INVITES);
		}

		static const char* conf_get_clan_min_invites(void)
		{
			return conf_get_int(prefs_runtime_config.clan_min_invites);
		}

		extern unsigned int prefs_get_passfail_count(void)
		{
			return prefs_runtime_config.passfail_count;
		}

		static int conf_set_passfail_count(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.passfail_count, valstr, 0);
		}

		static int conf_setdef_passfail_count(void)
		{
			return conf_set_int(&prefs_runtime_config.passfail_count, NULL, 0);
		}

		static const char* conf_get_passfail_count(void)
		{
			return conf_get_int(prefs_runtime_config.passfail_count);
		}


		extern unsigned int prefs_get_passfail_bantime(void)
		{
			return prefs_runtime_config.passfail_bantime;
		}

		static int conf_set_passfail_bantime(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.passfail_bantime, valstr, 0);
		}

		static int conf_setdef_passfail_bantime(void)
		{
			return conf_set_int(&prefs_runtime_config.passfail_bantime, NULL, 300);
		}

		static const char* conf_get_passfail_bantime(void)
		{
			return conf_get_int(prefs_runtime_config.passfail_bantime);
		}


		extern unsigned int prefs_get_maxusers_per_channel(void)
		{
			return prefs_runtime_config.maxusers_per_channel;
		}

		static int conf_set_maxusers_per_channel(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.maxusers_per_channel, valstr, 0);
		}

		static int conf_setdef_maxusers_per_channel(void)
		{
			return conf_set_int(&prefs_runtime_config.maxusers_per_channel, NULL, 0);
		}

		static const char* conf_get_maxusers_per_channel(void)
		{
			return conf_get_int(prefs_runtime_config.maxusers_per_channel);
		}


		extern char const * prefs_get_supportfile(void)
		{
			return prefs_runtime_config.supportfile;
		}

		static int conf_set_supportfile(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.supportfile, valstr, NULL);
		}

		static int conf_setdef_supportfile(void)
		{
			return conf_set_str(&prefs_runtime_config.supportfile, NULL, BNETD_SUPPORT_FILE);
		}

		static const char* conf_get_supportfile(void)
		{
			return prefs_runtime_config.supportfile;
		}


		extern char const * prefs_get_allowed_clients(void)
		{
			return prefs_runtime_config.allowed_clients;
		}

		static int conf_set_allowed_clients(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.allowed_clients, valstr, NULL);
		}

		static int conf_setdef_allowed_clients(void)
		{
			return conf_set_str(&prefs_runtime_config.allowed_clients, NULL, NULL);
		}

		static const char* conf_get_allowed_clients(void)
		{
			return prefs_runtime_config.allowed_clients;
		}


		extern char const * prefs_get_ladder_games(void)
		{
			return prefs_runtime_config.ladder_games;
		}

		static int conf_set_ladder_games(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.ladder_games, valstr, NULL);
		}

		static int conf_setdef_ladder_games(void)
		{
			return conf_set_str(&prefs_runtime_config.ladder_games, NULL, NULL);
		}

		static const char* conf_get_ladder_games(void)
		{
			return prefs_runtime_config.ladder_games;
		}


		extern char const * prefs_get_ladder_prefix(void)
		{
			return prefs_runtime_config.ladder_prefix;
		}

		static int conf_set_ladder_prefix(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.ladder_prefix, valstr, NULL);
		}

		static int conf_setdef_ladder_prefix(void)
		{
			return conf_set_str(&prefs_runtime_config.ladder_prefix, NULL, NULL);
		}

		static const char* conf_get_ladder_prefix(void)
		{
			return prefs_runtime_config.ladder_prefix;
		}


		extern unsigned int prefs_get_max_connections(void)
		{
			return prefs_runtime_config.max_connections;
		}

		static int conf_set_max_connections(const char *valstr)
		{
			return conf_set_int(&prefs_runtime_config.max_connections, valstr, 0);
		}

		static int conf_setdef_max_connections(void)
		{
			return conf_set_int(&prefs_runtime_config.max_connections, NULL, BNETD_MAX_SOCKETS);
		}

		static const char* conf_get_max_connections(void)
		{
			return conf_get_int(prefs_runtime_config.max_connections);
		}


		extern unsigned int prefs_get_sync_on_logoff(void)
		{
			return prefs_runtime_config.sync_on_logoff;
		}

		static int conf_set_sync_on_logoff(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.sync_on_logoff, valstr, 0);
		}

		static int conf_setdef_sync_on_logoff(void)
		{
			return conf_set_bool(&prefs_runtime_config.sync_on_logoff, NULL, 0);
		}

		static const char* conf_get_sync_on_logoff(void)
		{
			return conf_get_bool(prefs_runtime_config.sync_on_logoff);
		}

		extern char const * prefs_get_irc_network_name(void)
		{
			return prefs_runtime_config.irc_network_name;
		}

		static int conf_set_irc_network_name(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.irc_network_name, valstr, NULL);
		}

		static int conf_setdef_irc_network_name(void)
		{
			return conf_set_str(&prefs_runtime_config.irc_network_name, NULL, BNETD_IRC_NETWORK_NAME);
		}

		static const char* conf_get_irc_network_name(void)
		{
			return prefs_runtime_config.irc_network_name;
		}


		extern unsigned int prefs_get_localize_by_country(void)
		{
			return prefs_runtime_config.localize_by_country;
		}

		static int conf_set_localize_by_country(const char *valstr)
		{
			return conf_set_bool(&prefs_runtime_config.localize_by_country, valstr, 0);
		}

		static int conf_setdef_localize_by_country(void)
		{
			return conf_set_bool(&prefs_runtime_config.localize_by_country, NULL, 0);
		}

		static const char* conf_get_localize_by_country(void)
		{
			return conf_get_bool(prefs_runtime_config.localize_by_country);
		}


		/**
		*  Westwood Online Extensions
		*/
		extern char const * prefs_get_apireg_addrs(void)
		{
			return prefs_runtime_config.apiregaddrs;
		}

		static int conf_set_apireg_addrs(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.apiregaddrs, valstr, NULL);
		}

		static int conf_setdef_apireg_addrs(void)
		{
			return conf_set_str(&prefs_runtime_config.apiregaddrs, NULL, BNETD_APIREG_ADDRS);
		}

		static const char* conf_get_apireg_addrs(void)
		{
			return prefs_runtime_config.apiregaddrs;
		}

		extern char const * prefs_get_wgameres_addrs(void)
		{
			return prefs_runtime_config.wgameresaddrs;
		}

		static int conf_set_wgameres_addrs(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.wgameresaddrs, valstr, NULL);
		}

		static int conf_setdef_wgameres_addrs(void)
		{
			return conf_set_str(&prefs_runtime_config.wgameresaddrs, NULL, BNETD_WGAMERES_ADDRS);
		}

		static const char* conf_get_wgameres_addrs(void)
		{
			return prefs_runtime_config.wgameresaddrs;
		}

		extern char const * prefs_get_wolv1_addrs(void)
		{
			return prefs_runtime_config.wolv1addrs;
		}

		static int conf_set_wolv1_addrs(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.wolv1addrs, valstr, NULL);
		}

		static int conf_setdef_wolv1_addrs(void)
		{
			return conf_set_str(&prefs_runtime_config.wolv1addrs, NULL, BNETD_WOLV1_ADDRS);
		}

		static const char* conf_get_wolv1_addrs(void)
		{
			return prefs_runtime_config.wolv1addrs;
		}

		extern char const * prefs_get_wolv2_addrs(void)
		{
			return prefs_runtime_config.wolv2addrs;
		}

		static int conf_set_wolv2_addrs(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.wolv2addrs, valstr, NULL);
		}

		static int conf_setdef_wolv2_addrs(void)
		{
			return conf_set_str(&prefs_runtime_config.wolv2addrs, NULL, BNETD_WOLV2_ADDRS);
		}

		static const char* conf_get_wolv2_addrs(void)
		{
			return prefs_runtime_config.wolv2addrs;
		}

		static int  conf_set_wol_timezone(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.woltimezone, valstr, NULL);
		}

		extern char const * prefs_get_wol_timezone(void)
		{
			return prefs_runtime_config.woltimezone;
		}

		static char const * conf_get_wol_timezone(void)
		{
			return prefs_runtime_config.woltimezone;
		}

		static int conf_setdef_wol_timezone(void)
		{
			return conf_set_str(&prefs_runtime_config.woltimezone, NULL, 0);
		}

		static int  conf_set_wol_longitude(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.wollongitude, valstr, NULL);
		}

		extern char const * prefs_get_wol_longitude(void)
		{
			return prefs_runtime_config.wollongitude;
		}

		static char const * conf_get_wol_longitude(void)
		{
			return prefs_runtime_config.wollongitude;
		}

		static int conf_setdef_wol_longitude(void)
		{
			return conf_set_str(&prefs_runtime_config.wollongitude, NULL, 0);
		}

		static int conf_set_wol_latitude(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.wollatitude, valstr, NULL);
		}

		extern char const * prefs_get_wol_latitude(void)
		{
			return prefs_runtime_config.wollatitude;
		}

		static char const * conf_get_wol_latitude(void)
		{
			return prefs_runtime_config.wollatitude;
		}

		static int conf_setdef_wol_latitude(void)
		{
			return conf_set_str(&prefs_runtime_config.wollatitude, NULL, 0);
		}

		static int conf_set_wol_autoupdate_serverhost(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.wol_autoupdate_serverhost, valstr, NULL);
		}

		extern char const * prefs_get_wol_autoupdate_serverhost(void)
		{
			return prefs_runtime_config.wol_autoupdate_serverhost;
		}

		static char const * conf_get_wol_autoupdate_serverhost(void)
		{
			return prefs_runtime_config.wol_autoupdate_serverhost;
		}

		static int conf_setdef_wol_autoupdate_serverhost(void)
		{
			return conf_set_str(&prefs_runtime_config.wol_autoupdate_serverhost, NULL, 0);
		}

		static int conf_set_wol_autoupdate_username(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.wol_autoupdate_username, valstr, NULL);
		}

		extern char const * prefs_get_wol_autoupdate_username(void)
		{
			return prefs_runtime_config.wol_autoupdate_username;
		}

		static char const * conf_get_wol_autoupdate_username(void)
		{
			return prefs_runtime_config.wol_autoupdate_username;
		}

		static int conf_setdef_wol_autoupdate_username(void)
		{
			return conf_set_str(&prefs_runtime_config.wol_autoupdate_username, NULL, 0);
		}

		static int conf_set_wol_autoupdate_password(const char *valstr)
		{
			return conf_set_str(&prefs_runtime_config.wol_autoupdate_password, valstr, NULL);
		}

		extern char const * prefs_get_wol_autoupdate_password(void)
		{
			return prefs_runtime_config.wol_autoupdate_password;
		}

		static char const * conf_get_wol_autoupdate_password(void)
		{
			return prefs_runtime_config.wol_autoupdate_password;
		}

		static int conf_setdef_wol_autoupdate_password(void)
		{
			return conf_set_str(&prefs_runtime_config.wol_autoupdate_password, NULL, 0);
		}

	}

}
