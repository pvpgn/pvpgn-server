/*
 * Copyright (C) 2007  Pelish (pelish@gmail.com)
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
#include "handle_irc_common.h"

#include <cstring>
#include <cctype>
#include <cstdlib>

#include "compat/strcasecmp.h"
#include "common/eventlog.h"
#include "common/util.h"
#include "common/irc_protocol.h"

#include "compat/snprintf.h"

#include "handle_irc.h"
#include "handle_wol.h"
#include "handle_wserv.h"

#include "prefs.h"
#include "command.h"
#include "irc.h"

#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static int handle_irc_common_con_command(t_connection * conn, char const * command, int numparams, char ** params, char * text)
		{
			if (!conn) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			switch (conn_get_class(conn)) {
			case conn_class_irc:
				return handle_irc_con_command(conn, command, numparams, params, text);
			case conn_class_wserv:
				return handle_wserv_con_command(conn, command, numparams, params, text);
			case conn_class_wol:
			case conn_class_wladder:
			case conn_class_wgameres:
				return handle_wol_con_command(conn, command, numparams, params, text);
			default:
				return handle_irc_con_command(conn, command, numparams, params, text);
			}
		}

		static int handle_irc_common_log_command(t_connection * conn, char const * command, int numparams, char ** params, char * text)
		{
			if (!conn) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			switch (conn_get_class(conn)) {
			case conn_class_irc:
				return handle_irc_log_command(conn, command, numparams, params, text);
			case conn_class_wol:
			case conn_class_wgameres:
				return handle_wol_log_command(conn, command, numparams, params, text);
			default:
				return handle_irc_log_command(conn, command, numparams, params, text);
			}
		}

		static int handle_irc_common_set_class(t_connection * conn, char const * command, int numparams, char ** params, char * text)
		{
			if (!conn) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (conn_get_class(conn) != conn_class_ircinit) {
				DEBUG0("FIXME: conn_get_class(conn) != conn_class_ircinit");
				return -1;
			}
			else {
				if (strcasecmp(command, "VERCHK") == 0) {
					DEBUG0("Got WSERV packet");
					if (std::strcmp(prefs_get_wolv2_addrs(), "") != 0)
						conn_set_class(conn, conn_class_wserv);
					else
						conn_set_state(conn, conn_state_destroy);
					return 0;
				}
				else if (strcasecmp(command, "CVERS") == 0) {
					DEBUG0("Got WOL packet");
					/* FIXME: We can check it not by address but check if client is supported by tag_check_in_list() */
					if ((std::strcmp(prefs_get_wolv1_addrs(), "") != 0) || (std::strcmp(prefs_get_wolv2_addrs(), "") != 0))
						conn_set_class(conn, conn_class_wol);
					else
						conn_set_state(conn, conn_state_destroy);
					return 0;
				}
				else if ((strcasecmp(command, "LISTSEARCH") == 0) ||
					(strcasecmp(command, "RUNGSEARCH") == 0) ||
					(strcasecmp(command, "HIGHSCORE") == 0)) {
					DEBUG0("Got WOL Ladder packet");
					if (std::strcmp(prefs_get_wolv2_addrs(), "") != 0)
						conn_set_class(conn, conn_class_wladder); /* is handled in handle_wol.* now */
					else
						conn_set_state(conn, conn_state_destroy);
					return 0;
				}
				else if ((strcasecmp(command, "CRYPT") == 0) ||
					(strcasecmp(command, "LOGIN") == 0)) {
					DEBUG0("Got GameSpy packet");
					if (std::strcmp(prefs_get_irc_addrs(), "") != 0)
						conn_set_class(conn, conn_class_irc);
					//                conn_set_class(conn,conn_class_gspy_peerchat);
					else
						conn_set_state(conn, conn_state_destroy);
					return 0;
				}
				else {
					DEBUG0("Got IRC packet");
					if (std::strcmp(prefs_get_irc_addrs(), "") != 0)
						conn_set_class(conn, conn_class_irc);
					else
						conn_set_state(conn, conn_state_destroy);
					return 0;
				}
			}
		}

		static int handle_irc_common_line(t_connection * conn, char const * ircline)
		{
			/* [:prefix] <command> [[param1] [param2] ... [paramN]] [:<text>] */
			char * line; /* copy of ircline */
			char * prefix = NULL; /* optional; mostly NULL */
			char * command; /* mandatory */
			char ** params = NULL; /* optional (array of params) */
			char * text = NULL; /* optional */
			char * bnet_command = NULL;  /* amadeo: used for battle.net.commands */
			int unrecognized_before = 0;
			int linelen; /* amadeo: counter for stringlenghts */

			int numparams = 0;
			char * tempparams;
			int i;
			char paramtemp[MAX_IRC_MESSAGE_LEN * 2];
			int first = 1;

			if (!conn) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (!ircline) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL ircline");
				return -1;
			}
			if (ircline[0] == '\0') {
				/**
				 * PELISH: We dont send warning about that because it is client-side problem.
				 * eventlog(eventlog_level_error,__FUNCTION__,"got empty ircline");
				 */
				return -1;
			}

			//amadeo: code was sent by some unknown fellow of pvpgn, prevents buffer-overflow for
			// too long irc-lines
			//PELISH: According to RFC2812 we do truncation on 512 byte

			if (std::strlen(ircline) > MAX_IRC_MESSAGE_LEN) {
				char * tmp = (char *)ircline;
				eventlog(eventlog_level_warn, __FUNCTION__, "line too long, truncation...");
				tmp[MAX_IRC_MESSAGE_LEN] = '\0';
			}

			line = xstrdup(ircline);

			/* split the message */
			if (line[0] == ':') {
				/* The prefix is optional and is rarely provided */
				prefix = line;
				if (!(command = std::strchr(line, ' '))) {
					eventlog(eventlog_level_warn, __FUNCTION__, "got malformed line (missing command)");
					xfree(line);
					return -1;
				}
				*command++ = '\0';
			}
			else {
				/* In most cases command is the first thing on the line */
				command = line;
			}

			tempparams = std::strchr(command, ' ');
			if (tempparams) {
				*tempparams++ = '\0';
				if (tempparams[0] == ':') {
					text = tempparams + 1; /* theres just text, no params. skip the colon */
				}
				else {
					for (i = 0; tempparams[i] != '\0'; i++) {
						if ((tempparams[i] == ' ') && (tempparams[i + 1] == ':')) {
							text = tempparams + i;
							*text++ = '\0';
							text++; /* skip the colon */
							break; /* text found, stop search */
						}
					}
					params = irc_get_paramelems(tempparams);
				}
			}

			if (params) {
				/* count parameters */
				for (numparams = 0; params[numparams]; numparams++);
			}

			std::memset(paramtemp, 0, sizeof(paramtemp));
			for (i = 0; ((numparams > 0) && (params[i])); i++) {
				if (!first)
					std::strcat(paramtemp, " ");
				std::strcat(paramtemp, "\"");
				std::strcat(paramtemp, params[i]);
				std::strcat(paramtemp, "\"");
				first = 0;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "[%d] got \"%s\" \"%s\" [%s] \"%s\"", conn_get_socket(conn), ((prefix) ? (prefix) : ("")), command, paramtemp, ((text) ? (text) : ("")));

			if (conn_get_class(conn) == conn_class_ircinit) {
				handle_irc_common_set_class(conn, command, numparams, params, text);
			}

			if (conn_get_state(conn) == conn_state_connected) {
				conn_set_state(conn, conn_state_bot_username); /* PELISH: What is this for? */

				if ((conn_get_class(conn) != conn_class_wserv) &&
					(conn_get_class(conn) != conn_class_wladder)) {

					t_timer_data temp;
					temp.n = prefs_get_irc_latency();
					conn_test_latency(conn, std::time(NULL), temp);
				}
			}

			if (handle_irc_common_con_command(conn, command, numparams, params, text) != -1) {}
			else if (conn_get_state(conn) != conn_state_loggedin) {
				char temp[MAX_IRC_MESSAGE_LEN + 1];

				if ((38 + std::strlen(command) + 16 + 1) < sizeof(temp)) {
					snprintf(temp, sizeof(temp), ":Unrecognized command \"%s\" (before login)", command);
					irc_send(conn, ERR_UNKNOWNCOMMAND, temp);
				}
				else {
					irc_send(conn, ERR_UNKNOWNCOMMAND, ":Unrecognized command (before login)");
				}
			}
			else {
				/* command is handled later */
				unrecognized_before = 1;
			}
			/* --- The following should only be executable after login --- */
			if ((conn_get_state(conn) == conn_state_loggedin) && (unrecognized_before)) {

				if (handle_irc_common_log_command(conn, command, numparams, params, text) != -1) {}
				else if ((strstart(command, "LAG") != 0) && (strstart(command, "JOIN") != 0)){
					linelen = std::strlen(ircline);
					bnet_command = (char*)xmalloc(linelen + 2);
					bnet_command[0] = '/';
					std::strcpy(bnet_command + 1, ircline);
					handle_command(conn, bnet_command);
					xfree((void*)bnet_command);
				}
			} /* loggedin */
			if (params)
				irc_unget_paramelems(params);
			xfree(line);
			return 0;
		}


		extern int handle_irc_common_packet(t_connection * conn, t_packet const * const packet)
		{
			unsigned int i;
			char ircline[MAX_IRC_MESSAGE_LEN];
			char const * data;
			char test[MAX_IRC_MESSAGE_LEN];

			if (!packet) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
				return -1;
			}
			if ((conn_get_class(conn) != conn_class_ircinit) &&
				(conn_get_class(conn) != conn_class_irc) &&
				(conn_get_class(conn) != conn_class_wol) &&
				(conn_get_class(conn) != conn_class_wserv) &&
				(conn_get_class(conn) != conn_class_wladder)) {
				eventlog(eventlog_level_error, __FUNCTION__, "FIXME: handle_irc_packet without any reason (conn->class != conn_class_irc/ircinit/wol/wserv...)");
				return -1;
			}

			//    eventlog(eventlog_level_debug,__FUNCTION__,"got \"%s\"",packet_get_raw_data_const(packet,0));

			std::memset(ircline, 0, sizeof(ircline));
			data = conn_get_ircline(conn); /* fetch current status */
			if (data)
				std::strcpy(ircline, data);
			unsigned ircpos = std::strlen(ircline);
			data = (const char *)packet_get_raw_data_const(packet, 0);

			for (i = 0; i < packet_get_size(packet); i++) {
				if (data[i] == '\n') {
					/* end of line */
					handle_irc_common_line(conn, ircline);
					std::memset(ircline, 0, sizeof(ircline));
					ircpos = 0;
				}
				else {
					if (ircpos < MAX_IRC_MESSAGE_LEN - 1)
						ircline[ircpos++] = data[i];
					else {
						ircpos++; /* for the statistic :) */
						eventlog(eventlog_level_warn, __FUNCTION__, "[%d] client exceeded maximum allowed message length by %d characters", conn_get_socket(conn), ircpos - MAX_IRC_MESSAGE_LEN);
						if (ircpos > 100 + MAX_IRC_MESSAGE_LEN) {
							/* automatic flood protection */
							eventlog(eventlog_level_error, __FUNCTION__, "[%d] excess flood", conn_get_socket(conn));
							return -1;
						}
					}
				}
			}
			conn_set_ircline(conn, ircline); /* write back current status */
			return 0;
		}

	}

}
