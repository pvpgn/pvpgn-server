/*
 * Copyright (C) 2001  Marco Ziech (mmz@gmx.net)
 * Copyright (C) 2005  Bryan Biedenkapp (gatekeep@gmail.com)
 * Copyright (C) 2006,2007,2008  Pelish (pelish@gmail.com)
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
#include "irc.h"

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <string>

#include <fmt/format.h>
#include <nonstd/optional.hpp>

#include "compat/strcasecmp.h"

#include "common/irc_protocol.h"
#include "common/packet.h"
#include "common/eventlog.h"
#include "common/field_sizes.h"
#include "common/bnethash.h"
#include "common/xalloc.h"
#include "common/addr.h"
#include "common/tag.h"
#include "common/list.h"
#include "common/util.h"

#include "message.h"
#include "channel.h"
#include "game.h"
#include "connection.h"
#include "server.h"
#include "account.h"
#include "account_wrap.h"
#include "prefs.h"
#include "tick.h"
#include "handle_irc.h"
#include "handle_wol.h"
#include "command_groups.h"
#include "topic.h"
#include "clan.h"
#include "command.h"
#include "anongame_wol.h"
#include "i18n.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		typedef struct {
			char const* nick;
			char const* user;
			char const* host;
		} t_irc_message_from;


		static char** irc_split_elems(char* list, int separator, int ignoreblank);
		static int irc_unget_elems(char** elems);
		static std::string irc_message_preformat(const t_irc_message_from* from, const char* command, const char* dest, const char* text);

		static std::size_t irc_send_get_max_params_length(t_connection* conn)
		{
			const char* const ircname = server_get_hostname();
			const char* const nick = conn_get_loggeduser(conn);
			const int example_numeric = 0;

			// fmt::format(":{} {:03d} {} {}\r\n", ircname, numeric, nick ? nick : "UserName", params);
			auto overhead = fmt::formatted_size(":{} {:03d} {} {}\r\n", ircname, example_numeric, nick ? nick : "UserName", "");
			return MAX_IRC_MESSAGE_LEN - overhead;
		}

		extern int irc_send(t_connection* conn, int numeric, char const* params)
		{
			if (!conn)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if ((numeric > 999) || (numeric < 0))
			{
				/* more than 3 digits or negative */
				eventlog(eventlog_level_error, __FUNCTION__, "invalid irc message numeric ({})", numeric);
				return -1;
			}

			try
			{
				const char* hostname = server_get_hostname();
				std::string nick;
				{
					const char* tempnick = conn_get_loggeduser(conn);
					nick = tempnick ? tempnick : "UserName";
				}

				std::string data = fmt::format(":{} {:03d} {}", hostname, numeric, nick);
				if (params)
				{
					fmt::format_to(std::back_inserter(data), " {}", params);
				}
				fmt::format_to(std::back_inserter(data), "\r\n");

				if (data.length() > MAX_IRC_MESSAGE_LEN)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "irc message length too long (got {} bytes, max {} bytes)", data.length(), MAX_IRC_MESSAGE_LEN);
					eventlog(eventlog_level_debug, __FUNCTION__, "hostname: {}, numeric: {}, nick: {}, params: \"{}\"", hostname, numeric, nick, params);
					return -1;
				}

				t_packet* packet = packet_create(packet_class_raw);
				if (packet == nullptr)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not create packet");
					return -1;
				}
				packet_set_size(packet, 0);
				packet_append_data(packet, data.c_str(), data.length());
				conn_push_outqueue(conn, packet);
				packet_del_ref(packet);

				data.erase(data.end() - 2);
				eventlog(eventlog_level_debug, __FUNCTION__, "[{}] sent \"{}\"", conn_get_socket(conn), data);

				return 0;
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] failed to send IRC command ({})", conn_get_socket(conn), e.what());
				return -1;
			}
		}

		extern int irc_send_ping(t_connection* conn)
		{
			if (!conn)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if ((conn_get_class(conn) == conn_class_wserv) ||
				(conn_get_class(conn) == conn_class_wgameres) ||
				(conn_get_class(conn) == conn_class_wladder))
			{
				return 0;
			}

			try
			{
				conn_set_ircping(conn, get_ticks());

				// MAX_IRC_MESSAGE_LEN
				std::string data;

				if (conn_get_state(conn) == conn_state_bot_username)
				{
					data = fmt::format("PING :{}\r\n", conn_get_ircping(conn)); // Undernet doesn't reveal the servername yet ... neither do we
				}
				else
				{
					data = fmt::format("PING :{}\r\n", server_get_hostname());
				}

				if (data.length() > MAX_IRC_MESSAGE_LEN)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "irc message length too long (got {} bytes, max {} bytes)", data.length(), MAX_IRC_MESSAGE_LEN);
					return -1;
				}

				t_packet* p = packet_create(packet_class_raw);
				if (!p)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not create packet");
					return -1;
				}
				packet_set_size(p, 0);
				packet_append_data(p, data.c_str(), data.length());
				conn_push_outqueue(conn, p);
				packet_del_ref(p);

				data.erase(data.end() - 2);
				eventlog(eventlog_level_debug, __FUNCTION__, "[{}] sent \"{}\"", conn_get_socket(conn), data);

				return 0;
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] failed to send IRC command ({})", conn_get_socket(conn), e.what());
				return -1;
			}
		}

		extern int irc_send_pong(t_connection* conn, char const* params)
		{
			if (!conn)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			try
			{
				std::string data;
				if (params)
				{
					data = fmt::format(":{} PONG {} :{}\r\n", server_get_hostname(), server_get_hostname(), params);
				}
				else
				{
					data = fmt::format(":{} PONG {}\r\n", server_get_hostname(), server_get_hostname());
				}

				if (data.length() > MAX_IRC_MESSAGE_LEN)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "irc message length too long (got {} bytes, max {} bytes)", data.length(), MAX_IRC_MESSAGE_LEN);
					return -1;
				}

				t_packet* p = packet_create(packet_class_raw);
				if (!p)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not create packet");
					return -1;
				}
				packet_set_size(p, 0);
				packet_append_data(p, data.c_str(), data.length());
				conn_push_outqueue(conn, p);
				packet_del_ref(p);

				data.erase(data.end() - 2);
				eventlog(eventlog_level_debug, __FUNCTION__, "[{}] sent \"{}\"", conn_get_socket(conn), data);

				return 0;
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] failed to send IRC command ({})", conn_get_socket(conn), e.what());
				return -1;
			}
		}

		extern int irc_authenticate(t_connection* conn, char const* passhash)
		{
			/* FIXME: Move this function to handle_irc.* file! */
			t_hash h1;
			t_hash h2;
			t_account* a;
			char const* temphash;
			char const* username;

			if (!conn) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return 0;
			}
			if (!passhash) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL passhash");
				return 0;
			}
			username = conn_get_loggeduser(conn);
			if (!username) {
				/* redundant sanity check */
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn->protocol.loggeduser");
				return 0;
			}
			a = accountlist_find_account(username);
			if (!a) {
				message_send_text(conn, message_type_notice, NULL, "Authentication failed.");
				return 0;
			}

			if (connlist_find_connection_by_account(a) && prefs_get_kick_old_login() == 0) {
				std::string temp = fmt::format("{} :Account is already in use!", conn_get_loggeduser(conn));
				irc_send(conn, ERR_NICKNAMEINUSE, temp.c_str());
			}
			else if (account_get_auth_lock(a) == 1) {
				message_send_text(conn, message_type_notice, NULL, "Authentication rejected (account is locked) ");
			}
			else
			{
				hash_set_str(&h1, passhash);
				temphash = account_get_pass(a);
				hash_set_str(&h2, temphash);
				if (hash_eq(h1, h2)) {
					conn_login(conn, a, username);
					conn_set_state(conn, conn_state_loggedin);
					conn_set_clienttag(conn, CLIENTTAG_IIRC_UINT); /* IIRC hope here is ok */
					message_send_text(conn, message_type_notice, NULL, "Authentication successful. You are now logged in.");
					return 1;
				}
				else {
					message_send_text(conn, message_type_notice, NULL, "Authentication failed."); /* wrong password */
					conn_increment_passfail_count(conn);
				}
			}
			return 0;
		}

		/* Channel name conversion rules: */
		/* Not allowed in IRC (RFC2812): NUL, BELL, CR, LF, ' ', ':' and ','*/
		/*   ' '  -> '_'      */
		/*   '_'  -> '%_'     */
		/*   '%'  -> '%%'     */
		/*   '\b' -> '%b'     */
		/*   '\n' -> '%n'     */
		/*   '\r' -> '%r'     */
		/*   ':'  -> '%='     */
		/*   ','  -> '%-'     */
		/* In IRC a channel can be specified by '#'+channelname or '!'+channelid */
		extern char const* irc_convert_channel(t_channel const* channel, t_connection* c)
		{
			char const* bname;
			static char out[MAX_CHANNELNAME_LEN];
			unsigned int outpos;
			int i;

			if (!channel)
				return "*";

			std::memset(out, 0, sizeof(out));
			out[0] = '#';
			outpos = 1;

			if ((conn_get_wol(c) == 1) && (channel_get_clienttag(channel) != 0 && (conn_get_clienttag(c) == channel_get_clienttag(channel))))
			{
				bname = channel_get_shortname(channel); /* We converting unreadable "lob 18 0" names to human redable ones */
				if (!bname)
				{
					bname = channel_get_name(channel);
				}
			}
			else
				bname = channel_get_name(channel);

			for (i = 0; bname[i] != '\0'; i++) {
				if (bname[i] == ' ') {
					out[outpos++] = '_';
				}
				else if (bname[i] == '_') {
					out[outpos++] = '%';
					out[outpos++] = '_';
				}
				else if (bname[i] == '%') {
					out[outpos++] = '%';
					out[outpos++] = '%';
				}
				else if (bname[i] == '\b') {
					out[outpos++] = '%';
					out[outpos++] = 'b';
				}
				else if (bname[i] == '\n') {
					out[outpos++] = '%';
					out[outpos++] = 'n';
				}
				else if (bname[i] == '\r') {
					out[outpos++] = '%';
					out[outpos++] = 'r';
				}
				else if (bname[i] == ':') {
					out[outpos++] = '%';
					out[outpos++] = '=';
				}
				else if (bname[i] == ',') {
					out[outpos++] = '%';
					out[outpos++] = '-';
				}
				else {
					out[outpos++] = bname[i];
				}

				if ((outpos + 2) >= (sizeof(out)))
				{
					std::snprintf(out, sizeof(out), "!%u", channel_get_channelid(channel));
					return out;
				}
			}
			return out;
		}

		extern char const* irc_convert_ircname(char const* pircname)
		{
			static char out[MAX_CHANNELNAME_LEN];
			unsigned int outpos;
			int special;
			int i;
			char const* ircname = pircname + 1;

			if (!ircname) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL ircname");
				return NULL;
			}

			outpos = 0;
			std::memset(out, 0, sizeof(out));
			special = 0;
			if (pircname[0] == '!') {
				t_channel* channel;

				channel = channellist_find_channel_bychannelid(std::atoi(ircname));
				if (channel)
					return channel_get_name(channel);
				else
					return NULL;
			}
			else if (pircname[0] != '#') {
				return NULL;
			}
			for (i = 0; ircname[i] != '\0'; i++) {
				if (ircname[i] == '_') {
					out[outpos++] = ' ';
				}
				else if (ircname[i] == '%') {
					if (special) {
						out[outpos++] = '%';
						special = 0;
					}
					else {
						special = 1;
					}
				}
				else if (special) {
					if (ircname[i] == '_') {
						out[outpos++] = '_';
					}
					else if (ircname[i] == 'b') {
						out[outpos++] = '\b';
					}
					else if (ircname[i] == 'n') {
						out[outpos++] = '\n';
					}
					else if (ircname[i] == 'r') {
						out[outpos++] = '\r';
					}
					else if (ircname[i] == '=') {
						out[outpos++] = ':';
					}
					else if (ircname[i] == '-') {
						out[outpos++] = ',';
					}
					else {
						/* maybe it's just a typo :) */
						out[outpos++] = '%';
						out[outpos++] = ircname[i];
					}
				}
				else {
					out[outpos++] = ircname[i];
				}
				if ((outpos + 2) >= (sizeof(out))) {
					return NULL;
				}
			}
			return out;
		}

		/* splits an string list into its elements */
		/* (list will be modified) */
		static char** irc_split_elems(char* list, int separator, int ignoreblank)
		{
			int i;
			int count;
			char** out;

			if (!list) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL list");
				return NULL;
			}

			for (count = 0, i = 0; list[i] != '\0'; i++) {
				if (list[i] == separator) {
					count++;
					if (ignoreblank) {
						/* ignore more than one separators "in a row" */
						while ((list[i + 1] != '\0') && (list[i] == separator)) i++;
					}
				}
			}
			count++; /* count separators -> we have one more element ... */
			/* we also need a terminating element */
			out = (char**)xmalloc((count + 1) * sizeof(char*));

			out[0] = list;
			if (count > 1) {
				for (i = 1; i < count; i++) {
					out[i] = std::strchr(out[i - 1], separator);
					if (!out[i]) {
						eventlog(eventlog_level_error, __FUNCTION__, "BUG: wrong number of separators");
						xfree(out);
						return NULL;
					}
					if (ignoreblank)
						while ((*out[i] + 1) == separator) out[i]++;
					*out[i]++ = '\0';
				}
				if ((ignoreblank) && (out[count - 1]) && (*out[count - 1] == '\0')) {
					out[count - 1] = NULL; /* last element is blank */
				}
			}
			else if ((ignoreblank) && (*out[0] == '\0')) {
				out[0] = NULL; /* now we have 2 terminators ... never mind */
			}
			out[count] = NULL; /* terminating element */
			return out;
		}

		static int irc_unget_elems(char** elems)
		{
			if (!elems) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL elems");
				return -1;
			}
			xfree(elems);
			return 0;
		}

		extern char** irc_get_listelems(char* list)
		{
			return irc_split_elems(list, ',', 0);
		}

		extern int irc_unget_listelems(char** elems)
		{
			return irc_unget_elems(elems);
		}

		extern char** irc_get_paramelems(char* list)
		{
			return irc_split_elems(list, ' ', 1);
		}

		extern int irc_unget_paramelems(char** elems)
		{
			return irc_unget_elems(elems);
		}

		extern char** irc_get_ladderelems(char* list)
		{
			return irc_split_elems(list, ':', 1);
		}

		extern int irc_unget_ladderelems(char** elems)
		{
			return irc_unget_elems(elems);
		}

		static std::string irc_message_preformat(const t_irc_message_from* from, const char* command, const char* dest, const char* text)
		{
			if (!command)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL command");
				return NULL;
			}

			std::string myfrom;
			if (from)
			{
				if ((!from->nick) || (!from->user) || (!from->host))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got malformed from");
					return NULL;
				}

				myfrom = fmt::format("{}!{}@{}", from->nick, from->user, from->host);
			}
			else
			{
				myfrom = server_get_hostname();
			}

			return fmt::format(":{}\n{}\n{}\n{}", myfrom, command, dest ? dest : "", text ? text : "");
		}

		extern int irc_message_postformat(t_packet* packet, t_connection const* dest)
		{
			/* the four elements */
			char* e1;
			char* e1_2;
			char* e2;
			char* e3;
			char* e4;
			char const* tname = NULL;
			char const* toname = "AUTH"; /* fallback name */
			const char* temp;

			if (!packet) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
				return -1;
			}
			if (!dest) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL dest");
				return -1;
			}

			e1 = (char*)packet_get_raw_data(packet, 0);
			e2 = std::strchr(e1, '\n');
			if (!e2) {
				eventlog(eventlog_level_warn, __FUNCTION__, "malformed message (e2 missing)");
				return -1;
			}
			*e2++ = '\0';
			e3 = std::strchr(e2, '\n');
			if (!e3) {
				eventlog(eventlog_level_warn, __FUNCTION__, "malformed message (e3 missing)");
				return -1;
			}
			*e3++ = '\0';
			e4 = std::strchr(e3, '\n');
			if (!e4) {
				eventlog(eventlog_level_warn, __FUNCTION__, "malformed message (e4 missing)");
				return -1;
			}
			*e4++ = '\0';

			if (prefs_get_hide_addr() && !(account_get_command_groups(conn_get_account(dest)) & command_get_group("/admin-addr")))
			{
				e1_2 = std::strchr(e1, '@');
				if (e1_2)
				{
					*e1_2++ = '\0';
				}
			}
			else
				e1_2 = NULL;

			if (e3[0] == '\0') { /* fill in recipient */
				if ((tname = conn_get_loggeduser(dest)))
					toname = tname;
			}
			else
				toname = e3;

			if (std::strcmp(toname, "\r") == 0) {
				toname = ""; /* HACK: the target field is really empty */
				temp = "";
			}
			else
				temp = " ";

			try
			{
				std::string msg;
				if (e1_2)
				{
					msg = fmt::format("{}@hidden {} {}{}{}\r\n", e1, e2, toname, temp, e4);
				}
				else
				{
					msg = fmt::format("{} {} {}{}{}\r\n", e1, e2, toname, temp, e4);
				}

				if (msg.length() > MAX_IRC_MESSAGE_LEN)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "irc message length too long (got {} bytes, max {} bytes)", msg.length(), MAX_IRC_MESSAGE_LEN);

					if (tname)
					{
						conn_unget_chatname(dest, tname);
					}

					return -1;
				}

				packet_set_size(packet, 0);
				packet_append_data(packet, msg.c_str(), msg.length());

				if (tname)
				{
					conn_unget_chatname(dest, tname);
				}

				return 0;
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[?] failed to post format IRC message ({})", e.what());

				return -1;
			}
		}

		extern int irc_message_format(t_packet* packet, t_message_type type, t_connection* me, t_connection* dst, char const* text, unsigned int dstflags)
		{
			if (!packet)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
				return -1;
			}

			try
			{
				const char* ctag;
				if (me)
				{
					ctag = clienttag_uint_to_str(conn_get_clienttag(me));
				}
				else
				{
					ctag = clienttag_uint_to_str(CLIENTTAG_IIRC_UINT);
				}

				std::string msg;

				switch (type)
				{
					/* case message_type_adduser: this is sent manually in handle_irc */
				case message_type_adduser:
					/* when we do it somewhere else, then we can also make sure to not get our logs spammed */
					break;
				case message_type_join:
				{
					t_irc_message_from from;
					from.nick = conn_get_chatname(me);
					from.user = ctag;
					from.host = addr_num_to_ip_str(conn_get_addr(me));

					t_channel* channel = conn_get_channel(me);

					if (tag_check_wolv2(conn_get_clienttag(dst)))
					{
						/**
						*  For WOLv2 the channel JOIN output must be like the following:
						*  user!WWOL@hostname JOIN :clanID,longIP channelName
						*/

						t_clan* clan = account_get_clan(conn_get_account(me));

						unsigned int clanid = 0;
						if (clan)
						{
							clanid = clan_get_clanid(clan);
						}

						std::string temp = fmt::format(":{},{}", clanid, conn_get_addr(me));
						msg = irc_message_preformat(&from, "JOIN", temp.c_str(), irc_convert_channel(channel, dst));
					}
					else
					{
						msg = irc_message_preformat(&from, "JOIN", "\r", irc_convert_channel(channel, dst));
					}

					conn_unget_chatname(me, from.nick);

					break;
				}
				case message_type_part:
				{
					t_irc_message_from from;
					from.nick = conn_get_chatname(me);
					from.user = ctag;
					from.host = addr_num_to_ip_str(conn_get_addr(me));

					msg = irc_message_preformat(&from, "PART", "\r", irc_convert_channel(conn_get_channel(me), dst));

					conn_unget_chatname(me, from.nick);

					break;
				}
				case message_type_talk:
				case message_type_whisper:
				{
					t_irc_message_from from;
					if (me)
					{
						from.nick = conn_get_chatname(me);
						from.host = addr_num_to_ip_str(conn_get_addr(me));
					}
					else
					{
						from.nick = server_get_hostname();
						from.host = server_get_hostname();
					}
					from.user = ctag;

					const char* dest;
					if (type == message_type_talk)
					{
						dest = irc_convert_channel(conn_get_channel(me), dst); // FIXME: support more channels and choose right one!
					}
					else
					{
						dest = ""; // will be replaced with username in postformat
					}

					std::string temp = fmt::format(":{}", text);

					if ((conn_get_wol(me) != 1)
						&& (conn_get_wol(dst) == 1)
						&& (conn_wol_get_pageme(dst))
						&& (type == message_type_whisper)
						&& (conn_get_channel(me) != (conn_get_channel(dst))))
					{
						msg = irc_message_preformat(&from, "PAGE", nullptr, temp.c_str());
					}
					else
					{
						msg = irc_message_preformat(&from, "PRIVMSG", dest, temp.c_str());
					}

					if (me)
					{
						conn_unget_chatname(me, from.nick);
					}

					break;
				}
				case message_type_emote:
				{
					// PELISH: WOLv1, DUNE2000 and RENEGADE shows emotes automatically to self
					if ((me == dst) && ((tag_check_wolv1(conn_get_clienttag(dst))) ||
						(conn_get_clienttag(dst) == CLIENTTAG_DUNE2000_UINT) ||
						(conn_get_clienttag(dst) == CLIENTTAG_RENEGADE_UINT) ||
						(conn_get_clienttag(dst) == CLIENTTAG_RENGDFDS_UINT)))
					{
						break;
					}

					t_irc_message_from from;
					from.nick = conn_get_chatname(me);
					from.user = ctag;
					from.host = addr_num_to_ip_str(conn_get_addr(me));

					std::string temp;

					if (fmt::formatted_size(":\001ACTION {}\001", text) <= MAX_IRC_MESSAGE_LEN)
					{
						temp = fmt::format(":\001ACTION {}\001", text);
					}
					else
					{
						temp = ":\001ACTION (maximum message length exceeded)\001";
					}


					// FIXME: also supports whisper emotes?
					const char* dest = irc_convert_channel(conn_get_channel(me), dst); // FIXME: support more channels and choose right one!
					msg = irc_message_preformat(&from, "PRIVMSG", dest, temp.c_str());

					conn_unget_chatname(me, from.nick);

					break;
				}
				case message_type_broadcast:
				case message_type_info:
				case message_type_error:
				{
					std::string temp = fmt::format(":{}", text);

					if (conn_get_wol(dst) == 1)
					{
						if ((type == message_type_info) || (type == message_type_error))
						{
							msg = irc_message_preformat(nullptr, "PAGE", nullptr, temp.c_str());
						}
						else
						{
							msg = irc_message_preformat(nullptr, "NOTICE", nullptr, temp.c_str());
						}
					}
					else
					{
						msg = irc_message_preformat(nullptr, "NOTICE", nullptr, temp.c_str());
					}

					break;
				}
				case message_type_channel:
					/* ignore it */
					break;
				case message_type_userflags:
					/* ignore it but maybe will be used for set MODE +o if user is operator
					 * but at this time is this command sended only once when user join
					 * first channel */
					break;
				case message_type_mode:
				{
					t_irc_message_from from;
					from.nick = conn_get_chatname(me);
					from.user = ctag;
					from.host = addr_num_to_ip_str(conn_get_addr(me));

					msg = irc_message_preformat(&from, "MODE", "\r", text);

					conn_unget_chatname(me, from.nick);

					break;
				}
				case message_type_nick:
				{
					t_irc_message_from from;
					from.nick = conn_get_loggeduser(me);
					from.host = addr_num_to_ip_str(conn_get_addr(me));
					from.user = ctag;

					msg = irc_message_preformat(&from, "NICK", "\r", text);

					break;
				}
				case message_type_notice:
				{
					std::string temp = fmt::format(":{}", text);

					if (me && conn_get_chatname(me))
					{
						t_irc_message_from from;
						from.nick = conn_get_chatname(me);
						from.host = addr_num_to_ip_str(conn_get_addr(me));
						from.user = ctag;

						msg = irc_message_preformat(&from, "NOTICE", nullptr, temp.c_str());
					}
					else
					{
						msg = irc_message_preformat(nullptr, "NOTICE", nullptr, temp.c_str());
					}

					break;
				}
				case message_type_namreply:
				{
					t_channel* channel = conn_get_channel(me);

					irc_send_rpl_namreply(dst, channel);

					break;
				}
				case message_type_topic:
				{
					t_channel* channel = conn_get_channel(me);

					irc_send_topic(dst, channel);

					break;
				}
				case message_type_kick:
				{
					t_irc_message_from from;
					from.nick = conn_get_chatname(me);
					from.user = ctag;
					from.host = addr_num_to_ip_str(conn_get_addr(me));

					std::string temp;
					if (text)
					{
						temp = fmt::format("{} :{}", conn_get_chatname(me), text);
					}
					else
					{
						temp = fmt::format("{} :", conn_get_chatname(me));
					}

					msg = irc_message_preformat(&from, "KICK", irc_convert_channel(conn_get_channel(me), dst), temp.c_str());

					conn_unget_chatname(me, from.nick);

					break;
				}
				case message_type_quit:
					if (conn_get_clienttag(dst) == CLIENTTAG_IIRC_UINT)
					{
						t_irc_message_from from;
						from.nick = conn_get_chatname(me);
						from.host = addr_num_to_ip_str(conn_get_addr(me));
						from.user = ctag;

						std::string temp;
						if (text)
						{
							temp = fmt::format(":{}", text);
						}
						else
						{
							temp = ":";
						}

						msg = irc_message_preformat(&from, "QUIT", "\r", temp.c_str());
					}
					else
					{
						t_irc_message_from from;
						from.nick = conn_get_chatname(me);
						from.user = ctag;
						from.host = addr_num_to_ip_str(conn_get_addr(me));

						msg = irc_message_preformat(&from, "PART", "\r", irc_convert_channel(conn_get_channel(me), dst));

						conn_unget_chatname(me, from.nick);
					}
					break;

					/**
					*  Westwood Online Extensions
					*/
				case message_type_host:
				{
					t_irc_message_from from;
					from.nick = conn_get_chatname(me);
					from.user = ctag;
					from.host = addr_num_to_ip_str(conn_get_addr(me));

					msg = irc_message_preformat(&from, "HOST", "\r", text);

					conn_unget_chatname(me, from.nick);

					break;
				}
				case message_type_invmsg:
				{
					t_irc_message_from from;
					from.nick = conn_get_chatname(me);
					from.user = ctag;
					from.host = addr_num_to_ip_str(conn_get_addr(me));

					msg = irc_message_preformat(&from, "INVMSG", nullptr, text);

					conn_unget_chatname(me, from.nick);

					break;
				}
				case message_type_page:
				{
					t_irc_message_from from;
					from.nick = conn_get_chatname(me);
					from.user = ctag;
					from.host = addr_num_to_ip_str(conn_get_addr(me));

					std::string temp = fmt::format(":{}", text);

					msg = irc_message_preformat(&from, "PAGE", nullptr, temp.c_str());

					conn_unget_chatname(me, from.nick);
				}
				break;
				case message_wol_joingame:
				{
					t_irc_message_from from;
					from.nick = conn_get_chatname(me);
					from.user = ctag;
					from.host = addr_num_to_ip_str(conn_get_addr(me));

					msg = irc_message_preformat(&from, "JOINGAME", "\r", text);

					conn_unget_chatname(me, from.nick);

					break;
				}
				case message_type_gameopt_talk:
				case message_type_gameopt_whisper:
				{
					t_irc_message_from from;
					if (me)
					{
						from.nick = conn_get_chatname(me);
						from.host = addr_num_to_ip_str(conn_get_addr(me));
					}
					else
					{
						from.nick = server_get_hostname();
						from.host = server_get_hostname();
					}
					from.user = ctag;

					char const* dest;
					if (type == message_type_gameopt_talk)
					{
						dest = irc_convert_channel(conn_get_channel(me), dst); // FIXME: support more channels and choose right one!
					}
					else
					{
						dest = nullptr; // will be replaced with username in postformat
					}

					std::string temp = fmt::format(":{}", text);

					msg = irc_message_preformat(&from, "GAMEOPT", dest, temp.c_str());

					if (me)
					{
						conn_unget_chatname(me, from.nick);
					}

					break;
				}
				case message_wol_start_game:
				{
					t_irc_message_from from;
					from.nick = conn_get_chatname(me);
					from.user = ctag;
					from.host = addr_num_to_ip_str(conn_get_addr(me));

					msg = irc_message_preformat(&from, "STARTG", nullptr, text);

					conn_unget_chatname(me, from.nick);

					break;
				}
				case message_wol_advertr:
					msg = irc_message_preformat(nullptr, "ADVERTR", "\r", text);
					break;
				case message_wol_chanchk:
					msg = irc_message_preformat(nullptr, "CHANCHK", "\r", text);
					break;
				case message_wol_userip:
				{
					t_irc_message_from from;
					from.nick = conn_get_chatname(me);
					from.user = ctag;
					from.host = addr_num_to_ip_str(conn_get_addr(me));

					msg = irc_message_preformat(&from, "USERIP", "\r", text);

					conn_unget_chatname(me, from.nick);

					break;
				}
				default:
					eventlog(eventlog_level_warn, __FUNCTION__, "unknown message type");
					return -1;
				}

				if (!msg.empty())
				{
					if (msg.length() > MAX_IRC_MESSAGE_LEN)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "irc message length too long (got {} bytes, max {} bytes)", msg.length(), MAX_IRC_MESSAGE_LEN);
					}
					else
					{
						packet_append_string(packet, msg.c_str());
						return 0;
					}
				}

				return -1;
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] failed to send IRC message ({})", conn_get_socket(me), e.what());
				return -1;
			}
		}

		int irc_send_rpl_namreply_internal(t_connection* c, t_channel const* channel)
		{
			if (!channel)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return -1;
			}

			const char* channel_irc_name = irc_convert_channel(channel, c);
			if (!channel_irc_name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "channel has NULL ircname");
				return -1;
			}

			// '@' = secret; '*' = private; '=' = public
			const std::string beginning_params = fmt::format("{} {} :", channel_get_permanent(channel) ? '=' : '*', channel_irc_name);
			std::string params = beginning_params;
			const std::size_t max_params_length = irc_send_get_max_params_length(c);
			params.reserve(max_params_length);

			if (!(channel_get_flags(channel) & channel_flags_thevoid))
			{
				bool is_first_user = true; // for determining whether to prepend leading whitespace to current user

				// manually add 'matchbot' to the beginning of the user list if this message is being sent to a WOL client and
				// the channel is #Lob_38_0, #Lob_39_0, or #Lob_40_0.
				if (conn_get_wol(c) == 1 && ((std::strcmp(channel_irc_name, "#Lob_38_0") == 0) ||
					(std::strcmp(channel_irc_name, "#Lob_39_0") == 0) ||
					(std::strcmp(channel_irc_name, "#Lob_40_0") == 0)))
				{
					// FIXME: shouldn't we check if the t_connection pointed to by 'c' is a WOLv2 client or not
					params += "@matchbot,0,0";
					is_first_user = false;
				}

				for (t_connection* m = channel_get_first(channel); m; m = channel_get_next())
				{
					char const* name = conn_get_chatname(m);
					if (!name)
					{
						continue;
					}

					char flg[2] = {};
					unsigned int flags = conn_get_flags(m);
					if (flags & MF_BLIZZARD)
					{
						flg[0] = '@';
					}
					else if ((flags & MF_BNET) || (flags & MF_GAVEL))
					{
						flg[0] = '%';
					}
					else if (flags & MF_VOICE)
					{
						flg[0] = '+';
					}

					// specially handle 'flg' for all WOL clients
					if (conn_get_wol(c) == 1)
					{
						t_game* game = conn_get_game(m);
						if ((game) && (game_get_channel(game) == channel) && (game_get_owner(game) == m))
						{
							/* PELISH: Only game owners will have OP flag (this prevent official OP to be normal player) */
							flg[0] = '@';
						}
						else
						{
							if ((flags & MF_BNET) || (flags & MF_GAVEL))
							{
								/* PELISH: WOL does not understand to '%' char so we using '@' for TempOP too */
								flg[0] = '@';
							}
						}
					}

					if (tag_check_wolv2(conn_get_clienttag(c)))
					{
						// send IRC user list in custom format for WOLv2 clients

						/* BATTLECLAN Support */
						t_clan* clan = account_get_clan(conn_get_account(m));
						unsigned int clanid = 0;

						if (clan)
						{
							clanid = clan_get_clanid(clan);
						}

						if ((params.length() + fmt::formatted_size("{}{}{},{},{}", is_first_user ? "" : " ", flg, name, clanid, conn_get_addr(m))) > max_params_length)
						{
							// can't append next user to user list, send current user list out
							irc_send(c, RPL_NAMREPLY, params.c_str());
							params = beginning_params;
							is_first_user = true;
						}

						params += fmt::format("{}{}{},{},{}", is_first_user ? "" : " ", flg, name, clanid, conn_get_addr(m));
					}
					else
					{
						// send IRC user list in standard format for non-WOLv2 clients

						if ((params.length() + fmt::formatted_size("{}{}{}", is_first_user ? "" : " ", flg, name)) > max_params_length)
						{
							// can't append next user to user list, send current user list out
							irc_send(c, RPL_NAMREPLY, params.c_str());
							params = beginning_params;
							is_first_user = true;
						}

						params += fmt::format("{}{}{}", is_first_user ? "" : " ", flg, name);
					}

					is_first_user = false;

					conn_unget_chatname(m, name);
				}
			}

			irc_send(c, RPL_NAMREPLY, params.c_str());

			return 0;
		}

		extern int irc_send_rpl_namreply(t_connection* c, t_channel const* channel)
		{
			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			std::string temp;
			if (channel)
			{
				const char* ircname = irc_convert_channel(channel, c);
				if (!ircname)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "channel has NULL ircname");
					return -1;
				}

				irc_send_rpl_namreply_internal(c, channel);

				temp = fmt::format("{} :End of NAMES list", ircname);
			}
			else
			{
				t_elem const* curr;
				LIST_TRAVERSE_CONST(channellist(), curr)
				{
					t_channel* curr_channel = static_cast<t_channel*>(elem_get_data(curr));
					irc_send_rpl_namreply_internal(c, channel);
				}

				temp = "* :End of NAMES list";
			}

			irc_send(c, RPL_ENDOFNAMES, temp.c_str());

			return 0;
		}

		static int irc_who_connection(t_connection* dest, t_connection* c)
		{
			if (!dest)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL destination");
				return -1;
			}

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			try
			{
				t_account* a = conn_get_account(c);
				char const* tempuser;
				if (!(tempuser = clienttag_uint_to_str(conn_get_clienttag(c))))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL clienttag (tempuser)");
					return -1;
				}

				char const* tempowner;
				if (!(tempowner = account_get_ll_owner(a)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL ll_owner (tempowner)");
					return -1;
				}

				char const* tempname;
				if (!(tempname = conn_get_username(c)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL username (tempname)");
					return -1;
				}

				char const* tempip;
				if (!(tempip = addr_num_to_ip_str(conn_get_addr(c))))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL addr (tempip)");
					return -1;
				}

				char const* tempchannel;
				if (!(tempchannel = irc_convert_channel(conn_get_channel(c), c)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel (tempchannel)");
					return -1;
				}

				char const* tempflags = "@"; // FIXME: that's dumb
				std::string temp = fmt::format("{} {} {} {} {} {}{} :0 {}", tempchannel, tempuser, tempip, server_get_hostname(), tempname, 'H', tempflags, tempowner);
				if (temp.length() > MAX_IRC_MESSAGE_LEN)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "irc message length too long (got {} bytes, max {} bytes)", temp.length(), MAX_IRC_MESSAGE_LEN);
					return -1;
				}

				irc_send(dest, RPL_WHOREPLY, temp.c_str());

				return 0;
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] failed to send IRC message ({})", conn_get_socket(c), e.what());
				return -1;
			}
		}

		extern int irc_who(t_connection* c, char const* name)
		{
			// FIXME: support wildcards!

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (!name)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL name");
				return -1;
			}

			try
			{
				if ((name[0] == '#') || (name[0] == '&') || (name[0] == '!'))
				{
					// it's a channel
					const char* ircname = irc_convert_ircname(name);
					t_channel* channel = channellist_find_channel_by_name(ircname, nullptr, nullptr);

					if (!channel)
					{
						std::string temp = fmt::format(":No such channel {}", name);
						if (temp.length() > MAX_IRC_MESSAGE_LEN)
						{
							eventlog(eventlog_level_error, __FUNCTION__, "irc message length too long (got {} bytes, max {} bytes)", temp.length(), MAX_IRC_MESSAGE_LEN);
							irc_send(c, ERR_NOSUCHCHANNEL, ":No such channel");
						}
						else
						{
							irc_send(c, ERR_NOSUCHCHANNEL, temp.c_str());
						}

						return 0;
					}

					for (t_connection* info = channel_get_first(channel); info; info = channel_get_next())
					{
						irc_who_connection(c, info);
					}
				}
				else
				{
					// it's just one user
					t_connection* info;
					if ((info = connlist_find_connection_by_accountname(name)))
					{
						return irc_who_connection(c, info);
					}
				}

				return 0;
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] failed to send IRC message ({})", conn_get_socket(c), e.what());
				return -1;
			}
		}

		extern int irc_send_motd(t_connection* conn)
		{
			if (!conn)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			std::FILE* fp = nullptr;
			char* formatted_line = nullptr;
			try
			{
				bool send_failed_motd = false;

				std::string filename = i18n_filename(prefs_get_motdfile(), conn_get_gamelang_localized(conn));
				std::FILE* fp = std::fopen(filename.c_str(), "r");
				if (fp)
				{
					bool first = true;
					const char* line;
					while ((line = file_get_line(fp)))
					{
						if ((formatted_line = message_format_line(conn, line)))
						{
							formatted_line[0] = ' ';
							std::string send_line = fmt::format(":-{}", formatted_line);

							if (first)
							{
								irc_send(conn, RPL_MOTDSTART, send_line.c_str());
								first = false;
							}
							else
							{
								irc_send(conn, RPL_MOTD, send_line.c_str());
							}

							xfree(formatted_line);
							formatted_line = nullptr;
						}
					}

					file_get_line(nullptr); // clear file_get_line buffer
					std::fclose(fp);
					fp = nullptr;
				}
				else
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not open motd file \"{}\" for reading (std::fopen: {})", filename, std::strerror(errno));
					send_failed_motd = true;
				}

				if (send_failed_motd)
				{
					irc_send(conn, RPL_MOTDSTART, ":- Failed to load motd, sending default motd              ");
					irc_send(conn, RPL_MOTD, ":- ====================================================== ");
					irc_send(conn, RPL_MOTD, ":-                 http://www.pvpgn.pro                   ");
					irc_send(conn, RPL_MOTD, ":- ====================================================== ");
				}

				irc_send(conn, RPL_ENDOFMOTD, ":End of /MOTD command");

				return 0;
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] failed to send IRC message ({})", conn_get_socket(conn), e.what());

				if (fp)
				{
					std::fclose(fp);
				}

				if (formatted_line)
				{
					xfree(formatted_line);
				}

				file_get_line(nullptr); // clear file_get_line buffer

				return -1;
			}
		}

		int irc_welcome(t_connection* conn)
		{
			if (conn_get_wol(conn))
			{
				return handle_wol_welcome(conn);
			}
			else
			{
				return handle_irc_welcome(conn);
			}
		}

		extern int _handle_nick_command(t_connection* conn, int numparams, char** params, char* text)
		{
			/* FIXME: more strict param checking */

			try
			{
				if ((conn_get_loggeduser(conn)) &&
					(conn_get_state(conn) != conn_state_bot_password &&
						conn_get_state(conn) != conn_state_bot_username))
				{
					irc_send(conn, ERR_RESTRICTED, ":You can't change your nick after login");
				}
				else
				{
					if ((params) && (params[0]))
					{
						// truncate to (MAX_USERNAME_LEN - 1)
						std::string nickname = fmt::format("{:." + std::to_string(MAX_USERNAME_LEN - 1) + "}", params[0]);

						if (conn_get_loggeduser(conn))
						{
							std::string temp = fmt::format(":{}", nickname);
							message_send_text(conn, message_type_nick, conn, temp);
						}

						conn_set_loggeduser(conn, nickname.c_str());
					}
					else if (text)
					{
						// truncate to (MAX_USERNAME_LEN - 1)
						std::string nickname = fmt::format("{:." + std::to_string(MAX_USERNAME_LEN - 1) + "}", text);

						if (conn_get_loggeduser(conn))
						{
							std::string temp = fmt::format(":{}", nickname);
							message_send_text(conn, message_type_nick, conn, temp);
						}

						conn_set_loggeduser(conn, nickname.c_str());
					}
					else
					{
						irc_send(conn, ERR_NEEDMOREPARAMS, "NICK :Not enough parameters");
					}

					if ((conn_get_user(conn)) && (conn_get_loggeduser(conn)))
					{
						irc_welcome(conn); // only send the welcome if we have USER and NICK
					}
				}

				return 0;
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] failed to send IRC message ({})", conn_get_socket(conn), e.what());

				return -1;
			}
		}

		extern int _handle_ping_command(t_connection* conn, int numparams, char** params, char* text)
		{
			/* Dizzy: just ignore this because RFC says we should not reply client PINGs
			 * NOTE: RFC2812 doesn't seem to be very expressive about this ... */
			if (numparams)
			{
				irc_send_pong(conn, params[0]);
			}
			else
			{
				irc_send_pong(conn, text);
			}

			return 0;
		}

		extern int _handle_pong_command(t_connection* conn, int numparams, char** params, char* text)
		{
			/* NOTE: RFC2812 doesn't seem to be very expressive about this ... */
			if (conn_get_ircping(conn) == 0) {
				eventlog(eventlog_level_warn, __FUNCTION__, "[{}] PONG without PING", conn_get_socket(conn));
			}
			else {
				unsigned int val = 0;
				char* sname;

				if (numparams >= 1) {
					val = std::strtoul(params[0], NULL, 10);
					sname = params[0];
				}
				else if (text) {
					val = std::strtoul(text, NULL, 10);
					sname = text;
				}
				else {
					val = 0;
					sname = 0;
				}

				if (conn_get_ircping(conn) != val) {
					if ((!(sname)) || (std::strcmp(sname, server_get_hostname()) != 0)) {
						/* Actually the servername should not be always accepted but we aren't that pedantic :) */
						eventlog(eventlog_level_warn, __FUNCTION__, "[{}] got bad PONG ({}!={} && {}!={})", conn_get_socket(conn), val, conn_get_ircping(conn), sname, server_get_hostname());
						return -1;
					}
				}
				conn_set_latency(conn, get_ticks() - conn_get_ircping(conn));
				eventlog(eventlog_level_debug, __FUNCTION__, "[{}] latency is now {} ({}-{})", conn_get_socket(conn), get_ticks() - conn_get_ircping(conn), get_ticks(), conn_get_ircping(conn));
				conn_set_ircping(conn, 0);
			}
			return 0;
		}

		extern int _handle_join_command(t_connection* conn, int numparams, char** params, char* text)
		{
			if (numparams >= 1)
			{
				char** e;

				/* According to RFC2812 - if channelname is "0" we need to PART all channels */
				if (std::strcmp(params[0], "0") == 0)
				{
					conn_part_channel(conn);
					return 0;
				}

				e = irc_get_listelems(params[0]);
				if ((e) && (e[0]))
				{
					char temp[MAX_IRC_MESSAGE_LEN];
					char const* ircname = irc_convert_ircname(e[0]);
					t_channel* old_channel = conn_get_channel(conn);
					t_channel* channel;
					t_account* acc = conn_get_account(conn);

					if ((ircname) && (channel = channellist_find_channel_by_name(ircname, NULL, NULL)))
					{
						if (channel_check_banning(channel, conn))
						{
							std::snprintf(temp, sizeof(temp), "%s :You are banned from that channel.", e[0]);
							irc_send(conn, ERR_BANNEDFROMCHAN, temp);
							if (e)
							{
								irc_unget_listelems(e);
							}

							return 0;
						}

						if ((account_get_auth_admin(acc, NULL) != 1) && (account_get_auth_admin(acc, ircname) != 1) &&
							(account_get_auth_operator(acc, NULL) != 1) && (account_get_auth_operator(acc, ircname) != 1) &&
							(channel_get_max(channel) != -1) && (channel_get_curr(channel) >= channel_get_max(channel)))
						{

							std::snprintf(temp, sizeof(temp), "%s :The channel is currently full.", e[0]);
							irc_send(conn, ERR_CHANNELISFULL, temp);
							if (e)
							{
								irc_unget_listelems(e);
							}
							return 0;
						}
					}

					if ((!(ircname)) || (conn_set_channel(conn, ircname) < 0))
					{
						std::snprintf(temp, sizeof(temp), "%s :JOIN failed", e[0]);
						irc_send(conn, ERR_NOSUCHCHANNEL, temp); /* Anything is still bad */
					}
				}

				if (e)
				{
					irc_unget_listelems(e);
				}
			}
			else
			{
				irc_send(conn, ERR_NEEDMOREPARAMS, "JOIN :Not enough parameters");
			}

			return 0;
		}

		extern int irc_send_topic(t_connection* c, t_channel const* channel)
		{
			nonstd::optional<std::string> topic = channel_get_topic(channel);
			char temp[MAX_IRC_MESSAGE_LEN];

			if (topic.has_value())
			{
				std::snprintf(temp, sizeof(temp), "%s :%s", irc_convert_channel(channel, c), topic.value().c_str());
				irc_send(c, RPL_TOPIC, temp);
			}
			else
			{
				std::snprintf(temp, sizeof(temp), "%s :", irc_convert_channel(channel, c));
				irc_send(c, RPL_TOPIC, temp);
				/* PELISH: This is according to RFC2812 but brakes WOLv1/WOLv2 */
				//snprintf(temp, sizeof(temp), "%s :No topic is set", irc_convert_channel(channel,c));
				//irc_send(c, RPL_NOTOPIC, temp);
			}

			return 0;
		}

		extern int _handle_topic_command(t_connection* conn, int numparams, char** params, char* text)
		{
			char** e = NULL;
			char temp[MAX_IRC_MESSAGE_LEN];

			if (params && params[0])
			{
				if (conn_get_wol(conn) == 1) {
					t_channel* channel = conn_get_channel(conn);
					if (channel)
					{
						channel_set_topic(channel, text);
					}
					else
					{
						std::snprintf(temp, sizeof(temp), "%s :You're not on that channel", params[0]);
						irc_send(conn, ERR_NOTONCHANNEL, temp);
					}
				}
				e = irc_get_listelems(params[0]);
			}
			if ((e) && (e[0]))
			{
				t_channel* channel = conn_get_channel(conn);

				if (channel) {
					char const* ircname = irc_convert_ircname(e[0]);

					if ((ircname) && (strcasecmp(channel_get_name(channel), ircname) == 0)) {
						irc_send_topic(conn, channel);
					}
					else {
						std::snprintf(temp, sizeof(temp), "%s :You're not on that channel", e[0]);
						irc_send(conn, ERR_NOTONCHANNEL, temp);
					}
				}
				else {
					std::snprintf(temp, sizeof(temp), "%s :You're not on that channel", e[0]);
					irc_send(conn, ERR_NOTONCHANNEL, temp);
				}
			}
			else
				irc_send(conn, ERR_NEEDMOREPARAMS, "TOPIC :Not enough parameters");

			if (e)
			{
				irc_unget_listelems(e);
			}

			return 0;
		}

		extern int _handle_kick_command(t_connection* conn, int numparams, char** params, char* text)
		{
			/**
			*  Heres the input expected
			*  KICK [channel] [kicked_user],[kicked_user2]
			*
			*  Heres the output expected
			*  :user!WWOL@hostname KICK [channel] [kicked_user] :[text]
			*
			*  WOL in [text] sends Admin name
			*/

			if ((numparams != 2) || !(params[1]))
			{
				irc_send(conn, ERR_NEEDMOREPARAMS, "KICK :Not enough parameters");
				return 0;
			}

			try
			{
				char** e;
				if (e = irc_get_listelems(params[1]))
				{
					std::string temp;

					/* Make standard PvPGN KICK from RFC2812 KICK */
					if (text)
					{
						temp = fmt::format("/kick {} {}", e[0], text);
					}
					else
					{
						temp = fmt::format("/kick {}", e[0]);
					}

					handle_command(conn, temp.c_str());

					irc_unget_listelems(e);
				}

				return 0;
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[{}] failed to send IRC message ({})", conn_get_socket(conn), e.what());

				return -1;
			}
		}

		static int irc_send_banlist(t_connection* conn, t_channel* channel)
		{
			if (!conn)
			{
				ERROR0("got NULL conn");
				return -1;
			}

			if (!channel)
			{
				ERROR0("got NULL channel");
				return -1;
			}

			const char* server_hostname = server_get_hostname();
			const char* irc_channelname = irc_convert_channel(channel, conn);

			const t_elem* curr;
			LIST_TRAVERSE_CONST(channel_get_banlist(channel), curr)
			{
				try
				{
					const char* banned_username = static_cast<char*>(elem_get_data(curr));

					//FIXME: right now we lie about who have gives ban and also about bantime
					std::string temp = fmt::format("{} {}!*@* {} 1208297879", irc_channelname, banned_username, server_hostname);
					irc_send(conn, RPL_BANLIST, temp.c_str());
				}
				catch (...)
				{
					continue;
				}
			}

			return 0;
		}

		extern int _handle_mode_command(t_connection* conn, int numparams, char** params, char* text)
		{
			char temp[MAX_IRC_MESSAGE_LEN];
			t_account* acc = conn_get_account(conn);

			std::memset(temp, 0, sizeof(temp));

			if (numparams < 1) {
				irc_send(conn, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters");
				return 0;
			}

			if (params[0][0] == '#') {
				/* Channel mode */
				t_channel* channel;
				char const* ircname = irc_convert_ircname(params[0]);

				/* FIXME: Supports more than one channel in MODE command */
				if (!(channel = conn_get_channel(conn))) {
					std::snprintf(temp, sizeof(temp), "%s :No such channel", params[0]);
					irc_send(conn, ERR_NOSUCHCHANNEL, temp);
					return 0;
				}

				if (numparams == 1) {
					/* FIXME: Send real CHANELMODE flags! */
					std::snprintf(temp, sizeof(temp), "%s +tns", params[0]);
					irc_send(conn, RPL_CHANNELMODEIS, temp);
					return 0;
				}

				if (numparams == 2) {
					if (std::strcmp(params[1], "b") == 0) {
						irc_send_banlist(conn, channel);
						std::snprintf(temp, sizeof(temp), "%s :End of channel ban list", params[0]);
						irc_send(conn, RPL_ENDOFBANLIST, temp);
						return 0;
					}
					else {
						std::snprintf(temp, sizeof(temp), ":%s is unknown mode char to me for %s", params[1], params[0]);
						irc_send(conn, ERR_UNKNOWNMODE, temp);
						return 0;
					}
				}

				/* PELISH: Also tmpOP have setting modes allowed because all new channels have only tmpOP */
				if ((channel_conn_is_tmpOP(channel, conn) != 1) &&
					(account_get_auth_admin(acc, NULL) != 1) && (account_get_auth_admin(acc, ircname) != 1) &&
					(account_get_auth_operator(acc, NULL) != 1) && (account_get_auth_operator(acc, ircname) != 1)) {
					std::snprintf(temp, sizeof(temp), "%s :You're not channel operator", params[0]);
					irc_send(conn, ERR_CHANOPRIVSNEEDED, temp);
					return 0;
				}

				if (std::strcmp(params[1], "+b") == 0) {
					channel_ban_user(channel, params[2]);
					std::snprintf(temp, sizeof(temp), "%s %s %s!*@*", params[0], params[1], params[2]);
					channel_message_send(channel, message_type_mode, conn, temp);
				}
				else if (std::strcmp(params[1], "-b") == 0) {
					channel_unban_user(channel, params[2]);
					std::snprintf(temp, sizeof(temp), "%s %s %s!*@*", params[0], params[1], params[2]);
					channel_message_send(channel, message_type_mode, conn, temp);
				}
				else if (std::strcmp(params[1], "+o") == 0) {
					std::snprintf(temp, sizeof(temp), "/op %s", params[2]);
					handle_command(conn, temp);
					std::snprintf(temp, sizeof(temp), "%s %s %s", params[0], params[1], params[2]);
					channel_message_send(channel, message_type_mode, conn, temp);
				}
				else if (std::strcmp(params[1], "-o") == 0) {
					std::snprintf(temp, sizeof(temp), "/deop %s", params[2]);
					handle_command(conn, temp);
					std::snprintf(temp, sizeof(temp), "%s %s %s", params[0], params[1], params[2]);
					channel_message_send(channel, message_type_mode, conn, temp);
				}
				else if (std::strcmp(params[1], "+v") == 0) {
					std::snprintf(temp, sizeof(temp), "/voice %s", params[2]);
					handle_command(conn, temp);
					std::snprintf(temp, sizeof(temp), "%s %s %s", params[0], params[1], params[2]);
					channel_message_send(channel, message_type_mode, conn, temp);
				}
				else if (std::strcmp(params[1], "-v") == 0) {
					std::snprintf(temp, sizeof(temp), "/devoice %s", params[2]);
					handle_command(conn, temp);

					std::snprintf(temp, sizeof(temp), "%s %s %s", params[0], params[1], params[2]);
					channel_message_send(channel, message_type_mode, conn, temp);
				}
				else if (std::strcmp(params[1], "+i") == 0) {
					/* FIXME: channel will be only for invited */
				}
				else if (std::strcmp(params[1], "+l") == 0) {
					channel_set_max(channel, std::atoi(params[2]));
					std::snprintf(temp, sizeof(temp), "%s %s %s", params[0], params[1], params[2]);
					channel_message_send(channel, message_type_mode, conn, temp);
				}
				else if (std::strcmp(params[1], "-l") == 0) {
					channel_set_max(channel, -1);
					std::snprintf(temp, sizeof(temp), "%s %s", params[0], params[1]);
					channel_message_send(channel, message_type_mode, conn, temp);
				}
				else {
					std::snprintf(temp, sizeof(temp), ":%s is unknown mode char to me for %s", params[1], params[0]);
					irc_send(conn, ERR_UNKNOWNMODE, temp);
				}
			}
			else {
				/* User modes */
				/* FIXME: Support user modes (away, invisible...) */
				irc_send(conn, ERR_UMODEUNKNOWNFLAG, ":Unknown MODE flag");
			}
			return 0;
		}

		extern int _handle_time_command(t_connection* conn, int numparams, char** params, char* text)
		{
			char temp[MAX_IRC_MESSAGE_LEN] = {}; // PELISH: According to RFC2812
			std::time_t now;
			char const* ircname = server_get_hostname();

			if ((numparams >= 1) && (params[0]))
			{
				if (std::strcmp(params[0], ircname) == 0)
				{
					now = std::time(NULL);
					std::snprintf(temp, sizeof(temp), "%s :%" PRId64, ircname, static_cast<std::int64_t>(now));
					irc_send(conn, RPL_TIME, temp);
				}
				else
				{
					std::snprintf(temp, sizeof(temp), "%s :No such server", params[0]);
					irc_send(conn, ERR_NOSUCHSERVER, temp);
				}
			}
			else
			{
				/* RPL_TIME contains time and name of this server */
				now = std::time(NULL);
				std::snprintf(temp, sizeof(temp), "%s :%" PRId64, ircname, static_cast<std::int64_t>(now));
				irc_send(conn, RPL_TIME, temp);
			}

			return 0;
		}

	}

}
