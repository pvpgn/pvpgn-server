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

#ifndef INCLUDED_CLAN_TYPES
#define INCLUDED_CLAN_TYPES

#include "common/tag.h"
#ifdef JUST_NEED_TYPES
# include "common/bn_type.h"
# include "connection.h"
# include "message.h"
# include "common/packet.h"
#else
# define JUST_NEED_TYPES
# include "common/bn_type.h"
# include "connection.h"
# include "message.h"
# include "common/packet.h"
# include "common/tag.h"
# undef JUST_NEED_TYPES
#endif

#ifdef CLAN_INTERNAL_ACCESS
#ifdef JUST_NEED_TYPES
# include "common/list.h"
# include "account.h"
#else
# define JUST_NEED_TYPES
# include "common/list.h"
# include "account.h"
# undef JUST_NEED_TYPES
#endif

#endif

namespace pvpgn
{

	namespace bnetd
	{

		typedef t_tag t_clantag;

		typedef struct clan
#ifdef CLAN_INTERNAL_ACCESS
		{
			unsigned int clanid;
			t_clantag tag;
			char const *clanname;
			std::time_t creation_time;
			char const *clan_motd;
			t_list *members;
			int created;
			/* --by Soar
			   on create, set it to -count of invited members,
			   each accept packet will inc it by 1,
			   when it is increased to 0, means that all invited members have accepted,
			   then clan will be created and set this value to 1
			   */
			char modified;
			char channel_type;		/* 0--public 1--private */
		}
#endif
		t_clan;

		typedef struct _clanmember
#ifdef CLAN_INTERNAL_ACCESS
		{
			t_account * memberacc;
			char status;
			std::time_t join_time;
			t_clan * clan;
			int fullmember; /* PELISH: 0 -- clanmember is only invited,
									   1 -- clanmember is fullmember  */
#ifdef WITH_SQL
			char modified;
#endif
		}
#endif
		t_clanmember;

	}

}
#define CLAN_CHIEFTAIN 0x04
#define CLAN_SHAMAN    0x03
#define CLAN_GRUNT     0x02
#define CLAN_PEON      0x01
#define CLAN_NEW       0x00

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_CLAN_PROTOS
#define INCLUDED_CLAN_PROTOS

#define JUST_NEED_TYPES
#include "common/list.h"
#undef JUST_NEED_TYPES


namespace pvpgn
{

	namespace bnetd
	{

		extern t_list *clanlist(void);
		extern int clanlist_load(void);
		extern int clanlist_save(void);
		extern int clanlist_unload(void);
		extern int clanlist_remove_clan(t_clan * clan);
		extern int clanlist_add_clan(t_clan * clan);
		extern t_clan *clanlist_find_clan_by_clanid(unsigned cid);
		extern t_clan *clanlist_find_clan_by_clantag(t_clantag clantag);


		extern t_account *clanmember_get_account(t_clanmember * member);
		extern int clanmember_set_account(t_clanmember * member, t_account * memberacc);
		extern t_connection *clanmember_get_conn(t_clanmember * member);
		extern char clanmember_get_status(t_clanmember * member);
		extern int clanmember_set_status(t_clanmember * member, char status);
		extern int clanmember_set_join_time(t_clanmember * member, std::time_t join_time);
		extern std::time_t clanmember_get_join_time(t_clanmember * member);
		extern t_clan * clanmember_get_clan(t_clanmember * member);
		extern int clanmember_set_online(t_connection * c);
		extern int clanmember_set_offline(t_connection * c);
		extern int clanmember_get_fullmember(t_clanmember * member);
		extern int clanmember_set_fullmember(t_clanmember * member, int fullmember);

		extern const char *clanmember_get_online_status(t_clanmember * member, char *status);
		extern int clanmember_on_change_status(t_clanmember * member);
		extern const char *clanmember_get_online_status_by_connection(t_connection * conn, char *status);
		extern int clanmember_on_change_status_by_connection(t_connection * conn);

		extern t_clan *clan_create(t_account * chieftain_acc, t_clantag clantag, const char *clanname, const char *motd);
		extern int clan_destroy(t_clan * clan);

		extern int clan_unload_members(t_clan * clan);
		extern int clan_remove_all_members(t_clan * clan);

		extern int clan_save(t_clan * clan);
		extern int clan_remove(t_clantag clantag);

		extern int clan_get_created(t_clan * clan);
		extern int clan_set_created(t_clan * clan, int created);
		extern char clan_get_modified(t_clan * clan);
		extern int clan_set_modified(t_clan * clan, char modified);
		extern char clan_get_channel_type(t_clan * clan);
		extern int clan_set_channel_type(t_clan * clan, char channel_type);
		extern t_list *clan_get_members(t_clan * clan);
		extern char const *clan_get_name(t_clan * clan);
		extern t_clantag clan_get_clantag(t_clan * clan);
		extern char const *clan_get_motd(t_clan * clan);
		extern int clan_set_motd(t_clan * clan, const char *motd);
		extern unsigned int clan_get_clanid(t_clan * clan);
		extern int clan_set_creation_time(t_clan * clan, std::time_t c_time);
		extern std::time_t clan_get_creation_time(t_clan * clan);
		extern unsigned clan_get_member_count(t_clan * clan);

		extern t_clanmember *clan_add_member(t_clan * clan, t_account * memberacc, char status);
		extern int clan_remove_member(t_clan * clan, t_clanmember * member);

		extern t_clanmember *clan_find_member(t_clan * clan, t_account * memberacc);
		extern t_clanmember *clan_find_member_by_name(t_clan * clan, char const *membername);
		extern t_clanmember *clan_find_member_by_uid(t_clan * clan, unsigned int memberuid);

		extern int clan_send_message_to_online_members(t_clan * clan, t_message_type type, t_connection * me, char const * text);
		extern int clan_send_packet_to_online_members(t_clan * clan, t_packet * packet);
		extern int clan_get_possible_member(t_connection * c, t_packet const *const packet);
		extern int clan_send_status_window(t_connection * c);
		extern int clan_send_status_window_on_create(t_clan * clan);
		extern int clan_close_status_window(t_connection * c);
		extern int clan_close_status_window_on_disband(t_clan * clan);
		extern int clan_send_memberlist(t_connection * c, t_packet const *const packet);
		extern int clan_change_member_status(t_connection * c, t_packet const *const packet);
		extern int clan_send_motd_reply(t_connection * c, t_packet const *const packet);
		extern int clan_save_motd_chg(t_connection * c, t_packet const *const packet);

		extern t_clantag str_to_clantag(const char *str);
		extern const char* clantag_to_str(t_clantag tag);

	}

}

#endif
#endif
