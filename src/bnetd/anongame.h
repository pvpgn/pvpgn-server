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
#ifndef INCLUDED_ANONGAME_TYPES
#define INCLUDED_ANONGAME_TYPES

#include <cstdint>

#ifdef JUST_NEED_TYPES
# include "account.h"
# include "connection.h"
#else
# define JUST_NEED_TYPES
# include "account.h"
# include "connection.h"
# undef JUST_NEED_TYPES
#endif
# include "anongame_gameresult.h"

namespace pvpgn
{

	namespace bnetd
	{

		typedef struct
		{
			int				currentplayers;
			int				totalplayers;
			struct connection *		player[ANONGAME_MAX_GAMECOUNT];
			t_account *			account[ANONGAME_MAX_GAMECOUNT];
			int				result[ANONGAME_MAX_GAMECOUNT];
			t_anongame_gameresult *	results[ANONGAME_MAX_GAMECOUNT];
		} t_anongameinfo;

		typedef struct
		{
			t_anongameinfo *		info;
			int				count;
			std::uint32_t			id;
			std::uint32_t			tid;
			struct connection *		tc[ANONGAME_MAX_GAMECOUNT / 2];
			std::uint32_t			race;
			std::uint32_t			handle;
			unsigned int		addr;
			char			loaded;
			char			joined;
			std::uint8_t			playernum;
			std::uint32_t			map_prefs;
			std::uint8_t			type;
			std::uint8_t			gametype;
			int				queue;
		} t_anongame;

		typedef struct
		{
			struct connection *	c;
			std::uint32_t		map_prefs;
		} t_matchdata;

	}

}

#endif

/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_ANONGAME_PROTOS
#define INCLUDED_ANONGAME_PROTOS

#define JUST_NEED_TYPES
#include "common/packet.h"
#include "connection.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern int		anongame_matchlists_create(void);
		extern int		anongame_matchlists_destroy(void);

		extern int		handle_anongame_search(t_connection * c, t_packet const * packet);
		extern int		anongame_unqueue(t_connection * c, int queue);

		extern char		anongame_arranged(int queue);
		extern int		anongame_stats(t_connection * c);

		extern t_anongameinfo * anongameinfo_create(int totalplayers);
		extern void		anongameinfo_destroy(t_anongameinfo * i);

		extern t_anongameinfo * anongame_get_info(t_anongame * a);
		extern int              anongame_get_currentplayers(t_anongame *a);
		extern int              anongame_get_totalplayers(t_anongame *a);
		extern t_connection *   anongame_get_player(t_anongame * a, int plnum);
		extern int		anongame_get_count(t_anongame *a);
		extern std::uint32_t         anongame_get_id(t_anongame * a);
		extern t_connection *	anongame_get_tc(t_anongame *a, int tpnumber);
		extern std::uint32_t         anongame_get_race(t_anongame * a);
		extern std::uint32_t         anongame_get_handle(t_anongame * a);
		extern unsigned int     anongame_get_addr(t_anongame * a);
		extern char             anongame_get_loaded(t_anongame * a);
		extern char             anongame_get_joined(t_anongame * a);
		extern std::uint8_t          anongame_get_playernum(t_anongame * a);
		extern std::uint8_t          anongame_get_queue(t_anongame *a);

		extern void		anongame_set_result(t_anongame * a, int result);
		extern void		anongame_set_gameresults(t_anongame * a, t_anongame_gameresult * results);
		extern void		anongame_set_handle(t_anongame *a, std::uint32_t h);
		extern void		anongame_set_addr(t_anongame *a, unsigned int addr);
		extern void		anongame_set_loaded(t_anongame * a, char loaded);
		extern void		anongame_set_joined(t_anongame * a, char joined);

		extern int		handle_w3route_packet(t_connection * c, t_packet const * const packet);
		extern int		handle_anongame_join(t_connection * c);

		/* Currently used by WOL RAL2 and YURI clients*/
		extern const char * anongame_get_map_from_prefs(int queue, t_clienttag clienttag);
	}

}

#endif
#endif
