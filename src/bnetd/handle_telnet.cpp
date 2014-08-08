/*
 * Copyright (C) 2001  Ross Combs (rocombs@cs.nmsu.edu)
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
#include "handle_telnet.h"

#include <cstring>
#include <cctype>

#include "common/eventlog.h"
#include "common/packet.h"
#include "common/bnethash.h"
#include "common/xalloc.h"
#include "common/tag.h"

#include "connection.h"
#include "account.h"
#include "account_wrap.h"
#include "message.h"
#include "channel.h"
#include "command.h"
#include "common/setup_after.h"


namespace pvpgn
{

	namespace bnetd
	{

		extern int handle_telnet_packet(t_connection * c, t_packet const * const packet)
		{
			t_packet * rpacket;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] got NULL connection", conn_get_socket(c));
				return -1;
			}
			if (!packet)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] got NULL packet", conn_get_socket(c));
				return -1;
			}
			if (packet_get_class(packet) != packet_class_raw)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] got bad packet (class %d)", conn_get_socket(c), (int)packet_get_class(packet));
				return -1;
			}

			{
				char const * const linestr = packet_get_str_const(packet, 0, MAX_MESSAGE_LEN);

				if (packet_get_size(packet) < 2) /* empty line */
					return 0;
				if (!linestr)
				{
					eventlog(eventlog_level_warn, __FUNCTION__, "[%d] line too long", conn_get_socket(c));
					return 0;
				}

				switch (conn_get_state(c))
				{
				case conn_state_connected:
					conn_add_flags(c, MF_PLUG);
					conn_set_clienttag(c, CLIENTTAG_BNCHATBOT_UINT);

					{
						char const * temp = linestr;

						if (temp[0] == '\004') /* FIXME: no echo, ignore for now (we always do no echo) */
							temp = &temp[1];

						if (temp[0] == '\0') /* empty line */
						{
							conn_set_state(c, conn_state_bot_username); /* don't look for ^D or reset tag and flags */
							break;
						}

						conn_set_state(c, conn_state_bot_password);

						if (conn_set_loggeduser(c, temp) < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not set username to \"%s\"", conn_get_socket(c), temp);

						{
							char const * const msg = "\r\nPassword: ";

							if (!(rpacket = packet_create(packet_class_raw)))
							{
								eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not create rpacket", conn_get_socket(c));
								break;
							}
#if 0 /* don't echo */
							packet_append_ntstring(rpacket, conn_get_loggeduser(c));
#endif
							packet_append_ntstring(rpacket, msg);
							conn_push_outqueue(c, rpacket);
							packet_del_ref(rpacket);
						}
					}
					break;

				case conn_state_bot_username:
					conn_set_state(c, conn_state_bot_password);

					if (conn_set_loggeduser(c, linestr) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not set username to \"%s\"", conn_get_socket(c), linestr);

					{
						char const * const temp = "\r\nPassword: ";

						if (!(rpacket = packet_create(packet_class_raw)))
						{
							eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not create rpacket", conn_get_socket(c));
							break;
						}
#if 0 /* don't echo */
						packet_append_ntstring(rpacket, linestr);
#endif
						packet_append_ntstring(rpacket, temp);
						conn_push_outqueue(c, rpacket);
						packet_del_ref(rpacket);
					}
					break;

				case conn_state_bot_password:
				{
												char const * const tempa = "\r\nLogin failed.\r\n\r\nUsername: ";
												char const * const tempb = "\r\nAccount has no bot access.\r\n\r\nUsername: ";
												char const * const loggeduser = conn_get_loggeduser(c);
												t_account *        account;
												char const *       oldstrhash1;
												t_hash             trypasshash1;
												t_hash             oldpasshash1;
												char *             testpass;

												if (!loggeduser) /* error earlier in login */
												{
													/* no std::log message... */
													conn_set_state(c, conn_state_bot_username);

													if (!(rpacket = packet_create(packet_class_raw)))
													{
														eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not create rpacket", conn_get_socket(c));
														break;
													}

													packet_append_ntstring(rpacket, tempa);
													conn_push_outqueue(c, rpacket);
													packet_del_ref(rpacket);
													break;
												}
												if (connlist_find_connection_by_accountname(loggeduser))
												{
													eventlog(eventlog_level_info, __FUNCTION__, "[%d] bot login for \"%s\" refused (already logged in)", conn_get_socket(c), loggeduser);
													conn_set_state(c, conn_state_bot_username);

													if (!(rpacket = packet_create(packet_class_raw)))
													{
														eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not create rpacket", conn_get_socket(c));
														break;
													}

													packet_append_ntstring(rpacket, tempa);
													conn_push_outqueue(c, rpacket);
													packet_del_ref(rpacket);
													break;
												}
												if (!(account = accountlist_find_account(loggeduser)))
												{
													eventlog(eventlog_level_info, __FUNCTION__, "[%d] bot login for \"%s\" refused (bad account)", conn_get_socket(c), loggeduser);
													conn_set_state(c, conn_state_bot_username);

													if (!(rpacket = packet_create(packet_class_raw)))
													{
														eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not create rpacket", conn_get_socket(c));
														break;
													}

													packet_append_ntstring(rpacket, tempa);
													conn_push_outqueue(c, rpacket);
													packet_del_ref(rpacket);
													break;
												}
												if ((oldstrhash1 = account_get_pass(account)))
												{
													if (hash_set_str(&oldpasshash1, oldstrhash1) < 0)
													{
														eventlog(eventlog_level_info, __FUNCTION__, "[%d] bot login for \"%s\" refused (corrupted passhash1?)", conn_get_socket(c), account_get_name(account));
														conn_set_state(c, conn_state_bot_username);

														if (!(rpacket = packet_create(packet_class_raw)))
														{
															eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not create rpacket", conn_get_socket(c));
															break;
														}

														packet_append_ntstring(rpacket, tempa);
														conn_push_outqueue(c, rpacket);
														packet_del_ref(rpacket);
														break;
													}

													testpass = xstrdup(linestr);
													{
														unsigned int i;

														for (i = 0; i < std::strlen(testpass); i++)
														if (std::isupper((int)testpass[i]))
															testpass[i] = std::tolower((int)testpass[i]);
													}
													if (bnet_hash(&trypasshash1, std::strlen(testpass), testpass) < 0) /* FIXME: force to lowercase */
													{
														eventlog(eventlog_level_info, __FUNCTION__, "[%d] bot login for \"%s\" refused (unable to hash password)", conn_get_socket(c), account_get_name(account));
														xfree(testpass);

														conn_set_state(c, conn_state_bot_username);

														if (!(rpacket = packet_create(packet_class_raw)))
														{
															eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not create rpacket", conn_get_socket(c));
															break;
														}

														packet_append_ntstring(rpacket, tempa);
														conn_push_outqueue(c, rpacket);
														packet_del_ref(rpacket);
														break;
													}
													xfree(testpass);
													if (hash_eq(trypasshash1, oldpasshash1) != 1)
													{
														eventlog(eventlog_level_info, __FUNCTION__, "[%d] bot login for \"%s\" refused (wrong password)", conn_get_socket(c), account_get_name(account));
														conn_set_state(c, conn_state_bot_username);

														if (!(rpacket = packet_create(packet_class_raw)))
														{
															eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not create rpacket", conn_get_socket(c));
															break;
														}

														packet_append_ntstring(rpacket, tempa);
														conn_push_outqueue(c, rpacket);
														packet_del_ref(rpacket);
														break;
													}


													if (account_get_auth_botlogin(account) != 1) /* default to false */
													{
														eventlog(eventlog_level_info, __FUNCTION__, "[%d] bot login for \"%s\" refused (no bot access)", conn_get_socket(c), account_get_name(account));
														conn_set_state(c, conn_state_bot_username);

														if (!(rpacket = packet_create(packet_class_raw)))
														{
															eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not create rpacket", conn_get_socket(c));
															break;
														}

														packet_append_ntstring(rpacket, tempb);
														conn_push_outqueue(c, rpacket);
														packet_del_ref(rpacket);
														break;
													}
													else if (account_get_auth_lock(account) == 1) /* default to false */
													{
														eventlog(eventlog_level_info, __FUNCTION__, "[%d] bot login for \"%s\" refused (this account is locked)", conn_get_socket(c), account_get_name(account));
														conn_set_state(c, conn_state_bot_username);

														if (!(rpacket = packet_create(packet_class_raw)))
														{
															eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not create rpacket", conn_get_socket(c));
															break;
														}

														packet_append_ntstring(rpacket, tempb);
														conn_push_outqueue(c, rpacket);
														packet_del_ref(rpacket);
														break;
													}

													eventlog(eventlog_level_info, __FUNCTION__, "[%d] \"%s\" bot logged in (correct password)", conn_get_socket(c), account_get_name(account));
												}
												else
													eventlog(eventlog_level_info, __FUNCTION__, "[%d] \"%s\" bot logged in (no password)", conn_get_socket(c), account_get_name(account));

												if (!(rpacket = packet_create(packet_class_raw))) /* if we got this far, let them std::log in even if this fails */
													eventlog(eventlog_level_error, __FUNCTION__, "[%d] could not create rpacket", conn_get_socket(c));
												else
												{
													packet_append_ntstring(rpacket, "\r\n");
													conn_push_outqueue(c, rpacket);
													packet_del_ref(rpacket);
												}
												message_send_text(c, message_type_uniqueid, c, loggeduser);

												conn_login(c, account, loggeduser);

												if (conn_set_channel(c, CHANNEL_NAME_CHAT) < 0)
													conn_set_channel(c, CHANNEL_NAME_BANNED); /* should not fail */
				}
					break;

				case conn_state_loggedin:
				{
											t_channel const * channel;

											conn_set_idletime(c);

											if ((channel = conn_get_channel(c)))
												channel_message_log(channel, c, 1, linestr);
											/* we don't log game commands currently */

											if (linestr[0] == '/')
												handle_command(c, linestr);
											else
											if (channel && !conn_quota_exceeded(c, linestr))
												channel_message_send(channel, message_type_talk, c, linestr);
											/* else discard */
				}
					break;

				default:
					eventlog(eventlog_level_error, __FUNCTION__, "[%d] unknown telnet connection state %d", conn_get_socket(c), (int)conn_get_state(c));
				}
			}

			return 0;
		}

	}

}
