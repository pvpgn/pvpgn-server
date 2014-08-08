/*
 * Copyright (C) 2004 CreepLord (creeplord@pvpgn.org)
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
#include "handle_anongame.h"

#include <cstring>
#include <cstdio>

#include "common/eventlog.h"
#include "common/bn_type.h"
#include "common/bnettime.h"
#include "common/tag.h"
#include "common/list.h"

#include "clan.h"
#include "account.h"
#include "account_wrap.h"
#include "ladder.h"
#include "team.h"
#include "anongame.h"
#include "anongame_infos.h"
#include "server.h"
#include "tournament.h"
#include "channel.h"
#include "common/setup_after.h"
#include "icons.h"

namespace pvpgn
{

	namespace bnetd
	{

		/* option - handling function */

		/* 0x00 */ /* PG style search - handle_anongame_search() in anongame.c */
		/* 0x01 */ /* server side packet sent from handle_anongame_search() in anongame.c */
		/* 0x02 */ static int _client_anongame_infos(t_connection * c, t_packet const * const packet);
		/* 0x03 */ static int _client_anongame_cancel(t_connection * c);
		/* 0x04 */ static int _client_anongame_profile(t_connection * c, t_packet const * const packet);
		/* 0x05 */ /* AT style search - handle_anongame_search() in anongame.c */
		/* 0x06 */ /* AT style search (Inviter) handle_anongame_search() in anongame.c */
		/* 0x07 */ static int _client_anongame_tournament(t_connection * c, t_packet const * const packet);
		/* 0x08 */ static int _client_anongame_profile_clan(t_connection * c, t_packet const * const packet);
		/* 0x09 */ static int _client_anongame_get_icon(t_connection * c, t_packet const * const packet);
		/* 0x0A */ static int _client_anongame_set_icon(t_connection * c, t_packet const * const packet);
		static int check_user_icon(t_account * account, const char * user_icon);

		/* misc functions used by _client_anongame_tournament() */
		static unsigned int _tournament_time_convert(unsigned int time);

		/* and now the functions */

		static int _client_anongame_profile_clan(t_connection * c, t_packet const * const packet)
		{
			t_packet * rpacket;
			int clantag;
			int clienttag;
			int count;
			int temp;
			t_clan * clan;
			unsigned char rescount;

			if (packet_get_size(packet) < sizeof(t_client_findanongame_profile_clan))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] got bad ANONGAME_PROFILE_CLAN packet (expected %lu bytes, got %u)", conn_get_socket(c), sizeof(t_client_findanongame_profile_clan), packet_get_size(packet));
				return -1;
			}

			clantag = bn_int_get(packet->u.client_findanongame_profile_clan.clantag);
			clienttag = bn_int_get(packet->u.client_findanongame_profile_clan.clienttag);
			count = bn_int_get(packet->u.server_findanongame_profile_clan.count);
			clan = clanlist_find_clan_by_clantag(clantag);

			if ((rpacket = packet_create(packet_class_bnet)))
			{
				packet_set_size(rpacket, sizeof(t_server_findanongame_profile_clan));
				packet_set_type(rpacket, SERVER_FINDANONGAME_PROFILE_CLAN);
				bn_byte_set(&rpacket->u.server_findanongame_profile_clan.option, CLIENT_FINDANONGAME_PROFILE_CLAN);
				bn_int_set(&rpacket->u.server_findanongame_profile_clan.count, count);
				rescount = 0;

				if (!(clan))
				{
					temp = 0;
					packet_append_data(rpacket, &temp, 1);
				}
				else
				{
					temp = 0;
					packet_append_data(rpacket, &temp, 1);

					/* need to add clan stuff here:
					 format:
					 bn_int	ladder_tag (SNLC, 2NLC, 3NLC, 4NLC)
					 bn_int	wins
					 bn_int	losses
					 bn_byte rank
					 bn_byte progess bar
					 bn_int	xp
					 bn_int	rank
					 bn_byte 0x06 <-- random + 5 races
					 6 times:
					 bn_int  wins
					 bn_int	losses
					 */
				}

				bn_byte_set(&rpacket->u.server_findanongame_profile_clan.rescount, rescount);


				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
			}

			return 0;
		}

		static int _client_anongame_profile(t_connection * c, t_packet const * const packet)
		{
			t_packet * rpacket;
			char const * username;
			int Count, i;
			int temp;
			t_account * account;
			t_connection * dest_c;
			t_clienttag ctag;
			char clienttag_str[5];
			t_list * teamlist;
			unsigned char teamcount;
			unsigned char *atcountp;
			t_elem * curr;
			t_team * team;
			t_bnettime bn_time;
			bn_long ltime;


			Count = bn_int_get(packet->u.client_findanongame.count);
			eventlog(eventlog_level_info, __FUNCTION__, "[%d] got a FINDANONGAME PROFILE packet", conn_get_socket(c));

			if (!(username = packet_get_str_const(packet, sizeof(t_client_findanongame_profile), MAX_USERNAME_LEN)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "[%d] got bad FINDANONGAME_PROFILE (missing or too long username)", conn_get_socket(c));
				return -1;
			}

			//If no account is found then break
			if (!(account = accountlist_find_account(username)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Could not get account - PROFILE");
				return -1;
			}

			if (!(dest_c = connlist_find_connection_by_accountname(username))) {
				eventlog(eventlog_level_debug, __FUNCTION__, "account is offline -  try ll_clienttag");
				if (!(ctag = account_get_ll_clienttag(account))) return -1;
			}
			else
				ctag = conn_get_clienttag(dest_c);

			eventlog(eventlog_level_info, __FUNCTION__, "Looking up %s's %s Stats.", username, tag_uint_to_str(clienttag_str, ctag));

			if (account_get_ladder_level(account, ctag, ladder_id_solo) <= 0 &&
				account_get_ladder_level(account, ctag, ladder_id_team) <= 0 &&
				account_get_ladder_level(account, ctag, ladder_id_ffa) <= 0 &&
				account_get_teams(account) == NULL)
			{
				eventlog(eventlog_level_info, __FUNCTION__, "%s does not have WAR3 Stats.", username);
				if (!(rpacket = packet_create(packet_class_bnet)))
					return -1;
				packet_set_size(rpacket, sizeof(t_server_findanongame_profile2));
				packet_set_type(rpacket, SERVER_FINDANONGAME_PROFILE);
				bn_byte_set(&rpacket->u.server_findanongame_profile2.option, CLIENT_FINDANONGAME_PROFILE);
				bn_int_set(&rpacket->u.server_findanongame_profile2.count, Count);
				bn_int_set(&rpacket->u.server_findanongame_profile2.icon, account_icon_to_profile_icon(account_get_user_icon(account, ctag), account, ctag));
				bn_byte_set(&rpacket->u.server_findanongame_profile2.rescount, 0);
				temp = 0;
				packet_append_data(rpacket, &temp, 2);
				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
			}
			else // If they do have a profile then:
			{
				int solowins = account_get_ladder_wins(account, ctag, ladder_id_solo);
				int sololoss = account_get_ladder_losses(account, ctag, ladder_id_solo);
				int soloxp = account_get_ladder_xp(account, ctag, ladder_id_solo);
				int sololevel = account_get_ladder_level(account, ctag, ladder_id_solo);
				int solorank = account_get_ladder_rank(account, ctag, ladder_id_solo);

				int teamwins = account_get_ladder_wins(account, ctag, ladder_id_team);
				int teamloss = account_get_ladder_losses(account, ctag, ladder_id_team);
				int teamxp = account_get_ladder_xp(account, ctag, ladder_id_team);
				int teamlevel = account_get_ladder_level(account, ctag, ladder_id_team);
				int teamrank = account_get_ladder_rank(account, ctag, ladder_id_team);

				int ffawins = account_get_ladder_wins(account, ctag, ladder_id_ffa);
				int ffaloss = account_get_ladder_losses(account, ctag, ladder_id_ffa);
				int ffaxp = account_get_ladder_xp(account, ctag, ladder_id_ffa);
				int ffalevel = account_get_ladder_level(account, ctag, ladder_id_ffa);
				int ffarank = account_get_ladder_rank(account, ctag, ladder_id_ffa);

				int humanwins = account_get_racewins(account, W3_RACE_HUMANS, ctag);
				int humanlosses = account_get_racelosses(account, W3_RACE_HUMANS, ctag);
				int orcwins = account_get_racewins(account, W3_RACE_ORCS, ctag);
				int orclosses = account_get_racelosses(account, W3_RACE_ORCS, ctag);
				int undeadwins = account_get_racewins(account, W3_RACE_UNDEAD, ctag);
				int undeadlosses = account_get_racelosses(account, W3_RACE_UNDEAD, ctag);
				int nightelfwins = account_get_racewins(account, W3_RACE_NIGHTELVES, ctag);
				int nightelflosses = account_get_racelosses(account, W3_RACE_NIGHTELVES, ctag);
				int randomwins = account_get_racewins(account, W3_RACE_RANDOM, ctag);
				int randomlosses = account_get_racelosses(account, W3_RACE_RANDOM, ctag);
				int tourneywins = account_get_racewins(account, W3_RACE_DEMONS, ctag);
				int tourneylosses = account_get_racelosses(account, W3_RACE_DEMONS, ctag);

				unsigned char rescount;

				if (!(rpacket = packet_create(packet_class_bnet)))
					return -1;
				packet_set_size(rpacket, sizeof(t_server_findanongame_profile2));
				packet_set_type(rpacket, SERVER_FINDANONGAME_PROFILE);
				bn_byte_set(&rpacket->u.server_findanongame_profile2.option, CLIENT_FINDANONGAME_PROFILE);
				bn_int_set(&rpacket->u.server_findanongame_profile2.count, Count); //job count
				bn_int_set(&rpacket->u.server_findanongame_profile2.icon, account_icon_to_profile_icon(account_get_user_icon(account, ctag), account, ctag));

				rescount = 0;
				if (sololevel > 0) {
					bn_int_set((bn_int*)&temp, 0x534F4C4F); // SOLO backwards
					packet_append_data(rpacket, &temp, 4);
					temp = 0;
					bn_int_set((bn_int*)&temp, solowins);
					packet_append_data(rpacket, &temp, 2); //SOLO WINS
					bn_int_set((bn_int*)&temp, sololoss);
					packet_append_data(rpacket, &temp, 2); // SOLO LOSSES
					bn_int_set((bn_int*)&temp, sololevel);
					packet_append_data(rpacket, &temp, 1); // SOLO LEVEL
					bn_int_set((bn_int*)&temp, account_get_profile_calcs(account, soloxp, sololevel));
					packet_append_data(rpacket, &temp, 1); // SOLO PROFILE CALC
					bn_int_set((bn_int *)&temp, soloxp);
					packet_append_data(rpacket, &temp, 2); // SOLO XP
					bn_int_set((bn_int *)&temp, solorank);
					packet_append_data(rpacket, &temp, 4); // SOLO LADDER RANK
					rescount++;
				}

				if (teamlevel > 0) {
					//below is for team records. Add this after 2v2,3v3,4v4 are done
					bn_int_set((bn_int*)&temp, 0x5445414D);
					packet_append_data(rpacket, &temp, 4);
					bn_int_set((bn_int*)&temp, teamwins);
					packet_append_data(rpacket, &temp, 2);
					bn_int_set((bn_int*)&temp, teamloss);
					packet_append_data(rpacket, &temp, 2);
					bn_int_set((bn_int*)&temp, teamlevel);
					packet_append_data(rpacket, &temp, 1);
					bn_int_set((bn_int*)&temp, account_get_profile_calcs(account, teamxp, teamlevel));

					packet_append_data(rpacket, &temp, 1);
					bn_int_set((bn_int*)&temp, teamxp);
					packet_append_data(rpacket, &temp, 2);
					bn_int_set((bn_int*)&temp, teamrank);
					packet_append_data(rpacket, &temp, 4);
					//done of team game stats
					rescount++;
				}

				if (ffalevel > 0) {
					bn_int_set((bn_int*)&temp, 0x46464120);
					packet_append_data(rpacket, &temp, 4);
					bn_int_set((bn_int*)&temp, ffawins);
					packet_append_data(rpacket, &temp, 2);
					bn_int_set((bn_int*)&temp, ffaloss);
					packet_append_data(rpacket, &temp, 2);
					bn_int_set((bn_int*)&temp, ffalevel);
					packet_append_data(rpacket, &temp, 1);
					bn_int_set((bn_int*)&temp, account_get_profile_calcs(account, ffaxp, ffalevel));
					packet_append_data(rpacket, &temp, 1);
					bn_int_set((bn_int*)&temp, ffaxp);
					packet_append_data(rpacket, &temp, 2);
					bn_int_set((bn_int*)&temp, ffarank);
					packet_append_data(rpacket, &temp, 4);
					//End of FFA Stats
					rescount++;
				}
				/* set result count */
				bn_byte_set(&rpacket->u.server_findanongame_profile2.rescount, rescount);

				bn_int_set((bn_int*)&temp, 0x06); //start of race stats
				packet_append_data(rpacket, &temp, 1);
				bn_int_set((bn_int*)&temp, randomwins);
				packet_append_data(rpacket, &temp, 2); //random wins
				bn_int_set((bn_int*)&temp, randomlosses);
				packet_append_data(rpacket, &temp, 2); //random losses
				bn_int_set((bn_int*)&temp, humanwins);
				packet_append_data(rpacket, &temp, 2); //human wins
				bn_int_set((bn_int*)&temp, humanlosses);
				packet_append_data(rpacket, &temp, 2); //human losses
				bn_int_set((bn_int*)&temp, orcwins);
				packet_append_data(rpacket, &temp, 2); //orc wins
				bn_int_set((bn_int*)&temp, orclosses);
				packet_append_data(rpacket, &temp, 2); //orc losses
				bn_int_set((bn_int*)&temp, undeadwins);
				packet_append_data(rpacket, &temp, 2); //undead wins
				bn_int_set((bn_int*)&temp, undeadlosses);
				packet_append_data(rpacket, &temp, 2); //undead losses
				bn_int_set((bn_int*)&temp, nightelfwins);
				packet_append_data(rpacket, &temp, 2); //elf wins
				bn_int_set((bn_int*)&temp, nightelflosses);
				packet_append_data(rpacket, &temp, 2); //elf losses
				bn_int_set((bn_int*)&temp, tourneywins);
				packet_append_data(rpacket, &temp, 2); //tourney wins
				bn_int_set((bn_int*)&temp, tourneylosses);
				packet_append_data(rpacket, &temp, 2); //tourney losses
				//end of normal stats - Start of AT stats

				/* 1 byte team count place holder, set later */
				packet_append_data(rpacket, &temp, 1);

				/* we need to store the AT team count but we dont know yet the no
				 * of stored teams so we cache the pointer for later use
				 */
				atcountp = (unsigned char *)packet_get_raw_data(rpacket, packet_get_size(rpacket) - 1);

				teamlist = account_get_teams(account);
				teamcount = 0;
				if (teamlist)
				{
					int teamtype[] = { 0, 0x32565332, 0x33565333, 0x34565334, 0x35565335, 0x36565336 };

					LIST_TRAVERSE(teamlist, curr)
					{
						if (!(team = (t_team*)elem_get_data(curr)))
						{
							eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
							continue;
						}

						if (team_get_clienttag(team) != ctag)
							continue;

						bn_int_set((bn_int*)&temp, teamtype[team_get_size(team) - 1]);
						packet_append_data(rpacket, &temp, 4);

						bn_int_set((bn_int*)&temp, team_get_wins(team)); //at team wins
						packet_append_data(rpacket, &temp, 2);
						bn_int_set((bn_int*)&temp, team_get_losses(team)); //at team losses
						packet_append_data(rpacket, &temp, 2);
						bn_int_set((bn_int*)&temp, team_get_level(team));
						packet_append_data(rpacket, &temp, 1);
						bn_int_set((bn_int*)&temp, account_get_profile_calcs(account, team_get_xp(team), team_get_level(team))); // xp bar calc
						packet_append_data(rpacket, &temp, 1);
						bn_int_set((bn_int*)&temp, team_get_xp(team));
						packet_append_data(rpacket, &temp, 2);
						bn_int_set((bn_int*)&temp, team_get_rank(team)); //rank on AT ladder
						packet_append_data(rpacket, &temp, 4);

						bn_time = time_to_bnettime(temp, team_get_lastgame(team));
						bnettime_to_bn_long(bn_time, &ltime);
						packet_append_data(rpacket, &ltime, 8);

						bn_int_set((bn_int*)&temp, team_get_size(team) - 1);
						packet_append_data(rpacket, &temp, 1);

						for (i = 0; i < team_get_size(team); i++)
						{
							if ((team_get_memberuid(team, i) != account_get_uid(account)))
								packet_append_string(rpacket, account_get_name(team_get_member(team, i)));
							//now attach the names to the packet - not including yourself
							// [quetzal] 20020826

						}
						teamcount++;

						if ((teamcount >= 16)) break;
					}
				}

				*atcountp = (unsigned char)teamcount;

				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);

				eventlog(eventlog_level_info, __FUNCTION__, "Sent %s's WAR3 Stats (including %d teams) to requestor.", username, teamcount);
			}
			return 0;
		}

		static int _client_anongame_cancel(t_connection * c)
		{
			t_packet * rpacket;
			t_connection * tc[ANONGAME_MAX_GAMECOUNT / 2];

			// [quetzal] 20020809 - added a_count, so we dont refer to already destroyed anongame
			t_anongame *a = conn_get_anongame(c);
			int a_count, i;

			eventlog(eventlog_level_info, __FUNCTION__, "[%d] got FINDANONGAME CANCEL packet", conn_get_socket(c));

			if (!a)
				return -1;

			a_count = anongame_get_count(a);

			// anongame_unqueue(c, anongame_get_queue(a));
			// -- already doing unqueue in conn_destroy_anongame
			for (i = 0; i < ANONGAME_MAX_GAMECOUNT / 2; i++)
				tc[i] = anongame_get_tc(a, i);

			for (i = 0; i < ANONGAME_MAX_GAMECOUNT / 2; i++) {
				if (tc[i] == NULL)
					continue;

				conn_set_routeconn(tc[i], NULL);
				conn_destroy_anongame(tc[i]);
			}

			if (!(rpacket = packet_create(packet_class_bnet)))
				return -1;

			packet_set_size(rpacket, sizeof(t_server_findanongame_playgame_cancel));
			packet_set_type(rpacket, SERVER_FINDANONGAME_PLAYGAME_CANCEL);
			bn_byte_set(&rpacket->u.server_findanongame_playgame_cancel.cancel, SERVER_FINDANONGAME_CANCEL);
			bn_int_set(&rpacket->u.server_findanongame_playgame_cancel.count, a_count);
			conn_push_outqueue(c, rpacket);
			packet_del_ref(rpacket);
			return 0;
		}

		/* Open portrait in Warcraft 3 user profile */
		static int _client_anongame_get_icon(t_connection * c, t_packet const * const packet)
		{
			t_packet * rpacket;
			//BlacKDicK 04/20/2003 Need some huge re-work on this.
			{
				struct
				{
					char	 icon_code[4];
					unsigned int portrait_code;
					char	 race;
					bn_short	 required_wins;
					char	 client_enabled;
				} tempicon;

				//FIXME: Add those to the prefs and also merge them on accoun_wrap;
				// FIXED BY DJP 07/16/2003 FOR 110 CHANGE ( TOURNEY & RACE WINS ) + Table_witdh
				short icon_req_race_wins;
				short icon_req_tourney_wins;
				int race[] = { W3_RACE_RANDOM, W3_RACE_HUMANS, W3_RACE_ORCS, W3_RACE_UNDEAD, W3_RACE_NIGHTELVES, W3_RACE_DEMONS };
				char race_char[6] = { 'R', 'H', 'O', 'U', 'N', 'D' };
				char icon_pos[5] = { '2', '3', '4', '5', '6', };
				char table_width = 6;
				char table_height = 5;
				int i, j;
				char rico;
				unsigned int rlvl, rwins;
				t_clienttag clienttag;
				t_account * acc;

				char user_icon[5];
				char const * uicon;

				clienttag = conn_get_clienttag(c);
				acc = conn_get_account(c);
				/* WAR3 uses a different table size, might change if blizzard add tournament support to RoC */
				if (clienttag == CLIENTTAG_WARCRAFT3_UINT) {
					table_width = 5;
					table_height = 4;
				}

				eventlog(eventlog_level_info, __FUNCTION__, "[%d] got FINDANONGAME Get Icons packet", conn_get_socket(c));

				if ((rpacket = packet_create(packet_class_bnet)) == NULL) {
					eventlog(eventlog_level_error, __FUNCTION__, "could not create new packet");
					return -1;
				}

				packet_set_size(rpacket, sizeof(t_server_findanongame_iconreply));
				packet_set_type(rpacket, SERVER_FINDANONGAME_ICONREPLY);
				bn_int_set(&rpacket->u.server_findanongame_iconreply.count, bn_int_get(packet->u.client_findanongame_inforeq.count));
				bn_byte_set(&rpacket->u.server_findanongame_iconreply.option, CLIENT_FINDANONGAME_GET_ICON);
				
				
				if (prefs_get_custom_icons() == 1)
				{
					// get current custom icon
					t_icon_info * icon;
					if (icon = customicons_get_icon_by_account(acc, clienttag))
						std::memcpy(&rpacket->u.server_findanongame_iconreply.curricon, icon->icon_code, 4);
				}
				else if ((uicon = account_get_user_icon(acc, clienttag)))
				{
					std::memcpy(&rpacket->u.server_findanongame_iconreply.curricon, uicon, 4);
				}
				else
				{
					account_get_raceicon(acc, &rico, &rlvl, &rwins, clienttag);
					std::sprintf(user_icon, "%1d%c3W", rlvl, rico);
					std::memcpy(&rpacket->u.server_findanongame_iconreply.curricon, user_icon, 4);
				}

				bn_byte_set(&rpacket->u.server_findanongame_iconreply.table_width, table_width);
				bn_byte_set(&rpacket->u.server_findanongame_iconreply.table_size, table_width*table_height);
				for (j = 0; j < table_height; j++){
					icon_req_race_wins = anongame_infos_get_ICON_REQ(j + 1, clienttag);
					for (i = 0; i < table_width; i++){
						tempicon.race = i;
						tempicon.icon_code[0] = icon_pos[j];
						tempicon.icon_code[1] = race_char[i];
						tempicon.icon_code[2] = '3';
						tempicon.icon_code[3] = 'W';
						tempicon.portrait_code = (account_icon_to_profile_icon(tempicon.icon_code, acc, clienttag));
						if (i <= 4){
							//Building the icon for the races
							bn_short_set(&tempicon.required_wins, icon_req_race_wins);
							if (account_get_racewins(acc, race[i], clienttag) >= icon_req_race_wins) {
								if (prefs_get_custom_icons() == 1)
									tempicon.client_enabled = 0;
								else
									tempicon.client_enabled = 1;
							}
							else{
								tempicon.client_enabled = 0;
							}
						}
						else{
							//Building the icon for the tourney
							icon_req_tourney_wins = anongame_infos_get_ICON_REQ_TOURNEY(j + 1);
							bn_short_set(&tempicon.required_wins, icon_req_tourney_wins);
							if (account_get_racewins(acc, race[i], clienttag) >= icon_req_tourney_wins) {
								if (prefs_get_custom_icons() == 1)
									tempicon.client_enabled = 0;
								else
									tempicon.client_enabled = 1;
							}
							else{
								tempicon.client_enabled = 0;
							}
						}
						packet_append_data(rpacket, &tempicon, sizeof(tempicon));
					}
				}
				//Go,go,go
				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
			}
			return 0;
		}

		/* Choose icon by user from profile > portrait */
		static int _client_anongame_set_icon(t_connection * c, t_packet const * const packet)
		{
			//BlacKDicK 04/20/2003
			unsigned int desired_icon;
			char user_icon[5];
			t_account * account;
			t_clienttag ctag;

			// disable with custom icons
			if (prefs_get_custom_icons() == 1)
			{
				return 0;
			}

			/*FIXME: In this case we do not get a 'count' but insted of it we get the icon
			that the client wants to set.'W3H2' for an example. For now it is ok, since they share
			the same position	on the packet*/
			desired_icon = bn_int_get(packet->u.client_findanongame.count);
			user_icon[4] = 0;
			if (desired_icon == 0){
				std::strcpy(user_icon, "NULL");
				eventlog(eventlog_level_info, __FUNCTION__, "[%d] Set icon packet to DEFAULT ICON [%4.4s]", conn_get_socket(c), user_icon);
			}
			else{
				std::memcpy(user_icon, &desired_icon, 4);
				eventlog(eventlog_level_info, __FUNCTION__, "[%d] Set icon packet to ICON [%s]", conn_get_socket(c), user_icon);
			}

			account = conn_get_account(c);

			// ICON SWITCH HACK PROTECTION
			if (check_user_icon(account, user_icon) == 0)
			{
				eventlog(eventlog_level_info, __FUNCTION__, "[%s] \"%s\" ICON SWITCH hack attempt, icon set to default ", conn_get_username(c), user_icon);
				std::strcpy(user_icon, "NULL"); // set icon to default
				conn_set_state(c, conn_state_destroy); // kill user session
			}

			account_set_user_icon(conn_get_account(c), conn_get_clienttag(c), user_icon);
			//FIXME: Still need a way to 'refresh the user/channel'
			//_handle_rejoin_command(conn_get_account(c),"");
			/* ??? channel_update_userflags() */
			conn_update_w3_playerinfo(c);

			channel_rejoin(c);
			return 0;
		}

		/* Check user choice for illegal icon */
		static int check_user_icon(t_account * account, const char * user_icon)
		{
			unsigned int i, len;
			char temp_str[2];
			char user_race;
			int number;

			len = std::strlen(user_icon);
			if (len != 4)
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid user icon '%s'", user_icon);

			for (i = 0; i < len && i < 2; i++)
				temp_str[i] = user_icon[i];

			number = temp_str[0] - '0';
			user_race = temp_str[1];


			int race[] = { W3_RACE_RANDOM, W3_RACE_HUMANS, W3_RACE_ORCS, W3_RACE_UNDEAD, W3_RACE_NIGHTELVES, W3_RACE_DEMONS };
			char race_char[6] = { 'R', 'H', 'O', 'U', 'N', 'D' };
			int icon_pos[5] = { 2, 3, 4, 5, 6 };
			int icon_req_wins[5] = { 25, 150, 350, 750, 1500 };

			for (int i = 0; i < sizeof(race_char); i++)
			{
				if (user_race == race_char[i])
				{
					for (int j = 0; j < sizeof(icon_pos); j++)
					{
						if (number == icon_pos[j])
						{
							// compare account race wins and require wins
							if (account_get_racewins(account, race[i], account_get_ll_clienttag(account)) >= icon_req_wins[j])
								return 1;

							return 0;
						}
					}
				}
			}
			return 0;
		}






		static int _client_anongame_infos(t_connection * c, t_packet const * const packet)
		{
			t_packet * rpacket;

			if (bn_int_get(packet->u.client_findanongame_inforeq.count) > 1) {
				/* reply with 0 entries found */
				int	temp = 0;

				if ((rpacket = packet_create(packet_class_bnet)) == NULL) {
					eventlog(eventlog_level_error, __FUNCTION__, "could not create new packet");
					return -1;
				}

				packet_set_size(rpacket, sizeof(t_server_findanongame_inforeply));
				packet_set_type(rpacket, SERVER_FINDANONGAME_INFOREPLY);
				bn_byte_set(&rpacket->u.server_findanongame_inforeply.option, CLIENT_FINDANONGAME_INFOS);
				bn_int_set(&rpacket->u.server_findanongame_inforeply.count, bn_int_get(packet->u.client_findanongame_inforeq.count));
				bn_byte_set(&rpacket->u.server_findanongame_inforeply.noitems, 0);
				packet_append_data(rpacket, &temp, 1);

				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
			}
			else {
				int i;
				int client_tag;
				int server_tag_count = 0;
				int client_tag_unk;
				int server_tag_unk;
				bn_int temp;
				char noitems;
				char * tmpdata;
				int tmplen;
				t_clienttag clienttag = conn_get_clienttag(c);
				char last_packet = 0x00;
				char other_packet = 0x01;
				char langstr[5];
				t_gamelang gamelang = conn_get_gamelang(c);
				bn_int_tag_get((bn_int const *)&gamelang, langstr, 5);

				/* Send seperate packet for each item requested
				 * sending all at once overloaded w3xp
				 * [Omega] */
				for (i = 0; i < bn_byte_get(packet->u.client_findanongame_inforeq.noitems); i++){
					noitems = 0;

					if ((rpacket = packet_create(packet_class_bnet)) == NULL) {
						eventlog(eventlog_level_error, __FUNCTION__, "could not create new packet");
						return -1;
					}

					/* Starting the packet stuff */
					packet_set_size(rpacket, sizeof(t_server_findanongame_inforeply));
					packet_set_type(rpacket, SERVER_FINDANONGAME_INFOREPLY);
					bn_byte_set(&rpacket->u.server_findanongame_inforeply.option, CLIENT_FINDANONGAME_INFOS);
					bn_int_set(&rpacket->u.server_findanongame_inforeply.count, 1);

					std::memcpy(&temp, (packet_get_data_const(packet, 10 + (i * 8), 4)), sizeof(int));
					client_tag = bn_int_get(temp);
					std::memcpy(&temp, packet_get_data_const(packet, 14 + (i * 8), 4), sizeof(int));
					client_tag_unk = bn_int_get(temp);

					switch (client_tag){
					case CLIENT_FINDANONGAME_INFOTAG_URL:
						bn_int_set((bn_int*)&server_tag_unk, 0xBF1F1047);
						packet_append_data(rpacket, "LRU\0", 4);
						packet_append_data(rpacket, &server_tag_unk, 4);
						// FIXME: Maybe need do do some checks to avoid prefs empty strings.
						tmpdata = anongame_infos_data_get_url(clienttag, conn_get_versionid(c), &tmplen);
						packet_append_data(rpacket, tmpdata, tmplen);
						noitems++;
						server_tag_count++;
						eventlog(eventlog_level_debug, __FUNCTION__, "client_tag request tagid=(0x%01x) tag=(%s)  tag_unk=(0x%04x)", i, "CLIENT_FINDANONGAME_INFOTAG_URL", client_tag_unk);
						break;
					case CLIENT_FINDANONGAME_INFOTAG_MAP:
						bn_int_set((bn_int*)&server_tag_unk, 0x70E2E0D5);
						packet_append_data(rpacket, "PAM\0", 4);
						packet_append_data(rpacket, &server_tag_unk, 4);
						tmpdata = anongame_infos_data_get_map(clienttag, conn_get_versionid(c), &tmplen);
						packet_append_data(rpacket, tmpdata, tmplen);
						noitems++;
						server_tag_count++;
						eventlog(eventlog_level_debug, __FUNCTION__, "client_tag request tagid=(0x%01x) tag=(%s)  tag_unk=(0x%04x)", i, "CLIENT_FINDANONGAME_INFOTAG_MAP", client_tag_unk);
						break;
					case CLIENT_FINDANONGAME_INFOTAG_TYPE:
						bn_int_set((bn_int*)&server_tag_unk, 0x7C87DEEE);
						packet_append_data(rpacket, "EPYT", 4);
						packet_append_data(rpacket, &server_tag_unk, 4);
						tmpdata = anongame_infos_data_get_type(clienttag, conn_get_versionid(c), &tmplen);
						packet_append_data(rpacket, tmpdata, tmplen);
						noitems++;
						server_tag_count++;
						eventlog(eventlog_level_debug, __FUNCTION__, "client_tag request tagid=(0x%01x) tag=(%s) tag_unk=(0x%04x)", i, "CLIENT_FINDANONGAME_INFOTAG_TYPE", client_tag_unk);
						break;
					case CLIENT_FINDANONGAME_INFOTAG_DESC:
						bn_int_set((bn_int*)&server_tag_unk, 0xA4F0A22F);
						packet_append_data(rpacket, "CSED", 4);
						packet_append_data(rpacket, &server_tag_unk, 4);
						tmpdata = anongame_infos_data_get_desc(langstr, clienttag, conn_get_versionid(c), &tmplen);
						packet_append_data(rpacket, tmpdata, tmplen);
						eventlog(eventlog_level_debug, __FUNCTION__, "client_tag request tagid=(0x%01x) tag=(%s) tag_unk=(0x%04x)", i, "CLIENT_FINDANONGAME_INFOTAG_DESC", client_tag_unk);
						noitems++;
						server_tag_count++;
						break;
					case CLIENT_FINDANONGAME_INFOTAG_LADR:
						bn_int_set((bn_int*)&server_tag_unk, 0x3BADE25A);
						packet_append_data(rpacket, "RDAL", 4);
						packet_append_data(rpacket, &server_tag_unk, 4);
						tmpdata = anongame_infos_data_get_ladr(langstr, clienttag, conn_get_versionid(c), &tmplen);
						packet_append_data(rpacket, tmpdata, tmplen);
						noitems++;
						server_tag_count++;
						eventlog(eventlog_level_debug, __FUNCTION__, "client_tag request tagid=(0x%01x) tag=(%s) tag_unk=(0x%04x)", i, "CLIENT_FINDANONGAME_INFOTAG_LADR", client_tag_unk);
						break;
					default:
						eventlog(eventlog_level_debug, __FUNCTION__, "unrec client_tag request tagid=(0x%01x) tag=(0x%04x)", i, client_tag);

					}
					//Adding a last padding null-byte
					if (server_tag_count == bn_byte_get(packet->u.client_findanongame_inforeq.noitems))
						packet_append_data(rpacket, &last_packet, 1); /* only last packet in group gets 0x00 */
					else
						packet_append_data(rpacket, &other_packet, 1); /* the rest get 0x01 */

					//Go,go,go
					bn_byte_set(&rpacket->u.server_findanongame_inforeply.noitems, noitems);
					conn_push_outqueue(c, rpacket);
					packet_del_ref(rpacket);
				}
			}
			return 0;
		}

		/* tournament notice disabled at this time, but responce is sent to cleint */
		static int _client_anongame_tournament(t_connection * c, t_packet const * const packet)
		{
			t_packet * rpacket;

			t_account * account = conn_get_account(c);
			t_clienttag clienttag = conn_get_clienttag(c);

			unsigned int start_prelim = tournament_get_start_preliminary();
			unsigned int end_signup = tournament_get_end_signup();
			unsigned int end_prelim = tournament_get_end_preliminary();
			unsigned int start_r1 = tournament_get_start_round_1();

			if ((rpacket = packet_create(packet_class_bnet)) == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not create new packet");
				return -1;
			}

			packet_set_size(rpacket, sizeof(t_server_anongame_tournament_reply));
			packet_set_type(rpacket, SERVER_FINDANONGAME_TOURNAMENT_REPLY);
			bn_byte_set(&rpacket->u.server_anongame_tournament_reply.option, 7);
			bn_int_set(&rpacket->u.server_anongame_tournament_reply.count,
				bn_int_get(packet->u.client_anongame_tournament_request.count));

			if (!start_prelim || (end_signup <= now && tournament_user_signed_up(account) < 0) ||
				tournament_check_client(clienttag) < 0) { /* No Tournament Notice */
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.type, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown, 0);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown4, 0);
				bn_int_set(&rpacket->u.server_anongame_tournament_reply.timestamp, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown5, 0);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.countdown, 0);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown2, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.wins, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.losses, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.ties, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown3, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.selection, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.descnum, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.nulltag, 0);
			}
			else if (start_prelim >= now) { /* Tournament Notice */
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.type, 1);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown, 0);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown4, 0x0000); /* random */
				bn_int_set(&rpacket->u.server_anongame_tournament_reply.timestamp, _tournament_time_convert(start_prelim));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown5, 0x01);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.countdown, start_prelim - now);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown2, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.wins, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.losses, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.ties, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown3, 0x00);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.selection, 2);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.descnum, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.nulltag, 0);
			}
			else if (end_signup >= now) { /* Tournament Signup Notice - Play Game Active */
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.type, 2);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown, 0);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown4, 0x0828); /* random */
				bn_int_set(&rpacket->u.server_anongame_tournament_reply.timestamp, _tournament_time_convert(end_signup));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown5, 0x01);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.countdown, end_signup - now);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown2, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.wins, tournament_get_stat(account, 1));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.losses, tournament_get_stat(account, 2));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.ties, tournament_get_stat(account, 3));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown3, 0x08);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.selection, 2);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.descnum, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.nulltag, 0);
			}
			else if (end_prelim >= now) { /* Tournament Prelim Period - Play Game Active */
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.type, 3);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown, 0);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown4, 0x0828); /* random */
				bn_int_set(&rpacket->u.server_anongame_tournament_reply.timestamp, _tournament_time_convert(end_prelim));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown5, 0x01);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.countdown, end_prelim - now);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown2, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.wins, tournament_get_stat(account, 1));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.losses, tournament_get_stat(account, 2));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.ties, tournament_get_stat(account, 3));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown3, 0x08);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.selection, 2);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.descnum, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.nulltag, 0);
			}
			else if (start_r1 >= now && (tournament_get_game_in_progress())) { /* Prelim Period Over - Shows user stats (not all prelim games finished) */
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.type, 4);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown, 0);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown4, 0x0000); /* random */
				bn_int_set(&rpacket->u.server_anongame_tournament_reply.timestamp, _tournament_time_convert(start_r1));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown5, 0x01);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.countdown, start_r1 - now);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown2, 0); /* 00 00 */
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.wins, tournament_get_stat(account, 1));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.losses, tournament_get_stat(account, 2));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.ties, tournament_get_stat(account, 3));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown3, 0x08);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.selection, 2);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.descnum, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.nulltag, 0);
			}
			else if (!(tournament_get_in_finals_status(account))) { /* Prelim Period Over - user did not make finals - Shows user stats */
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.type, 5);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown, 0);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown4, 0);
				bn_int_set(&rpacket->u.server_anongame_tournament_reply.timestamp, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown5, 0);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.countdown, 0);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown2, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.wins, tournament_get_stat(account, 1));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.losses, tournament_get_stat(account, 2));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.ties, tournament_get_stat(account, 3));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown3, 0x04);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.selection, 2);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.descnum, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.nulltag, 0);
			}
			/* cycle through [type-6] & [type-7] packets
			 *
			 * use [type-6] to show client "eliminated" or "continue"
			 *     timestamp , countdown & round number (of next round) must be set if clinet continues
			 *
			 * use [type-7] to make cleint wait for 44FF packet option 1 to start game (A guess, not tested)
			 *
			 * not sure if there is overall winner packet sent at end of last final round
			 */

			else if ((0)) { /* User in finals - Shows user stats and start of next round*/
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.type, 6);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown, 0);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown4, 0x0000);
				bn_int_set(&rpacket->u.server_anongame_tournament_reply.timestamp, _tournament_time_convert(start_r1));
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown5, 0x01);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.countdown, start_r1 - now);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown2, 0x0000); /* 00 00 */
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.wins, 4); /* round number */
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.losses, 0); /* 0 = continue , 1= eliminated */
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.ties, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown3, 0x04); /* number of rounds in finals */
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.selection, 2);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.descnum, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.nulltag, 0);
			}
			else if ((0)) { /* user waiting for match to be made */
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.type, 7);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown, 0);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown4, 0);
				bn_int_set(&rpacket->u.server_anongame_tournament_reply.timestamp, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown5, 0);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.countdown, 0);
				bn_short_set(&rpacket->u.server_anongame_tournament_reply.unknown2, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.wins, 1); /* round number */
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.losses, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.ties, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.unknown3, 0x04); /* number of finals */
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.selection, 2);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.descnum, 0);
				bn_byte_set(&rpacket->u.server_anongame_tournament_reply.nulltag, 0);
			}

			conn_push_outqueue(c, rpacket);
			packet_del_ref(rpacket);
			return 0;
		}

		static unsigned int _tournament_time_convert(unsigned int time)
		{
			/* it works, don't ask me how */ /* some time drift reportd by testers */
			unsigned int tmp1, tmp2, tmp3;

			tmp1 = time - 1059179400;	/* 0x3F21CB88  */
			tmp2 = static_cast<unsigned int>(tmp1*0.59604645);
			tmp3 = tmp2 + 3276999960U;
			/*eventlog(eventlog_level_trace,__FUNCTION__,"time: 0x%08x, tmp1: 0x%08x, tmp2 0x%08x, tmp3 0x%08x",time,tmp1,tmp2,tmp3);*/

			return tmp3;
		}

		extern int handle_anongame_packet(t_connection * c, t_packet const * const packet)
		{
			switch (bn_byte_get(packet->u.client_anongame.option))
			{
			case CLIENT_FINDANONGAME_PROFILE:
				return _client_anongame_profile(c, packet);

			case CLIENT_FINDANONGAME_CANCEL:
				return _client_anongame_cancel(c);

			case CLIENT_FINDANONGAME_SEARCH:
			case CLIENT_FINDANONGAME_AT_INVITER_SEARCH:
			case CLIENT_FINDANONGAME_AT_SEARCH:
				return handle_anongame_search(c, packet); /* located in anongame.c */

			case CLIENT_FINDANONGAME_GET_ICON:
				return _client_anongame_get_icon(c, packet);

			case CLIENT_FINDANONGAME_SET_ICON:
				return _client_anongame_set_icon(c, packet);

			case CLIENT_FINDANONGAME_INFOS:
				return _client_anongame_infos(c, packet);

			case CLIENT_ANONGAME_TOURNAMENT:
				return _client_anongame_tournament(c, packet);

			case CLIENT_FINDANONGAME_PROFILE_CLAN:
				return _client_anongame_profile_clan(c, packet);

			default:
				eventlog(eventlog_level_error, __FUNCTION__, "got unhandled option %d", bn_byte_get(packet->u.client_findanongame.option));
				return -1;
			}
		}

	}

}
