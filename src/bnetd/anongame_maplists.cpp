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
#include "anongame_maplists.h"

#include <cstring>
#include <cstdio>

#include "common/packet.h"
#include "common/tag.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "prefs.h"
#include "common/setup_after.h"

#define MAXMAPS 100
#define MAXMAPS_PER_QUEUE 32 /* cannot be changed (map_prefs only supports 32 maps) */

/**********************************************************************************/
namespace pvpgn
{

	namespace bnetd
	{

		static char *	maplist_war3[MAXMAPS];
		static char *	maplist_w3xp[MAXMAPS];
		static char *	maplist_ral2[MAXMAPS];
		static char *	maplist_yuri[MAXMAPS];
		static int	number_maps_war3 = 0;
		static int	number_maps_w3xp = 0;
		static int	number_maps_ral2 = 0;
		static int	number_maps_yuri = 0;
		static char	maplists_war3[ANONGAME_TYPES][MAXMAPS_PER_QUEUE + 1];
		static char	maplists_w3xp[ANONGAME_TYPES][MAXMAPS_PER_QUEUE + 1];
		static char	maplists_ral2[ANONGAME_TYPES][MAXMAPS_PER_QUEUE + 1];
		static char	maplists_yuri[ANONGAME_TYPES][MAXMAPS_PER_QUEUE + 1];


		static int	_maplists_type_get_queue(const char * type);
		static const char * _maplists_queue_get_type(int queue);

		static void	_maplists_add_map(t_clienttag clienttag, char * mapname, int queue);

		/*****/
		static const char * queue_names[ANONGAME_TYPES] = {
			"1v1", "2v2", "3v3", "4v4", "sffa", "at2v2", "tffa", "at3v3", "at4v4",
			"TY",
			"5v5", "6v6", "2v2v2", "3v3v3", "4v4v4", "2v2v2v2", "3v3v3v3",
			"at2v2v2"
		};
		/**********************************************************************************/
		static int _maplists_type_get_queue(const char * type)
		{
			int i;

			for (i = 0; i < ANONGAME_TYPES; i++)
			if (std::strcmp(type, queue_names[i]) == 0)
				return i;

			return -1;
		}


		static const char * _maplists_queue_get_type(int queue)
		{
			return queue_names[queue];
		}

		static void _maplists_add_map(t_clienttag clienttag, char * mapname, int queue)
		{
			/* this function does two things
		 *	1. adds the mapname to maplist for MAPS section
		 *		maplist[mapnumber] = mapname
		 *		number_maps = total maps held in maplist[]
		 *	2. adds the mapnumber to maplists for TYPE section
		 *		maplists[queue][0] = number of maps for queue
		 *		maplists[queue][1..32] = mapnumber
		 *	ie. maplist[maplists[queue][1..32]] = mapname
		 */
			int in_list = 0;
			int j;
			char clienttag_str[5];

			if (clienttag == CLIENTTAG_WARCRAFT3_UINT) {
				for (j = 0; j < number_maps_war3; j++) {
					if (std::strcmp(maplist_war3[j], mapname) == 0) { /* already in list */
						in_list = 1;
						break;
					}
				}

				if (!in_list)
					maplist_war3[number_maps_war3++] = xstrdup(mapname);

				if (maplists_war3[queue][0] < MAXMAPS_PER_QUEUE) {
					maplists_war3[queue][0]++;
					maplists_war3[queue][(int)maplists_war3[queue][0]] = j;
				}
				else {
					eventlog(eventlog_level_error, __FUNCTION__,
						"cannot add map \"%s\" for gametype: %s (maxmaps per qametype: %d)",
						mapname, _maplists_queue_get_type(queue), MAXMAPS_PER_QUEUE);
				}
			}

			else if (clienttag == CLIENTTAG_WAR3XP_UINT) {
				for (j = 0; j < number_maps_w3xp; j++) {
					if (std::strcmp(maplist_w3xp[j], mapname) == 0) { /* already in list */
						in_list = 1;
						break;
					}
				}

				if (!in_list)
					maplist_w3xp[number_maps_w3xp++] = xstrdup(mapname);

				if (maplists_w3xp[queue][0] < MAXMAPS_PER_QUEUE) {
					maplists_w3xp[queue][0]++;
					maplists_w3xp[queue][(int)maplists_w3xp[queue][0]] = j;
				}
				else {
					eventlog(eventlog_level_error, __FUNCTION__,
						"cannot add map \"%s\" for gametype: %s (maxmaps per qametype: %d)",
						mapname, _maplists_queue_get_type(queue), MAXMAPS_PER_QUEUE);
				}
			}

			else if (clienttag == CLIENTTAG_REDALERT2_UINT) {
				for (j = 0; j < number_maps_ral2; j++) {
					if (std::strcmp(maplist_ral2[j], mapname) == 0) { /* already in list */
						in_list = 1;
						break;
					}
				}

				if (!in_list)
					maplist_ral2[number_maps_ral2++] = xstrdup(mapname);

				if (maplists_ral2[queue][0] < MAXMAPS_PER_QUEUE) {
					maplists_ral2[queue][0]++;
					maplists_ral2[queue][(int)maplists_ral2[queue][0]] = j;
				}
				else {
					eventlog(eventlog_level_error, __FUNCTION__,
						"cannot add map \"%s\" for gametype: %s (maxmaps per qametype: %d)",
						mapname, _maplists_queue_get_type(queue), MAXMAPS_PER_QUEUE);
				}
			}

			else if (clienttag == CLIENTTAG_YURISREV_UINT) {
				for (j = 0; j < number_maps_yuri; j++) {
					if (std::strcmp(maplist_yuri[j], mapname) == 0) { /* already in list */
						in_list = 1;
						break;
					}
				}

				if (!in_list)
					maplist_yuri[number_maps_yuri++] = xstrdup(mapname);

				if (maplists_yuri[queue][0] < MAXMAPS_PER_QUEUE) {
					maplists_yuri[queue][0]++;
					maplists_yuri[queue][(int)maplists_yuri[queue][0]] = j;
				}
				else {
					eventlog(eventlog_level_error, __FUNCTION__,
						"cannot add map \"%s\" for gametype: %s (maxmaps per qametype: %d)",
						mapname, _maplists_queue_get_type(queue), MAXMAPS_PER_QUEUE);
				}
			}

			else eventlog(eventlog_level_error, __FUNCTION__, "invalid clienttag: %s", tag_uint_to_str(clienttag_str, clienttag));
		}

		/**********************************************************************************/
		extern int anongame_maplists_create(void)
		{
			std::FILE *mapfd;
			char buffer[256];
			int len, i, queue;
			char *p, *q, *r, *u;

			if (prefs_get_mapsfile() == NULL) {
				eventlog(eventlog_level_error, "anongame_maplists_create", "invalid mapsfile, check your config");
				return -1;
			}

			if ((mapfd = std::fopen(prefs_get_mapsfile(), "rt")) == NULL) {
				eventlog(eventlog_level_error, "anongame_maplists_create", "could not open mapsfile : \"%s\"", prefs_get_mapsfile());
				return -1;
			}

			/* init the maps, they say static vars are 0-ed anyway but u never know :) */
			for (i = 0; i < ANONGAME_TYPES; i++) {
				maplists_war3[i][0] = 0;
				maplists_w3xp[i][0] = 0;
			}

			while (std::fgets(buffer, 256, mapfd)) {
				len = std::strlen(buffer);
				if (len < 1) continue;
				if (buffer[len - 1] == '\n') {
					buffer[len - 1] = '\0';
				}

				/* search for comments and comment them out */
				for (p = buffer; *p; p++)
				if (*p == '#') {
					*p = '\0';
					break;
				}

				/* skip spaces and/or tabs */
				for (p = buffer; *p && (*p == ' ' || *p == '\t'); p++); /* p = clienttag */
				if (*p == '\0') continue;

				/* find next delimiter */
				for (q = p; *q && *q != ' ' && *q != '\t'; q++);
				if (*q == '\0' || q - p != 4) continue; /* clienttag needs to have 4 chars */

				*q = '\0'; /* end of clienttag */

				/* skip spaces and/or tabs */
				for (q++; *q && (*q == ' ' || *q == '\t'); q++); /* q = type */
				if (*q == '\0') continue;

				/* find next delimiter */
				for (r = q + 1; *r && *r != ' ' && *r != '\t'; r++);

				*r = '\0'; /* end of type */

				/* skip spaces and/or tabs */
				for (r++; *r && (*r == ' ' || *r == '\t'); r++); /* r = mapname */
				if (*r == '\0') continue;

				if (*r != '\"') /* mapname without quotes */
					/* find next delimiter */
				for (u = r + 1; *u && *u != ' ' && *u != '\t'; u++);
				else /* mapname with quotes */
				{
					r++; /* skip quote */
					for (u = r + 1; *u && *u != '\"'; u++); /* find end quote or end of buffer */
					if (*u != '\"')
					{
						eventlog(eventlog_level_error, __FUNCTION__, "missing \" at the end of the map name, presume it's ok anyway");
					}
				}
				*u = '\0'; /* end of mapname */

				if ((queue = _maplists_type_get_queue(q)) < 0) continue; /* invalid queue */

				_maplists_add_map(tag_case_str_to_uint(p), r, queue);
			}
			std::fclose(mapfd);
			return 0;
		}

		/* used by the MAPS section */
		extern int maplists_get_totalmaps(t_clienttag clienttag)
		{
			switch (clienttag) {
			case CLIENTTAG_WARCRAFT3_UINT:
				return number_maps_war3;
			case CLIENTTAG_WAR3XP_UINT:
				return number_maps_w3xp;
			case CLIENTTAG_REDALERT2_UINT:
				return number_maps_ral2;
			case CLIENTTAG_YURISREV_UINT:
				return number_maps_yuri;
			default:
				ERROR0("Unknown clienttag");
				return 0;
			}
			return 0;
		}

		extern void maplists_add_maps_to_packet(t_packet * packet, t_clienttag clienttag)
		{
			int i;

			if (clienttag == CLIENTTAG_WARCRAFT3_UINT)
			for (i = 0; i < number_maps_war3; i++)
				packet_append_string(packet, maplist_war3[i]);

			if (clienttag == CLIENTTAG_WAR3XP_UINT)
			for (i = 0; i < number_maps_w3xp; i++)
				packet_append_string(packet, maplist_w3xp[i]);
		}

		/* used by TYPE section */
		extern int maplists_get_totalmaps_by_queue(t_clienttag clienttag, int queue)
		{
			switch (clienttag) {
			case CLIENTTAG_WARCRAFT3_UINT:
				return maplists_war3[queue][0];
			case CLIENTTAG_WAR3XP_UINT:
				return maplists_w3xp[queue][0];
			case CLIENTTAG_REDALERT2_UINT:
				return maplists_ral2[queue][0];
			case CLIENTTAG_YURISREV_UINT:
				return maplists_yuri[queue][0];
			default:
				ERROR0("Unknown clienttag");
				return 0;
			}
			return 0;
		}

		extern void maplists_add_map_info_to_packet(t_packet * rpacket, t_clienttag clienttag, int queue)
		{
			int i;

			if (clienttag == CLIENTTAG_WARCRAFT3_UINT) {
				for (i = 0; i < maplists_war3[queue][0] + 1; i++)
					packet_append_data(rpacket, &maplists_war3[queue][i], 1);
			}
			if (clienttag == CLIENTTAG_WAR3XP_UINT) {
				for (i = 0; i < maplists_w3xp[queue][0] + 1; i++)
					packet_append_data(rpacket, &maplists_w3xp[queue][i], 1);
			}
		}

		/* used by _get_map_from_prefs() */
		extern char * maplists_get_map(int queue, t_clienttag clienttag, int mapnumber)
		{
			switch (clienttag) {
			case CLIENTTAG_WARCRAFT3_UINT:
				return maplist_war3[(int)maplists_war3[queue][mapnumber]];
			case CLIENTTAG_WAR3XP_UINT:
				return maplist_w3xp[(int)maplists_w3xp[queue][mapnumber]];
			case CLIENTTAG_REDALERT2_UINT:
				return maplist_ral2[(int)maplists_ral2[queue][mapnumber]];
			case CLIENTTAG_YURISREV_UINT:
				return maplist_yuri[(int)maplists_yuri[queue][mapnumber]];
			default:
				ERROR0("Unknown clienttag");
				return NULL;
			}
		}

		extern void anongame_maplists_destroy()
		{
			int i;

			for (i = 0; i < MAXMAPS; i++) {
				if (maplist_war3[i])
					xfree((void *)maplist_war3[i]);
				if (maplist_w3xp[i])
					xfree((void *)maplist_w3xp[i]);
				if (maplist_ral2[i])
					xfree((void *)maplist_ral2[i]);
				if (maplist_yuri[i])
					xfree((void *)maplist_yuri[i]);
			}
		}

		extern int anongame_add_tournament_map(t_clienttag clienttag, char * mapname)
		{
			_maplists_add_map(clienttag, mapname, ANONGAME_TYPE_TY);
			return 0;
		}

		extern void anongame_tournament_maplists_destroy(void)
		{
			return; /* nothing to destroy */
		}

	}

}
