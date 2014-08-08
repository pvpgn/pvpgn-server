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
#define TIMER_INTERNAL_ACCESS
#include "common/setup_before.h"
#include "anongame_infos.h"

#include <cstring>
#include <cerrno>
#include <cstdlib>

#include "common/list.h"
#include "common/eventlog.h"
#include "common/util.h"
#include "common/packet.h"
#include "common/tag.h"
#include "common/bn_type.h"
#include "common/xalloc.h"
#include "zlib.h"
#include "tournament.h"
#include "anongame_maplists.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static std::FILE *fp = NULL;

		static t_anongame_infos *anongame_infos;

		static int zlib_compress(void const *src, int srclen, char **dest, int *destlen);
		static int anongame_infos_data_load(void);

		static int anongame_infos_URL_init(t_anongame_infos * anongame_infos)
		{
			char **anongame_infos_URL;
			int i;

			if (!(anongame_infos))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos");
				return -1;
			}

			anongame_infos_URL = (char**)xmalloc(sizeof(char*)*anongame_infos_URL_count);

			for (i = 0; i < anongame_infos_URL_count; i++)
				anongame_infos_URL[i] = NULL;

			anongame_infos->anongame_infos_URL = anongame_infos_URL;

			return 0;
		}

		static int anongame_infos_URL_destroy(char ** anongame_infos_URL)
		{
			int i;

			if (!(anongame_infos_URL))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos_URL");
				return -1;
			}

			for (i = 0; i < anongame_infos_URL_count; i++)
			{
				if (anongame_infos_URL[i])
				{
					xfree((void *)anongame_infos_URL[i]);
				}
			}

			xfree((void *)anongame_infos_URL);

			return 0;
		}

		static t_anongame_infos_DESC *anongame_infos_DESC_init(void)
		{
			int i;
			char ** descs;
			t_anongame_infos_DESC *anongame_infos_DESC;

			anongame_infos_DESC = (t_anongame_infos_DESC*)xmalloc(sizeof(t_anongame_infos_DESC));

			anongame_infos_DESC->langID = NULL;
			descs = (char**)xmalloc(sizeof(char *)*anongame_infos_DESC_count);

			for (i = 0; i < anongame_infos_DESC_count; i++)
				descs[i] = NULL;

			anongame_infos_DESC->descs = descs;

			return anongame_infos_DESC;
		}

		static int anongame_infos_DESC_destroy(t_anongame_infos_DESC * anongame_infos_DESC)
		{
			int i;
			char ** descs;

			if (!(anongame_infos_DESC))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos_DESC");
				return -1;
			}

			if (anongame_infos_DESC->langID)
				xfree((void *)anongame_infos_DESC->langID);
			if ((descs = anongame_infos_DESC->descs))
			{
				for (i = 0; i < anongame_infos_DESC_count; i++)
				{
					if ((descs[i]))
						free((void *)descs[i]);
				}
				xfree((void *)descs);
			}

			xfree((void *)anongame_infos_DESC);

			return 0;
		}

		static int anongame_infos_THUMBSDOWN_init(t_anongame_infos * anongame_infos)
		{
			char * anongame_infos_THUMBSDOWN;
			int i;

			if (!(anongame_infos))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos");
				return -1;
			}

			anongame_infos_THUMBSDOWN = anongame_infos->anongame_infos_THUMBSDOWN;

			for (i = 0; i < anongame_infos_THUMBSDOWN_count; i++)
				anongame_infos_THUMBSDOWN[i] = 0;

			return 0;
		}

		static int anongame_infos_ICON_REQ_init(t_anongame_infos * anongame_infos)
		{
			int *anongame_infos_ICON_REQ;

			if (!(anongame_infos))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos");
				return -1;
			}

			anongame_infos_ICON_REQ = anongame_infos->anongame_infos_ICON_REQ;

			anongame_infos_ICON_REQ[ICON_REQ_WAR3_Level1] = 25;
			anongame_infos_ICON_REQ[ICON_REQ_WAR3_Level2] = 250;
			anongame_infos_ICON_REQ[ICON_REQ_WAR3_Level3] = 500;
			anongame_infos_ICON_REQ[ICON_REQ_WAR3_Level4] = 1500;

			anongame_infos_ICON_REQ[ICON_REQ_W3XP_Level1] = 25;
			anongame_infos_ICON_REQ[ICON_REQ_W3XP_Level2] = 150;
			anongame_infos_ICON_REQ[ICON_REQ_W3XP_Level3] = 350;
			anongame_infos_ICON_REQ[ICON_REQ_W3XP_Level4] = 750;
			anongame_infos_ICON_REQ[ICON_REQ_W3XP_Level5] = 1500;

			anongame_infos_ICON_REQ[ICON_REQ_TRNY_Level1] = 10;
			anongame_infos_ICON_REQ[ICON_REQ_TRNY_Level2] = 75;
			anongame_infos_ICON_REQ[ICON_REQ_TRNY_Level3] = 150;
			anongame_infos_ICON_REQ[ICON_REQ_TRNY_Level4] = 250;
			anongame_infos_ICON_REQ[ICON_REQ_TRNY_Level5] = 500;

			return 0;
		}

		static t_anongame_infos_data_lang *anongame_infos_data_lang_init(char *langID)
		{
			t_anongame_infos_data_lang *anongame_infos_data_lang;

			anongame_infos_data_lang = (t_anongame_infos_data_lang*)xmalloc(sizeof(t_anongame_infos_data_lang));

			anongame_infos_data_lang->langID = xstrdup(langID);

			anongame_infos_data_lang->desc_data = NULL;
			anongame_infos_data_lang->ladr_data = NULL;

			anongame_infos_data_lang->desc_comp_data = NULL;
			anongame_infos_data_lang->ladr_comp_data = NULL;

			anongame_infos_data_lang->desc_len = 0;
			anongame_infos_data_lang->ladr_len = 0;

			anongame_infos_data_lang->desc_comp_len = 0;
			anongame_infos_data_lang->ladr_comp_len = 0;

			return anongame_infos_data_lang;
		}

		static int anongame_infos_data_lang_destroy(t_anongame_infos_data_lang * anongame_infos_data_lang)
		{
			if (!(anongame_infos_data_lang))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos_data_lang");
				return -1;
			}

			if (anongame_infos_data_lang->langID)
				xfree((void *)anongame_infos_data_lang->langID);

			if (anongame_infos_data_lang->desc_data)
				xfree((void *)anongame_infos_data_lang->desc_data);
			if (anongame_infos_data_lang->ladr_data)
				xfree((void *)anongame_infos_data_lang->ladr_data);

			if (anongame_infos_data_lang->desc_comp_data)
				xfree((void *)anongame_infos_data_lang->desc_comp_data);
			if (anongame_infos_data_lang->ladr_comp_data)
				xfree((void *)anongame_infos_data_lang->ladr_comp_data);

			xfree((void *)anongame_infos_data_lang);

			return 0;
		}

		static int anongame_infos_data_init(t_anongame_infos * anongame_infos)
		{
			t_anongame_infos_data *anongame_infos_data;
			t_list *anongame_infos_data_lang;

			if (!(anongame_infos))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos");
				return -1;
			}

			anongame_infos_data = (t_anongame_infos_data*)xmalloc(sizeof(t_anongame_infos_data));

			anongame_infos_data_lang = list_create();

			anongame_infos_data->url_comp_data = NULL;
			anongame_infos_data->url_comp_data_115 = NULL;
			anongame_infos_data->map_comp_data = NULL;
			anongame_infos_data->type_comp_data = NULL;
			anongame_infos_data->desc_comp_data = NULL;
			anongame_infos_data->ladr_comp_data = NULL;

			anongame_infos_data->url_comp_len = 0;
			anongame_infos_data->url_comp_len_115 = 0;
			anongame_infos_data->map_comp_len = 0;
			anongame_infos_data->type_comp_len = 0;
			anongame_infos_data->desc_comp_len = 0;
			anongame_infos_data->ladr_comp_len = 0;

			anongame_infos->anongame_infos_data_war3 = anongame_infos_data;
			anongame_infos->anongame_infos_data_lang_war3 = anongame_infos_data_lang;

			anongame_infos_data = (t_anongame_infos_data*)xmalloc(sizeof(t_anongame_infos_data));

			anongame_infos_data_lang = list_create();

			anongame_infos_data->url_comp_data = NULL;
			anongame_infos_data->url_comp_data_115 = NULL;
			anongame_infos_data->map_comp_data = NULL;
			anongame_infos_data->type_comp_data = NULL;
			anongame_infos_data->desc_comp_data = NULL;
			anongame_infos_data->ladr_comp_data = NULL;

			anongame_infos_data->url_comp_len = 0;
			anongame_infos_data->url_comp_len_115 = 0;
			anongame_infos_data->map_comp_len = 0;
			anongame_infos_data->type_comp_len = 0;
			anongame_infos_data->desc_comp_len = 0;
			anongame_infos_data->ladr_comp_len = 0;

			anongame_infos->anongame_infos_data_w3xp = anongame_infos_data;
			anongame_infos->anongame_infos_data_lang_w3xp = anongame_infos_data_lang;

			return 0;
		}

		static int anongame_infos_data_destroy(t_anongame_infos_data * anongame_infos_data, t_list * anongame_infos_data_lang)
		{
			t_elem *curr;
			t_anongame_infos_data_lang *entry;

			if (anongame_infos_data->url_comp_data)
				xfree((void *)anongame_infos_data->url_comp_data);
			if (anongame_infos_data->url_comp_data_115)
				xfree((void *)anongame_infos_data->url_comp_data_115);
			if (anongame_infos_data->map_comp_data)
				xfree((void *)anongame_infos_data->map_comp_data);
			if (anongame_infos_data->type_comp_data)
				xfree((void *)anongame_infos_data->type_comp_data);
			if (anongame_infos_data->desc_comp_data)
				xfree((void *)anongame_infos_data->desc_comp_data);
			if (anongame_infos_data->ladr_comp_data)
				xfree((void *)anongame_infos_data->ladr_comp_data);

			xfree((void *)anongame_infos_data);

			if (anongame_infos_data_lang)
			{
				LIST_TRAVERSE(anongame_infos_data_lang, curr)
				{
					if (!(entry = (t_anongame_infos_data_lang*)elem_get_data(curr)))
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
					else
					{
						anongame_infos_data_lang_destroy(entry);
					}
					list_remove_elem(anongame_infos_data_lang, &curr);
				}
				list_destroy(anongame_infos_data_lang);
			}
			return 0;
		}

		t_anongame_infos *anongame_infos_init(void)
		{
			t_anongame_infos *anongame_infos;

			anongame_infos = (t_anongame_infos*)xmalloc(sizeof(t_anongame_infos));

			if (anongame_infos_URL_init(anongame_infos) != 0)
			{
				xfree((void *)anongame_infos);
				return NULL;
			}

			if (anongame_infos_THUMBSDOWN_init(anongame_infos) != 0)
			{
				anongame_infos_URL_destroy(anongame_infos->anongame_infos_URL);
				xfree((void *)anongame_infos);
				return NULL;
			}

			if (anongame_infos_ICON_REQ_init(anongame_infos) != 0)
			{
				anongame_infos_URL_destroy(anongame_infos->anongame_infos_URL);
				xfree((void *)anongame_infos);
				return NULL;
			}

			if (anongame_infos_data_init(anongame_infos) != 0)
			{
				anongame_infos_URL_destroy(anongame_infos->anongame_infos_URL);
				xfree((void *)anongame_infos);
				return NULL;
			}

			anongame_infos->anongame_infos_DESC = NULL;

			anongame_infos->anongame_infos_DESC_list = list_create();

			return anongame_infos;
		}

		static int anongame_infos_destroy(t_anongame_infos * anongame_infos)
		{
			t_elem *curr;
			t_anongame_infos_DESC *entry;

			if (!(anongame_infos))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos");
				return -1;
			}

			if (anongame_infos->anongame_infos_DESC_list)
			{
				LIST_TRAVERSE(anongame_infos->anongame_infos_DESC_list, curr)
				{
					if (!(entry = (t_anongame_infos_DESC*)elem_get_data(curr)))
						eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
					else
					{
						anongame_infos_DESC_destroy(entry);
					}
					list_remove_elem(anongame_infos->anongame_infos_DESC_list, &curr);
				}
				list_destroy(anongame_infos->anongame_infos_DESC_list);
				anongame_infos->anongame_infos_DESC_list = NULL;
			}

			anongame_infos_DESC_destroy(anongame_infos->anongame_infos_DESC);
			anongame_infos_URL_destroy(anongame_infos->anongame_infos_URL);
			anongame_infos_data_destroy(anongame_infos->anongame_infos_data_war3, anongame_infos->anongame_infos_data_lang_war3);
			anongame_infos_data_destroy(anongame_infos->anongame_infos_data_w3xp, anongame_infos->anongame_infos_data_lang_w3xp);

			xfree((void *)anongame_infos);

			return 0;
		}

		static int anongame_infos_set_str(char **dst, const char *src, const char *errstr)
		{
			char *temp;

			if (!(src))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL %s", errstr);
				return -1;
			}

			temp = xstrdup(src);
			if (*dst)
				xfree((void *)*dst);
			*dst = temp;

			return 0;
		}

		static int anongame_infos_URL_set_URL(int member, const char *URL)
		{
			char **anongame_infos_URLs;

			if (!(anongame_infos_URLs = anongame_infos->anongame_infos_URL))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "detected NULL anongame_infos_URL");
				return -1;
			}

			return anongame_infos_set_str(&anongame_infos_URLs[member], URL, "URL");
		}

		extern char *anongame_infos_URL_get_URL(int member)
		{
			char **anongame_infos_URLs;

			if (!(anongame_infos_URLs = anongame_infos->anongame_infos_URL))
				return NULL;
			else return anongame_infos_URLs[member];
		}


		static int anongame_infos_DESC_set_DESC(t_anongame_infos_DESC * anongame_infos_DESC, int member, const char *DESC)
		{
			char ** descs;

			if (!(anongame_infos_DESC))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos_DESC");
				return -1;
			}

			if (!(descs = anongame_infos_DESC->descs))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "anongame_infos_DESC had NULL descs");
				return -1;
			}

			return anongame_infos_set_str(&descs[member], DESC, "DESC");
		}

		static t_anongame_infos_DESC *anongame_infos_get_anongame_infos_DESC_by_langID(t_anongame_infos * anongame_infos, char *langID)
		{
			t_elem *curr;
			t_anongame_infos_DESC *entry;

			if (!(anongame_infos))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos");
				return NULL;
			}

			if (!(langID))
				return anongame_infos->anongame_infos_DESC;

			if (!(anongame_infos->anongame_infos_DESC_list))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos_DESC_list - default values");
				return anongame_infos->anongame_infos_DESC;
			}

			LIST_TRAVERSE(anongame_infos->anongame_infos_DESC_list, curr)
			{
				if (!(entry = (t_anongame_infos_DESC *)elem_get_data(curr)))
					eventlog(eventlog_level_error, __FUNCTION__, "found NULL entry in list");
				else
				{
					if ((entry->langID) && (std::strcmp(entry->langID, langID) == 0))
						return entry;
				}
			}

			return anongame_infos->anongame_infos_DESC;
		}

		extern char *anongame_infos_DESC_get_DESC(char *langID, int member)
		{
			char *result;
			t_anongame_infos_DESC * DESC;

			if ((DESC = anongame_infos_get_anongame_infos_DESC_by_langID(anongame_infos, langID)))
			if ((DESC->descs) && (result = DESC->descs[member]))
				return result;

			if ((DESC = anongame_infos->anongame_infos_DESC))
			if ((DESC->descs) && (result = DESC->descs[member]))
				return result;

			return NULL;
		}


		/**********/
		extern char *anongame_infos_get_short_desc(char *langID, int queue)
		{
			int member = 0;

			switch (queue)
			{
			case ANONGAME_TYPE_1V1:
				member = gametype_1v1_short;
				break;
			case ANONGAME_TYPE_2V2:
				member = gametype_2v2_short;
				break;
			case ANONGAME_TYPE_3V3:
				member = gametype_3v3_short;
				break;
			case ANONGAME_TYPE_4V4:
				member = gametype_4v4_short;
				break;
			case ANONGAME_TYPE_5V5:
				member = gametype_5v5_short;
				break;
			case ANONGAME_TYPE_6V6:
				member = gametype_6v6_short;
				break;
			case ANONGAME_TYPE_2V2V2:
				member = gametype_2v2v2_short;
				break;
			case ANONGAME_TYPE_3V3V3:
				member = gametype_3v3v3_short;
				break;
			case ANONGAME_TYPE_4V4V4:
				member = gametype_4v4v4_short;
				break;
			case ANONGAME_TYPE_2V2V2V2:
				member = gametype_2v2v2v2_short;
				break;
			case ANONGAME_TYPE_3V3V3V3:
				member = gametype_3v3v3v3_short;
				break;
			case ANONGAME_TYPE_SMALL_FFA:
				member = gametype_sffa_short;
				break;
			case ANONGAME_TYPE_TEAM_FFA:
				member = gametype_tffa_short;
				break;
			case ANONGAME_TYPE_AT_2V2:
				member = gametype_2v2_short;
				break;
			case ANONGAME_TYPE_AT_3V3:
				member = gametype_3v3_short;
				break;
			case ANONGAME_TYPE_AT_4V4:
				member = gametype_4v4_short;
				break;
			case ANONGAME_TYPE_AT_2V2V2:
				member = gametype_2v2v2_short;
				break;
			case ANONGAME_TYPE_TY:
				return tournament_get_format();
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "invalid queue (%d)", queue);
				return NULL;
			}
			return anongame_infos_DESC_get_DESC(langID, member);
		}

		extern char *anongame_infos_get_long_desc(char *langID, int queue)
		{
			int member = 0;

			switch (queue)
			{
			case ANONGAME_TYPE_1V1:
				member = gametype_1v1_long;
				break;
			case ANONGAME_TYPE_2V2:
				member = gametype_2v2_long;
				break;
			case ANONGAME_TYPE_3V3:
				member = gametype_3v3_long;
				break;
			case ANONGAME_TYPE_4V4:
				member = gametype_4v4_long;
				break;
			case ANONGAME_TYPE_5V5:
				member = gametype_5v5_long;
				break;
			case ANONGAME_TYPE_6V6:
				member = gametype_6v6_long;
				break;
			case ANONGAME_TYPE_2V2V2:
				member = gametype_2v2v2_long;
				break;
			case ANONGAME_TYPE_3V3V3:
				member = gametype_3v3v3_long;
				break;
			case ANONGAME_TYPE_4V4V4:
				member = gametype_4v4v4_long;
				break;
			case ANONGAME_TYPE_2V2V2V2:
				member = gametype_2v2v2v2_long;
				break;
			case ANONGAME_TYPE_3V3V3V3:
				member = gametype_3v3v3v3_long;
				break;
			case ANONGAME_TYPE_SMALL_FFA:
				member = gametype_sffa_long;
				break;
			case ANONGAME_TYPE_TEAM_FFA:
				member = gametype_tffa_long;
				break;
			case ANONGAME_TYPE_AT_2V2:
				member = gametype_2v2_long;
				break;
			case ANONGAME_TYPE_AT_3V3:
				member = gametype_3v3_long;
				break;
			case ANONGAME_TYPE_AT_4V4:
				member = gametype_4v4_long;
				break;
			case ANONGAME_TYPE_AT_2V2V2:
				member = gametype_2v2v2_long;
				break;
			case ANONGAME_TYPE_TY:
				return tournament_get_sponsor();;
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "invalid queue (%d)", queue);
				return NULL;
			}
			return anongame_infos_DESC_get_DESC(langID, member);
		}

		/**********/
		static int anongame_infos_THUMBSDOWN_set_THUMBSDOWN(char * anongame_infos_THUMBSDOWN, int member, char value)
		{
			if (!anongame_infos_THUMBSDOWN)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos_THUMBSDOWN");
				return -1;
			}

			anongame_infos_THUMBSDOWN[member] = value;

			return 0;
		}

		/**********/
		extern char anongame_infos_get_thumbsdown(int queue)
		{
			int member = 0;
			switch (queue)
			{
			case ANONGAME_TYPE_1V1:
				member = PG_1v1;
				break;
			case ANONGAME_TYPE_2V2:
				member = PG_2v2;
				break;
			case ANONGAME_TYPE_3V3:
				member = PG_3v3;
				break;
			case ANONGAME_TYPE_4V4:
				member = PG_4v4;
				break;
			case ANONGAME_TYPE_5V5:
				member = PG_5v5;
				break;
			case ANONGAME_TYPE_6V6:
				member = PG_6v6;
				break;
			case ANONGAME_TYPE_2V2V2:
				member = PG_2v2v2;
				break;
			case ANONGAME_TYPE_3V3V3:
				member = PG_3v3v3;
				break;
			case ANONGAME_TYPE_4V4V4:
				member = PG_4v4v4;
				break;
			case ANONGAME_TYPE_2V2V2V2:
				member = PG_2v2v2v2;
				break;
			case ANONGAME_TYPE_3V3V3V3:
				member = PG_3v3v3v3;
				break;
			case ANONGAME_TYPE_SMALL_FFA:
				member = PG_ffa;
				break;
			case ANONGAME_TYPE_TEAM_FFA:
				member = AT_ffa;
				break;
			case ANONGAME_TYPE_AT_2V2:
				member = AT_2v2;
				break;
			case ANONGAME_TYPE_AT_3V3:
				member = AT_3v3;
				break;
			case ANONGAME_TYPE_AT_4V4:
				member = AT_4v4;
				break;
			case ANONGAME_TYPE_AT_2V2V2:
				member = AT_2v2v2;
				break;
			case ANONGAME_TYPE_TY:
				return tournament_get_thumbs_down();
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "invalid queue (%d)", queue);
				return 1;
			}
			return anongame_infos->anongame_infos_THUMBSDOWN[member];
		}

		/**********/

		static int anongame_infos_ICON_REQ_set_REQ(t_anongame_infos * anongame_infos, int member, int value)
		{
			int * anongame_infos_ICON_REQ;

			if (!anongame_infos)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos");
				return -1;
			}

			anongame_infos_ICON_REQ = anongame_infos->anongame_infos_ICON_REQ;

			anongame_infos_ICON_REQ[member] = value;

			return 0;
		}

		extern short anongame_infos_get_ICON_REQ(int Level, t_clienttag clienttag)
		{
			switch (clienttag)
			{
			case CLIENTTAG_WARCRAFT3_UINT:
				switch (Level)
				{
				case 0:
					return 0;
				case 1:
					return anongame_infos->anongame_infos_ICON_REQ[ICON_REQ_WAR3_Level1];
				case 2:
					return anongame_infos->anongame_infos_ICON_REQ[ICON_REQ_WAR3_Level2];
				case 3:
					return anongame_infos->anongame_infos_ICON_REQ[ICON_REQ_WAR3_Level3];
				case 4:
					return anongame_infos->anongame_infos_ICON_REQ[ICON_REQ_WAR3_Level4];
				default:
					return -1;
				}
			case CLIENTTAG_WAR3XP_UINT:
				switch (Level)
				{
				case 0:
					return 0;
				case 1:
					return anongame_infos->anongame_infos_ICON_REQ[ICON_REQ_W3XP_Level1];
				case 2:
					return anongame_infos->anongame_infos_ICON_REQ[ICON_REQ_W3XP_Level2];
				case 3:
					return anongame_infos->anongame_infos_ICON_REQ[ICON_REQ_W3XP_Level3];
				case 4:
					return anongame_infos->anongame_infos_ICON_REQ[ICON_REQ_W3XP_Level4];
				case 5:
					return anongame_infos->anongame_infos_ICON_REQ[ICON_REQ_W3XP_Level5];
				default:
					return -1;
				}
			default:
				eventlog(eventlog_level_error, __FUNCTION__, "invalid clienttag");
				return -1;
			}
		}

		extern short anongame_infos_get_ICON_REQ_TOURNEY(int Level)
		{
			switch (Level)
			{
			case 0:
				return 0;
			case 1:
				return anongame_infos->anongame_infos_ICON_REQ[ICON_REQ_TRNY_Level1];
			case 2:
				return anongame_infos->anongame_infos_ICON_REQ[ICON_REQ_TRNY_Level2];
			case 3:
				return anongame_infos->anongame_infos_ICON_REQ[ICON_REQ_TRNY_Level3];
			case 4:
				return anongame_infos->anongame_infos_ICON_REQ[ICON_REQ_TRNY_Level4];
			case 5:
				return anongame_infos->anongame_infos_ICON_REQ[ICON_REQ_TRNY_Level5];
			default:
				return -1;
			}
		}

		/**********/

		extern char *anongame_infos_data_get_url(t_clienttag clienttag, int versionid, int *len)
		{
			if (clienttag == CLIENTTAG_WARCRAFT3_UINT)
			{
				if (versionid <= 0x0000000E)
				{
					(*len) = anongame_infos->anongame_infos_data_war3->url_comp_len;
					return anongame_infos->anongame_infos_data_war3->url_comp_data;
				}
				else
				{
					(*len) = anongame_infos->anongame_infos_data_war3->url_comp_len_115;
					return anongame_infos->anongame_infos_data_war3->url_comp_data_115;
				}
			}
			else
			{
				if (versionid <= 0x0000000E)
				{
					(*len) = anongame_infos->anongame_infos_data_w3xp->url_comp_len;
					return anongame_infos->anongame_infos_data_w3xp->url_comp_data;
				}
				else
				{
					(*len) = anongame_infos->anongame_infos_data_w3xp->url_comp_len_115;
					return anongame_infos->anongame_infos_data_w3xp->url_comp_data_115;
				}
			}
		}

		extern char *anongame_infos_data_get_map(t_clienttag clienttag, int versionid, int *len)
		{
			if (clienttag == CLIENTTAG_WARCRAFT3_UINT)
			{
				(*len) = anongame_infos->anongame_infos_data_war3->map_comp_len;
				return anongame_infos->anongame_infos_data_war3->map_comp_data;
			}
			else
			{
				(*len) = anongame_infos->anongame_infos_data_w3xp->map_comp_len;
				return anongame_infos->anongame_infos_data_w3xp->map_comp_data;
			}
		}

		extern char *anongame_infos_data_get_type(t_clienttag clienttag, int versionid, int *len)
		{
			if (clienttag == CLIENTTAG_WARCRAFT3_UINT)
			{
				(*len) = anongame_infos->anongame_infos_data_war3->type_comp_len;
				return anongame_infos->anongame_infos_data_war3->type_comp_data;
			}
			else
			{
				(*len) = anongame_infos->anongame_infos_data_w3xp->type_comp_len;
				return anongame_infos->anongame_infos_data_w3xp->type_comp_data;
			}
		}

		extern char *anongame_infos_data_get_desc(char const *langID, t_clienttag clienttag, int versionid, int *len)
		{
			t_elem *curr;
			t_anongame_infos_data_lang *entry;
			if (clienttag == CLIENTTAG_WARCRAFT3_UINT)
			{
				if (langID != NULL)
				{
					LIST_TRAVERSE(anongame_infos->anongame_infos_data_lang_war3, curr)
					{
						if ((entry = (t_anongame_infos_data_lang*)elem_get_data(curr)) && std::strcmp(entry->langID, langID) == 0)
						{
							(*len) = entry->desc_comp_len;
							return entry->desc_comp_data;
						}
					}
				}
				(*len) = anongame_infos->anongame_infos_data_war3->desc_comp_len;
				return anongame_infos->anongame_infos_data_war3->desc_comp_data;
			}
			else
			{
				if (langID != NULL)
				{
					LIST_TRAVERSE(anongame_infos->anongame_infos_data_lang_w3xp, curr)
					{
						if ((entry = (t_anongame_infos_data_lang*)elem_get_data(curr)) && std::strcmp(entry->langID, langID) == 0)
						{
							(*len) = entry->desc_comp_len;
							return entry->desc_comp_data;
						}
					}
				}
				(*len) = anongame_infos->anongame_infos_data_w3xp->desc_comp_len;
				return anongame_infos->anongame_infos_data_w3xp->desc_comp_data;
			}
		}

		extern char *anongame_infos_data_get_ladr(char const *langID, t_clienttag clienttag, int versionid, int *len)
		{
			t_elem *curr;
			t_anongame_infos_data_lang *entry;
			if (clienttag == CLIENTTAG_WARCRAFT3_UINT)
			{
				if (langID != NULL)
				{
					LIST_TRAVERSE(anongame_infos->anongame_infos_data_lang_war3, curr)
					{
						if ((entry = (t_anongame_infos_data_lang*)elem_get_data(curr)) && std::strcmp(entry->langID, langID) == 0)
						{
							(*len) = entry->ladr_comp_len;
							return entry->ladr_comp_data;
						}
					}
				}
				(*len) = anongame_infos->anongame_infos_data_war3->ladr_comp_len;
				return anongame_infos->anongame_infos_data_war3->ladr_comp_data;
			}
			else
			{
				if (langID != NULL)
				{
					LIST_TRAVERSE(anongame_infos->anongame_infos_data_lang_w3xp, curr)
					{
						if ((entry = (t_anongame_infos_data_lang*)elem_get_data(curr)) && std::strcmp(entry->langID, langID) == 0)
						{
							(*len) = entry->ladr_comp_len;
							return entry->ladr_comp_data;
						}
					}
				}
				(*len) = anongame_infos->anongame_infos_data_w3xp->ladr_comp_len;
				return anongame_infos->anongame_infos_data_w3xp->ladr_comp_data;
			}
		}

		/**********/

		static void anongame_infos_set_defaults(t_anongame_infos * anongame_infos)
		{
			char ** anongame_infos_URL;
			t_anongame_infos_DESC *anongame_infos_DESC;
			char ** anongame_infos_DESCs;


			if (!(anongame_infos))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos");
				return;
			}

			anongame_infos_URL = anongame_infos->anongame_infos_URL;
			anongame_infos_DESC = anongame_infos->anongame_infos_DESC;

			if (!(anongame_infos_URL))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos_URL, trying to init");
				if (anongame_infos_URL_init(anongame_infos) != 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "failed to init... PANIC!");
					return;
				}
			}

			if (!(anongame_infos_DESC))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL anongame_infos_DESC, trying to init");
				if (!(anongame_infos_DESC = anongame_infos_DESC_init()))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "failed to init... PANIC!");
					return;
				}
				else
					anongame_infos->anongame_infos_DESC = anongame_infos_DESC;
			}
			// now set default values

			if (!(anongame_infos_URL[URL_server]))
				anongame_infos_URL_set_URL(URL_server, PVPGN_DEFAULT_URL);
			if (!(anongame_infos_URL[URL_player]))
				anongame_infos_URL_set_URL(URL_player, PVPGN_DEFAULT_URL);
			if (!(anongame_infos_URL[URL_tourney]))
				anongame_infos_URL_set_URL(URL_tourney, PVPGN_DEFAULT_URL);
			if (!(anongame_infos_URL[URL_clan]))
				anongame_infos_URL_set_URL(URL_clan, PVPGN_DEFAULT_URL);
			if (!(anongame_infos_URL[URL_ladder_PG_1v1]))
				anongame_infos_URL_set_URL(URL_ladder_PG_1v1, PVPGN_DEFAULT_URL);
			if (!(anongame_infos_URL[URL_ladder_PG_ffa]))
				anongame_infos_URL_set_URL(URL_ladder_PG_ffa, PVPGN_DEFAULT_URL);
			if (!(anongame_infos_URL[URL_ladder_PG_team]))
				anongame_infos_URL_set_URL(URL_ladder_PG_team, PVPGN_DEFAULT_URL);
			if (!(anongame_infos_URL[URL_ladder_AT_2v2]))
				anongame_infos_URL_set_URL(URL_ladder_AT_2v2, PVPGN_DEFAULT_URL);
			if (!(anongame_infos_URL[URL_ladder_AT_3v3]))
				anongame_infos_URL_set_URL(URL_ladder_AT_3v3, PVPGN_DEFAULT_URL);
			if (!(anongame_infos_URL[URL_ladder_AT_4v4]))
				anongame_infos_URL_set_URL(URL_ladder_AT_4v4, PVPGN_DEFAULT_URL);
			if (!(anongame_infos_URL[URL_ladder_clan_1v1]))
				anongame_infos_URL_set_URL(URL_ladder_clan_1v1, PVPGN_DEFAULT_URL);
			if (!(anongame_infos_URL[URL_ladder_clan_2v2]))
				anongame_infos_URL_set_URL(URL_ladder_clan_2v2, PVPGN_DEFAULT_URL);
			if (!(anongame_infos_URL[URL_ladder_clan_3v3]))
				anongame_infos_URL_set_URL(URL_ladder_clan_3v3, PVPGN_DEFAULT_URL);
			if (!(anongame_infos_URL[URL_ladder_clan_4v4]))
				anongame_infos_URL_set_URL(URL_ladder_clan_4v4, PVPGN_DEFAULT_URL);

			if (!(anongame_infos_DESCs = anongame_infos_DESC->descs))
				return;

			if (!(anongame_infos_DESCs[ladder_PG_1v1_desc]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, ladder_PG_1v1_desc, PVPGN_PG_1V1_DESC);
			if (!(anongame_infos_DESCs[ladder_PG_ffa_desc]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, ladder_PG_ffa_desc, PVPGN_PG_FFA_DESC);
			if (!(anongame_infos_DESCs[ladder_PG_team_desc]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, ladder_PG_team_desc, PVPGN_PG_TEAM_DESC);
			if (!(anongame_infos_DESCs[ladder_AT_2v2_desc]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, ladder_AT_2v2_desc, PVPGN_AT_2V2_DESC);
			if (!(anongame_infos_DESCs[ladder_AT_3v3_desc]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, ladder_AT_3v3_desc, PVPGN_AT_3V3_DESC);
			if (!(anongame_infos_DESCs[ladder_AT_4v4_desc]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, ladder_AT_4v4_desc, PVPGN_AT_4V4_DESC);
			if (!(anongame_infos_DESCs[ladder_clan_1v1_desc]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, ladder_clan_1v1_desc, PVPGN_CLAN_1V1_DESC);
			if (!(anongame_infos_DESCs[ladder_clan_2v2_desc]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, ladder_clan_2v2_desc, PVPGN_CLAN_2V2_DESC);
			if (!(anongame_infos_DESCs[ladder_clan_3v3_desc]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, ladder_clan_3v3_desc, PVPGN_CLAN_3V3_DESC);
			if (!(anongame_infos_DESCs[ladder_clan_4v4_desc]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, ladder_clan_4v4_desc, PVPGN_CLAN_4V4_DESC);

			if (!(anongame_infos_DESCs[gametype_1v1_short]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_1v1_short, PVPGN_1V1_GT_DESC);
			if (!(anongame_infos_DESCs[gametype_1v1_long]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_1v1_long, PVPGN_1V1_GT_LONG);
			if (!(anongame_infos_DESCs[gametype_2v2_short]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_2v2_short, PVPGN_2V2_GT_DESC);
			if (!(anongame_infos_DESCs[gametype_2v2_long]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_2v2_long, PVPGN_2V2_GT_LONG);
			if (!(anongame_infos_DESCs[gametype_3v3_short]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_3v3_short, PVPGN_3V3_GT_DESC);
			if (!(anongame_infos_DESCs[gametype_3v3_long]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_3v3_long, PVPGN_3V3_GT_LONG);
			if (!(anongame_infos_DESCs[gametype_4v4_short]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_4v4_short, PVPGN_4V4_GT_DESC);
			if (!(anongame_infos_DESCs[gametype_4v4_long]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_4v4_long, PVPGN_4V4_GT_LONG);
			if (!(anongame_infos_DESCs[gametype_sffa_short]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_sffa_short, PVPGN_SFFA_GT_DESC);
			if (!(anongame_infos_DESCs[gametype_sffa_long]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_sffa_long, PVPGN_SFFA_GT_LONG);
			if (!(anongame_infos_DESCs[gametype_tffa_short]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_tffa_short, PVPGN_TFFA_GT_DESC);
			if (!(anongame_infos_DESCs[gametype_tffa_long]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_tffa_long, PVPGN_TFFA_GT_LONG);
			if (!(anongame_infos_DESCs[gametype_2v2v2_short]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_2v2v2_short, PVPGN_2V2V2_GT_DESC);
			if (!(anongame_infos_DESCs[gametype_2v2v2_long]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_2v2v2_long, PVPGN_2V2V2_GT_LONG);
			if (!(anongame_infos_DESCs[gametype_3v3v3_short]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_3v3v3_short, PVPGN_3V3V3_GT_DESC);
			if (!(anongame_infos_DESCs[gametype_3v3v3_long]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_3v3v3_long, PVPGN_3V3V3_GT_LONG);
			if (!(anongame_infos_DESCs[gametype_4v4v4_short]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_4v4v4_short, PVPGN_4V4V4_GT_DESC);
			if (!(anongame_infos_DESCs[gametype_4v4v4_long]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_4v4v4_long, PVPGN_4V4V4_GT_LONG);
			if (!(anongame_infos_DESCs[gametype_2v2v2v2_short]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_2v2v2v2_short, PVPGN_2V2V2V2_GT_DESC);
			if (!(anongame_infos_DESCs[gametype_2v2v2v2_long]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_2v2v2v2_long, PVPGN_2V2V2V2_GT_LONG);
			if (!(anongame_infos_DESCs[gametype_3v3v3v3_short]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_3v3v3v3_short, PVPGN_3V3V3V3_GT_DESC);
			if (!(anongame_infos_DESCs[gametype_3v3v3v3_long]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_3v3v3v3_long, PVPGN_3V3V3V3_GT_LONG);
			if (!(anongame_infos_DESCs[gametype_5v5_short]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_5v5_short, PVPGN_5V5_GT_DESC);
			if (!(anongame_infos_DESCs[gametype_5v5_long]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_5v5_long, PVPGN_5V5_GT_LONG);
			if (!(anongame_infos_DESCs[gametype_6v6_short]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_6v6_short, PVPGN_6V6_GT_DESC);
			if (!(anongame_infos_DESCs[gametype_6v6_long]))
				anongame_infos_DESC_set_DESC(anongame_infos_DESC, gametype_6v6_long, PVPGN_6V6_GT_LONG);


		}

		typedef struct {
			const char *anongame_infos_URL_string;
			int member;
		} t_anongame_infos_URL_table_row;

		typedef struct {
			const char *anongame_infos_DESC_string;
			int member;
		} t_anongame_infos_DESC_table_row;

		typedef struct {
			const char *anongame_infos_THUMBSDOWN_string;
			int member;
		} t_anongame_infos_THUMBSDOWN_table_row;

		typedef struct {
			const char *anongame_infos_ICON_REQ_WAR3_string;
			int member;
		} t_anongame_infos_ICON_REQ_WAR3_table_row;

		typedef struct {
			const char *anongame_infos_ICON_REQ_W3XP_string;
			int member;
		} t_anongame_infos_ICON_REQ_W3XP_table_row;

		typedef struct {
			const char *anongame_infos_ICON_REQ_TOURNEY_string;
			int member;
		} t_anongame_infos_ICON_REQ_TOURNEY_table_row;


		static const t_anongame_infos_URL_table_row URL_handler_table[] = {
			{ "server_URL", URL_server },
			{ "player_URL", URL_player },
			{ "tourney_URL", URL_tourney },
			{ "clan_URL", URL_clan },
			{ "ladder_PG_1v1_URL", URL_ladder_PG_1v1 },
			{ "ladder_PG_ffa_URL", URL_ladder_PG_ffa },
			{ "ladder_PG_team_URL", URL_ladder_PG_team },
			{ "ladder_AT_2v2_URL", URL_ladder_AT_2v2 },
			{ "ladder_AT_3v3_URL", URL_ladder_AT_3v3 },
			{ "ladder_AT_4v4_URL", URL_ladder_AT_4v4 },
			{ "ladder_clan_1v1_URL", URL_ladder_clan_1v1 },
			{ "ladder_clan_2v2_URL", URL_ladder_clan_2v2 },
			{ "ladder_clan_3v3_URL", URL_ladder_clan_3v3 },
			{ "ladder_clan_4v4_URL", URL_ladder_clan_4v4 },
			{ NULL, -1 }
		};

		static const t_anongame_infos_DESC_table_row DESC_handler_table[] = {
			{ "ladder_PG_1v1_desc", ladder_PG_1v1_desc },
			{ "ladder_PG_ffa_desc", ladder_PG_ffa_desc },
			{ "ladder_PG_team_desc", ladder_PG_team_desc },
			{ "ladder_AT_2v2_desc", ladder_AT_2v2_desc },
			{ "ladder_AT_3v3_desc", ladder_AT_3v3_desc },
			{ "ladder_AT_4v4_desc", ladder_AT_4v4_desc },
			{ "ladder_clan_1v1_desc", ladder_clan_1v1_desc },
			{ "ladder_clan_2v2_desc", ladder_clan_2v2_desc },
			{ "ladder_clan_3v3_desc", ladder_clan_3v3_desc },
			{ "ladder_clan_4v4_desc", ladder_clan_4v4_desc },

			{ "gametype_1v1_short", gametype_1v1_short },
			{ "gametype_1v1_long", gametype_1v1_long },
			{ "gametype_2v2_short", gametype_2v2_short },
			{ "gametype_2v2_long", gametype_2v2_long },
			{ "gametype_3v3_short", gametype_3v3_short },
			{ "gametype_3v3_long", gametype_3v3_long },
			{ "gametype_4v4_short", gametype_4v4_short },
			{ "gametype_4v4_long", gametype_4v4_long },
			{ "gametype_sffa_short", gametype_sffa_short },
			{ "gametype_sffa_long", gametype_sffa_long },
			{ "gametype_tffa_short", gametype_tffa_short },
			{ "gametype_tffa_long", gametype_tffa_long },
			{ "gametype_2v2v2_short", gametype_2v2v2_short },
			{ "gametype_2v2v2_long", gametype_2v2v2_long },
			{ "gametype_3v3v3_short", gametype_3v3v3_short },
			{ "gametype_3v3v3_long", gametype_3v3v3_long },
			{ "gametype_4v4v4_short", gametype_4v4v4_short },
			{ "gametype_4v4v4_long", gametype_4v4v4_long },
			{ "gametype_2v2v2v2_short", gametype_2v2v2v2_short },
			{ "gametype_2v2v2v2_long", gametype_2v2v2v2_long },
			{ "gametype_3v3v3v3_short", gametype_3v3v3v3_short },
			{ "gametype_3v3v3v3_long", gametype_3v3v3v3_long },
			{ "gametype_5v5_short", gametype_5v5_short },
			{ "gametype_5v5_long", gametype_5v5_long },
			{ "gametype_6v6_short", gametype_6v6_short },
			{ "gametype_6v6_long", gametype_6v6_long },

			{ NULL, -1 }
		};

		static const t_anongame_infos_THUMBSDOWN_table_row THUMBSDOWN_handler_table[] = {
			{ "PG_1v1", PG_1v1 },
			{ "PG_2v2", PG_2v2 },
			{ "PG_3v3", PG_3v3 },
			{ "PG_4v4", PG_4v4 },
			{ "PG_ffa", PG_ffa },
			{ "AT_2v2", AT_2v2 },
			{ "AT_3v3", AT_3v3 },
			{ "AT_4v4", AT_4v4 },
			{ "AT_ffa", AT_ffa },
			{ "PG_5v5", PG_5v5 },
			{ "PG_6v6", PG_6v6 },
			{ "PG_2v2v2", PG_2v2v2 },
			{ "PG_3v3v3", PG_3v3v3 },
			{ "PG_4v4v4", PG_4v4v4 },
			{ "PG_2v2v2v2", PG_2v2v2v2 },
			{ "PG_3v3v3v3", PG_3v3v3v3 },
			{ "AT_2v2v2", AT_2v2v2 },

			{ NULL, -1 }
		};

		static const t_anongame_infos_ICON_REQ_WAR3_table_row ICON_REQ_WAR3_handler_table[] = {
			{ "Level1", ICON_REQ_WAR3_Level1 },
			{ "Level2", ICON_REQ_WAR3_Level2 },
			{ "Level3", ICON_REQ_WAR3_Level3 },
			{ "Level4", ICON_REQ_WAR3_Level4 },
			{ NULL, -1 }
		};

		static const t_anongame_infos_ICON_REQ_W3XP_table_row ICON_REQ_W3XP_handler_table[] = {
			{ "Level1", ICON_REQ_W3XP_Level1 },
			{ "Level2", ICON_REQ_W3XP_Level2 },
			{ "Level3", ICON_REQ_W3XP_Level3 },
			{ "Level4", ICON_REQ_W3XP_Level4 },
			{ "Level5", ICON_REQ_W3XP_Level5 },
			{ NULL, -1 }
		};

		static const t_anongame_infos_ICON_REQ_TOURNEY_table_row ICON_REQ_TOURNEY_handler_table[] = {
			{ "Level1", ICON_REQ_TRNY_Level1 },
			{ "Level2", ICON_REQ_TRNY_Level2 },
			{ "Level3", ICON_REQ_TRNY_Level3 },
			{ "Level4", ICON_REQ_TRNY_Level4 },
			{ "Level5", ICON_REQ_TRNY_Level5 },
			{ NULL, -1 }
		};

		typedef enum {
			parse_UNKNOWN,
			parse_URL,
			parse_DESC,
			parse_THUMBSDOWN,
			parse_ICON_REQ_WAR3,
			parse_ICON_REQ_W3XP,
			parse_ICON_REQ_TOURNEY
		} t_parse_mode;

		typedef enum {
			changed,
			unchanged
		} t_parse_state;

		static t_parse_mode switch_parse_mode(char *text, char *langID)
		{
			if (!(text))
				return parse_UNKNOWN;
			else if (std::strcmp(text, "[URL]") == 0)
				return parse_URL;
			else if (std::strcmp(text, "[THUMBS_DOWN_LIMIT]") == 0)
				return parse_THUMBSDOWN;
			else if (std::strcmp(text, "[ICON_REQUIRED_RACE_WINS_WAR3]") == 0)
				return parse_ICON_REQ_WAR3;
			else if (std::strcmp(text, "[ICON_REQUIRED_RACE_WINS_W3XP]") == 0)
				return parse_ICON_REQ_W3XP;
			else if (std::strcmp(text, "[ICON_REQUIRED_TOURNEY_WINS]") == 0)
				return parse_ICON_REQ_TOURNEY;
			else if (std::strcmp(text, "[DEFAULT_DESC]") == 0)
			{
				langID[0] = '\0';
				return parse_DESC;
			}
			else if (std::strlen(text) == 6)
			{
				std::strncpy(langID, &(text[1]), 4);
				langID[4] = '\0';
				return parse_DESC;
			}
			else
				eventlog(eventlog_level_error, __FUNCTION__, "got invalid section name: %s", text);
			return parse_UNKNOWN;
		}

		extern int anongame_infos_load(char const *filename)
		{
			unsigned int line;
			unsigned int pos;
			char *buff;
			char *temp;
			char langID[5];
			t_parse_mode parse_mode = parse_UNKNOWN;
			t_parse_state parse_state = unchanged;
			t_anongame_infos_DESC *anongame_infos_DESC = NULL;
			char *pointer;
			char *variable;
			char *value = NULL;
			t_anongame_infos_DESC_table_row const *DESC_table_row;
			t_anongame_infos_URL_table_row const *URL_table_row;
			t_anongame_infos_THUMBSDOWN_table_row const *THUMBSDOWN_table_row;
			t_anongame_infos_ICON_REQ_WAR3_table_row const *ICON_REQ_WAR3_table_row;
			t_anongame_infos_ICON_REQ_W3XP_table_row const *ICON_REQ_W3XP_table_row;
			t_anongame_infos_ICON_REQ_TOURNEY_table_row const *ICON_REQ_TOURNEY_table_row;
			int int_value;
			char char_value;

			langID[0] = '\0';

			if (!filename)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
				return -1;
			}

			if (!(anongame_infos = anongame_infos_init()))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not init anongame_infos");
				return -1;
			}

			if (!(fp = std::fopen(filename, "r")))
			{
				eventlog(eventlog_level_error, "anongameinfo_load", "could not open file \"%s\" for reading (std::fopen: %s), using default values", filename, std::strerror(errno));
				goto anongame_infos_loading_failure;
			}

			for (line = 1; (buff = file_get_line(fp)); line++)
			{
				for (pos = 0; buff[pos] == '\t' || buff[pos] == ' '; pos++);
				if (buff[pos] == '\0' || buff[pos] == '#')
				{
					continue;
				}
				if ((temp = std::strrchr(buff, '#')))
				{
					unsigned int len;
					unsigned int endpos;

					*temp = '\0';
					len = std::strlen(buff) + 1;
					for (endpos = len - 1; buff[endpos] == '\t' || buff[endpos] == ' '; endpos--);
					buff[endpos + 1] = '\0';
				}

				if ((buff[0] == '[') && (buff[std::strlen(buff) - 1] == ']'))
				{
					if ((parse_state == unchanged) && (anongame_infos_DESC != NULL))
					{
						if (langID[0] != '\0')
							list_append_data(anongame_infos->anongame_infos_DESC_list, anongame_infos_DESC);
						else
						{
							if (anongame_infos->anongame_infos_DESC == NULL)
								anongame_infos->anongame_infos_DESC = anongame_infos_DESC;
							else
							{
								eventlog(eventlog_level_error, __FUNCTION__, "found another default_DESC block, deleting previous");
								anongame_infos_DESC_destroy(anongame_infos->anongame_infos_DESC);
								anongame_infos->anongame_infos_DESC = anongame_infos_DESC;
							}
						}
						anongame_infos_DESC = NULL;
					}
					parse_mode = switch_parse_mode(buff, langID);
					parse_state = changed;
				}
				else if (buff[0] != '\0')
					switch (parse_mode)
				{
					case parse_UNKNOWN:
					{
										  if ((buff[0] != '[') || (buff[std::strlen(buff) - 1] != ']'))
										  {
											  eventlog(eventlog_level_error, __FUNCTION__, "expected [] section start, but found %s on line %u", buff, line);
										  }
										  else
										  {
											  parse_mode = switch_parse_mode(buff, langID);
											  parse_state = changed;
										  }
										  break;
					}
					case parse_URL:
					{
									  parse_state = unchanged;
									  variable = buff;
									  pointer = std::strchr(variable, '=');
									  for (pointer--; pointer[0] == ' '; pointer--);
									  pointer[1] = '\0';
									  pointer++;
									  pointer++;
									  pointer = std::strchr(pointer, '\"');
									  pointer++;
									  value = pointer;
									  pointer = std::strchr(pointer, '\"');
									  pointer[0] = '\0';

									  for (URL_table_row = URL_handler_table; URL_table_row->anongame_infos_URL_string != NULL; URL_table_row++)
									  if (std::strcmp(URL_table_row->anongame_infos_URL_string, variable) == 0)
									  {
										  if (URL_table_row->member != -1)
											  anongame_infos_URL_set_URL(URL_table_row->member, value);
									  }

									  break;
					}
					case parse_DESC:
					{
									   if (parse_state == changed)
									   {
										   anongame_infos_DESC = anongame_infos_DESC_init();
										   parse_state = unchanged;
										   eventlog(eventlog_level_info, __FUNCTION__, "got langID: [%s]", langID);
										   if (langID[0] != '\0')
											   anongame_infos_DESC->langID = xstrdup(langID);
									   }

									   variable = buff;
									   pointer = std::strchr(variable, '=');
									   for (pointer--; pointer[0] == ' '; pointer--);
									   pointer[1] = '\0';
									   pointer++;
									   pointer++;
									   pointer = std::strchr(pointer, '\"');
									   pointer++;
									   value = pointer;
									   pointer = std::strchr(pointer, '\"');
									   pointer[0] = '\0';

									   for (DESC_table_row = DESC_handler_table; DESC_table_row->anongame_infos_DESC_string != NULL; DESC_table_row++)
									   if (std::strcmp(DESC_table_row->anongame_infos_DESC_string, variable) == 0)
									   {
										   if (DESC_table_row->member != -1)
											   anongame_infos_DESC_set_DESC(anongame_infos_DESC, DESC_table_row->member, value);
									   }
									   break;
					}
					case parse_THUMBSDOWN:
					{
											 parse_state = unchanged;
											 variable = buff;
											 pointer = std::strchr(variable, '=');
											 for (pointer--; pointer[0] == ' '; pointer--);
											 pointer[1] = '\0';
											 pointer++;
											 pointer++;
											 pointer = std::strchr(pointer, '=');
											 pointer++;
											 int_value = std::atoi(pointer);
											 if (int_value < 0)
												 int_value = 0;
											 if (int_value > 127)
												 int_value = 127;
											 char_value = (char)int_value;

											 for (THUMBSDOWN_table_row = THUMBSDOWN_handler_table; THUMBSDOWN_table_row->anongame_infos_THUMBSDOWN_string != NULL; THUMBSDOWN_table_row++)
											 if (std::strcmp(THUMBSDOWN_table_row->anongame_infos_THUMBSDOWN_string, variable) == 0)
											 {
												 if (THUMBSDOWN_table_row->member != -1)
													 anongame_infos_THUMBSDOWN_set_THUMBSDOWN(anongame_infos->anongame_infos_THUMBSDOWN,
													 THUMBSDOWN_table_row->member, char_value);
											 }
											 break;
					}
					case parse_ICON_REQ_WAR3:
					{
												parse_state = unchanged;
												variable = buff;
												pointer = std::strchr(variable, '=');
												for (pointer--; pointer[0] == ' '; pointer--);
												pointer[1] = '\0';
												pointer++;
												pointer++;
												pointer = std::strchr(pointer, '=');
												pointer++;
												int_value = std::atoi(pointer);
												if (int_value < 0)
													int_value = 0;

												for (ICON_REQ_WAR3_table_row = ICON_REQ_WAR3_handler_table; ICON_REQ_WAR3_table_row->anongame_infos_ICON_REQ_WAR3_string != NULL; ICON_REQ_WAR3_table_row++)
												if (std::strcmp(ICON_REQ_WAR3_table_row->anongame_infos_ICON_REQ_WAR3_string, variable) == 0)
												{
													if (ICON_REQ_WAR3_table_row->member != -1)
														anongame_infos_ICON_REQ_set_REQ(anongame_infos, ICON_REQ_WAR3_table_row->member, int_value);
												}
												break;
					}
					case parse_ICON_REQ_W3XP:
					{
												parse_state = unchanged;
												variable = buff;
												pointer = std::strchr(variable, '=');
												for (pointer--; pointer[0] == ' '; pointer--);
												pointer[1] = '\0';
												pointer++;
												pointer++;
												pointer = std::strchr(pointer, '=');
												pointer++;
												int_value = std::atoi(pointer);
												if (int_value < 0)
													int_value = 0;

												for (ICON_REQ_W3XP_table_row = ICON_REQ_W3XP_handler_table; ICON_REQ_W3XP_table_row->anongame_infos_ICON_REQ_W3XP_string != NULL; ICON_REQ_W3XP_table_row++)
												if (std::strcmp(ICON_REQ_W3XP_table_row->anongame_infos_ICON_REQ_W3XP_string, variable) == 0)
												{
													if (ICON_REQ_W3XP_table_row->member != -1)
														anongame_infos_ICON_REQ_set_REQ(anongame_infos, ICON_REQ_W3XP_table_row->member, int_value);
												}
												break;
					}
					case parse_ICON_REQ_TOURNEY:
					{
												   parse_state = unchanged;
												   variable = buff;
												   pointer = std::strchr(variable, '=');
												   for (pointer--; pointer[0] == ' '; pointer--);
												   pointer[1] = '\0';
												   pointer++;
												   pointer++;
												   pointer = std::strchr(pointer, '=');
												   pointer++;
												   int_value = std::atoi(pointer);
												   if (int_value < 0)
													   int_value = 0;

												   for (ICON_REQ_TOURNEY_table_row = ICON_REQ_TOURNEY_handler_table; ICON_REQ_TOURNEY_table_row->anongame_infos_ICON_REQ_TOURNEY_string != NULL; ICON_REQ_TOURNEY_table_row++)
												   if (std::strcmp(ICON_REQ_TOURNEY_table_row->anongame_infos_ICON_REQ_TOURNEY_string, variable) == 0)
												   {
													   if (ICON_REQ_TOURNEY_table_row->member != -1)
														   anongame_infos_ICON_REQ_set_REQ(anongame_infos, ICON_REQ_TOURNEY_table_row->member, int_value);
												   }
												   break;
					}
				}
			}

			if (anongame_infos_DESC)
			{
				if (langID[0] != '\0')
				{
					list_append_data(anongame_infos->anongame_infos_DESC_list, anongame_infos_DESC);
				}
				else
				{
					if (anongame_infos->anongame_infos_DESC == NULL)
						anongame_infos->anongame_infos_DESC = anongame_infos_DESC;
					else
					{
						eventlog(eventlog_level_error, __FUNCTION__, "found another default_DESC block, deleting previous");
						anongame_infos_DESC_destroy(anongame_infos->anongame_infos_DESC);
						anongame_infos->anongame_infos_DESC = anongame_infos_DESC;
					}
				}
			}

			file_get_line(NULL); // clear file_get_line buffer
			std::fclose(fp);

		anongame_infos_loading_failure:
			anongame_infos_set_defaults(anongame_infos);
			anongame_infos_data_load();
			return 0;
		}

		static int anongame_infos_data_load(void)
		{
			t_elem *curr;
			t_packet *raw;
			int j, k, size;
			char ladr_count = 0;
			char desc_count;
			char mapscount_total;
			char value;
			char PG_gamestyles;
			char AT_gamestyles;
			char TY_gamestyles;

			char anongame_prefix[ANONGAME_TYPES][5] = {			/* queue */
				/* PG 1v1       */{ 0x00, 0x00, 0x03, 0x3F, 0x00 }, 	/*  0   */
				/* PG 2v2       */{ 0x01, 0x00, 0x02, 0x3F, 0x00 },	/*  1   */
				/* PG 3v3       */{ 0x02, 0x00, 0x01, 0x3F, 0x00 },	/*  2   */
				/* PG 4v4       */{ 0x03, 0x00, 0x01, 0x3F, 0x00 },	/*  3   */
				/* PG sffa      */{ 0x04, 0x00, 0x02, 0x3F, 0x00 },	/*  4   */

				/* AT 2v2       */{ 0x00, 0x00, 0x02, 0x3F, 0x02 },	/*  5   */
				/* AT tffa      */{ 0x01, 0x00, 0x02, 0x3F, 0x02 },	/*  6   */
				/* AT 3v3       */{ 0x02, 0x00, 0x02, 0x3F, 0x03 },	/*  7   */
				/* AT 4v4       */{ 0x03, 0x00, 0x02, 0x3F, 0x04 },	/*  8   */

				/* TY           */{ 0x00, 0x01, 0x00, 0x3F, 0x00 },
				/*  9   */
				/* PG 5v5       */{ 0x05, 0x00, 0x01, 0x3F, 0x00 }, 	/* 10   */
				/* PG 6v6       */{ 0x06, 0x00, 0x01, 0x3F, 0x00 },	/* 11   */
				/* PG 2v2v2     */{ 0x07, 0x00, 0x01, 0x3F, 0x00 },	/* 12   */
				/* PG 3v3v3     */{ 0x08, 0x00, 0x01, 0x3F, 0x00 },	/* 13   */
				/* PG 4v4v4     */{ 0x09, 0x00, 0x01, 0x3F, 0x00 },	/* 14   */
				/* PG 2v2v2v2   */{ 0x0A, 0x00, 0x01, 0x3F, 0x00 },	/* 15   */
				/* PG 3v3v3v3   */{ 0x0B, 0x00, 0x01, 0x3F, 0x00 },	/* 16   */
				/* AT 2v2v2     */{ 0x04, 0x00, 0x02, 0x3F, 0x02 }	/* 17   */
			};

			/* hack to give names for new gametypes untill there added to anongame_infos.c */
			const char * anongame_gametype_names[ANONGAME_TYPES] = {
				"One vs. One",
				"Two vs. Two",
				"Three vs. Three",
				"Four vs. Four",
				"Small Free for All",
				"Two vs. Two",
				"Team Free for All",
				"Three vs. Three",
				"Four vs. Four",
				"Tournament Game",
				"Five vs. Five",
				"Six Vs. Six",
				"Two vs. Two vs. Two",
				"3 vs. 3 vs. 3",
				"4 vs. 4 vs. 4",
				"2 vs. 2 vs. 2 vs. 2",
				"3 vs. 3 vs. 3 vs. 3",
				"Two vs. Two vs. Two"
			};

			t_clienttag game_clienttag[2] = { CLIENTTAG_WARCRAFT3_UINT, CLIENTTAG_WAR3XP_UINT };

			char anongame_PG_section = 0x00;
			char anongame_AT_section = 0x01;
			char anongame_TY_section = 0x02;

			/* set thumbsdown from the conf file */
			for (j = 0; j < ANONGAME_TYPES; j++)
				anongame_prefix[j][2] = anongame_infos_get_thumbsdown(j);

			if ((raw = packet_create(packet_class_raw)) != NULL)
			{
				// assemble URL part with 3 URLs ( <1.15 )
				packet_append_string(raw, anongame_infos_URL_get_URL(URL_server));
				packet_append_string(raw, anongame_infos_URL_get_URL(URL_player));
				packet_append_string(raw, anongame_infos_URL_get_URL(URL_tourney));
				size = packet_get_size(raw);
				// create compressed data
				zlib_compress(packet_get_data_const(raw, 0, size), size, &anongame_infos->anongame_infos_data_war3->url_comp_data, &anongame_infos->anongame_infos_data_war3->url_comp_len);
				zlib_compress(packet_get_data_const(raw, 0, size), size, &anongame_infos->anongame_infos_data_w3xp->url_comp_data, &anongame_infos->anongame_infos_data_w3xp->url_comp_len);

				// append 4th URL for >= 1.15 clients
				packet_append_string(raw, anongame_infos_URL_get_URL(URL_clan));
				size = packet_get_size(raw);
				zlib_compress(packet_get_data_const(raw, 0, size), size, &anongame_infos->anongame_infos_data_war3->url_comp_data_115, &anongame_infos->anongame_infos_data_war3->url_comp_len_115);
				zlib_compress(packet_get_data_const(raw, 0, size), size, &anongame_infos->anongame_infos_data_w3xp->url_comp_data_115, &anongame_infos->anongame_infos_data_w3xp->url_comp_len_115);

				for (k = 0; k < 2; k++)
				{
					packet_set_size(raw, 0);
					mapscount_total = maplists_get_totalmaps(game_clienttag[k]);
					packet_append_data(raw, &mapscount_total, 1);
					maplists_add_maps_to_packet(raw, game_clienttag[k]);
					size = packet_get_size(raw);
					if (k == 0)
						zlib_compress(packet_get_data_const(raw, 0, size), size, &anongame_infos->anongame_infos_data_war3->map_comp_data, &anongame_infos->anongame_infos_data_war3->map_comp_len);
					else
						zlib_compress(packet_get_data_const(raw, 0, size), size, &anongame_infos->anongame_infos_data_w3xp->map_comp_data, &anongame_infos->anongame_infos_data_w3xp->map_comp_len);
				}


				for (k = 0; k < 2; k++)
				{
					packet_set_size(raw, 0);
					value = 0;
					PG_gamestyles = 0;
					AT_gamestyles = 0;
					TY_gamestyles = 0;
					/* count of gametypes (PG, AT, TY) */
					for (j = 0; j < ANONGAME_TYPES; j++)
					if (maplists_get_totalmaps_by_queue(game_clienttag[k], j))
					{
						if (!anongame_prefix[j][1] && !anongame_prefix[j][4])
							PG_gamestyles++;
						if (!anongame_prefix[j][1] && anongame_prefix[j][4])
							AT_gamestyles++;
						if (anongame_prefix[j][1])
							TY_gamestyles++;
					}

					if (PG_gamestyles)
						value++;
					if (AT_gamestyles)
						value++;
					if (TY_gamestyles)
						value++;

					packet_append_data(raw, &value, 1);

					/* PG */
					if (PG_gamestyles)
					{
						packet_append_data(raw, &anongame_PG_section, 1);
						packet_append_data(raw, &PG_gamestyles, 1);
						for (j = 0; j < ANONGAME_TYPES; j++)
						if (!anongame_prefix[j][1] && !anongame_prefix[j][4] && maplists_get_totalmaps_by_queue(game_clienttag[k], j))
						{
							packet_append_data(raw, &anongame_prefix[j], 5);
							maplists_add_map_info_to_packet(raw, game_clienttag[k], j);
						}
					}

					/* AT */
					if (AT_gamestyles)
					{
						packet_append_data(raw, &anongame_AT_section, 1);
						packet_append_data(raw, &AT_gamestyles, 1);
						for (j = 0; j < ANONGAME_TYPES; j++)
						if (!anongame_prefix[j][1] && anongame_prefix[j][4] && maplists_get_totalmaps_by_queue(game_clienttag[k], j))
						{
							packet_append_data(raw, &anongame_prefix[j], 5);
							maplists_add_map_info_to_packet(raw, game_clienttag[k], j);
						}
					}

					/* TY */
					if (TY_gamestyles)
					{
						packet_append_data(raw, &anongame_TY_section, 1);
						packet_append_data(raw, &TY_gamestyles, 1);
						for (j = 0; j < ANONGAME_TYPES; j++)
						if (anongame_prefix[j][1] && maplists_get_totalmaps_by_queue(game_clienttag[k], j))
						{
							/* set tournament races available */
							anongame_prefix[j][3] = tournament_get_races();
							/* set tournament type (PG or AT)
							 * PG = 0
							 * AT = number players per team */
							if (tournament_is_arranged())
								anongame_prefix[j][4] = tournament_get_game_type();
							else
								anongame_prefix[j][4] = 0;

							packet_append_data(raw, &anongame_prefix[j], 5);
							maplists_add_map_info_to_packet(raw, game_clienttag[k], j);
						}
					}

					size = packet_get_size(raw);
					if (k == 0)
						zlib_compress(packet_get_data_const(raw, 0, size), size, &anongame_infos->anongame_infos_data_war3->type_comp_data, &anongame_infos->anongame_infos_data_war3->type_comp_len);
					else
						zlib_compress(packet_get_data_const(raw, 0, size), size, &anongame_infos->anongame_infos_data_w3xp->type_comp_data, &anongame_infos->anongame_infos_data_w3xp->type_comp_len);
				}

				for (k = 0; k < 2; k++)
				{
					desc_count = 0;
					packet_set_size(raw, 0);
					for (j = 0; j < ANONGAME_TYPES; j++)
					if (maplists_get_totalmaps_by_queue(game_clienttag[k], j))
						desc_count++;
					packet_append_data(raw, &desc_count, 1);
					/* PG description section */
					for (j = 0; j < ANONGAME_TYPES; j++)
					if (!anongame_prefix[j][1] && !anongame_prefix[j][4] && maplists_get_totalmaps_by_queue(game_clienttag[k], j))
					{
						packet_append_data(raw, &anongame_PG_section, 1);
						packet_append_data(raw, &anongame_prefix[j][0], 1);

						if (anongame_infos_get_short_desc(NULL, j) == NULL)
							packet_append_string(raw, anongame_gametype_names[j]);
						else
							packet_append_string(raw, anongame_infos_get_short_desc(NULL, j));

						if (anongame_infos_get_long_desc(NULL, j) == NULL)
							packet_append_string(raw, "No Descreption");
						else
							packet_append_string(raw, anongame_infos_get_long_desc(NULL, j));
					}
					/* AT description section */
					for (j = 0; j < ANONGAME_TYPES; j++)
					if (!anongame_prefix[j][1] && anongame_prefix[j][4] && maplists_get_totalmaps_by_queue(game_clienttag[k], j))
					{
						packet_append_data(raw, &anongame_AT_section, 1);
						packet_append_data(raw, &anongame_prefix[j][0], 1);
						packet_append_string(raw, anongame_infos_get_short_desc(NULL, j));
						packet_append_string(raw, anongame_infos_get_long_desc(NULL, j));
					}
					/* TY description section */
					for (j = 0; j < ANONGAME_TYPES; j++)
					if (anongame_prefix[j][1] && maplists_get_totalmaps_by_queue(game_clienttag[k], j))
					{
						packet_append_data(raw, &anongame_TY_section, 1);
						packet_append_data(raw, &anongame_prefix[j][0], 1);
						packet_append_string(raw, anongame_infos_get_short_desc(NULL, j));
						packet_append_string(raw, anongame_infos_get_long_desc(NULL, j));
					}
					size = packet_get_size(raw);
					if (k == 0)
						zlib_compress(packet_get_data_const(raw, 0, size), size, &anongame_infos->anongame_infos_data_war3->desc_comp_data, &anongame_infos->anongame_infos_data_war3->desc_comp_len);
					else
						zlib_compress(packet_get_data_const(raw, 0, size), size, &anongame_infos->anongame_infos_data_w3xp->desc_comp_data, &anongame_infos->anongame_infos_data_w3xp->desc_comp_len);
				}

				packet_set_size(raw, 0);
				/*FIXME: Still adding a static number (10)
				   Also maybe need do do some checks to avoid prefs empty strings. */
				ladr_count = 10;
				packet_append_data(raw, &ladr_count, 1);
				packet_append_data(raw, "OLOS", 4);
				packet_append_string(raw, anongame_infos_DESC_get_DESC(NULL, ladder_PG_1v1_desc));
				packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_PG_1v1));
				packet_append_data(raw, "MAET", 4);
				packet_append_string(raw, anongame_infos_DESC_get_DESC(NULL, ladder_PG_team_desc));
				packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_PG_team));
				packet_append_data(raw, " AFF", 4);
				packet_append_string(raw, anongame_infos_DESC_get_DESC(NULL, ladder_PG_ffa_desc));
				packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_PG_ffa));
				packet_append_data(raw, "2SV2", 4);
				packet_append_string(raw, anongame_infos_DESC_get_DESC(NULL, ladder_AT_2v2_desc));
				packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_AT_2v2));
				packet_append_data(raw, "3SV3", 4);
				packet_append_string(raw, anongame_infos_DESC_get_DESC(NULL, ladder_AT_3v3_desc));
				packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_AT_3v3));
				packet_append_data(raw, "4SV4", 4);
				packet_append_string(raw, anongame_infos_DESC_get_DESC(NULL, ladder_AT_4v4_desc));
				packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_AT_4v4));
				packet_append_data(raw, "SNLC", 4);
				packet_append_string(raw, anongame_infos_DESC_get_DESC(NULL, ladder_clan_1v1_desc));
				packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_clan_1v1));
				packet_append_data(raw, "2NLC", 4);
				packet_append_string(raw, anongame_infos_DESC_get_DESC(NULL, ladder_clan_2v2_desc));
				packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_clan_2v2));
				packet_append_data(raw, "3NLC", 4);
				packet_append_string(raw, anongame_infos_DESC_get_DESC(NULL, ladder_clan_3v3_desc));
				packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_clan_3v3));
				packet_append_data(raw, "4NLC", 4);
				packet_append_string(raw, anongame_infos_DESC_get_DESC(NULL, ladder_clan_4v4_desc));
				packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_clan_4v4));

				size = packet_get_size(raw);
				zlib_compress(packet_get_data_const(raw, 0, size), size, &anongame_infos->anongame_infos_data_war3->ladr_comp_data, &anongame_infos->anongame_infos_data_war3->ladr_comp_len);
				zlib_compress(packet_get_data_const(raw, 0, size), size, &anongame_infos->anongame_infos_data_w3xp->ladr_comp_data, &anongame_infos->anongame_infos_data_w3xp->ladr_comp_len);

				packet_destroy(raw);
			}

			if ((raw = packet_create(packet_class_raw)) != NULL)
			{
				t_anongame_infos_DESC *anongame_infos_DESC;
				t_anongame_infos_data_lang *anongame_infos_data_lang_war3;
				t_anongame_infos_data_lang *anongame_infos_data_lang_w3xp;
				char * langID;

				LIST_TRAVERSE(anongame_infos->anongame_infos_DESC_list, curr)
				{
					anongame_infos_DESC = (t_anongame_infos_DESC*)elem_get_data(curr);
					langID = anongame_infos_DESC->langID;
					anongame_infos_data_lang_war3 = anongame_infos_data_lang_init(langID);
					anongame_infos_data_lang_w3xp = anongame_infos_data_lang_init(langID);

					for (k = 0; k < 2; k++)
					{
						desc_count = 0;
						packet_set_size(raw, 0);
						for (j = 0; j < ANONGAME_TYPES; j++)
						if (maplists_get_totalmaps_by_queue(game_clienttag[k], j))
							desc_count++;
						packet_append_data(raw, &desc_count, 1);
						/* PG description section */
						for (j = 0; j < ANONGAME_TYPES; j++)
						if (!anongame_prefix[j][1] && !anongame_prefix[j][4] && maplists_get_totalmaps_by_queue(game_clienttag[k], j))
						{
							packet_append_data(raw, &anongame_PG_section, 1);
							packet_append_data(raw, &anongame_prefix[j][0], 1);

							if (anongame_infos_get_short_desc(langID, j) == NULL)
								packet_append_string(raw, anongame_gametype_names[j]);
							else
								packet_append_string(raw, anongame_infos_get_short_desc(langID, j));

							if (anongame_infos_get_long_desc(langID, j) == NULL)
								packet_append_string(raw, "No Descreption");
							else
								packet_append_string(raw, anongame_infos_get_long_desc(langID, j));
						}
						/* AT description section */
						for (j = 0; j < ANONGAME_TYPES; j++)
						if (!anongame_prefix[j][1] && anongame_prefix[j][4] && maplists_get_totalmaps_by_queue(game_clienttag[k], j))
						{
							packet_append_data(raw, &anongame_AT_section, 1);
							packet_append_data(raw, &anongame_prefix[j][0], 1);
							packet_append_string(raw, anongame_infos_get_short_desc(langID, j));
							packet_append_string(raw, anongame_infos_get_long_desc(langID, j));
						}
						/* TY description section */
						for (j = 0; j < ANONGAME_TYPES; j++)
						if (anongame_prefix[j][1] && maplists_get_totalmaps_by_queue(game_clienttag[k], j))
						{
							packet_append_data(raw, &anongame_TY_section, 1);
							packet_append_data(raw, &anongame_prefix[j][0], 1);
							packet_append_string(raw, anongame_infos_get_short_desc(langID, j));
							packet_append_string(raw, anongame_infos_get_long_desc(langID, j));
						}
						size = packet_get_size(raw);
						if (k == 0)
							zlib_compress(packet_get_data_const(raw, 0, size), size, &anongame_infos_data_lang_war3->desc_comp_data, &anongame_infos_data_lang_war3->desc_comp_len);
						else
							zlib_compress(packet_get_data_const(raw, 0, size), size, &anongame_infos_data_lang_w3xp->desc_comp_data, &anongame_infos_data_lang_w3xp->desc_comp_len);
					}

					packet_set_size(raw, 0);
					/*FIXME: Still adding a static number (10)
					   Also maybe need do do some checks to avoid prefs empty strings. */
					ladr_count = 10;
					packet_append_data(raw, &ladr_count, 1);
					packet_append_data(raw, "OLOS", 4);
					packet_append_string(raw, anongame_infos_DESC_get_DESC(langID, ladder_PG_1v1_desc));
					packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_PG_1v1));
					packet_append_data(raw, "MAET", 4);
					packet_append_string(raw, anongame_infos_DESC_get_DESC(langID, ladder_PG_team_desc));
					packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_PG_team));
					packet_append_data(raw, " AFF", 4);
					packet_append_string(raw, anongame_infos_DESC_get_DESC(langID, ladder_PG_ffa_desc));
					packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_PG_ffa));
					packet_append_data(raw, "2SV2", 4);
					packet_append_string(raw, anongame_infos_DESC_get_DESC(langID, ladder_AT_2v2_desc));
					packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_AT_2v2));
					packet_append_data(raw, "3SV3", 4);
					packet_append_string(raw, anongame_infos_DESC_get_DESC(langID, ladder_AT_3v3_desc));
					packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_AT_3v3));
					packet_append_data(raw, "4SV4", 4);
					packet_append_string(raw, anongame_infos_DESC_get_DESC(langID, ladder_AT_4v4_desc));
					packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_AT_4v4));
					packet_append_data(raw, "SNLC", 4);
					packet_append_string(raw, anongame_infos_DESC_get_DESC(langID, ladder_clan_1v1_desc));
					packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_clan_1v1));
					packet_append_data(raw, "2NLC", 4);
					packet_append_string(raw, anongame_infos_DESC_get_DESC(langID, ladder_clan_2v2_desc));
					packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_clan_2v2));
					packet_append_data(raw, "3NLC", 4);
					packet_append_string(raw, anongame_infos_DESC_get_DESC(langID, ladder_clan_3v3_desc));
					packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_clan_3v3));
					packet_append_data(raw, "4NLC", 4);
					packet_append_string(raw, anongame_infos_DESC_get_DESC(langID, ladder_clan_4v4_desc));
					packet_append_string(raw, anongame_infos_URL_get_URL(URL_ladder_clan_4v4));
					anongame_infos_data_lang_war3->ladr_len = packet_get_size(raw);
					anongame_infos_data_lang_war3->ladr_data = (char *)xmalloc(anongame_infos_data_lang_war3->ladr_len);
					std::memcpy(anongame_infos_data_lang_war3->ladr_data, packet_get_data_const(raw, 0, anongame_infos_data_lang_war3->ladr_len), anongame_infos_data_lang_war3->ladr_len);
					zlib_compress(anongame_infos_data_lang_war3->ladr_data, anongame_infos_data_lang_war3->ladr_len, &anongame_infos_data_lang_war3->ladr_comp_data, &anongame_infos_data_lang_war3->ladr_comp_len);
					list_append_data(anongame_infos->anongame_infos_data_lang_war3, anongame_infos_data_lang_war3);
					anongame_infos_data_lang_w3xp->ladr_len = packet_get_size(raw);
					anongame_infos_data_lang_w3xp->ladr_data = (char *)xmalloc(anongame_infos_data_lang_w3xp->ladr_len);
					std::memcpy(anongame_infos_data_lang_w3xp->ladr_data, packet_get_data_const(raw, 0, anongame_infos_data_lang_w3xp->ladr_len), anongame_infos_data_lang_w3xp->ladr_len);
					zlib_compress(anongame_infos_data_lang_w3xp->ladr_data, anongame_infos_data_lang_w3xp->ladr_len, &anongame_infos_data_lang_w3xp->ladr_comp_data, &anongame_infos_data_lang_w3xp->ladr_comp_len);
					list_append_data(anongame_infos->anongame_infos_data_lang_w3xp, anongame_infos_data_lang_w3xp);
				}
				packet_destroy(raw);
			}
			return 0;
		}

		extern int anongame_infos_unload(void)
		{
			return anongame_infos_destroy(anongame_infos);
		}

		static int zlib_compress(void const *src, int srclen, char **dest, int *destlen)
		{
			char *tmpdata;
			z_stream zcpr;
			int ret;
			unsigned lorigtodo;
			int lorigdone;
			int all_read_before;

			ret = Z_OK;
			lorigtodo = srclen;
			lorigdone = 0;
			*dest = NULL;

			tmpdata = (char *)xmalloc(srclen + (srclen / 0x10) + 0x200 + 0x8000);

			std::memset(&zcpr, 0, sizeof(z_stream));
			deflateInit(&zcpr, 9);
			zcpr.next_in = (Bytef*)src;
			zcpr.next_out = (Bytef*)tmpdata;
			do
			{
				all_read_before = zcpr.total_in;
				zcpr.avail_in = (lorigtodo < 0x8000) ? lorigtodo : 0x8000;
				zcpr.avail_out = 0x8000;
				ret = deflate(&zcpr, (zcpr.avail_in == lorigtodo) ? Z_FINISH : Z_SYNC_FLUSH);
				lorigdone += (zcpr.total_in - all_read_before);
				lorigtodo -= (zcpr.total_in - all_read_before);
			} while (ret == Z_OK);

			(*destlen) = zcpr.total_out;
			if ((*destlen) > 0)
			{
				(*dest) = (char*)xmalloc((*destlen) + 4);
				bn_short_set((bn_short *)(*dest), lorigdone);
				bn_short_set((bn_short *)(*dest + 2), *destlen);
				std::memcpy((*dest) + 4, tmpdata, (*destlen));
				(*destlen) += 4;
			}
			deflateEnd(&zcpr);

			xfree((void *)tmpdata);

			return 0;
		}

	}

}
