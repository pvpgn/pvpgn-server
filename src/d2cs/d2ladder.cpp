/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
 * Copyright (C) 2001		sousou	(liupeng.cs@263.net)
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
#include "setup.h"
#include "d2ladder.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include "compat/strcasecmp.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/tag.h"
#include "prefs.h"
#include "d2charfile.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace d2cs
	{

		static t_d2ladder	* ladder_data = NULL;
		static unsigned int	max_ladder_type = 0;

		static int d2ladderlist_create(unsigned int maxtype);
		static int d2ladder_create(unsigned int type, unsigned int len);
		static int d2ladder_append_ladder(unsigned int type, t_d2ladderfile_ladderinfo * info);
		static int d2ladder_readladder(void);

		extern int d2ladder_init(void)
		{
			if (d2ladder_readladder() < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "failed to initialize ladder data");
				return -1;
			}
			eventlog(eventlog_level_info, __FUNCTION__, "ladder data initialized");
			return 0;
		}

		static int d2ladder_readladder(void)
		{
			std::FILE				* fp;
			t_d2ladderfile_ladderindex	* ladderheader;
			t_d2ladderfile_ladderinfo	* ladderinfo;
			char				* ladderfile;
			t_d2ladderfile_header		header;
			unsigned int			i, n, temp, count, type, number;

			ladderfile = (char*)xmalloc(std::strlen(prefs_get_ladder_dir()) + 1 + std::strlen(LADDER_FILE_PREFIX) + 1 +
				std::strlen(CLIENTTAG_DIABLO2DV) + 1);
			std::sprintf(ladderfile, "%s/%s.%s", prefs_get_ladder_dir(), LADDER_FILE_PREFIX, CLIENTTAG_DIABLO2DV);
			if (!(fp = std::fopen(ladderfile, "rb"))) {
				eventlog(eventlog_level_error, __FUNCTION__, "error opening ladder file \"%s\" for reading (std::fopen: %s)", ladderfile, std::strerror(errno));
				xfree(ladderfile);
				return -1;
			}
			xfree(ladderfile);
			if (std::fread(&header, 1, sizeof(header), fp) != sizeof(header)) {
				eventlog(eventlog_level_error, __FUNCTION__, "error reading ladder file");
				std::fclose(fp);
				return -1;
			}
			max_ladder_type = bn_int_get(header.maxtype);
			if (d2ladderlist_create(max_ladder_type) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error create ladder list");
				std::fclose(fp);
				return -1;
			}
			temp = max_ladder_type * sizeof(*ladderheader);
			ladderheader = (t_d2ladderfile_ladderindex*)xmalloc(temp);
			if (std::fread(ladderheader, 1, temp, fp) != temp) {
				eventlog(eventlog_level_error, __FUNCTION__, "error read ladder file");
				xfree(ladderheader);
				std::fclose(fp);
				return -1;
			}
			for (i = 0, count = 0; i < max_ladder_type; i++) {
				type = bn_int_get(ladderheader[i].type);
				number = bn_int_get(ladderheader[i].number);
				if (d2ladder_create(type, number) < 0) {
					eventlog(eventlog_level_error, __FUNCTION__, "error create ladder %d", type);
					continue;
				}
				std::fseek(fp, bn_int_get(ladderheader[i].offset), SEEK_SET);
				temp = number * sizeof(*ladderinfo);
				ladderinfo = (t_d2ladderfile_ladderinfo*)xmalloc(temp);
				if (std::fread(ladderinfo, 1, temp, fp) != temp) {
					eventlog(eventlog_level_error, __FUNCTION__, "error read ladder file");
					xfree(ladderinfo);
					continue;
				}
				for (n = 0; n < number; n++) {
					d2ladder_append_ladder(type, ladderinfo + n);
				}
				xfree(ladderinfo);
				if (number) count++;
			}
			xfree(ladderheader);
			std::fclose(fp);
			eventlog(eventlog_level_info, __FUNCTION__, "ladder file loaded successfully (%d types %d maxtype)", count, max_ladder_type);
			return 0;
		}

		static int d2ladderlist_create(unsigned int maxtype)
		{
			ladder_data = (t_d2ladder*)xmalloc(maxtype * sizeof(*ladder_data));
			std::memset(ladder_data, 0, maxtype * sizeof(*ladder_data));
			return 0;
		}

		extern int d2ladder_refresh(void)
		{
			d2ladder_destroy();
			return d2ladder_readladder();
		}

		static int d2ladder_create(unsigned int type, unsigned int len)
		{
			if (type > max_ladder_type) {
				eventlog(eventlog_level_error, __FUNCTION__, "ladder type %d exceed max ladder type %d", type, max_ladder_type);
				return -1;
			}
			ladder_data[type].info = (t_d2cs_client_ladderinfo*)xmalloc(sizeof(t_d2cs_client_ladderinfo)* len);
			ladder_data[type].len = len;
			ladder_data[type].type = type;
			ladder_data[type].curr_len = 0;
			return 0;
		}

		static int d2ladder_append_ladder(unsigned int type, t_d2ladderfile_ladderinfo * info)
		{
			t_d2cs_client_ladderinfo	* ladderinfo;
			unsigned short			ladderstatus;
			unsigned short			status;
			unsigned char			chclass;

			if (!info) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL info");
				return -1;
			}
			if (type > max_ladder_type) {
				eventlog(eventlog_level_error, __FUNCTION__, "ladder type %d exceed max ladder type %d", type, max_ladder_type);
				return -1;
			}
			if (!ladder_data[type].info) {
				eventlog(eventlog_level_error, __FUNCTION__, "ladder data info not initialized");
				return -1;
			}
			if (ladder_data[type].curr_len >= ladder_data[type].len) {
				eventlog(eventlog_level_error, __FUNCTION__, "ladder data overflow %d > %d", ladder_data[type].curr_len, ladder_data[type].len);
				return -1;
			}
			status = bn_short_get(info->status);
			chclass = bn_byte_get(info->chclass);
			ladderstatus = (status & LADDERSTATUS_FLAG_DIFFICULTY);
			if (charstatus_get_hardcore(status)) {
				ladderstatus |= LADDERSTATUS_FLAG_HARDCORE;
				if (charstatus_get_dead(status)) {
					ladderstatus |= LADDERSTATUS_FLAG_DEAD;
				}
			}
			if (charstatus_get_expansion(status)) {
				ladderstatus |= LADDERSTATUS_FLAG_EXPANSION;
				ladderstatus |= std::min<unsigned>(chclass, D2CHAR_EXP_CLASS_MAX);
			}
			else {
				ladderstatus |= std::min<unsigned>(chclass, D2CHAR_CLASS_MAX);
			}
			ladderinfo = ladder_data[type].info + ladder_data[type].curr_len;
			bn_int_set(&ladderinfo->explow, bn_int_get(info->experience));
			bn_int_set(&ladderinfo->exphigh, 0);
			bn_short_set(&ladderinfo->status, ladderstatus);
			bn_byte_set(&ladderinfo->level, bn_int_get(info->level));
			bn_byte_set(&ladderinfo->u1, 0);
			std::strncpy(ladderinfo->charname, info->charname, MAX_CHARNAME_LEN);
			ladder_data[type].curr_len++;
			return 0;
		}

		extern int d2ladder_destroy(void)
		{
			unsigned int i;

			if (ladder_data) {
				for (i = 0; i < max_ladder_type; i++) {
					if (ladder_data[i].info) {
						xfree(ladder_data[i].info);
						ladder_data[i].info = NULL;
					}
				}
				xfree(ladder_data);
				ladder_data = NULL;
			}
			max_ladder_type = 0;
			return 0;
		}

		extern int d2ladder_get_ladder(unsigned int * from, unsigned int * count, unsigned int type,
			t_d2cs_client_ladderinfo const * * info)
		{
			t_d2ladder	* ladder;

			if (!ladder_data) return -1;
			ladder = ladder_data + type;
			if (!ladder->curr_len || !ladder->info) {
				eventlog(eventlog_level_warn, __FUNCTION__, "ladder type %d not found", type);
				return -1;
			}
			if (ladder->type != type) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad ladder data");
				return -1;
			}
			if (ladder->curr_len < *count) {
				*from = 0;
				*count = ladder->curr_len;
			}
			else if (*from + *count> ladder->curr_len) {
				*from = ladder->curr_len - *count;
			}
			*info = ladder->info + *from;
			return 0;
		}

		extern int d2ladder_find_character_pos(unsigned int type, char const * charname)
		{
			unsigned int	i;
			t_d2ladder	* ladder;

			if (!ladder_data) return -1;
			ladder = ladder_data + type;
			if (!ladder->curr_len || !ladder->info) {
				eventlog(eventlog_level_warn, __FUNCTION__, "ladder type %d not found", type);
				return -1;
			}
			if (ladder->type != type) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad ladder data");
				return -1;
			}
			for (i = 0; i < ladder->curr_len; i++) {
				if (!strcasecmp(ladder->info[i].charname, charname)) return i;
			}
			return -1;
		}

	}

}
