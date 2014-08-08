/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999,2001,2002  Ross Combs (rocombs@cs.nmsu.edu)
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
#define MESSAGE_INTERNAL_ACCESS
#include "message.h"

#include <cstring>
#include <cerrno>
#include <string>

#include "compat/gethostname.h"
#include "common/xalloc.h"
#include "common/eventlog.h"
#include "common/addr.h"
#include "common/tag.h"
#include "common/packet.h"
#include "common/bot_protocol.h"
#include "common/bn_type.h"
#include "common/list.h"
#include "common/util.h"
#include "common/xstring.h"

#include "account.h"
#include "account_wrap.h"
#include "channel.h"
#include "channel_conv.h"
#include "game.h"
#include "mail.h"
#include "prefs.h"
#include "connection.h"
#include "irc.h"
#include "command.h"
#include "i18n.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static int message_telnet_format(t_packet * packet, t_message_type type, t_connection * me, t_connection * dst, char const * text, unsigned int dstflags);
		static int message_bot_format(t_packet * packet, t_message_type type, t_connection * me, t_connection * dst, char const * text, unsigned int dstflags);
		static int message_bnet_format(t_packet * packet, t_message_type type, t_connection * me, t_connection * dst, char const * text, unsigned int dstflags);
		static t_packet * message_cache_lookup(t_message * message, t_connection *dst, unsigned int flags);

		static char const * message_type_get_str(t_message_type type)
		{
			switch (type)
			{
			case message_type_adduser:
				return "adduser";
			case message_type_join:
				return "join";
			case message_type_part:
				return "part";
			case message_type_whisper:
				return "whisper";
			case message_type_talk:
				return "talk";
			case message_type_broadcast:
				return "broadcast";
			case message_type_channel:
				return "channel";
			case message_type_userflags:
				return "userflags";
			case message_type_whisperack:
				return "whisperack";
			case message_type_friendwhisperack:  //[zap-zero] 20020518
				return "friendwhisperack";
			case message_type_channelfull:
				return "channelfull";
			case message_type_channeldoesnotexist:
				return "channeldoesnotexist";
			case message_type_channelrestricted:
				return "channelrestricted";
			case message_type_info:
				return "info";
			case message_type_error:
				return "error";
			case message_type_emote:
				return "emote";
			case message_type_uniqueid:
				return "uniqueid";
			case message_type_notice:
				return "notice";
			case message_type_nick:
				return "nick";
			case message_type_namreply:
				return "namreply";
			case message_type_topic:
				return "topic";
			case message_type_mode:
				return "mode";
			case message_type_host:
				return "host";
			case message_type_invmsg:
				return "invmsg";
			case message_type_page:
				return "page";
			case message_type_kick:
				return "kick";
			case message_type_quit:
				return "quit";
			case message_wol_joingame:
				return "wol_joingame";
			case message_type_gameopt_talk:
				return "gameopt_talk";
			case message_type_gameopt_whisper:
				return "gameopt_whisper";
			case message_wol_start_game:
				return "wol_start_game";
			case message_wol_advertr:
				return "wol_advertr";
			case message_wol_chanchk:
				return "wol_chanchk";
			case message_wol_userip:
				return "wol_userip";
			case message_type_null:
				return "null";
			default:
				return "UNKNOWN";
			}
		}


		/* make sure none of the expanded format symbols is longer than this (with null) */
#define MAX_INC 64

		extern char * message_format_line(t_connection const * c, char const * in)
		{
			char *       out;
			unsigned int inpos;
			unsigned int outpos;
			unsigned int outlen = MAX_INC;
			unsigned int inlen;
			char         clienttag_str[5];

			out = (char*)xmalloc(outlen + 1);

			inlen = std::strlen(in);
			out[0] = 'I';
			for (inpos = 0, outpos = 1; inpos < inlen; inpos++)
			{
				if (in[inpos] != '%')
				{
					out[outpos] = in[inpos];
					outpos += 1;
				}
				else
					switch (in[++inpos])
				{
					case '%':
						out[outpos++] = '%';
						break;

					case 'a':
						std::sprintf(&out[outpos], "%u", accountlist_get_length());
						outpos += std::strlen(&out[outpos]);
						break;

					case 'c':
						std::sprintf(&out[outpos], "%d", channellist_get_length());
						outpos += std::strlen(&out[outpos]);
						break;

					case 'g':
						std::sprintf(&out[outpos], "%d", gamelist_get_length());
						outpos += std::strlen(&out[outpos]);
						break;

					case 'h':
						if (gethostname(&out[outpos], MAX_INC) < 0)
						{
							eventlog(eventlog_level_error, __FUNCTION__, "could not get hostname (gethostname: %s)", std::strerror(errno));
							std::strcpy(&out[outpos], "localhost"); /* not much else you can do */
						}
						outpos += std::strlen(&out[outpos]);
						break;

					case 'i':
						std::sprintf(&out[outpos], UID_FORMAT, conn_get_userid(c));
						outpos += std::strlen(&out[outpos]);
						break;

					case 'l':
					{
								char const * tname;

								std::strncpy(&out[outpos], (tname = (conn_get_chatname(c) ? conn_get_chatname(c) : conn_get_loggeduser(c))), MAX_USERNAME_LEN - 1);
								conn_unget_chatname(c, tname);
					}
						out[outpos + MAX_USERNAME_LEN - 1] = '\0';
						outpos += std::strlen(&out[outpos]);
						break;

					case 'm':
					{
								unsigned mails = check_mail(c);
								if (mails > 0)
									std::sprintf(&out[outpos], "You have %u message(s) in your mailbox.", mails);
								else std::strcpy(&out[outpos], "You have no mail.");
								outpos += std::strlen(&out[outpos]);
								break;
					}

					case 'r':
						std::strncpy(&out[outpos], addr_num_to_ip_str(conn_get_addr(c)), MAX_INC - 1);
						out[outpos + MAX_INC - 1] = '\0';
						outpos += std::strlen(&out[outpos]);
						break;

					case 's':
						std::sprintf(&out[outpos], "%s", prefs_get_servername());
						outpos += std::strlen(&out[outpos]);
						break;

					case 't':
						std::sprintf(&out[outpos], "%s", tag_uint_to_str(clienttag_str, conn_get_clienttag(c)));
						outpos += std::strlen(&out[outpos]);
						break;

					case 'u':
						std::sprintf(&out[outpos], "%d", connlist_login_get_length());
						outpos += std::strlen(&out[outpos]);
						break;

					case 'v':
						std::strcpy(&out[outpos], PVPGN_SOFTWARE" "PVPGN_VERSION);
						outpos += std::strlen(&out[outpos]);
						break;

					case 'C': /* simulated command */
						out[0] = 'C';
						break;

					case 'B': /* BROADCAST */
						out[0] = 'B';
						break;

					case 'E': /* ERROR */
						out[0] = 'E';
						break;

					case 'G':
						std::sprintf(&out[outpos], "%d", game_get_count_by_clienttag(conn_get_clienttag(c)));
						outpos += std::strlen(&out[outpos]);
						break;

					case 'H':
						std::strcpy(&out[outpos], prefs_get_contact_name());
						outpos += std::strlen(&out[outpos]);
						break;

					case 'I': /* INFO */
						out[0] = 'I';
						break;

					case 'M': /* MESSAGE */
						out[0] = 'M';
						break;

					case 'N':
						std::strcpy(&out[outpos], clienttag_get_title(conn_get_clienttag(c)));
						outpos += std::strlen(&out[outpos]);
						break;

					case 'T': /* EMOTE */
						out[0] = 'T';
						break;

					case 'U':
						std::sprintf(&out[outpos], "%d", conn_get_user_count_by_clienttag(conn_get_clienttag(c)));
						outpos += std::strlen(&out[outpos]);
						break;

					case 'W': /* INFO */
						out[0] = 'W';
						break;

					default:
						eventlog(eventlog_level_warn, __FUNCTION__, "bad formatter \"%%%c\"", in[inpos - 1]);
				}

				if ((outpos + MAX_INC) >= outlen)
				{
					char * newout;

					outlen += MAX_INC;
					newout = (char*)xrealloc(out, outlen);
					out = newout;
				}
			}
			out[outpos] = '\0';

			return out;
		}


		static int message_telnet_format(t_packet * packet, t_message_type type, t_connection * me, t_connection * dst, char const * text, unsigned int dstflags)
		{
			char * msgtemp;

			if (!packet)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
				return -1;
			}

			switch (type)
			{
			case message_type_uniqueid:
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				msgtemp = (char*)xmalloc(std::strlen(text) + 32);
				std::sprintf(msgtemp, "Your unique name: %s\r\n", text);
				break;
			case message_type_adduser:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				{
					char const * tname;

					tname = conn_get_chatcharname(me, dst);
					msgtemp = (char*)xmalloc(std::strlen(tname) + 32);
					std::sprintf(msgtemp, "[%s is here]\r\n", tname);
					conn_unget_chatcharname(me, tname);
				}
				break;
			case message_type_join:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				if (me == dst)
					return -1;
				else
				{
					char const * tname;

					tname = conn_get_chatcharname(me, dst);
					msgtemp = (char*)xmalloc(std::strlen(tname) + 32);
					std::sprintf(msgtemp, "[%s enters]\r\n", tname);
					conn_unget_chatcharname(me, tname);
				}
				break;
			case message_type_part:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				{
					char const * tname;

					tname = conn_get_chatcharname(me, dst);
					msgtemp = (char*)xmalloc(std::strlen(tname) + 32);
					std::sprintf(msgtemp, "[%s leaves]\r\n", tname);
					conn_unget_chatcharname(me, tname);
				}
				break;
			case message_type_kick:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				{
					char const * tname;

					tname = conn_get_chatcharname(me, dst);
					msgtemp = (char*)xmalloc(std::strlen(tname) + 32);
					std::sprintf(msgtemp, "[%s has been kicked]\r\n", tname);
					conn_unget_chatcharname(me, tname);
				}
				break;
			case message_type_quit:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				{
					char const * tname;

					tname = conn_get_chatcharname(me, dst);
					msgtemp = (char*)xmalloc(std::strlen(tname) + 32);
					std::sprintf(msgtemp, "[%s quit]\r\n", tname);
					conn_unget_chatcharname(me, tname);
				}
				break;
			case message_type_whisper:
			case message_type_notice:
			case message_type_page:
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				if (dstflags&MF_X)
					return -1; /* player is ignored */
				{
					char const * tname;
					char const * newtext;

					if (me)
						tname = conn_get_chatcharname(me, dst);
					else
						tname = prefs_get_servername();

					if ((newtext = escape_chars(text, std::strlen(text))))
					{
						msgtemp = (char*)xmalloc(std::strlen(tname) + 8 + std::strlen(newtext) + 4);
						std::sprintf(msgtemp, "<from %s> %s\r\n", tname, newtext);
						xfree((void *)newtext); /* avoid warning */
					}
					else
					{
						msgtemp = (char*)xmalloc(16 + std::strlen(tname));
						std::sprintf(msgtemp, "<from %s> \r\n", tname);
					}
					if (me)
						conn_unget_chatcharname(me, tname);
				}
				break;
			case message_type_talk:
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				if (dstflags&MF_X)
					return -1; /* player is ignored */
				{
					char const * tname;
					char const * newtext;

					if (me)
						tname = conn_get_chatcharname(me, dst);
					else
						tname = prefs_get_servername();

					if ((newtext = escape_chars(text, std::strlen(text))))
					{
						msgtemp = (char*)xmalloc(std::strlen(tname) + 4 + std::strlen(newtext) + 4);
						std::sprintf(msgtemp, "<%s> %s\r\n", tname, newtext);
						xfree((void *)newtext); /* avoid warning */
					}
					else
					{
						msgtemp = (char*)xmalloc(std::strlen(tname) + 8);
						std::sprintf(msgtemp, "<%s> \r\n", tname);
					}
					if (me)
						conn_unget_chatcharname(me, tname);
				}
				break;
			case message_type_broadcast:
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				if (dstflags&MF_X)
					return -1; /* player is ignored */
				{
					char const * newtext;

					if ((newtext = escape_chars(text, std::strlen(text))))
					{
						msgtemp = (char*)xmalloc(16 + std::strlen(newtext) + 4);
						std::sprintf(msgtemp, "Broadcast: %s\r\n", newtext); /* FIXME: show source? */
						xfree((void *)newtext); /* avoid warning */
					}
					else
					{
						msgtemp = (char*)xmalloc(16);
						std::sprintf(msgtemp, "Broadcast: \r\n"); /* FIXME: show source? */
					}
				}
				break;
			case message_type_channel:
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				msgtemp = (char*)xmalloc(std::strlen(text) + 32);
				std::sprintf(msgtemp, "Joining channel: \"%s\"\r\n", text);
				break;
			case message_type_userflags:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				msgtemp = xstrdup("");
				break;
			case message_type_whisperack:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				{
					char const * tname;
					char const * newtext;

					tname = conn_get_chatcharname(me, dst);
					if ((newtext = escape_chars(text, std::strlen(text))))
					{
						msgtemp = (char*)xmalloc(std::strlen(tname) + 8 + std::strlen(newtext) + 4);
						std::sprintf(msgtemp, "<to %s> %s\r\n", tname, newtext);
						xfree((void *)newtext); /* avoid warning */
					}
					else
					{
						msgtemp = (char*)xmalloc(std::strlen(tname) + 8 + std::strlen(text) + 4);
						std::sprintf(msgtemp, "<to %s> %s\r\n", tname, text);
					}
					conn_unget_chatcharname(me, tname);
				}
				break;
			case message_type_friendwhisperack:   // [zap-zero] 20020518
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				{
					char const * newtext;

					if ((newtext = escape_chars(text, std::strlen(text))))
					{
						msgtemp = (char*)xmalloc(14 + 8 + std::strlen(newtext) + 4);
						std::sprintf(msgtemp, "<to your friends> %s\r\n", newtext);
						xfree((void *)newtext); /* avoid warning */
					}
					else
					{
						msgtemp = (char*)xmalloc(14 + 8 + std::strlen(text) + 4);
						std::sprintf(msgtemp, "<to your friends> %s\r\n", text);
					}
				}
				break;

			case message_type_channelfull:
				/* FIXME */
				msgtemp = xstrdup("");
				break;
			case message_type_channeldoesnotexist:
				/* FIXME */
				msgtemp = xstrdup("");
				break;
			case message_type_channelrestricted:
				/* FIXME */
				msgtemp = xstrdup("");
				break;
			case message_type_info:
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				{
					char const * newtext;

					if ((newtext = escape_chars(text, std::strlen(text))))
					{
						msgtemp = (char*)xmalloc(std::strlen(newtext) + 4);
						std::sprintf(msgtemp, "%s\r\n", newtext);
						xfree((void *)newtext); /* avoid warning */
					}
					else
					{
						msgtemp = (char*)xmalloc(std::strlen(text) + 4);
						std::sprintf(msgtemp, "%s\r\n", text);
					}
				}
				break;
			case message_type_error:
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				{
					char const * newtext;

					if ((newtext = escape_chars(text, std::strlen(text))))
					{
						msgtemp = (char*)xmalloc(8 + std::strlen(newtext) + 4);
						std::sprintf(msgtemp, "ERROR: %s\r\n", newtext);
						xfree((void *)newtext); /* avoid warning */
					}
					else
					{
						msgtemp = (char*)xmalloc(8 + std::strlen(text) + 4);
						std::sprintf(msgtemp, "ERROR: %s\r\n", text);
					}
				}
				break;
			case message_type_emote:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				if (dstflags&MF_X)
					return -1; /* player is ignored */
				{
					char const * tname;
					char const * newtext;

					tname = conn_get_chatcharname(me, dst);
					if ((newtext = escape_chars(text, std::strlen(text))))
					{
						msgtemp = (char*)xmalloc(std::strlen(tname) + 4 + std::strlen(newtext) + 4);
						std::sprintf(msgtemp, "<%s %s>\r\n", tname, newtext);
						xfree((void *)newtext); /* avoid warning */
					}
					else
					{
						msgtemp = (char*)xmalloc(std::strlen(tname) + 4 + std::strlen(text) + 4);
						std::sprintf(msgtemp, "<%s %s>\r\n", tname, text);
					}
					conn_unget_chatcharname(me, tname);
				}
				break;
			case message_type_mode:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				{
					char const * tname;

					tname = conn_get_chatcharname(me, dst);
					msgtemp = (char*)xmalloc(std::strlen(tname) + 32);
					std::sprintf(msgtemp, "%s change mode: %s\r\n", tname, text);
					conn_unget_chatcharname(me, tname);
				}
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "got bad message type %d", (int)type);
				return -1;
			}

			{
				int retval;

				retval = packet_append_ntstring(packet, msgtemp);
				xfree(msgtemp);
				return retval;
			}
		}


		static int message_bot_format(t_packet * packet, t_message_type type, t_connection * me, t_connection * dst, char const * text, unsigned int dstflags)
		{
			char * msgtemp;
			char clienttag_str[5];

			if (!packet)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
				return -1;
			}

			/* special-case the login banner so it doesn't have numbers
			 * at the start of each line
			 */
			if (me &&
				conn_get_state(me) != conn_state_loggedin &&
				conn_get_state(me) != conn_state_destroy &&
				type != message_type_null) /* this does not apply for NULL messages */
			{
				if (!text)
				{
#if 0
					/* battle.net actually sends them during login */
					if (type == message_type_null)
						return 0; /* don't display null messages during the login */
#endif
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for non-loggedin state");
					return -1;
				}
				msgtemp = (char*)xmalloc(std::strlen(text) + 4);
				std::sprintf(msgtemp, "%s\r\n", text);
			}
			else
				switch (type)
			{
				case message_type_null:
					msgtemp = (char*)xmalloc(32);
					std::sprintf(msgtemp, "%u %s\r\n", EID_NULL, "NULL");
					break;
				case message_type_uniqueid: /* FIXME: need to send this for some bots, also needed to support guest accounts */
					if (!text)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
						return -1;
					}
					msgtemp = (char*)xmalloc(std::strlen(text) + 32);
					std::sprintf(msgtemp, "%u %s %s\r\n", EID_UNIQUENAME, "NAME", text);
					break;
				case message_type_adduser:
					if (!me)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
						return -1;
					}
					{
						char const * tname;

						tname = conn_get_chatcharname(me, dst);
						msgtemp = (char*)xmalloc(32 + std::strlen(tname) + 32);
						std::sprintf(msgtemp, "%u %s %s %04x [%s]\r\n", EID_SHOWUSER, "USER", tname, conn_get_flags(me) | dstflags, tag_uint_to_str(clienttag_str, conn_get_fake_clienttag(me)));
						conn_unget_chatcharname(me, tname);
					}
					break;
				case message_type_join:
					if (!me)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
						return -1;
					}
					if (me == dst)
						return -1;
					else
					{
						char const * tname;

						tname = conn_get_chatcharname(me, dst);
						msgtemp = (char*)xmalloc(32 + std::strlen(tname) + 32);
						std::sprintf(msgtemp, "%u %s %s %04x [%s]\r\n", EID_JOIN, "JOIN", tname, conn_get_flags(me) | dstflags, tag_uint_to_str(clienttag_str, conn_get_fake_clienttag(me)));
						conn_unget_chatcharname(me, tname);
					}
					break;
				case message_type_part:
				case message_type_kick:
				case message_type_quit:
					if (!me)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
						return -1;
					}
					{
						char const * tname;

						tname = conn_get_chatcharname(me, dst);
						msgtemp = (char*)xmalloc(32 + std::strlen(tname) + 32);
						std::sprintf(msgtemp, "%u %s %s %04x\r\n", EID_LEAVE, "LEAVE", tname, conn_get_flags(me) | dstflags);
						conn_unget_chatcharname(me, tname);
					}
					break;
				case message_type_whisper:
				case message_type_notice:
				case message_type_page:
					if (!text)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
						return -1;
					}
					if (dstflags&MF_X)
						return -1; /* player is ignored */
					{
						char const * tname;

						if (me)
							tname = conn_get_chatcharname(me, dst);
						else
							tname = prefs_get_servername();

						msgtemp = (char*)xmalloc(32 + std::strlen(tname) + 32 + std::strlen(text));
						std::sprintf(msgtemp, "%u %s %s %04x \"%s\"\r\n", EID_WHISPER, "WHISPER", tname, me ? conn_get_flags(me) | dstflags : dstflags, text);
						if (me)
							conn_unget_chatcharname(me, tname);
					}
					break;
				case message_type_talk:
					if (!me)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
						return -1;
					}
					if (!text)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
						return -1;
					}
					if (dstflags&MF_X)
						return -1; /* player is ignored */
					{
						char const * tname;

						tname = conn_get_chatcharname(me, dst);
						msgtemp = (char*)xmalloc(32 + std::strlen(tname) + 32 + std::strlen(text));
						std::sprintf(msgtemp, "%u %s %s %04x \"%s\"\r\n", EID_TALK, "TALK", tname, conn_get_flags(me) | dstflags, text);
						conn_unget_chatcharname(me, tname);
					}
					break;
				case message_type_broadcast:
					if (!text)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
						return -1;
					}
					if (dstflags&MF_X)
						return -1; /* player is ignored */
					msgtemp = (char*)xmalloc(32 + 32 + std::strlen(text));
					std::sprintf(msgtemp, "%u %s \"%s\"\r\n", EID_BROADCAST, "_", text); /* FIXME: what does this look like on Battle.net? */
					break;
				case message_type_channel:
					if (!text)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
						return -1;
					}
					msgtemp = (char*)xmalloc(32 + std::strlen(text));
					std::sprintf(msgtemp, "%u %s \"%s\"\r\n", EID_CHANNEL, "CHANNEL", text);
					break;
				case message_type_userflags:
					if (!me)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
						return -1;
					}
					{
						char const * tname;

						tname = conn_get_chatcharname(me, dst);
						msgtemp = (char*)xmalloc(32 + std::strlen(tname) + 16);
						std::sprintf(msgtemp, "%u %s %s %04x\r\n", EID_USERFLAGS, "USER", tname, conn_get_flags(me) | dstflags);
						conn_unget_chatcharname(me, tname);
					}
					break;
				case message_type_whisperack:
					if (!me)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
						return -1;
					}
					if (!text)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
						return -1;
					}
					{
						char const * tname;

						tname = conn_get_chatcharname(me, dst);
						msgtemp = (char*)xmalloc(32 + std::strlen(tname) + 32 + std::strlen(text));
						std::sprintf(msgtemp, "%u %s %s %04x \"%s\"\r\n", EID_WHISPERSENT, "WHISPER", tname, conn_get_flags(me) | dstflags, text);
						conn_unget_chatcharname(me, tname);
					}
					break;
				case message_type_friendwhisperack: // [zap-zero] 20020518
					if (!me)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
						return -1;
					}
					if (!text)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
						return -1;
					}
					{
						msgtemp = (char*)xmalloc(32 + 16 + 32 + std::strlen(text));
						std::sprintf(msgtemp, "%u %s \"your friends\" %04x \"%s\"\r\n", EID_WHISPERSENT, "WHISPER", conn_get_flags(me) | dstflags, text);
					}
					break;

				case message_type_channelfull:
					msgtemp = (char*)xmalloc(32);
					std::sprintf(msgtemp, "%u \r\n", EID_CHANNELFULL); /* FIXME */
					break;
				case message_type_channeldoesnotexist:
					msgtemp = (char*)xmalloc(32);
					std::sprintf(msgtemp, "%u \r\n", EID_CHANNELDOESNOTEXIST); /* FIXME */
					break;
				case message_type_channelrestricted:
					msgtemp = (char*)xmalloc(32);
					std::sprintf(msgtemp, "%u \r\n", EID_CHANNELRESTRICTED); /* FIXME */
					break;
				case message_type_info:
					if (!text)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
						return -1;
					}
					msgtemp = (char*)xmalloc(32 + 16 + std::strlen(text));
					std::sprintf(msgtemp, "%u %s \"%s\"\r\n", EID_INFO, "INFO", text);
					break;
				case message_type_error:
					if (!text)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
						return -1;
					}
					msgtemp = (char*)xmalloc(32 + 16 + std::strlen(text));
					std::sprintf(msgtemp, "%u %s \"%s\"\r\n", EID_ERROR, "ERROR", text);
					break;
				case message_type_emote:
					if (!me)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
						return -1;
					}
					if (!text)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
						return -1;
					}
					if (dstflags&MF_X)
						return -1; /* player is ignored */
					{
						char const * tname;

						tname = conn_get_chatcharname(me, dst);
						msgtemp = (char*)xmalloc(32 + std::strlen(tname) + 32 + std::strlen(text));
						std::sprintf(msgtemp, "%u %s %s %04x \"%s\"\r\n", EID_EMOTE, "EMOTE", tname, conn_get_flags(me) | dstflags, text);
						conn_unget_chatcharname(me, tname);
					}
					break;
				default:
					eventlog(eventlog_level_error, __FUNCTION__, "got bad message type %d", (int)type);
					return -1;
			}

			if (std::strlen(msgtemp) > MAX_MESSAGE_LEN)
				msgtemp[MAX_MESSAGE_LEN] = '\0'; /* now truncate to max size */

			{
				int retval;

				retval = packet_append_ntstring(packet, msgtemp);
				xfree(msgtemp);
				return retval;
			}
		}


		static int message_bnet_format(t_packet * packet, t_message_type type, t_connection * me, t_connection * dst, char const * text, unsigned int dstflags)
		{
			if (!packet)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
				return -1;
			}

			// empty messages are needed to separate text (or example, in bnhelp output)
			if (text && text[0] == '\0')
				text = " "; /* empty messages crash some clients, just send whitespace */

			std::string temp;
			if (text && (std::strlen(text) > MAX_MESSAGE_LEN)) {
				/* PELISH: We are trying to support MAX_IRC_MESSAGE_LEN for IRC and also
						   MAX_MESSAGE_LEN for bnet */
				temp = std::string(text, text + MAX_MESSAGE_LEN);
				text = temp.c_str();
			}

			packet_set_size(packet, sizeof(t_server_message));
			packet_set_type(packet, SERVER_MESSAGE);
			bn_int_set(&packet->u.server_message.player_ip, SERVER_MESSAGE_PLAYER_IP_DUMMY);
			bn_int_nset(&packet->u.server_message.account_num, SERVER_MESSAGE_ACCOUNT_NUM);
			bn_int_set(&packet->u.server_message.reg_auth, SERVER_MESSAGE_REG_AUTH);

			switch (type)
			{
			case message_type_adduser:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_ADDUSER);
				bn_int_set(&packet->u.server_message.flags, conn_get_flags(me) | dstflags);
				bn_int_set(&packet->u.server_message.latency, conn_get_latency(me));
				{
					char const * tname;
					char const * playerinfo;

					tname = conn_get_chatcharname(me, dst);
					packet_append_string(packet, tname);
					conn_unget_chatcharname(me, tname);
					if ((conn_get_clienttag(me) == CLIENTTAG_WARCRAFT3_UINT) || (conn_get_clienttag(me) == CLIENTTAG_WAR3XP_UINT))
						playerinfo = conn_get_w3_playerinfo(me);
					else playerinfo = conn_get_playerinfo(me);

					if (playerinfo == NULL) { playerinfo = ""; }
					packet_append_string(packet, playerinfo);
				}
				break;
			case message_type_join:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_JOIN);
				bn_int_set(&packet->u.server_message.flags, conn_get_flags(me) | dstflags);
				bn_int_set(&packet->u.server_message.latency, conn_get_latency(me));
				if (me == dst)
					return -1;
				else
				{
					char const * tname;
					char const * playerinfo;

					tname = conn_get_chatcharname(me, dst);
					packet_append_string(packet, tname);
					conn_unget_chatcharname(me, tname);

					if ((conn_get_clienttag(me) == CLIENTTAG_WARCRAFT3_UINT) || (conn_get_clienttag(me) == CLIENTTAG_WAR3XP_UINT))
						playerinfo = conn_get_w3_playerinfo(me);
					else playerinfo = conn_get_playerinfo(me);

					if (playerinfo == NULL) { playerinfo = ""; }
					packet_append_string(packet, playerinfo);
				}
				break;
			case message_type_part:
			case message_type_kick:
			case message_type_quit:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_PART);
				bn_int_set(&packet->u.server_message.flags, conn_get_flags(me) | dstflags);
				bn_int_set(&packet->u.server_message.latency, conn_get_latency(me));
				{
					char const * tname;

					tname = conn_get_chatcharname(me, dst);
					packet_append_string(packet, tname);
					conn_unget_chatcharname(me, tname);
					packet_append_string(packet, "");
				}
				break;
			case message_type_whisper:
			case message_type_notice:
			case message_type_page:
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				if (dstflags&MF_X)
					return -1; /* player is ignored */
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_WHISPER);
				bn_int_set(&packet->u.server_message.flags, me ? conn_get_flags(me) | dstflags : dstflags);
				bn_int_set(&packet->u.server_message.latency, me ? conn_get_latency(me) : 0);

				if (me)
				{
					char const * tname;

					tname = conn_get_chatcharname(me, dst);
					packet_append_string(packet, tname);
					conn_unget_chatcharname(me, tname);
				}
				else
					packet_append_string(packet, prefs_get_servername());

				packet_append_string(packet, text);

				break;
			case message_type_talk:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				if (dstflags&MF_X)
					return -1; /* player is ignored */
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_TALK);
				bn_int_set(&packet->u.server_message.flags, conn_get_flags(me) | dstflags);
				bn_int_set(&packet->u.server_message.latency, conn_get_latency(me));
				{
					char const * tname;

					tname = conn_get_chatcharname(me, dst);
					packet_append_string(packet, tname);
					conn_unget_chatcharname(me, tname);
					packet_append_string(packet, text);
				}
				break;
			case message_type_broadcast:
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				if (dstflags&MF_X)
					return -1; /* player is ignored */
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_BROADCAST);
				bn_int_set(&packet->u.server_message.flags, conn_get_flags(me) | dstflags);
				bn_int_set(&packet->u.server_message.latency, conn_get_latency(me));
				{
					char const * tname;

					tname = conn_get_chatcharname(me, dst);
					packet_append_string(packet, tname);
					conn_unget_chatcharname(me, tname);
					packet_append_string(packet, text);
				}
				break;
			case message_type_channel:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_CHANNEL);
				{
					t_channel const * channel;

					if (!(channel = conn_get_channel(me)))
						bn_int_set(&packet->u.server_message.flags, 0);
					else
						bn_int_set(&packet->u.server_message.flags, cflags_to_bncflags(channel_get_flags(channel)));
				}
				bn_int_set(&packet->u.server_message.latency, conn_get_latency(me));
				{
					char const * tname;

					tname = conn_get_chatname(me);
					packet_append_string(packet, tname);
					conn_unget_chatname(me, tname);
					packet_append_string(packet, text);
				}
				break;
			case message_type_userflags:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_USERFLAGS);
				bn_int_set(&packet->u.server_message.flags, conn_get_flags(me) | dstflags);
				bn_int_set(&packet->u.server_message.latency, conn_get_latency(me));
				{
					char const * tname;
					char const * playerinfo;

					tname = conn_get_chatcharname(me, dst);
					packet_append_string(packet, tname);
					conn_unget_chatcharname(me, tname);
					if ((conn_get_clienttag(me) == CLIENTTAG_WARCRAFT3_UINT) || (conn_get_clienttag(me) == CLIENTTAG_WAR3XP_UINT))
						playerinfo = conn_get_w3_playerinfo(me);
					else playerinfo = conn_get_playerinfo(me);

					if (playerinfo == NULL) { playerinfo = ""; }

					packet_append_string(packet, playerinfo);
				}
				break;
			case message_type_whisperack:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_WHISPERACK);
				bn_int_set(&packet->u.server_message.flags, conn_get_flags(me) | dstflags);
				bn_int_set(&packet->u.server_message.latency, conn_get_latency(me));
				{
					char const * tname;

					tname = conn_get_chatcharname(me, dst);
					packet_append_string(packet, tname);
					conn_unget_chatcharname(me, tname);
					packet_append_string(packet, text);
				}
				break;
			case message_type_friendwhisperack:  // [zap-zero] 20020518
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_WHISPERACK);
				bn_int_set(&packet->u.server_message.flags, conn_get_flags(me) | dstflags);
				bn_int_set(&packet->u.server_message.latency, conn_get_latency(me));
				{

					packet_append_string(packet, "your friends");
					packet_append_string(packet, text);
				}
				break;

			case message_type_channelfull: /* FIXME */
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_CHANNELFULL);
				bn_int_set(&packet->u.server_message.flags, 0);
				bn_int_set(&packet->u.server_message.latency, 0);
				packet_append_string(packet, "");
				packet_append_string(packet, "");
				break;
			case message_type_channeldoesnotexist:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_CHANNELDOESNOTEXIST);
				bn_int_set(&packet->u.server_message.flags, conn_get_flags(me) | dstflags);
				bn_int_set(&packet->u.server_message.latency, conn_get_latency(me));
				{
					char const * tname;

					tname = conn_get_chatname(me);
					packet_append_string(packet, tname);
					conn_unget_chatname(me, tname);
					packet_append_string(packet, text);
				}
				break;
			case message_type_channelrestricted: /* FIXME */
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_CHANNELRESTRICTED);
				bn_int_set(&packet->u.server_message.flags, 0);
				bn_int_set(&packet->u.server_message.latency, 0);
				packet_append_string(packet, "");
				packet_append_string(packet, "");
				break;
			case message_type_info:
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_INFO);
				bn_int_set(&packet->u.server_message.flags, 0);
				bn_int_set(&packet->u.server_message.latency, 0);
				packet_append_string(packet, "");
				packet_append_string(packet, text);
				break;
			case message_type_error:
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_ERROR);
				bn_int_set(&packet->u.server_message.flags, 0);
				bn_int_set(&packet->u.server_message.latency, 0);
				packet_append_string(packet, "");
				packet_append_string(packet, text);
				break;
			case message_type_emote:
				if (!me)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection for %s", message_type_get_str(type));
					return -1;
				}
				if (!text)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL text for %s", message_type_get_str(type));
					return -1;
				}
				if (dstflags&MF_X)
					return -1; /* player is ignored */
				bn_int_set(&packet->u.server_message.type, SERVER_MESSAGE_TYPE_EMOTE);
				bn_int_set(&packet->u.server_message.flags, conn_get_flags(me) | dstflags);
				bn_int_set(&packet->u.server_message.latency, conn_get_latency(me));
				{
					char const * tname;

					tname = conn_get_chatcharname(me, dst);
					packet_append_string(packet, tname);
					conn_unget_chatcharname(me, tname);
					packet_append_string(packet, text);
				}
				break;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "got bad message type %d", (int)type);
				return -1;
			}

			return 0;
		}


		extern t_message * message_create(t_message_type type, t_connection * src, char const * text)
		{
			t_message * message;

			message = (t_message*)xmalloc(sizeof(t_message));
			message->num_cached = 0;
			message->packets = NULL;
			message->classes = NULL;
			message->dstflags = NULL;
			message->mclasses = NULL;
			message->type = type;
			message->src = src;
			message->text = text;

			return message;
		}


		extern int message_destroy(t_message * message)
		{
			unsigned int i;

			if (!message)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL message");
				return -1;
			}

			for (i = 0; i < message->num_cached; i++)
			if (message->packets[i])
				packet_del_ref(message->packets[i]);
			if (message->packets)
				xfree(message->packets);
			if (message->classes)
				xfree(message->classes);
			if (message->dstflags)
				xfree(message->dstflags);
			if (message->mclasses)
				xfree(message->mclasses);
			xfree(message);

			return 0;
		}


		static t_packet * message_cache_lookup(t_message * message, t_connection *dst, unsigned int dstflags)
		{
			unsigned int i;
			t_packet * packet;
			t_message_class mclass;
			t_conn_class cclass;

			if (!message)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL message");
				return NULL;
			}

			cclass = conn_get_class(dst);
			mclass = conn_get_message_class(message->src, dst);
			for (i = 0; i < message->num_cached; i++)
			if (message->classes[i] == cclass && message->dstflags[i] == dstflags
				&& message->mclasses[i] == mclass)
				return message->packets[i];

			{
				t_packet * *   temp_packets;
				t_conn_class * temp_classes;
				unsigned int * temp_dstflags;
				t_message_class *temp_mclasses;

				if (!message->packets)
					temp_packets = (t_packet**)xmalloc(sizeof(t_packet *)*(message->num_cached + 1));
				else
					temp_packets = (t_packet**)xrealloc(message->packets, sizeof(t_packet *)*(message->num_cached + 1));

				if (!message->classes)
					temp_classes = (t_conn_class*)xmalloc(sizeof(t_conn_class)*(message->num_cached + 1));
				else
					temp_classes = (t_conn_class*)xrealloc(message->classes, sizeof(t_conn_class)*(message->num_cached + 1));

				if (!message->dstflags)
					temp_dstflags = (unsigned int *)xmalloc(sizeof(unsigned int)*(message->num_cached + 1));
				else
					temp_dstflags = (unsigned int *)xrealloc(message->dstflags, sizeof(unsigned int)*(message->num_cached + 1));

				if (!message->mclasses)
					temp_mclasses = (t_message_class*)xmalloc(sizeof(t_message_class)*(message->num_cached + 1));
				else
					temp_mclasses = (t_message_class*)xrealloc(message->mclasses, sizeof(t_message_class)*(message->num_cached + 1));

				message->packets = temp_packets;
				message->classes = temp_classes;
				message->dstflags = temp_dstflags;
				message->mclasses = temp_mclasses;
			}

			switch (cclass)
			{
			case conn_class_telnet:
				if (!(packet = packet_create(packet_class_raw)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not create packet");
					return NULL;
				}
				if (message_telnet_format(packet, message->type, message->src, dst, message->text, dstflags) < 0)
				{
					packet_del_ref(packet);
					packet = NULL; /* we can cache the NULL too */
				}
				break;
			case conn_class_bot:
				if (!(packet = packet_create(packet_class_raw)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not create packet");
					return NULL;
				}
				if (message_bot_format(packet, message->type, message->src, dst, message->text, dstflags) < 0)
				{
					packet_del_ref(packet);
					packet = NULL; /* we can cache the NULL too */
				}
				break;
			case conn_class_bnet:
				if (!(packet = packet_create(packet_class_bnet)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not create packet");
					return NULL;
				}
				if (message_bnet_format(packet, message->type, message->src, dst, message->text, dstflags) < 0)
				{
					packet_del_ref(packet);
					packet = NULL; /* we can cache the NULL too */
				}
				break;
			case conn_class_irc:
			case conn_class_wol:
			case conn_class_wserv:
			case conn_class_wgameres:
				if (!(packet = packet_create(packet_class_raw)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not create packet");
					return NULL;
				}
				/* irc_message_format() is in irc.c */
				if (irc_message_format(packet, message->type, message->src, dst, message->text, dstflags) < 0)
				{
					packet_del_ref(packet);
					packet = NULL; /* we can cache the NULL too */
				}
				break;
			case conn_class_init:
			case conn_class_file:
			case conn_class_d2cs_bnetd:
			case conn_class_w3route:
				packet = NULL;
				break; /* cache the NULL but dont send any error,
						* this are normal connections */
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "unsupported connection class %d", (int)cclass);
				packet = NULL; /* we can cache the NULL too */
			}

			message->num_cached++;
			message->packets[i] = packet;
			message->classes[i] = cclass;
			message->dstflags[i] = dstflags;
			message->mclasses[i] = mclass;

			return packet;
		}


		extern int message_send(t_message * message, t_connection * dst)
		{
			t_packet *   packet;
			unsigned int dstflags;

			if (!message)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL message");
				return -1;
			}
			if (!dst)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL dst connection");
				return -1;
			}

			dstflags = 0;
			if (message->src)
			{
				char const * tname;

				if ((tname = conn_get_chatname(message->src)) && conn_check_ignoring(dst, tname) == 1)
				{
					conn_unget_chatname(message->src, tname);
					dstflags |= MF_X;
				}
				if (tname)
					conn_unget_chatname(message->src, tname);
			}

			if (!(packet = message_cache_lookup(message, dst, dstflags)))
				return -1;

			/* FIXME: this is not needed now, message has dst */
			if ((conn_get_class(dst) == conn_class_irc) || (conn_get_class(dst) == conn_class_wol) || (conn_get_class(dst) == conn_class_wserv) || (conn_get_class(dst) == conn_class_wgameres)) {
				/* HACK: IRC message always need the recipient and are therefore bad to cache. */
				/*       So we only cache a pseudo packet and convert it to a real packet later ... */
				packet = packet_duplicate(packet); /* we want to modify packet so we have to create a copy ... */
				if (irc_message_postformat(packet, dst) < 0) {
					packet_del_ref(packet); /* we don't need the previously created copy anymore ... */
					return -1;
				}
			}

			conn_push_outqueue(dst, packet);

			if ((conn_get_class(dst) == conn_class_irc) || (conn_get_class(dst) == conn_class_wol) || (conn_get_class(dst) == conn_class_wserv) || (conn_get_class(dst) == conn_class_wgameres))
				packet_del_ref(packet); /* we don't need the previously created copy anymore ... */

			return 0;
		}


		extern int message_send_all(t_message * message)
		{
			t_connection * c;
			t_elem const * curr;
			int            rez;

			if (!message)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL message");
				return -1;
			}

			rez = -1;
			LIST_TRAVERSE_CONST(connlist(), curr)
			{
				c = (t_connection*)elem_get_data(curr);
				if (message_send(message, c) == 0)
					rez = 0;
			}

			return rez;
		}

		extern int message_send_text(t_connection * dst, t_message_type type, t_connection * src, std::string text)
		{
			return message_send_text(dst, type, src, text.c_str());
		}

		extern int message_send_text(t_connection * dst, t_message_type type, t_connection * src, char const * text)
		{
			t_message * message;
			int         rez;

			if (!dst)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (!(message = message_create(type, src, text)))
				return -1;
			rez = message_send(message, dst);
			message_destroy(message);

			return rez;
		}


		extern int message_send_admins(t_connection * src, t_message_type type, char const * text)
		{
			t_elem	const * curr;
			t_connection *	tc;
			int			counter = 0;

			LIST_TRAVERSE_CONST(connlist(), curr)
			{
				tc = (t_connection*)elem_get_data(curr);
				if (!tc)
					continue;
				if (account_get_auth_admin(conn_get_account(tc), NULL) == 1 && tc != src)
				{
					message_send_text(tc, type, src, text);
					counter++;
				}
			}

			return counter;
		}


		extern int message_send_formatted(t_connection * dst, char const * text)
		{
			char * line;

			if (!dst)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if (!(line = message_format_line(dst, text)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not format input text \"%s\"", text);
				return -1;
			}

			i18n_convert(dst, line);

			/* caller beware: empty messages can crash Blizzard clients */
			switch (line[0])
			{
			case 'C':
				if (line[1] == '/')
					handle_command(dst, &line[1]);
				else
				if (conn_get_channel(dst) && !conn_quota_exceeded(dst, &line[1]))
					channel_message_send(conn_get_channel(dst), message_type_talk, dst, &line[1]);
				break;
			case 'B':
				message_send_text(dst, message_type_broadcast, dst, &line[1]);
				break;
			case 'E':
				message_send_text(dst, message_type_error, dst, &line[1]);
				break;
			case 'M':
				message_send_text(dst, message_type_talk, dst, &line[1]);
				break;
			case 'T':
				message_send_text(dst, message_type_emote, dst, &line[1]);
				break;
			case 'I':
			case 'W':
				message_send_text(dst, message_type_info, dst, &line[1]);
				break;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "unknown message type '%c'", line[0]);
				xfree(line);
				return -1;
			}

			xfree(line);
			return 0;
		}


		extern int message_send_file(t_connection * dst, std::FILE * fd)
		{
			char * buff;

			if (!dst)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (!fd)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL fd");
				return -1;
			}

			while ((buff = file_get_line(fd)))
			{
				message_send_formatted(dst, buff);
			}
			file_get_line(NULL); // clear file_get_line buffer

			return 0;
		}

		/* Show message box on a client side (https://github.com/HarpyWar/pvpgn/issues/15) */
		extern int messagebox_show(t_connection * dst, char const * text, char const * caption, int type)
		{
			t_packet *rpacket;

			std::string newtext = str_replace_nl(text);

			if ((rpacket = packet_create(packet_class_bnet)))
			{
				packet_set_size(rpacket, sizeof(t_server_messagebox));
				packet_set_type(rpacket, SERVER_MESSAGEBOX);
				bn_int_set(&rpacket->u.server_messagebox.style, type); 
				packet_append_string(rpacket, newtext.c_str());
				packet_append_string(rpacket, caption);
				conn_push_outqueue(dst, rpacket);
				packet_del_ref(rpacket);
			}
			return 0;
		}

	}

}
