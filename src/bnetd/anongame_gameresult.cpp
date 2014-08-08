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
#include "common/setup_before.h"
#define ANONGAME_GAMERESULT_INTERNAL_ACCESS
#include "anongame_gameresult.h"

#include <cassert>

#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/packet.h"
#include "common/bn_type.h"


#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		extern int gameresult_destroy(t_anongame_gameresult * gameresult)
		{
			if (!(gameresult))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL gameresult");
				return -1;
			}

			if ((gameresult->players)) xfree((void *)gameresult->players);
			if ((gameresult->heroes)) xfree((void *)gameresult->heroes);
			xfree((void *)gameresult);

			return 0;
		}

		extern t_anongame_gameresult * anongame_gameresult_parse(t_packet const * const packet)
		{
			t_anongame_gameresult 			* gameresult;
			t_client_w3route_gameresult_player 		* player;
			t_client_w3route_gameresult_part2 		* part2;
			t_client_w3route_gameresult_hero		* hero;
			t_client_w3route_gameresult_part3		* part3;

			int counter, heroes_count;
			unsigned expectedsize;  //still without hero infos
			unsigned int offset = 0;

			int result_count = bn_byte_get(packet->u.client_w3route_gameresult.number_of_results);
			expectedsize = sizeof(t_client_w3route_gameresult)+
				sizeof(t_client_w3route_gameresult_player)* result_count +
				sizeof(t_client_w3route_gameresult_part2)+
				sizeof(t_client_w3route_gameresult_part3);

			if (packet_get_size(packet) < expectedsize)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "gameresult packet is smaller than expected");
				return NULL;
			}

			gameresult = (t_anongame_gameresult*)xmalloc(sizeof(t_anongame_gameresult));

			gameresult->players = (t_anongame_player*)xmalloc(sizeof(t_anongame_player)*result_count);

			gameresult->number_of_results = result_count;

			offset += sizeof(t_client_w3route_gameresult);

			for (counter = 0; counter < result_count; counter++)
			{
				player = (t_client_w3route_gameresult_player *)packet_get_raw_data_const(packet, offset);

				gameresult->players[counter].number = bn_byte_get(player->number);
				gameresult->players[counter].result = bn_int_get(player->result);
				gameresult->players[counter].race = bn_int_get(player->race);

				offset += sizeof(t_client_w3route_gameresult_player);
			}

			part2 = (t_client_w3route_gameresult_part2 *)packet_get_raw_data_const(packet, offset);

			gameresult->unit_score = bn_int_get(part2->unit_score);
			gameresult->heroes_score = bn_int_get(part2->heroes_score);
			gameresult->resource_score = bn_int_get(part2->resource_score);
			gameresult->units_produced = bn_int_get(part2->units_produced);
			gameresult->units_killed = bn_int_get(part2->units_killed);
			gameresult->buildings_produced = bn_int_get(part2->buildings_produced);
			gameresult->buildings_razed = bn_int_get(part2->buildings_razed);
			gameresult->largest_army = bn_int_get(part2->largest_army);
			heroes_count = bn_int_get(part2->heroes_used_count);
			gameresult->heroes_used_count = heroes_count;

			offset += sizeof(t_client_w3route_gameresult_part2);

			if ((heroes_count))
			{
				gameresult->heroes = (t_anongame_hero*)xmalloc(sizeof(t_anongame_hero)*heroes_count);

				if (packet_get_size(packet) < expectedsize + sizeof(t_client_w3route_gameresult_hero)*heroes_count)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "gameresult packet is smaller than expected");
					return NULL;
				}

				for (counter = 0; counter < heroes_count; counter++)
				{
					hero = (t_client_w3route_gameresult_hero *)packet_get_raw_data_const(packet, offset);

					gameresult->heroes[counter].level = bn_int_get(hero->level);
					gameresult->heroes[counter].race_and_name = bn_int_get(hero->race_and_name);
					gameresult->heroes[counter].hero_xp = bn_int_get(hero->hero_xp);

					offset += sizeof(t_client_w3route_gameresult_hero);
				}
			}
			else
				gameresult->heroes = NULL;

			part3 = (t_client_w3route_gameresult_part3 *)packet_get_raw_data_const(packet, offset);

			gameresult->heroes_killed = bn_int_get(part3->heroes_killed);
			gameresult->items_obtained = bn_int_get(part3->items_obtained);
			gameresult->mercenaries_hired = bn_int_get(part3->mercenaries_hired);
			gameresult->total_hero_xp = bn_int_get(part3->total_hero_xp);
			gameresult->gold_mined = bn_int_get(part3->gold_mined);
			gameresult->lumber_harvested = bn_int_get(part3->lumber_harvested);
			gameresult->resources_traded_given = bn_int_get(part3->resources_traded_given);
			gameresult->resources_traded_taken = bn_int_get(part3->resources_traded_taken);
			gameresult->tech_percentage = bn_int_get(part3->tech_percentage);
			gameresult->gold_lost_to_upkeep = bn_int_get(part3->gold_lost_to_upkeep);

			return gameresult;
		}

		extern char gameresult_get_number_of_results(t_anongame_gameresult * gameresult)
		{
			assert(gameresult);

			return gameresult->number_of_results;
		}

		extern int gameresult_get_player_result(t_anongame_gameresult * gameresult, int player)
		{
			if (!(gameresult))
				return -1;

			if (player >= gameresult->number_of_results)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "request for invalid player number");
				return -1;
			}

			return gameresult->players[player].result;
		}

		extern int gameresult_get_player_number(t_anongame_gameresult * gameresult, int player)
		{
			if (!(gameresult))
				return -1;

			if (player >= gameresult->number_of_results)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "request for invalid player number");
				return -1;
			}

			return gameresult->players[player].number;
		}

	}

}
