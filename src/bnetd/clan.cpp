/*
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
#define CLAN_INTERNAL_ACCESS
#include "common/setup_before.h"
#include "clan.h"
#include "compat/strcasecmp.h"
#include "compat/strncasecmp.h"
#include "compat/pdir.h"
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#include "common/packet.h"
#include "common/bnet_protocol.h"
#include "common/util.h"
#include "common/bnettime.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "common/proginfo.h"
#include "common/bn_type.h"
#include "common/xalloc.h"
#include "connection.h"
#include "anongame.h"
#include "prefs.h"
#include "friends.h"
#include "game.h"
#include "message.h"
#include "account.h"
#include "channel.h"
#include "anongame.h"
#include "storage.h"
#include "server.h"
#include "compat/uint.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static t_list *clanlist_head = NULL;
		unsigned max_clanid = 0;

		/* callback function for storage use */

		static int _cb_load_clans(void *clan)
		{
			if (clanlist_add_clan((t_clan*)clan) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "failed to add clan to clanlist");
				return -1;
			}

			if (((t_clan *)clan)->clanid > max_clanid)
				max_clanid = ((t_clan *)clan)->clanid;
			return 0;
		}

		/*
		** Message functions
		*/

		extern int clan_send_message_to_online_members(t_clan * clan, t_message_type type, t_connection * me, char const * text)
		{
			/* PELISH: Send message to online clan_members
			   returns: an error == -1, done but no one heard == 0, done with message sended == 1 */
			t_list * cl_member_list;
			t_elem * curr;
			t_clanmember * dest_member;
			t_connection * dest_conn;
			bool heard = false;

			if (!clan) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return -1;
			}

			if (!me) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connecion");
				return -1;
			}

			cl_member_list = clan_get_members(clan);

			LIST_TRAVERSE(cl_member_list, curr) {
				if (!(dest_member = (t_clanmember*)elem_get_data(curr))) {
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
					continue;
				}

				if ((dest_conn = clanmember_get_conn(dest_member)) && (dest_conn != me) &&
					(clanmember_get_fullmember(dest_member) == 1)) {
					message_send_text(dest_conn, type, me, text);
					heard = true;
				}
			}

			if (heard)
				return 1;
			else
				return 0;
		}

		/*
		** Packet Management
		*/

		extern int clan_send_packet_to_online_members(t_clan * clan, t_packet * packet)
		{
			t_elem *curr;

			if (!clan)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return -1;
			}

			if (!packet)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL packet");
				return -1;
			}

			LIST_TRAVERSE(clan->members, curr)
			{
				t_clanmember *	member;
				t_clienttag	clienttag;
				t_connection *	conn;

				if (!(member = (t_clanmember*)elem_get_data(curr)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL elem in list");
					continue;
				}
				if (!(conn = clanmember_get_conn(member)))
					continue;				// not online

				if (!(clienttag = conn_get_clienttag(conn)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "conn has NULL clienttag");
					continue;
				}

				if ((clienttag != CLIENTTAG_WARCRAFT3_UINT) && (clienttag != CLIENTTAG_WAR3XP_UINT))
					continue;				// online but wrong client

				conn_push_outqueue(conn, packet);
			}

			return 0;
		}

		extern int clan_send_status_window_on_create(t_clan * clan)
		{
			t_packet * rpacket;
			t_elem *curr;

			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return -1;
			}

			if ((rpacket = packet_create(packet_class_bnet)))
			{
				char channelname[10];
				if (clan->tag)
					std::sprintf(channelname, "Clan %s", clantag_to_str(clan->tag));
				else
				{
					std::sprintf(channelname, "Clans");
					eventlog(eventlog_level_error, __FUNCTION__, "clan has NULL clantag");
				}

				packet_set_size(rpacket, sizeof(t_server_clan_clanack));
				packet_set_type(rpacket, SERVER_CLAN_CLANACK);
				bn_byte_set(&rpacket->u.server_clan_clanack.unknow1, 0);
				bn_int_set(&rpacket->u.server_clan_clanack.clantag, clan->tag);

				LIST_TRAVERSE(clan->members, curr)
				{
					t_clanmember *	member;
					t_clienttag 	clienttag;
					t_connection *	conn;

					if (!(member = (t_clanmember*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL elem in list");
						continue;
					}
					if (!(conn = clanmember_get_conn(member)))
						continue;			// not online;

					if (!(clienttag = conn_get_clienttag(conn)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "conn has NULL clienttag");
						continue;
					}
					if ((clienttag != CLIENTTAG_WARCRAFT3_UINT) && (clienttag != CLIENTTAG_WAR3XP_UINT))
						continue;			// online but wrong client

					if (conn_get_channel(conn))
					{
						conn_update_w3_playerinfo(conn);
						channel_set_userflags(conn);
						if (conn_set_channel(conn, channelname) < 0)
							conn_set_channel(conn, CHANNEL_NAME_BANNED);	/* should not fail */
					}
					bn_byte_set(&rpacket->u.server_clan_clanack.status, member->status);
					conn_push_outqueue(conn, rpacket);
				}
				packet_del_ref(rpacket);
			}
			return 0;
		}

		extern int clan_close_status_window_on_disband(t_clan * clan)
		{
			t_packet * rpacket;
			t_elem *curr;

			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return -1;
			}

			if ((rpacket = packet_create(packet_class_bnet)))
			{
				packet_set_size(rpacket, sizeof(t_server_clanquitnotify));
				packet_set_type(rpacket, SERVER_CLANQUITNOTIFY);
				bn_byte_set(&rpacket->u.server_clan_clanack.status, SERVER_CLANQUITNOTIFY_STATUS_REMOVED_FROM_CLAN);
				LIST_TRAVERSE(clan->members, curr)
				{
					t_clanmember *	member;
					t_clienttag 	clienttag;
					t_connection *	conn;

					if (!(member = (t_clanmember*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL elem in list");
						continue;
					}
					if (!(conn = clanmember_get_conn(member)))
						continue;			// not online;

					if (!(clienttag = conn_get_clienttag(conn)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "conn has NULL clienttag");
						continue;
					}

					if ((clienttag != CLIENTTAG_WARCRAFT3_UINT) && (clienttag != CLIENTTAG_WAR3XP_UINT))
						continue;			// online but wrong client

					conn_push_outqueue(conn, rpacket);
					conn_update_w3_playerinfo(conn);
				}
				packet_del_ref(rpacket);
			}


			return 0;
		}

		extern int clan_send_status_window(t_connection * c)
		{
			t_packet * rpacket;
			t_account *acc;
			t_clanmember *member;
			t_clienttag clienttag;
			t_clan * clan;

			if (!(acc = conn_get_account(c)))
				return 0;

			if (!(member = account_get_clanmember(acc)))
				return 0;

			if (!(clan = member->clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "member has NULL clan");
				return -1;
			}

			if (!(clan->tag))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "clan has NULL clantag");
				return -1;
			}

			if (!(clienttag = conn_get_clienttag(c)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "conn has NULL clienttag");
				return -1;
			}

			if ((clienttag != CLIENTTAG_WARCRAFT3_UINT) && (clienttag != CLIENTTAG_WAR3XP_UINT))
				return 0;

			if ((rpacket = packet_create(packet_class_bnet)))
			{
				packet_set_size(rpacket, sizeof(t_server_clan_clanack));
				packet_set_type(rpacket, SERVER_CLAN_CLANACK);
				bn_byte_set(&rpacket->u.server_clan_clanack.unknow1, 0);
				bn_int_set(&rpacket->u.server_clan_clanack.clantag, member->clan->tag);
				bn_byte_set(&rpacket->u.server_clan_clanack.status, member->status);
				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
			}
			return 0;
		}

		extern int clan_close_status_window(t_connection * c)
		{
			t_packet * rpacket;
			t_clienttag clienttag;

			if (!(clienttag = conn_get_clienttag(c)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "conn has NULL clienttag");
				return -1;
			}

			if ((clienttag != CLIENTTAG_WARCRAFT3_UINT) && (clienttag != CLIENTTAG_WAR3XP_UINT))
				return 0;

			if ((rpacket = packet_create(packet_class_bnet)))
			{
				packet_set_size(rpacket, sizeof(t_server_clanquitnotify));
				packet_set_type(rpacket, SERVER_CLANQUITNOTIFY);
				bn_byte_set(&rpacket->u.server_clanquitnotify.status, SERVER_CLANQUITNOTIFY_STATUS_REMOVED_FROM_CLAN);
				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
			}
			return 0;
		}

		extern int clan_send_memberlist(t_connection * c, t_packet const *const packet)
		{
			t_packet * rpacket;
			t_elem *curr;
			char const *username;
			t_clanmember *member;
			t_clan *clan;
			t_account *account;
			int count = 0;
			char tmpstr[2];
			const char *append_str;

			if (!(account = conn_get_account(c)))
				return -1;

			if (!(clan = account_get_clan(account)))
				return -1;

			if ((rpacket = packet_create(packet_class_bnet)))
			{
				t_account * memberacc;

				packet_set_size(rpacket, sizeof(t_server_clanmemberlist_reply));
				packet_set_type(rpacket, SERVER_CLANMEMBERLIST_REPLY);
				bn_int_set(&rpacket->u.server_clanmemberlist_reply.count,
					bn_int_get(packet->u.client_clanmemberlist_req.count));
				LIST_TRAVERSE(clan->members, curr)
				{
					if (!(member = (t_clanmember*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "got NULL element in list");
						continue;
					}

					if (!(memberacc = (t_account*)member->memberacc))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "member has NULL account");
						continue;
					}

					username = account_get_name(memberacc);
					packet_append_string(rpacket, username);
					tmpstr[0] = member->status;
					append_str = clanmember_get_online_status(member, &tmpstr[1]);
					packet_append_data(rpacket, tmpstr, 2);
					if (append_str)
						packet_append_string(rpacket, append_str);
					else
						packet_append_string(rpacket, "");
					count++;
				}
				bn_byte_set(&rpacket->u.server_clanmemberlist_reply.member_count, count);
				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
				return 0;
			}

			return -1;
		}

		extern int clan_save_motd_chg(t_connection * c, t_packet const *const packet)
		{
			t_account *account;
			char const *motd;
			int offset;
			t_clan *clan;

			if ((account = conn_get_account(c)) == NULL)
				return -1;
			if ((clan = account_get_clan(account)) == NULL)
				return -1;
			offset = sizeof(packet->u.client_clan_motdchg);
			motd = packet_get_str_const(packet, offset, CLAN_MOTD_MAX);
			eventlog(eventlog_level_trace, __FUNCTION__, "[%d] got W3XP_CLAN_MOTDCHG packet : %s", conn_get_socket(c), motd);
			if (clan_set_motd(clan, motd) != 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Failed to set clan motd.");
				return -1;
			}
			clan->modified = 1;
			return 0;
		}


		extern int clan_send_motd_reply(t_connection * c, t_packet const *const packet)
		{
			t_packet * rpacket;
			t_account *account;
			t_clan *clan;

			if ((account = conn_get_account(c)) == NULL)
				return -1;
			if ((clan = account_get_clan(account)) == NULL)
				return -1;
			if (clan->clan_motd == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Failed to get clan motd.");
				return -1;
			}
			if ((rpacket = packet_create(packet_class_bnet)))
			{
				packet_set_size(rpacket, sizeof(t_server_clan_motdreply));
				packet_set_type(rpacket, SERVER_CLAN_MOTDREPLY);
				bn_int_set(&rpacket->u.server_clan_motdreply.count, bn_int_get(packet->u.client_clan_motdreq.count));
				bn_int_set(&rpacket->u.server_clan_motdreply.unknow1, SERVER_CLAN_MOTDREPLY_UNKNOW1);
				packet_append_string(rpacket, clan->clan_motd);
				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
			}

			return 0;
		}

		/*
		** String / Function Management
		*/

		extern int clan_get_possible_member(t_connection * c, t_packet const *const packet)
		{
			t_packet * rpacket;
			t_channel *channel;
			t_connection *conn;
			char const *username;
			t_account * account;

			int friend_count = 0;
			t_clantag clantag;
			clantag = bn_int_get(packet->u.client_clan_createreq.clantag);
			if ((rpacket = packet_create(packet_class_bnet)) == NULL)
			{
				return -1;
			}
			packet_set_size(rpacket, sizeof(t_server_clan_createreply));
			packet_set_type(rpacket, SERVER_CLAN_CREATEREPLY);
			bn_int_set(&rpacket->u.server_clan_createreply.count, bn_int_get(packet->u.client_clan_createreq.count));
			if (clanlist_find_clan_by_clantag(clantag) != NULL)
			{
				bn_byte_set(&rpacket->u.server_clan_createreply.check_result, SERVER_CLAN_CREATEREPLY_CHECK_ALLREADY_IN_USE);
				bn_byte_set(&rpacket->u.server_clan_createreply.friend_count, 0);
				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
				return 0;
			}
			if ((account = conn_get_account(c)) && (account_get_clan(account) != NULL || account_get_creating_clan(account) != NULL))
			{
				bn_byte_set(&rpacket->u.server_clan_createreply.check_result, SERVER_CLAN_CREATEREPLY_CHECK_EXCEPTION);
				bn_byte_set(&rpacket->u.server_clan_createreply.friend_count, 0);
				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
				return 0;
			}
			bn_byte_set(&rpacket->u.server_clan_createreply.check_result, SERVER_CLAN_CREATEREPLY_CHECK_OK);
			channel = conn_get_channel(c);
			if (channel_get_permanent(channel))
			{
				/* If not in a private channel, retreive number of mutual friend connected */
				t_list *flist = account_get_friends(conn_get_account(c));
				t_elem const *curr;
				t_friend *fr;

				LIST_TRAVERSE_CONST(flist, curr)
				{
					if ((fr = (t_friend*)elem_get_data(curr)) != NULL)
					{
						t_account *fr_acc = friend_get_account(fr);
						t_clienttag clienttag;
						if (fr->mutual
							&& ((conn = connlist_find_connection_by_account(fr_acc)) != NULL)
							&& (conn_get_channel(conn) == channel)
							&& (!account_get_clan(fr_acc))
							&& (!account_get_creating_clan(fr_acc))
							&& (clienttag = conn_get_clienttag(conn))
							&& ((clienttag == CLIENTTAG_WAR3XP_UINT) || (clienttag == CLIENTTAG_WARCRAFT3_UINT))
							&& (username = account_get_name(fr_acc)))
						{
							friend_count++;
							packet_append_string(rpacket, username);
						}
					}
				}
			}
			else
			{
				/* If in a private channel, retreive all non-clan war3/w3xp users in the channel */
				for (conn = channel_get_first(channel); conn; conn = channel_get_next())
				{
					t_account * acc;
					t_clienttag clienttag;
					if ((conn != c)
						&& (acc = conn_get_account(conn))
						&& (!account_get_clan(acc))
						&& (!account_get_creating_clan(acc))
						&& (clienttag = conn_get_clienttag(conn))
						&& ((clienttag == CLIENTTAG_WAR3XP_UINT) || (clienttag == CLIENTTAG_WARCRAFT3_UINT))
						&& (username = conn_get_username(conn)))
					{
						friend_count++;
						packet_append_string(rpacket, username);
					}
				}
			}
			bn_byte_set(&rpacket->u.server_clan_createreply.friend_count, friend_count);
			conn_push_outqueue(c, rpacket);
			packet_del_ref(rpacket);

			return 0;
		}

		extern int clanmember_on_change_status(t_clanmember * member)
		{
			t_packet * rpacket;
			if (member == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clanmember");
				return -1;
			}
			if (member->clan == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return -1;
			}
			if ((rpacket = packet_create(packet_class_bnet)) != NULL)
			{
				char tmpstr[2];
				const char *append_str;
				packet_set_size(rpacket, sizeof(t_server_clanmemberupdate));
				packet_set_type(rpacket, SERVER_CLANMEMBERUPDATE);
				packet_append_string(rpacket, account_get_name((t_account*)member->memberacc));
				tmpstr[0] = member->status;
				append_str = clanmember_get_online_status(member, &tmpstr[1]);
				packet_append_data(rpacket, tmpstr, 2);
				if (append_str)
					packet_append_string(rpacket, append_str);
				else
					packet_append_string(rpacket, "");
				clan_send_packet_to_online_members(member->clan, rpacket);
				packet_del_ref(rpacket);
			}
			return 0;
		}

		extern int clanmember_on_change_status_by_connection(t_connection * conn)
		{
			t_packet * rpacket;
			t_account *acc;
			t_clanmember *member;
			if (!(conn))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return -1;
			}
			if ((acc = conn_get_account(conn)) == NULL)
				return -1;
			if ((member = account_get_clanmember(acc)) == NULL)
				return -1;
			if (member->clan == NULL)
				return -1;
			if ((rpacket = packet_create(packet_class_bnet)) != NULL)
			{
				char tmpstr[2];
				const char *append_str;
				packet_set_size(rpacket, sizeof(t_server_clanmemberupdate));
				packet_set_type(rpacket, SERVER_CLANMEMBERUPDATE);
				packet_append_string(rpacket, account_get_name(acc));
				tmpstr[0] = member->status;
				append_str = clanmember_get_online_status_by_connection(conn, &tmpstr[1]);
				packet_append_data(rpacket, tmpstr, 2);
				if (append_str)
					packet_append_string(rpacket, append_str);
				else
					packet_append_string(rpacket, "");
				clan_send_packet_to_online_members(member->clan, rpacket);
				packet_del_ref(rpacket);
			}
			return 0;
		}

		extern int clan_unload_members(t_clan * clan)
		{
			t_elem *curr;
			t_clanmember *member;

			if (clan->members)
			{
				LIST_TRAVERSE(clan->members, curr)
				{
					if (!(member = (t_clanmember*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
						continue;
					}
					list_remove_elem(clan->members, &curr);
					xfree((void *)member);
				}

				if (list_destroy(clan->members) < 0)
					return -1;

				clan->members = NULL;
			}

			return 0;
		}

		extern int clan_remove_all_members(t_clan * clan)
		{
			t_elem *curr;
			t_clanmember *member;

			if (clan->members)
			{
				LIST_TRAVERSE(clan->members, curr)
				{
					if (!(member = (t_clanmember*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
						continue;
					}
					if (member->memberacc != NULL)
						account_set_clanmember((t_account*)member->memberacc, NULL);
					list_remove_elem(clan->members, &curr);
					xfree((void *)member);
				}

				if (list_destroy(clan->members) < 0)
					return -1;

				clan->members = NULL;
			}

			return 0;
		}

		extern int clanlist_remove_clan(t_clan * clan)
		{
			t_elem * elem;
			if (clan == NULL)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "get NULL clan");
				return -1;
			}
			if (list_remove_data(clanlist_head, clan, &elem) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not delete clan entry");
				return -1;
			}
			return 0;
		}

		extern int clan_remove(t_clantag clantag)
		{
			return storage->remove_clan(clantag);
		}

		extern int clan_save(t_clan * clan)
		{
			if (clan->created <= 0)
			{
				if (now - clan->creation_time > 120)
				{
					clanlist_remove_clan(clan);
					clan_destroy(clan);
				}
				return 0;
			}

			storage->write_clan(clan);

			clan->modified = 0;

			return 0;
		}

		extern t_list *clanlist(void)
		{
			return clanlist_head;
		}

		extern int clanlist_add_clan(t_clan * clan)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return -1;
			}

			if (!(clan->clanid))
				clan->clanid = ++max_clanid;

			list_append_data(clanlist_head, clan);

			return clan->clanid;
		}

		int clanlist_load(void)
		{
			// make sure to unload previous clanlist before loading again
			if (clanlist_head)
				clanlist_unload();

			clanlist_head = list_create();

			storage->load_clans(_cb_load_clans);

			return 0;
		}

		extern int clanlist_save(void)
		{
			t_elem *curr;
			t_clan *clan;

			if (clanlist_head)
			{
				LIST_TRAVERSE(clanlist_head, curr)
				{
					if (!(clan = (t_clan*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
						continue;
					}
					if (clan->modified)
						clan_save(clan);
				}

			}

			return 0;
		}

		extern int clanlist_unload(void)
		{
			t_elem *curr;
			t_clan *clan;

			if (clanlist_head)
			{
				LIST_TRAVERSE(clanlist_head, curr)
				{
					if (!(clan = (t_clan*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
						continue;
					}
					if (clan->clanname)
						xfree((void *)clan->clanname);
					if (clan->clan_motd)
						xfree((void *)clan->clan_motd);
					clan_unload_members(clan);
					xfree((void *)clan);
					list_remove_elem(clanlist_head, &curr);
				}

				if (list_destroy(clanlist_head) < 0)
					return -1;

				clanlist_head = NULL;
			}

			return 0;
		}

		extern t_clan *clanlist_find_clan_by_clanid(unsigned cid)
		{
			t_elem *curr;
			t_clan *clan;

			if (clanlist_head)
			{
				LIST_TRAVERSE(clanlist_head, curr)
				{
					if (!(clan = (t_clan*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
						continue;
					}
					eventlog(eventlog_level_trace, __FUNCTION__, "trace %d", clan->clanid);
					if (clan->created && (clan->clanid == cid))
						return clan;
				}

			}

			return NULL;
		}

		extern t_clan *clanlist_find_clan_by_clantag(t_clantag clantag)
		{
			t_elem *curr;
			t_clan *clan;
			char * needle;

			if (clantag == 0)
				return NULL;

			needle = xstrdup(clantag_to_str(clantag));
			if (clanlist_head)
			{
				LIST_TRAVERSE(clanlist_head, curr)
				{
					if (!(clan = (t_clan*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
						continue;
					}
					if (clan->created && !strcasecmp(needle, clantag_to_str(clan->tag))) {
						xfree(needle);
						return clan;
					}
				}

			}

			xfree(needle);
			return NULL;
		}

		extern t_clanmember *clan_find_member(t_clan * clan, t_account * memberacc)
		{
			t_clanmember *member;
			t_elem *curr;
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return NULL;
			}
			if (!(clan->members))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "found NULL clan->members");
				return NULL;
			}
			LIST_TRAVERSE(clan->members, curr)
			{
				if (!(member = (t_clanmember*)elem_get_data(curr)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL element in list");
					return NULL;
				}
				if (member->memberacc == memberacc)
					return member;
			}

			return NULL;
		}

		extern t_clanmember *clan_find_member_by_name(t_clan * clan, char const *membername)
		{
			t_clanmember *member;
			t_elem *curr;
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return NULL;
			}
			if (!(clan->members))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "found NULL clan->members");
				return NULL;
			}
			LIST_TRAVERSE(clan->members, curr)
			{
				if (!(member = (t_clanmember*)elem_get_data(curr)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL element in list");
					return NULL;
				}
				if (strcasecmp(account_get_name((t_account*)member->memberacc), membername) == 0)
					return member;
			}

			return NULL;
		}

		extern t_clanmember *clan_find_member_by_uid(t_clan * clan, unsigned int memberuid)
		{
			t_clanmember *member;
			t_elem *curr;
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return NULL;
			}
			if (!(clan->members))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "found NULL clan->members");
				return NULL;
			}
			LIST_TRAVERSE(clan->members, curr)
			{
				if (!(member = (t_clanmember*)elem_get_data(curr)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL element in list");
					return NULL;
				}
				if (account_get_uid((t_account*)member->memberacc) == memberuid)
					return member;
			}

			return NULL;
		}

		extern t_account *clanmember_get_account(t_clanmember * member)
		{
			if (!(member))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clanmember");
				return NULL;
			}

			return (t_account *)member->memberacc;
		}

		extern int clanmember_set_account(t_clanmember * member, t_account * memberacc)
		{
			if (!(member))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clanmember");
				return -1;
			}

			member->memberacc = memberacc;
			return 0;
		}

		extern t_connection *clanmember_get_conn(t_clanmember * member)
		{
			t_account * account;

			if (!(member))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clanmember");
				return NULL;
			}

			if (!(account = (t_account*)member->memberacc))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "member as NULL account");
				return NULL;
			}

			return account_get_conn(account);
		}

		extern char clanmember_get_status(t_clanmember * member)
		{
			if (!(member))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clanmember");
				return 0;
			}

			if ((member->status == CLAN_NEW) && (now - member->join_time > prefs_get_clan_newer_time() * 3600))
			{
				member->status = CLAN_PEON;
				member->clan->modified = 1;
#ifdef WITH_SQL
				member->modified = 1;
#endif
			}

			return member->status;
		}

		extern int clanmember_set_status(t_clanmember * member, char status)
		{
			if (!(member))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clanmember");
				return -1;
			}

			if (member->status != status)
			{
				member->status = status;
				member->clan->modified = 1;
#ifdef WITH_SQL
				member->modified = 1;
#endif
			}
			return 0;
		}

		extern int clanmember_set_join_time(t_clanmember * member, std::time_t join_time)
		{
			if (!(member)) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clanmember");
				return 0;
			}

			member->join_time = join_time;

			return 0;
		}

		extern std::time_t clanmember_get_join_time(t_clanmember * member)
		{
			if (!(member))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clanmember");
				return 0;
			}

			return member->join_time;
		}

		extern t_clan *clanmember_get_clan(t_clanmember * member)
		{
			if (!(member))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clanmember");
				return 0;
			}

			return member->clan;
		}

		extern int clanmember_get_fullmember(t_clanmember * member)
		{
			if (!(member)) {
				ERROR0("got NULL clanmember");
				return -1;
			}

			return member->fullmember;
		}

		extern int clanmember_set_fullmember(t_clanmember * member, int fullmember)
		{
			if (!(member)) {
				ERROR0("got NULL clanmember");
				return -1;
			}

			if (!(fullmember)) {
				ERROR0("got NULL fullmember");
				return -1;
			}

			member->fullmember = fullmember;

			return 0;
		}

		extern const char *clanmember_get_online_status(t_clanmember * member, char *status)
		{
			return clanmember_get_online_status_by_connection(clanmember_get_conn(member), status);
		}

		extern const char *clanmember_get_online_status_by_connection(t_connection * conn, char *status)
		{
			if (conn && (conn_get_state(conn) != conn_state_empty))
			{
				t_game *game;
				t_channel *channel;
				if ((game = conn_get_game(conn)) != NULL)
				{
					if (game_get_flag(game) == game_flag_private)
						(*status) = SERVER_CLAN_MEMBER_PRIVATE_GAME;
					else
						(*status) = SERVER_CLAN_MEMBER_GAME;
					return game_get_name(game);
				}
				if ((channel = conn_get_channel(conn)) != NULL)
				{
					(*status) = SERVER_CLAN_MEMBER_CHANNEL;
					return channel_get_name(channel);
				}

				(*status) = SERVER_CLAN_MEMBER_ONLINE;
			}
			else
				(*status) = SERVER_CLAN_MEMBER_OFFLINE;
			return NULL;
		}

		extern int clanmember_set_online(t_connection * c)
		{
			t_clanmember *member;
			t_account *acc;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if ((acc = conn_get_account(c)) && (member = account_get_clanmember(acc)))
			{
				clanmember_on_change_status(member);
			}

			return 0;
		}

		extern int clanmember_set_offline(t_connection * c)
		{
			t_clanmember *member;
			t_account *acc;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}

			if ((acc = conn_get_account(c)) && (member = account_get_clanmember_forced(acc)))
			{
				clanmember_on_change_status(member);
			}

			return 0;
		}

		extern int clan_get_created(t_clan * clan)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return -1;
			}

			return clan->created;
		}

		extern int clan_set_created(t_clan * clan, int created)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return -1;
			}

			clan->created = created;

			return 0;
		}

		extern char clan_get_modified(t_clan * clan)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return -1;
			}

			return clan->modified;
		}

		extern int clan_set_modified(t_clan * clan, char modified)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return -1;
			}

			clan->modified = modified;

			return 0;
		}

		extern char clan_get_channel_type(t_clan * clan)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return -1;
			}

			return clan->channel_type;
		}

		extern int clan_set_channel_type(t_clan * clan, char channel_type)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return -1;
			}

			clan->channel_type = channel_type;

			return 0;
		}

		extern t_list *clan_get_members(t_clan * clan)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return NULL;
			}

			return clan->members;
		}

		extern char const *clan_get_name(t_clan * clan)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return NULL;
			}

			return clan->clanname;
		}

		extern t_clantag clan_get_clantag(t_clan * clan)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return 0;
			}

			return clan->tag;
		}

		extern char const *clan_get_motd(t_clan * clan)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return NULL;
			}

			return clan->clan_motd;
		}

		extern int clan_set_motd(t_clan * clan, const char *motd)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return -1;
			}
			if (!(motd))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL motd");
				return -1;
			}
			else
			{
				if (clan->clan_motd)
					xfree((void *)clan->clan_motd);
				clan->clan_motd = xstrdup(motd);
			}
			return 0;
		}

		extern unsigned int clan_get_clanid(t_clan * clan)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return 0;
			}

			return clan->clanid;
		}

		extern int clan_set_creation_time(t_clan * clan, std::time_t c_time)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return 0;
			}

			clan->creation_time = c_time;

			return 0;
		}

		extern std::time_t clan_get_creation_time(t_clan * clan)
		{
			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return 0;
			}

			return clan->creation_time;
		}

		extern t_clanmember *clan_add_member(t_clan * clan, t_account * memberacc, char status)
		{
			t_clanmember *member;

			if (!(clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clan");
				return NULL;
			}

			if (!(clan->members))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "found NULL clan->members");
				return NULL;
			}

			member = (t_clanmember*)xmalloc(sizeof(t_clanmember));
			member->memberacc = memberacc;
			member->status = status;
			member->join_time = now;
			member->clan = clan;
			member->fullmember = 0; /* clanmember is invited */
#ifdef WITH_SQL
			member->modified = 1;
#endif

			list_append_data(clan->members, member);

			account_set_clanmember(memberacc, member);

			clan->modified = 1;

			return member;
		}

		extern int clan_remove_member(t_clan * clan, t_clanmember * member)
		{
			t_elem * elem;

			if (!member)
				return -1;
			if (list_remove_data(clan->members, member, &elem) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not remove member");
				return -1;
			}
			if (member->memberacc != NULL)
			{
				account_set_clanmember((t_account*)member->memberacc, NULL);
				if (member->fullmember == true)
					storage->remove_clanmember(account_get_uid((t_account*)member->memberacc));
			}
			xfree((void *)member);
			clan->modified = 1;
			return 0;
		}

		extern t_clan *clan_create(t_account * chieftain_acc, t_clantag clantag, const char *clanname, const char *motd)
		{
			t_clan *clan;
			t_clanmember *member;

			clan = (t_clan*)xmalloc(sizeof(t_clan));
			member = (t_clanmember*)xmalloc(sizeof(t_clanmember));

			if (!(clanname))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL clanname");
				xfree((void *)clan);
				xfree((void *)member);
				return NULL;
			}

			clan->clanname = xstrdup(clanname);

			if (!(motd))
				clan->clan_motd = xstrdup("This is a newly created clan");
			else
				clan->clan_motd = xstrdup(motd);

			clan->creation_time = now;
			clan->tag = clantag;
			clan->clanid = ++max_clanid;
			clan->created = 0;
			clan->modified = 1;
			clan->channel_type = prefs_get_clan_channel_default_private();

			clan->members = list_create();

			member->memberacc = chieftain_acc;
			member->status = CLAN_CHIEFTAIN;
			member->join_time = clan->creation_time;
			member->fullmember = 1; /* chief should be considered invited */
			member->clan = clan;
#ifdef WITH_SQL
			member->modified = 1;
#endif

			list_append_data(clan->members, member);

			account_set_clanmember(chieftain_acc, member);

			return clan;
		}

		extern int clan_destroy(t_clan * clan)
		{
			if (!clan)
				return 0;
			if (clan->clanname)
				xfree((void *)clan->clanname);
			if (clan->clan_motd)
				xfree((void *)clan->clan_motd);
			clan_remove_all_members(clan);
			xfree((void *)clan);
			return 0;
		}

		extern unsigned clan_get_member_count(t_clan * clan)
		{
			t_elem *curr;
			unsigned count = 0;
			LIST_TRAVERSE(clan->members, curr)
			{
				if ((elem_get_data(curr)) != NULL)
					count++;
			}
			return count;
		}

		extern t_clantag str_to_clantag(const char *str)
		{
			t_clantag tag = 0;

			if (str[0])
			{
				tag |= str[0] << 24;
				if (str[1])
				{
					tag |= str[1] << 16;
					if (str[2])
					{
						tag |= str[2] << 8;
						if (str[3])
							tag |= str[3];
					}
				}
			}

			return tag;
		}

		extern const char * clantag_to_str(t_clantag tag)
		{
			static char tagstr[sizeof(tag)+1];

			std::sprintf(tagstr, "%c%c%c%c", tag >> 24, (tag >> 16) & 0xff, (tag >> 8) & 0xff, tag & 0xff);
			return tagstr;
		}

	}

}
