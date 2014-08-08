/*
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

#include <cstring>
#include <cstdio>
#include <cerrno>

#include "compat/strncasecmp.h"
#include "compat/rename.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/tag.h"
#include "prefs.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace d2dbs
	{

		char			* d2ladder_ladder_file = NULL;
		char			* d2ladder_backup_file = NULL;

		t_d2ladderlist    	* d2ladder_list = NULL;
		unsigned long 		d2ladder_maxtype;
		int 			d2ladder_change_count = 0;
		int			d2ladder_need_rebuild = 0;
		const char              * XMLname = "d2ladder.xml";


		int d2ladderlist_init(void);
		int d2ladderlist_destroy(void);
		int d2ladder_empty(void);
		int d2ladder_initladderfile(void);

		t_d2ladder * d2ladderlist_find_type(unsigned int type);

		int d2ladder_check(void);
		int d2ladder_readladder(void);

		int d2ladder_insert(t_d2ladder * d2ladder, t_d2ladder_info * pcharladderinfo);
		int d2ladder_find_char_all(t_d2ladder * d2ladder, t_d2ladder_info * info);
		int d2ladder_find_pos(t_d2ladder * d2ladder, t_d2ladder_info * info);
		int d2ladder_update_info_and_pos(t_d2ladder * d2ladder, t_d2ladder_info * info, int oldpos, int newpos);
		int d2ladder_checksum(unsigned char const * data, unsigned int len, unsigned int offset);
		int d2ladder_checksum_set(void);
		int d2ladder_checksum_check(void);


		extern int d2ladder_update(t_d2ladder_info * pcharladderinfo)
		{
			t_d2ladder * d2ladder;
			unsigned short hardcore, expansion, status;
			unsigned char chclass;
			unsigned int ladder_overall_type, ladder_class_type;

			if (!pcharladderinfo->charname[0]) return 0;
			chclass = pcharladderinfo->chclass;
			status = pcharladderinfo->status;

			if (prefs_get_ladder_chars_only() && (!charstatus_get_ladder(status)))
				return -1;

			hardcore = charstatus_get_hardcore(status);
			expansion = charstatus_get_expansion(status);
			ladder_overall_type = 0;
			if (!expansion && chclass > D2CHAR_CLASS_MAX) return -1;
			if (expansion && chclass > D2CHAR_EXP_CLASS_MAX) return -1;
			if (hardcore && expansion) {
				ladder_overall_type = D2LADDER_EXP_HC_OVERALL;
			}
			else if (!hardcore && expansion) {
				ladder_overall_type = D2LADDER_EXP_STD_OVERALL;
			}
			else if (hardcore && !expansion) {
				ladder_overall_type = D2LADDER_HC_OVERALL;
			}
			else if (!hardcore && !expansion) {
				ladder_overall_type = D2LADDER_STD_OVERALL;
			}

			ladder_class_type = ladder_overall_type + chclass + 1;

			d2ladder = d2ladderlist_find_type(ladder_overall_type);
			if (d2ladder_insert(d2ladder, pcharladderinfo) == 1) {
				d2ladder_change_count++;
			}

			d2ladder = d2ladderlist_find_type(ladder_class_type);
			if (d2ladder_insert(d2ladder, pcharladderinfo) == 1) {
				d2ladder_change_count++;
			}
			return 0;
		}

		int d2ladder_initladderfile(void)
		{
			std::FILE * fdladder;
			t_d2ladderfile_ladderindex lhead[D2LADDER_MAXTYPE];
			t_d2ladderfile_header fileheader;
			int start;
			unsigned long maxtype;
			t_d2ladderfile_ladderinfo emptydata;
			unsigned int i, j, number;

			maxtype = D2LADDER_MAXTYPE;
			start = sizeof(t_d2ladderfile_header)+sizeof(lhead);
			for (i = 0; i<D2LADDER_MAXTYPE; i++) {
				bn_int_set(&lhead[i].type, i);
				bn_int_set(&lhead[i].offset, start);
				if (i == D2LADDER_HC_OVERALL ||
					i == D2LADDER_STD_OVERALL ||
					i == D2LADDER_EXP_HC_OVERALL ||
					i == D2LADDER_EXP_STD_OVERALL) {
					number = D2LADDER_OVERALL_MAXNUM;
				}
				else if ((i>D2LADDER_HC_OVERALL && i <= D2LADDER_HC_OVERALL + D2CHAR_CLASS_MAX + 1) ||
					(i > D2LADDER_STD_OVERALL && i <= D2LADDER_STD_OVERALL + D2CHAR_CLASS_MAX + 1) ||
					(i > D2LADDER_EXP_HC_OVERALL && i <= D2LADDER_EXP_HC_OVERALL + D2CHAR_EXP_CLASS_MAX + 1) ||
					(i > D2LADDER_EXP_STD_OVERALL && i <= D2LADDER_EXP_STD_OVERALL + D2CHAR_EXP_CLASS_MAX + 1)) {
					number = D2LADDER_MAXNUM;
				}
				else {
					number = 0;
				}
				bn_int_set(&lhead[i].number, number);
				start += number*sizeof(emptydata);
			}
			std::memset(&emptydata, 0, sizeof(emptydata));
			if (!d2ladder_ladder_file) return -1;
			fdladder = std::fopen(d2ladder_ladder_file, "wb");
			if (fdladder) {
				bn_int_set(&fileheader.maxtype, maxtype);
				bn_int_set(&fileheader.checksum, 0);
				std::fwrite(&fileheader, 1, sizeof(fileheader), fdladder);
				std::fwrite(lhead, 1, sizeof(lhead), fdladder);
				for (i = 0; i < maxtype; i++) {
					for (j = 0; j < bn_int_get(lhead[i].number); j++) {
						std::fwrite(&emptydata, 1, sizeof(emptydata), fdladder);
					}
				}
				std::fclose(fdladder);
				d2ladder_checksum_set();
			}
			else {
				eventlog(eventlog_level_error, __FUNCTION__, "error open ladder file %s", d2ladder_ladder_file);
				return -1;
			}
			return 0;
		}

		int d2ladder_find_pos(t_d2ladder * d2ladder, t_d2ladder_info * info)
		{
			int	i;

			if (!d2ladder || !info) return -1;

			// only allow if the experience threshold is reached
			if (info->experience < prefs_get_ladderupdate_threshold()) return -1;

			i = d2ladder->len;
			while (i--) {
				if (d2ladder->info[i].experience > info->experience) {
					if (strncasecmp(d2ladder->info[i].charname, info->charname, MAX_CHARNAME_LEN)) {
						i++;
					}
					break;
				}
				if (i <= 0) break;
			}
			return i;
		}


		int d2ladder_insert(t_d2ladder * d2ladder, t_d2ladder_info * info)
		{
			int	oldpos, newpos;

			newpos = d2ladder_find_pos(d2ladder, info);
			/* we currectly do nothing when character is being kick out of ladder for simple */
			/*
			if (newpos<0 || newpos >= d2ladder->len) return 0;
			*/
			oldpos = d2ladder_find_char_all(d2ladder, info);
			return d2ladder_update_info_and_pos(d2ladder, info, oldpos, newpos);
		}

		int d2ladder_find_char_all(t_d2ladder * d2ladder, t_d2ladder_info * info)
		{
			int		i;
			t_d2ladder_info * ladderdata;

			if (!d2ladder || !info) return -1;
			ladderdata = d2ladder->info;
			if (!ladderdata) return -1;
			i = d2ladder->len;
			while (i--) {
				if (!strncasecmp(ladderdata[i].charname, info->charname, MAX_CHARNAME_LEN)) return i;
			}
			return -1;
		}

		int d2ladder_update_info_and_pos(t_d2ladder * d2ladder, t_d2ladder_info * info, int oldpos, int newpos)
		{
			int	i;
			int	direction;
			int	outflag;
			t_d2ladder_info * ladderdata;

			if (!d2ladder || !info) return -1;
			ladderdata = d2ladder->info;
			if (!ladderdata) return -1;

			/* character not in ladder before */
			outflag = 0;
			if (oldpos < 0 || oldpos >= (signed)d2ladder->len) {
				oldpos = d2ladder->len - 1;
			}
			if (newpos < 0 || newpos >= (signed)d2ladder->len) {
				newpos = d2ladder->len - 1;
				outflag = 1;
			}
			if ((oldpos == (signed)d2ladder->len - 1) && outflag) {
				return 0;
			}
			if (newpos > oldpos && !outflag) newpos--;
			direction = (newpos > oldpos) ? 1 : -1;
			for (i = oldpos; i != newpos; i += direction) {
				ladderdata[i] = ladderdata[i + direction];
			}
			ladderdata[i] = *info;
			return 1;
		}


		extern int d2ladder_rebuild(void)
		{
			d2ladder_empty();
			d2ladder_need_rebuild = 0;
			return 0;
		}

		int d2ladder_check(void)
		{
			if (!d2ladder_ladder_file) return -1;
			if (!d2ladder_backup_file) return -1;
			if (d2ladder_checksum_check() != 1) {
				eventlog(eventlog_level_error, __FUNCTION__, "ladder file checksum error,try to use backup file");
				if (p_rename(d2ladder_backup_file, d2ladder_ladder_file) == -1) {
					eventlog(eventlog_level_error, __FUNCTION__, "error std::rename %s to %s", d2ladder_backup_file, d2ladder_ladder_file);
				}
				if (d2ladder_checksum_check() != 1) {
					eventlog(eventlog_level_error, __FUNCTION__, "ladder backup file checksum error,rebuild ladder");
					if (d2ladder_initladderfile() < 0) return -1;
					else {
						d2ladder_need_rebuild = 1;
						return 0;
					}
				}
			}
			return 1;
		}


		t_d2ladder * d2ladderlist_find_type(unsigned int type)
		{
			t_d2ladder *   d2ladder;
			t_elem const * elem;

			if (!d2ladder_list) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL d2ladder_list");
				return NULL;
			}

			LIST_TRAVERSE_CONST(d2ladder_list, elem)
			{
				if (!(d2ladder = (t_d2ladder*)elem_get_data(elem))) continue;
				if (d2ladder->type == type) return d2ladder;
			}
			eventlog(eventlog_level_error, __FUNCTION__, "could not find type %d in d2ladder_list", type);
			return NULL;
		}

		extern int d2dbs_d2ladder_init(void)
		{
			d2ladder_change_count = 0;
			d2ladder_maxtype = 0;
			d2ladder_ladder_file = (char*)xmalloc(std::strlen(d2dbs_prefs_get_ladder_dir()) + 1 +
				std::strlen(LADDER_FILE_PREFIX) + 1 + std::strlen(CLIENTTAG_DIABLO2DV) + 1 + 10);
			d2ladder_backup_file = (char*)xmalloc(std::strlen(d2dbs_prefs_get_ladder_dir()) + 1 +
				std::strlen(LADDER_BACKUP_PREFIX) + 1 + std::strlen(CLIENTTAG_DIABLO2DV) + 1 + 10);
			std::sprintf(d2ladder_ladder_file, "%s/%s.%s", d2dbs_prefs_get_ladder_dir(), \
				LADDER_FILE_PREFIX, CLIENTTAG_DIABLO2DV);

			std::sprintf(d2ladder_backup_file, "%s/%s.%s", d2dbs_prefs_get_ladder_dir(), \
				LADDER_BACKUP_PREFIX, CLIENTTAG_DIABLO2DV);

			if (d2ladderlist_init() < 0) {
				return -1;
			}
			if (d2ladder_check() < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "ladder file checking error");
				return -1;
			}
			if (d2ladder_readladder() < 0) {
				return -1;
			}
			if (d2ladder_need_rebuild) d2ladder_rebuild();
			d2ladder_saveladder();
			return 0;
		}

		int d2ladderlist_init(void)
		{
			t_d2ladder 	* d2ladder;
			unsigned int 	i;


			if (!d2ladder_ladder_file) return -1;
			d2ladder_list = list_create();
			d2ladder_maxtype = D2LADDER_MAXTYPE;
			for (i = 0; i < d2ladder_maxtype; i++) {
				d2ladder = (t_d2ladder*)xmalloc(sizeof(t_d2ladder));
				d2ladder->type = i;
				d2ladder->info = NULL;
				d2ladder->len = 0;
				list_append_data(d2ladder_list, d2ladder);
			}
			return 0;
		}

		int d2ladder_readladder(void)
		{
			t_d2ladder 		* d2ladder;
			t_d2ladderfile_header fileheader;
			std::FILE * 			fdladder;
			t_d2ladderfile_ladderindex 	* lhead;
			t_d2ladderfile_ladderinfo 	* ldata;
			t_d2ladder_info			* info;
			t_d2ladder_info			temp;
			long 			leftsize, blocksize;
			unsigned int		laddertype;
			unsigned int		tempmaxtype;
			int 			readlen;
			unsigned int			i, number;

			if (!d2ladder_ladder_file) return -1;
			fdladder = std::fopen(d2ladder_ladder_file, "rb");
			if (!fdladder) {
				eventlog(eventlog_level_error, __FUNCTION__, "canot open ladder file");
				return -1;
			}

			std::fseek(fdladder, 0, SEEK_END);
			leftsize = std::ftell(fdladder);
			std::rewind(fdladder);

			blocksize = sizeof(fileheader);
			if (leftsize < blocksize) {
				eventlog(eventlog_level_error, __FUNCTION__, "file size error");
				std::fclose(fdladder);
				return -1;
			}

			readlen = std::fread(&fileheader, 1, sizeof(fileheader), fdladder);
			if (readlen <= 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "file %s read error(read:%s)", d2ladder_ladder_file, std::strerror(errno));
				std::fclose(fdladder);
				return -1;
			}
			tempmaxtype = bn_int_get(fileheader.maxtype);
			leftsize -= blocksize;

			if (tempmaxtype > D2LADDER_MAXTYPE) {
				eventlog(eventlog_level_error, __FUNCTION__, "ladder type > D2LADDER_MAXTYPE error");
				std::fclose(fdladder);
				return -1;
			}
			d2ladder_maxtype = tempmaxtype;

			blocksize = d2ladder_maxtype*sizeof(*lhead);
			if (leftsize < blocksize) {
				eventlog(eventlog_level_error, __FUNCTION__, "file size error");
				std::fclose(fdladder);
				return -1;
			}

			lhead = (t_d2ladderfile_ladderindex*)xmalloc(blocksize);
			readlen = std::fread(lhead, 1, d2ladder_maxtype*sizeof(*lhead), fdladder);
			if (readlen <= 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "file %s read error(read:%s)", d2ladder_ladder_file, std::strerror(errno));
				xfree(lhead);
				std::fclose(fdladder);
				return -1;
			}
			leftsize -= blocksize;

			blocksize = 0;
			for (i = 0; i < d2ladder_maxtype; i++) {
				blocksize += bn_int_get(lhead[i].number)*sizeof(*ldata);
			}
			if (leftsize < blocksize) {
				eventlog(eventlog_level_error, __FUNCTION__, "file size error");
				xfree(lhead);
				std::fclose(fdladder);
				return -1;
			}

			for (laddertype = 0; laddertype < d2ladder_maxtype; laddertype++) {
				number = bn_int_get(lhead[laddertype].number);
				if (number <= 0) continue;
				d2ladder = d2ladderlist_find_type(laddertype);
				if (!d2ladder) {
					eventlog(eventlog_level_error, __FUNCTION__, "could not find ladder type %d", laddertype);
					continue;
				}
				ldata = (t_d2ladderfile_ladderinfo*)xmalloc(number*sizeof(*ldata));
				info = (t_d2ladder_info*)xmalloc(number * sizeof(*info));
				std::memset(info, 0, number * sizeof(*info));
				std::fseek(fdladder, bn_int_get(lhead[laddertype].offset), SEEK_SET);
				readlen = std::fread(ldata, 1, number*sizeof(*ldata), fdladder);
				if (readlen <= 0) {
					eventlog(eventlog_level_error, __FUNCTION__, "file %s read error(read:%s)", d2ladder_ladder_file, std::strerror(errno));
					xfree(ldata);
					xfree(info);
					continue;
				}
				d2ladder->info = info;
				d2ladder->len = number;
				for (i = 0; i < number; i++) {
					if (!ldata[i].charname[0]) continue;
					temp.experience = bn_int_get(ldata[i].experience);
					temp.status = bn_short_get(ldata[i].status);
					temp.level = bn_byte_get(ldata[i].level);
					temp.chclass = bn_byte_get(ldata[i].chclass);
					std::strncpy(temp.charname, ldata[i].charname, sizeof(info[i].charname));
					if (d2ladder_update_info_and_pos(d2ladder, &temp,
						d2ladder_find_char_all(d2ladder, &temp),
						d2ladder_find_pos(d2ladder, &temp)) == 1) {
						d2ladder_change_count++;
					}
				}
				xfree(ldata);
			}
			leftsize -= blocksize;

			xfree(lhead);
			std::fclose(fdladder);
			return 0;
		}

		extern int d2dbs_d2ladder_destroy(void)
		{
			unsigned int i;
			t_d2ladder * d2ladder;

			d2ladder_saveladder();
			for (i = 0; i < d2ladder_maxtype; i++) {
				d2ladder = d2ladderlist_find_type(i);
				if (d2ladder)
				{
					if (d2ladder->info)
						xfree(d2ladder->info);
					d2ladder->info = NULL;
					d2ladder->len = 0;
				}
			}
			d2ladderlist_destroy();
			if (d2ladder_ladder_file) {
				xfree(d2ladder_ladder_file);
				d2ladder_ladder_file = NULL;
			}
			if (d2ladder_backup_file) {
				xfree(d2ladder_backup_file);
				d2ladder_backup_file = NULL;
			}
			return 0;
		}

		int d2ladderlist_destroy(void)
		{
			t_d2ladder * d2ladder;
			t_elem *	elem;

			if (!d2ladder_list) return -1;
			LIST_TRAVERSE(d2ladder_list, elem)
			{
				if (!(d2ladder = (t_d2ladder*)elem_get_data(elem))) continue;
				xfree(d2ladder);
				list_remove_elem(d2ladder_list, &elem);
			}
			list_destroy(d2ladder_list);
			return 0;
		}

		int d2ladder_empty(void)
		{
			unsigned int i;
			t_d2ladder * d2ladder;

			for (i = 0; i < d2ladder_maxtype; i++) {
				d2ladder = d2ladderlist_find_type(i);
				if (d2ladder) {
					std::memset(d2ladder->info, 0, d2ladder->len * sizeof(*d2ladder->info));
				}
			}
			return 0;
		}

		const char * get_prefix(int type, int status, int chclass)
		{
			int  difficulty;
			static char prefix[4][4][2][16] =
			{ { { "", "" }, { "", "" }, { "", "" }, { "", "" } },

			{ { "Count", "Countess" }, { "Sir", "Dame" },
			{ "Destroyer", "Destroyer" }, { "Slayer", "Slayer" } },

			{ { "Duke", "Duchess" }, { "Lord", "Lady" },
			{ "Conqueror", "Conqueror" }, { "Champion", "Champion" } },

			{ { "King", "Queen" }, { "Baron", "Baroness" },
			{ "Guardian", "Guardian" }, { "Patriarch", "Matriarch" } } };

			static int sex[11] = { 0, 1, 1, 0, 0, 0, 0, 1, 0, 0, 0 };

			if (type == 0 || type == 1) // D2
				difficulty = charstatus_get_difficulty(status);
			else if (type == 2 || type == 3) // D2XP
				difficulty = charstatus_get_difficulty_expansion(status);

			return prefix[difficulty][type][sex[chclass]];
		}

		int d2ladder_print_XML(std::FILE *ladderstrm)
		{
			// modified version of d2ladder_print - changes done by jfro with a little help of aaron
			t_d2ladder * d2ladder;
			t_d2ladder_info * ldata;
			int overalltype, classtype;
			unsigned int i, type;
			char laddermode[4][20] = { "Hardcore", "Standard", "Expansion HC", "Expansion" };
			char charclass[11][12] = { "OverAll", "Amazon", "Sorceress", "Necromancer", "Paladin", \
				"Barbarian", "Druid", "Assassin", "", "", "" };

			std::fprintf(ladderstrm, "<?xml version=\"1.0\"?>\n<D2_ladders>\n");
			for (type = 0; type < d2ladder_maxtype; type++) {
				d2ladder = d2ladderlist_find_type(type);
				if (!d2ladder)
					continue;
				if (d2ladder->len <= 0)
					continue;
				ldata = d2ladder->info;

				overalltype = 0;
				classtype = 0;

				if (type <= D2LADDER_HC_OVERALL + D2CHAR_CLASS_MAX + 1)
				{
					overalltype = 0;
					classtype = type - D2LADDER_HC_OVERALL;
				}
				else if (type >= D2LADDER_STD_OVERALL && type <= D2LADDER_STD_OVERALL + D2CHAR_CLASS_MAX + 1)
				{
					overalltype = 1;
					classtype = type - D2LADDER_STD_OVERALL;
				}
				else if (type >= D2LADDER_EXP_HC_OVERALL && type <= D2LADDER_EXP_HC_OVERALL + D2CHAR_EXP_CLASS_MAX + 1)
				{
					overalltype = 2;
					classtype = type - D2LADDER_EXP_HC_OVERALL;
				}
				else if (type >= D2LADDER_EXP_STD_OVERALL && type <= D2LADDER_EXP_STD_OVERALL + D2CHAR_EXP_CLASS_MAX + 1)
				{
					overalltype = 3;
					classtype = type - D2LADDER_EXP_STD_OVERALL;
				}

				std::fprintf(ladderstrm, "<ladder>\n\t<type>%d</type>\n\t<mode>%s</mode>\n\t<class>%s</class>\n",
					type, laddermode[overalltype], charclass[classtype]);
				for (i = 0; i < d2ladder->len; i++)
				{
					if ((ldata[i].charname != NULL) && (ldata[i].charname[0] != '\0'))
					{
						std::fprintf(ladderstrm, "\t<char>\n\t\t<rank>%2d</rank>\n\t\t<name>%s</name>\n\t\t<level>%2d</level>\n",
							i + 1, ldata[i].charname, ldata[i].level);
						std::fprintf(ladderstrm, "\t\t<experience>%u</experience>\n\t\t<class>%s</class>\n",
							ldata[i].experience, charclass[ldata[i].chclass + 1]);
						std::fprintf(ladderstrm, "\t\t<prefix>%s</prefix>\n",
							get_prefix(overalltype, ldata[i].status, ldata[i].chclass + 1));
						if (((ldata[i].status) & (D2CHARINFO_STATUS_FLAG_DEAD | D2CHARINFO_STATUS_FLAG_HARDCORE)) ==
							(D2CHARINFO_STATUS_FLAG_DEAD | D2CHARINFO_STATUS_FLAG_HARDCORE))
							std::fprintf(ladderstrm, "\t\t<status>dead</status>\n\t</char>\n");
						else
							std::fprintf(ladderstrm, "\t\t<status>alive</status>\n\t</char>\n");
					}
				}
				std::fprintf(ladderstrm, "</ladder>\n");
				std::fflush(ladderstrm);
			}
			std::fprintf(ladderstrm, "</D2_ladders>\n");
			return 0;
		}

		extern int d2ladder_saveladder(void)
		{
			t_d2ladderfile_ladderindex	lhead[D2LADDER_MAXTYPE];
			t_d2ladderfile_header		fileheader;
			std::FILE				* fdladder;
			int				start;
			unsigned int			i, j, number;
			t_d2ladder			* d2ladder;
			t_d2ladderfile_ladderinfo	* ldata;
			char                            * XMLfilename;
			std::FILE                            * XMLfile;

			/*
				if(!d2ladder_change_count) {
				eventlog(eventlog_level_debug,__FUNCTION__,"ladder data unchanged, skip saving");
				return 0;
				}
				*/
			start = sizeof(fileheader)+sizeof(lhead);

			for (i = 0; i < D2LADDER_MAXTYPE; i++) {
				d2ladder = d2ladderlist_find_type(i);
				bn_int_set(&lhead[i].type, d2ladder->type);
				bn_int_set(&lhead[i].offset, start);
				bn_int_set(&lhead[i].number, d2ladder->len);
				start += d2ladder->len*sizeof(*ldata);
			}

			if (!d2ladder_ladder_file) return -1;
			if (!d2ladder_backup_file) return -1;

			if (d2ladder_checksum_check() == 1) {
				eventlog(eventlog_level_info, __FUNCTION__, "backup ladder file");
				if (p_rename(d2ladder_ladder_file, d2ladder_backup_file) == -1) {
					eventlog(eventlog_level_warn, __FUNCTION__, "error std::rename %s to %s", d2ladder_ladder_file, d2ladder_backup_file);
				}
			}

			fdladder = std::fopen(d2ladder_ladder_file, "wb");
			if (!fdladder) {
				eventlog(eventlog_level_error, __FUNCTION__, "error open ladder file %s", d2ladder_ladder_file);
				return -1;
			}

			// aaron: add extra output for XML ladder here --->
			if (d2dbs_prefs_get_XML_output_ladder())
			{
				XMLfilename = (char*)xmalloc(std::strlen(d2dbs_prefs_get_ladder_dir()) + 1 + std::strlen(XMLname) + 1);
				std::sprintf(XMLfilename, "%s/%s", d2dbs_prefs_get_ladder_dir(), XMLname);
				if (!(XMLfile = std::fopen(XMLfilename, "w")))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not open XML ladder file for output");
				}
				else
				{
					d2ladder_print_XML(XMLfile);
					std::fclose(XMLfile);
					xfree(XMLfilename);
				}
			}

			// <---

			bn_int_set(&fileheader.maxtype, d2ladder_maxtype);
			bn_int_set(&fileheader.checksum, 0);
			std::fwrite(&fileheader, 1, sizeof(fileheader), fdladder);
			std::fwrite(lhead, 1, sizeof(lhead), fdladder);
			for (i = 0; i < d2ladder_maxtype; i++) {
				number = bn_int_get(lhead[i].number);
				if (number <= 0) continue;
				d2ladder = d2ladderlist_find_type(i);
				ldata = (t_d2ladderfile_ladderinfo*)xmalloc(number * sizeof(*ldata));
				std::memset(ldata, 0, number * sizeof(*ldata));
				for (j = 0; j < number; j++) {
					bn_int_set(&ldata[j].experience, d2ladder->info[j].experience);
					bn_short_set(&ldata[j].status, d2ladder->info[j].status);
					bn_byte_set(&ldata[j].level, d2ladder->info[j].level);
					bn_byte_set(&ldata[j].chclass, d2ladder->info[j].chclass);
					std::strncpy(ldata[j].charname, d2ladder->info[j].charname, sizeof(ldata[j].charname));
				}
				std::fwrite(ldata, 1, number*sizeof(*ldata), fdladder);
				xfree(ldata);
			}
			std::fclose(fdladder);
			d2ladder_checksum_set();
			eventlog(eventlog_level_info, __FUNCTION__, "ladder file saved (%d changes)", d2ladder_change_count);
			d2ladder_change_count = 0;
			return 0;
		}

		int d2ladder_print(std::FILE *ladderstrm)
		{
			t_d2ladder * d2ladder;
			t_d2ladder_info * ldata;
			unsigned int i, type;
			int overalltype, classtype;
			char laddermode[4][20] = { "Hardcore", "Standard", "Expansion HC", "Expansion" };
			char charclass[11][12] = { "OverAll", "Amazon", "Sorceress", "Necromancer", "Paladin", \
				"Barbarian", "Druid", "Assassin", "", "", "" };

			for (type = 0; type < d2ladder_maxtype; type++) {
				d2ladder = d2ladderlist_find_type(type);
				if (!d2ladder)
					continue;
				if (d2ladder->len <= 0)
					continue;
				ldata = d2ladder->info;

				overalltype = 0;
				classtype = 0;

				if (type <= D2LADDER_HC_OVERALL + D2CHAR_CLASS_MAX + 1)
				{
					overalltype = 0;
					classtype = type - D2LADDER_HC_OVERALL;
				}
				else if (type >= D2LADDER_STD_OVERALL && type <= D2LADDER_STD_OVERALL + D2CHAR_CLASS_MAX + 1)
				{
					overalltype = 1;
					classtype = type - D2LADDER_STD_OVERALL;
				}
				else if (type >= D2LADDER_EXP_HC_OVERALL && type <= D2LADDER_EXP_HC_OVERALL + D2CHAR_EXP_CLASS_MAX + 1)
				{
					overalltype = 2;
					classtype = type - D2LADDER_EXP_HC_OVERALL;
				}
				else if (type >= D2LADDER_EXP_STD_OVERALL && type <= D2LADDER_EXP_STD_OVERALL + D2CHAR_EXP_CLASS_MAX + 1)
				{
					overalltype = 3;
					classtype = type - D2LADDER_EXP_STD_OVERALL;
				}

				std::fprintf(ladderstrm, "ladder type %d  %s %s\n", type, laddermode[overalltype], charclass[classtype]);
				std::fprintf(ladderstrm, "************************************************************************\n");
				std::fprintf(ladderstrm, "No    character name    level      std::exp       status   title   class     \n");
				for (i = 0; i < d2ladder->len; i++)
				{
					std::fprintf(ladderstrm, "NO.%2d  %-16s    %2d   %10d       %2X       %1X    %s\n",
						i + 1,
						ldata[i].charname,
						ldata[i].level,
						ldata[i].experience,
						ldata[i].status,
						1,
						charclass[ldata[i].chclass + 1]);
				}
				std::fprintf(ladderstrm, "************************************************************************\n");
				std::fflush(ladderstrm);
			}
			return 0;
		}

		int d2ladder_checksum(unsigned char const * data, unsigned int len, unsigned int offset)
		{
			int		checksum;
			unsigned int	i;
			unsigned int	ch;

			if (!data) return 0;
			checksum = 0;
			for (i = 0; i < len; i++) {
				ch = data[i];
				if (i >= offset && i < offset + sizeof(int)) ch = 0;
				ch += (checksum < 0);
				checksum = 2 * checksum + ch;
			}
			return checksum;
		}

		int d2ladder_checksum_set(void)
		{
			std::FILE		* fdladder;
			long		filesize;
			int		curlen, readlen, len;
			unsigned char * buffer;
			bn_int		checksum;

			if (!d2ladder_ladder_file) return -1;
			fdladder = std::fopen(d2ladder_ladder_file, "r+b");
			if (!fdladder) {
				eventlog(eventlog_level_error, __FUNCTION__, "error open ladder file %s", d2ladder_ladder_file);
				return -1;
			}
			std::fseek(fdladder, 0, SEEK_END);
			filesize = std::ftell(fdladder);
			std::rewind(fdladder);
			if (filesize == -1) {
				eventlog(eventlog_level_error, __FUNCTION__, "lseek() error in ladder file %s", d2ladder_ladder_file);
				std::fclose(fdladder);
				return -1;
			}
			if (filesize < (signed)sizeof(t_d2ladderfile_header)) {
				eventlog(eventlog_level_error, __FUNCTION__, "ladder file size error :%s", d2ladder_ladder_file);
				std::fclose(fdladder);
				return -1;
			}
			buffer = (unsigned char*)xmalloc(filesize);

			curlen = 0;
			while (curlen<filesize) {
				if (filesize - curlen > 2000)
					len = 2000;
				else
					len = filesize - curlen;
				readlen = std::fread(buffer + curlen, 1, len, fdladder);
				if (readlen <= 0) {
					xfree(buffer);
					std::fclose(fdladder);
					eventlog(eventlog_level_error, __FUNCTION__, "got bad save file or read error(read:%s)", std::strerror(errno));
					return -1;
				}
				curlen += readlen;
			}

			bn_int_set(&checksum, d2ladder_checksum(buffer, filesize, LADDERFILE_CHECKSUM_OFFSET));
			std::fseek(fdladder, LADDERFILE_CHECKSUM_OFFSET, SEEK_SET);
			std::fwrite(&checksum, 1, sizeof(checksum), fdladder);
			xfree(buffer);
			std::fclose(fdladder);
			return 0;
		}

		int d2ladder_checksum_check(void)
		{
			std::FILE		* fdladder;
			long		filesize;
			int		curlen, readlen, len;
			unsigned char	* buffer;
			int		checksum, oldchecksum;
			t_d2ladderfile_header	* header;

			if (!d2ladder_ladder_file) return -1;
			fdladder = std::fopen(d2ladder_ladder_file, "rb");
			if (!fdladder) {
				eventlog(eventlog_level_error, __FUNCTION__, "error open ladder file %s", d2ladder_ladder_file);
				return -1;
			}
			std::fseek(fdladder, 0, SEEK_END);
			filesize = std::ftell(fdladder);
			std::rewind(fdladder);
			if (filesize == -1) {
				eventlog(eventlog_level_error, __FUNCTION__, "lseek() error in  ladder file %s", d2ladder_ladder_file);
				std::fclose(fdladder);
				return -1;
			}
			if (filesize < (signed)sizeof(t_d2ladderfile_header)) {
				eventlog(eventlog_level_error, __FUNCTION__, "ladder file size error :%s", d2ladder_ladder_file);
				std::fclose(fdladder);
				return -1;
			}
			buffer = (unsigned char*)xmalloc(filesize);
			header = (t_d2ladderfile_header *)buffer;
			curlen = 0;
			while (curlen<filesize) {
				if (filesize - curlen > 2000)
					len = 2000;
				else
					len = filesize - curlen;
				readlen = std::fread(buffer + curlen, 1, len, fdladder);
				if (readlen <= 0) {
					xfree(buffer);
					std::fclose(fdladder);
					eventlog(eventlog_level_error, __FUNCTION__, "got bad save file or read error(read:%s)", std::strerror(errno));
					return -1;
				}
				curlen += readlen;
			}
			std::fclose(fdladder);

			oldchecksum = bn_int_get(header->checksum);
			checksum = d2ladder_checksum(buffer, filesize, LADDERFILE_CHECKSUM_OFFSET);
			xfree(buffer);

			if (oldchecksum == checksum) {
				eventlog(eventlog_level_info, __FUNCTION__, "ladder file check pass (checksum=0x%X)", checksum);
				return 1;
			}
			else {
				eventlog(eventlog_level_debug, __FUNCTION__, "ladder file checksum mismatch 0x%X - 0x%X", oldchecksum, checksum);
				return 0;
			}
		}

	}

}
