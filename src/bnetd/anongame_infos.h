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
#ifndef INCLUDED_ANONGAME_INFOS_TYPES
#define INCLUDED_ANONGAME_INFOS_TYPES

#ifdef JUST_NEED_TYPES
#include "common/list.h"
#else
#define JUST_NEED_TYPES
#include "common/list.h"
#undef JUST_NEED_TYPES
#endif

#define anongame_infos_URL_count 14

namespace pvpgn
{

	namespace bnetd
	{

		typedef enum
		{
			URL_server,
			URL_player,
			URL_tourney,
			URL_clan,

			URL_ladder_PG_1v1,
			URL_ladder_PG_ffa,
			URL_ladder_PG_team,

			URL_ladder_AT_2v2,
			URL_ladder_AT_3v3,
			URL_ladder_AT_4v4,

			URL_ladder_clan_1v1,
			URL_ladder_clan_2v2,
			URL_ladder_clan_3v3,
			URL_ladder_clan_4v4
		} t_anongame_infos_URLs;

#define anongame_infos_DESC_count 38

		typedef enum {
			ladder_PG_1v1_desc,
			ladder_PG_ffa_desc,
			ladder_PG_team_desc,

			ladder_AT_2v2_desc,
			ladder_AT_3v3_desc,
			ladder_AT_4v4_desc,

			ladder_clan_1v1_desc,
			ladder_clan_2v2_desc,
			ladder_clan_3v3_desc,
			ladder_clan_4v4_desc,

			gametype_1v1_short,
			gametype_1v1_long,
			gametype_2v2_short,
			gametype_2v2_long,
			gametype_3v3_short,
			gametype_3v3_long,
			gametype_4v4_short,
			gametype_4v4_long,
			gametype_ffa_short,
			gametype_ffa_long,
			gametype_2v2v2_short,
			gametype_2v2v2_long,

			gametype_sffa_short,
			gametype_sffa_long,
			gametype_tffa_short,
			gametype_tffa_long,
			gametype_3v3v3_short,
			gametype_3v3v3_long,
			gametype_4v4v4_short,
			gametype_4v4v4_long,
			gametype_2v2v2v2_short,
			gametype_2v2v2v2_long,
			gametype_3v3v3v3_short,
			gametype_3v3v3v3_long,
			gametype_5v5_short,
			gametype_5v5_long,
			gametype_6v6_short,
			gametype_6v6_long
		} t_anongame_infos_DESCs;

		typedef struct
		{
			char *  langID;
			char ** descs;
		} t_anongame_infos_DESC;

#define anongame_infos_THUMBSDOWN_count 17

		typedef enum {
			PG_1v1,
			PG_2v2,
			PG_3v3,
			PG_4v4,
			PG_ffa,
			AT_2v2,
			AT_3v3,
			AT_4v4,
			PG_2v2v2,
			AT_ffa,
			PG_5v5,
			PG_6v6,
			PG_3v3v3,
			PG_4v4v4,
			PG_2v2v2v2,
			PG_3v3v3v3,
			AT_2v2v2
		} t_anongame_infos_THUMBDOWNs;

#define anongame_infos_ICON_REQ_count 14

		typedef enum {
			ICON_REQ_WAR3_Level1,
			ICON_REQ_WAR3_Level2,
			ICON_REQ_WAR3_Level3,
			ICON_REQ_WAR3_Level4,
			ICON_REQ_W3XP_Level1,
			ICON_REQ_W3XP_Level2,
			ICON_REQ_W3XP_Level3,
			ICON_REQ_W3XP_Level4,
			ICON_REQ_W3XP_Level5,
			ICON_REQ_TRNY_Level1,
			ICON_REQ_TRNY_Level2,
			ICON_REQ_TRNY_Level3,
			ICON_REQ_TRNY_Level4,
			ICON_REQ_TRNY_Level5
		} t_anongame_infos_ICON_REQs;

		typedef struct {
			char * langID;

			char * desc_data;
			char * ladr_data;

			char * desc_comp_data;
			char * ladr_comp_data;

			int desc_len;
			int ladr_len;

			int desc_comp_len;
			int ladr_comp_len;
		} t_anongame_infos_data_lang;

		typedef struct {
			char * langID;

			char * url_comp_data;
			char * url_comp_data_115;
			char * map_comp_data;
			char * type_comp_data;
			char * desc_comp_data;
			char * ladr_comp_data;

			int url_comp_len;
			int url_comp_len_115;
			int map_comp_len;
			int type_comp_len;
			int desc_comp_len;
			int ladr_comp_len;
		} t_anongame_infos_data;

		typedef struct {
			char			** anongame_infos_URL;
			t_anongame_infos_DESC	* anongame_infos_DESC;		/* for default DESC */
			t_list			* anongame_infos_DESC_list;	/* for localized DESC's */
			char			anongame_infos_THUMBSDOWN[anongame_infos_THUMBSDOWN_count];
			int			anongame_infos_ICON_REQ[anongame_infos_ICON_REQ_count];
			t_anongame_infos_data * anongame_infos_data_war3;
			t_anongame_infos_data * anongame_infos_data_w3xp;
			t_list * anongame_infos_data_lang_war3;
			t_list * anongame_infos_data_lang_w3xp;
		} t_anongame_infos;

	}

}

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_ANONGAME_INFOS_PROTOS
#define INCLUDED_ANONGAME_INFOS_PROTOS

#define JUST_NEED_TYPES
#include "common/tag.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern int anongame_infos_load(char const * filename);
		extern int anongame_infos_unload(void);

		extern char anongame_infos_get_thumbsdown(int queue);

		extern short anongame_infos_get_ICON_REQ(int Level, t_clienttag clienttag);
		extern short anongame_infos_get_ICON_REQ_TOURNEY(int Level);

		extern char * anongame_infos_data_get_url(t_clienttag clienttag, int versionid, int * len);
		extern char * anongame_infos_data_get_map(t_clienttag clienttag, int versionid, int * len);
		extern char * anongame_infos_data_get_type(t_clienttag clienttag, int versionid, int * len);
		extern char * anongame_infos_data_get_desc(char const * langID, t_clienttag clienttag, int versionid, int * len);
		extern char * anongame_infos_data_get_ladr(char const * langID, t_clienttag clienttag, int versionid, int * len);

	}

}

#endif
#endif
