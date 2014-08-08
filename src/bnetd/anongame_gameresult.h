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
#ifndef INCLUDED_ANONGAME_GAMERESULT_TYPES
#define INCLUDED_ANONGAME_GAMERESULT_TYPES

namespace pvpgn
{

	namespace bnetd
	{

#ifdef ANONGAME_GAMERESULT_INTERNAL_ACCESS
		typedef struct
		{
			char			number;
			int			result;
			int			race;
		} t_anongame_player;

		typedef struct
		{
			int			level;
			int			race_and_name;
			int			hero_xp;
		} t_anongame_hero;
#endif

		typedef struct anongame_gameresult_struct
#ifdef ANONGAME_GAMERESULT_INTERNAL_ACCESS
		{
			char			number_of_results;
			t_anongame_player	*players;
			int			unit_score;
			int			heroes_score;
			int			resource_score;
			int			units_produced;
			int			units_killed;
			int			buildings_produced;
			int 			buildings_razed;
			int			largest_army;
			int			heroes_used_count;
			t_anongame_hero		*heroes;
			int			heroes_killed;
			int			items_obtained;
			int			mercenaries_hired;
			int			total_hero_xp;
			int			gold_mined;
			int			lumber_harvested;
			int			resources_traded_given;
			int			resources_traded_taken;
			int			tech_percentage;
			int			gold_lost_to_upkeep;
		}
#endif
		t_anongame_gameresult;

	}

}

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_ANONGAME_GAMERESULT_PROTOS
#define INCLUDED_ANONGAME_GAMERESULT_PROTOS

#define JUST_NEED_TYPES
# include "common/packet.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern t_anongame_gameresult * anongame_gameresult_parse(t_packet const * const packet);
		extern int gameresult_destroy(t_anongame_gameresult * gameresult);

		extern char gameresult_get_number_of_results(t_anongame_gameresult * gameresult);
		extern int gameresult_get_player_result(t_anongame_gameresult * gameresult, int player);
		extern int gameresult_get_player_number(t_anongame_gameresult * gameresult, int player);

	}

}

#endif
#endif
