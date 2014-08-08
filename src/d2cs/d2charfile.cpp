/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
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
#include "d2charfile.h"

#include <algorithm>
#include <cstring>
#include <ctime>
#include <cstdio>
#include <cctype>
#include <cerrno>

#include "compat/access.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/d2char_checksum.h"
#include "common/xstring.h"
#include "prefs.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace d2cs
	{
		typedef enum
		{
			character_class_amazon,
			character_class_sorceress,
			character_class_necromancer,
			character_class_paladin,
			character_class_barbarian,
			character_class_druid,
			character_class_assassin
		} t_character_class;

		static int d2charsave_init(void * buffer, char const * charname, unsigned char chclass, unsigned short status);
		static int d2charsave_init_from_d2s(unsigned char * buffer, char const * charname, unsigned char chclass, unsigned short status, unsigned int size);
		static int d2charinfo_init(t_d2charinfo_file * chardata, char const * account, char const * charname,
			unsigned char chclass, unsigned short status);

		static int d2charsave_init(void * buffer, char const * charname, unsigned char chclass, unsigned short status)
		{
			ASSERT(buffer, -1);
			ASSERT(charname, -1);
			bn_byte_set((bn_byte *)((char *)buffer + D2CHARSAVE_CLASS_OFFSET), chclass);
			bn_short_set((bn_short *)((char *)buffer + D2CHARSAVE_STATUS_OFFSET), status);
			std::strncpy((char *)buffer + D2CHARSAVE_CHARNAME_OFFSET, charname, MAX_CHARNAME_LEN);
			return 0;
		}

		/* create new character from newbie.save that is normal d2s character file */
		static int d2charsave_init_from_d2s(unsigned char * buffer, char const * charname, unsigned char chclass, unsigned short status, unsigned int size)
		{
			unsigned int checksum;

			ASSERT(buffer, -1);
			ASSERT(charname, -1);

			// class
			bn_byte_set((bn_byte *)((char *)buffer + D2CHARSAVE_CLASS_OFFSET_109), chclass);

			// status (ladder, hardcore, expansion, etc)
			bn_short_set((bn_short *)((char *)buffer + D2CHARSAVE_STATUS_OFFSET_109), status);

			// charname
			std::strncpy((char *)buffer + D2CHARSAVE_CHARNAME_OFFSET_109, new char[MAX_CHARNAME_LEN], MAX_CHARNAME_LEN); // clear first
			std::strncpy((char *)buffer + D2CHARSAVE_CHARNAME_OFFSET_109, charname, MAX_CHARNAME_LEN); 

			// checksum
			checksum = d2charsave_checksum((unsigned char *)buffer, size, D2CHARSAVE_CHECKSUM_OFFSET);
			bn_int_set((bn_int *)(buffer + D2CHARSAVE_CHECKSUM_OFFSET), 0); // clear first
			bn_int_set((bn_int *)(buffer + D2CHARSAVE_CHECKSUM_OFFSET), checksum);

			return 0;
		}


		static int d2charinfo_init(t_d2charinfo_file * chardata, char const * account, char const * charname,
			unsigned char chclass, unsigned short status)
		{
			unsigned int		i;
			std::time_t		now;

			now = std::time(NULL);
			bn_int_set(&chardata->header.magicword, D2CHARINFO_MAGICWORD);
			bn_int_set(&chardata->header.version, D2CHARINFO_VERSION);
			bn_int_set(&chardata->header.create_time, now);
			bn_int_set(&chardata->header.last_time, now);
			bn_int_set(&chardata->header.total_play_time, 0);

			std::memset(chardata->header.charname, 0, MAX_CHARNAME_LEN);
			std::strncpy((char*)chardata->header.charname, charname, MAX_CHARNAME_LEN);
			std::memset(chardata->header.account, 0, MAX_USERNAME_LEN);
			std::strncpy((char*)chardata->header.account, account, MAX_USERNAME_LEN);
			std::memset(chardata->header.realmname, 0, MAX_REALMNAME_LEN);
			std::strncpy((char*)chardata->header.realmname, prefs_get_realmname(), MAX_REALMNAME_LEN);
			bn_int_set(&chardata->header.checksum, 0);
			for (i = 0; i < NELEMS(chardata->header.reserved); i++) {
				bn_int_set(&chardata->header.reserved[i], 0);
			}
			bn_int_set(&chardata->summary.charlevel, 1);
			bn_int_set(&chardata->summary.experience, 0);
			bn_int_set(&chardata->summary.charclass, chclass);
			bn_int_set(&chardata->summary.charstatus, status);

			std::memset(chardata->portrait.gfx, D2CHARINFO_PORTRAIT_PADBYTE, sizeof(chardata->portrait.gfx));
			std::memset(chardata->portrait.color, D2CHARINFO_PORTRAIT_PADBYTE, sizeof(chardata->portrait.color));
			std::memset(chardata->portrait.u2, D2CHARINFO_PORTRAIT_PADBYTE, sizeof(chardata->portrait.u2));
			std::memset(chardata->portrait.u1, D2CHARINFO_PORTRAIT_MASK, sizeof(chardata->portrait.u1));
			std::memset(chardata->pad, 0, sizeof(chardata->pad));

			bn_short_set(&chardata->portrait.header, D2CHARINFO_PORTRAIT_HEADER);
			bn_byte_set(&chardata->portrait.status, status | D2CHARINFO_PORTRAIT_MASK);
			bn_byte_set(&chardata->portrait.chclass, chclass + 1);
			bn_byte_set(&chardata->portrait.level, 1);
			if (charstatus_get_ladder(status))
				bn_byte_set(&chardata->portrait.ladder, 1);
			else
				bn_byte_set(&chardata->portrait.ladder, D2CHARINFO_PORTRAIT_PADBYTE);
			bn_byte_set(&chardata->portrait.end, '\0');

			std::memset(chardata->pad, 0, sizeof(chardata->pad));

			return 0;
		}


		extern int d2char_create(char const * account, char const * charname, unsigned char chclass, unsigned short status)
		{
			t_d2charinfo_file	chardata;
			char			* savefile, *infofile;
			const char *			newbiefile;
			unsigned char			buffer[MAX_SAVEFILE_SIZE];
			unsigned int		size;
			unsigned int	version;
			int			ladder_time, now;
			std::FILE			* fp;


			int status_init = status;

			ASSERT(account, -1);
			ASSERT(charname, -1);
			if (chclass > D2CHAR_MAX_CLASS) chclass = 0;
			status &= D2CHARINFO_STATUS_FLAG_INIT_MASK;
			charstatus_set_init(status, 1);

			/*	We need to make sure we are creating the correct character (Classic or Expansion)
				for the type of game server we are running. If lod_realm = 1 then only Expansion
				characters can be created and if set to 0 then only Classic character can
				be created	*/

			if (!(prefs_get_lod_realm() == 2)) {
				if (prefs_get_lod_realm() && ((status & 0x20) != 0x20)) {
					eventlog(eventlog_level_warn, __FUNCTION__, "This Realm is for LOD Characters Only");
					return -1;
				}
				if (!prefs_get_lod_realm() && ((status & 0x20) != 0x0)) {
					eventlog(eventlog_level_warn, __FUNCTION__, "This Realm is for Classic Characters Only");
					return -1;
				}
			}

			/*	Once correct type of character is varified then continue with creation of character */

			if (!prefs_allow_newchar()) {
				eventlog(eventlog_level_warn, __FUNCTION__, "creation of new character is disabled");
				return -1;
			}
			if (d2char_check_charname(charname) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad character name \"%s\"", charname);
				return -1;
			}
			if (d2char_check_acctname(account) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad account name \"%s\"", account);
				return -1;
			}

			/* get character template file depending of it's class */
			switch ((t_character_class)chclass)
			{
				case character_class_amazon:
						newbiefile = prefs_get_charsave_newbie_amazon();
						break;
				case character_class_sorceress:
						newbiefile = prefs_get_charsave_newbie_sorceress();
						break;
				case character_class_necromancer:
						newbiefile = prefs_get_charsave_newbie_necromancer();
						break;
				case character_class_paladin:
						newbiefile = prefs_get_charsave_newbie_paladin();
						break;
				case character_class_barbarian:
						newbiefile = prefs_get_charsave_newbie_barbarian();
						break;
				case character_class_druid:
						newbiefile = prefs_get_charsave_newbie_druid();
						break;
				case character_class_assassin:
						newbiefile = prefs_get_charsave_newbie_assasin();
						break;
			}

			size = sizeof(buffer);
			if (file_read(newbiefile, buffer, &size) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error loading newbie save file");
				return -1;
			}
			if (size >= sizeof(buffer)) {
				eventlog(eventlog_level_error, __FUNCTION__, "newbie save file \"%s\" is corrupt (length %lu, expected <%lu)", newbiefile, (unsigned long)size, (unsigned long)sizeof(buffer));
				return -1;
			}

			savefile = (char*)xmalloc(std::strlen(prefs_get_charsave_dir()) + 1 + std::strlen(charname) + 1);
			d2char_get_savefile_name(savefile, charname);
			if ((fp = std::fopen(savefile, "rb"))) {
				eventlog(eventlog_level_warn, __FUNCTION__, "character save file \"%s\" for \"%s\" already exist", savefile, charname);
				std::fclose(fp);
				xfree(savefile);
				return -1;
			}

			infofile = (char*)xmalloc(std::strlen(prefs_get_charinfo_dir()) + 1 + std::strlen(account) + 1 + std::strlen(charname) + 1);
			d2char_get_infofile_name(infofile, account, charname);

			now = std::time(NULL);
			ladder_time = prefs_get_ladder_start_time();
			if ((ladder_time > 0) && (now < ladder_time))
				charstatus_set_ladder(status, 0);

			/* create from newbie.save or normal d2s template? */
			version = bn_int_get(buffer + D2CHARSAVE_VERSION_OFFSET);
			if (version >= 0x0000005C)
				d2charsave_init_from_d2s(buffer, charname, chclass, status_init, size);
			else
				d2charsave_init(buffer, charname, chclass, status);

			d2charinfo_init(&chardata, account, charname, chclass, status);

			if (file_write(infofile, &chardata, sizeof(chardata)) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error writing info file \"%s\"", infofile);
				std::remove(infofile);
				xfree(infofile);
				xfree(savefile);
				return -1;
			}

			if (file_write(savefile, buffer, size) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error writing save file \"%s\"", savefile);
				std::remove(infofile);
				std::remove(savefile);
				xfree(savefile);
				xfree(infofile);
				return -1;
			}
			xfree(savefile);
			xfree(infofile);
			eventlog(eventlog_level_info, __FUNCTION__, "character %s(*%s) class %d status 0x%X created", charname, account, chclass, status);
			return 0;
		}


		extern int d2char_find(char const * account, char const * charname)
		{
			char		* file;
			std::FILE		* fp;

			ASSERT(account, -1);
			ASSERT(charname, -1);
			file = (char*)xmalloc(std::strlen(prefs_get_charinfo_dir()) + 1 + std::strlen(account) + 1 + std::strlen(charname) + 1);
			d2char_get_infofile_name(file, account, charname);
			fp = std::fopen(file, "rb");
			xfree(file);
			if (fp) {
				std::fclose(fp);
				return 0;
			}
			return -1;
		}


		extern int d2char_convert(char const * account, char const * charname)
		{
			std::FILE			* fp;
			char			* file;
			unsigned char		buffer[MAX_SAVEFILE_SIZE];
			unsigned int		status_offset;
			unsigned char		status;
			unsigned int		charstatus;
			t_d2charinfo_file	charinfo;
			unsigned int		size;
			unsigned int		version;
			unsigned int		checksum;

			ASSERT(account, -1);
			ASSERT(charname, -1);

			/*	Playing with a expanstion char on a classic realm
				will cause the game server to crash, therefore
				I recommed setting allow_convert = 0 in the d2cs.conf
				We need to do this to prevent creating classic char
				and converting to expantion on a classic realm.
				LOD Char must be created on LOD realm	*/

			if (!prefs_get_allow_convert()) {
				eventlog(eventlog_level_info, __FUNCTION__, "Convert char has been disabled");
				return -1;
			}

			/*	Procedure is stopped here and returned if
				allow_convet = 0 in d2cs.conf */

			if (d2char_check_charname(charname) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad character name \"%s\"", charname);
				return -1;
			}
			if (d2char_check_acctname(account) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad account name \"%s\"", account);
				return -1;
			}
			file = (char*)xmalloc(std::strlen(prefs_get_charinfo_dir()) + 1 + std::strlen(account) + 1 + std::strlen(charname) + 1);
			d2char_get_infofile_name(file, account, charname);
			if (!(fp = std::fopen(file, "rb+"))) {
				eventlog(eventlog_level_error, __FUNCTION__, "unable to open charinfo file \"%s\" for reading and writing (std::fopen: %s)", file, std::strerror(errno));
				xfree(file);
				return -1;
			}
			xfree(file);
			if (std::fread(&charinfo, 1, sizeof(charinfo), fp) != sizeof(charinfo)) {
				eventlog(eventlog_level_error, __FUNCTION__, "error reading charinfo file for character \"%s\" (std::fread: %s)", charname, std::strerror(errno));
				std::fclose(fp);
				return -1;
			}
			charstatus = bn_int_get(charinfo.summary.charstatus);
			charstatus_set_expansion(charstatus, 1);
			bn_int_set(&charinfo.summary.charstatus, charstatus);

			status = bn_byte_get(charinfo.portrait.status);
			charstatus_set_expansion(status, 1);
			bn_byte_set(&charinfo.portrait.status, status);

			std::fseek(fp, 0, SEEK_SET); /* FIXME: check return */
			if (std::fwrite(&charinfo, 1, sizeof(charinfo), fp) != sizeof(charinfo)) {
				eventlog(eventlog_level_error, __FUNCTION__, "error writing charinfo file for character \"%s\" (std::fwrite: %s)", charname, std::strerror(errno));
				std::fclose(fp);
				return -1;
			}
			if (std::fclose(fp) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not close charinfo file for character \"%s\" after writing (std::fclose: %s)", charname, std::strerror(errno));
				return -1;
			}

			file = (char*)xmalloc(std::strlen(prefs_get_charsave_dir()) + 1 + std::strlen(charname) + 1);
			d2char_get_savefile_name(file, charname);
			if (!(fp = std::fopen(file, "rb+"))) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not open charsave file \"%s\" for reading and writing (std::fopen: %s)", file, std::strerror(errno));
				xfree(file);
				return -1;
			}
			xfree(file);
			size = std::fread(buffer, 1, sizeof(buffer), fp);
			if (!std::feof(fp)) {
				eventlog(eventlog_level_error, __FUNCTION__, "error reading charsave file for character \"%s\" (std::fread: %s)", charname, std::strerror(errno));
				std::fclose(fp);
				return -1;
			}
			version = bn_int_get(buffer + D2CHARSAVE_VERSION_OFFSET);
			if (version >= 0x0000005C) {
				status_offset = D2CHARSAVE_STATUS_OFFSET_109;
			}
			else {
				status_offset = D2CHARSAVE_STATUS_OFFSET;
			}
			status = bn_byte_get(buffer + status_offset);
			charstatus_set_expansion(status, 1);
			bn_byte_set((bn_byte *)(buffer + status_offset), status); /* FIXME: shouldn't abuse bn_*_set()... what's the best way to do this? */
			if (version >= 0x0000005C) {
				checksum = d2charsave_checksum(buffer, size, D2CHARSAVE_CHECKSUM_OFFSET);
				bn_int_set((bn_int *)(buffer + D2CHARSAVE_CHECKSUM_OFFSET), checksum); /* FIXME: shouldn't abuse bn_*_set()... what's the best way to do this? */
			}
			std::fseek(fp, 0, SEEK_SET); /* FIXME: check return */
			if (std::fwrite(buffer, 1, size, fp) != size) {
				eventlog(eventlog_level_error, __FUNCTION__, "error writing charsave file for character %s (std::fwrite: %s)", charname, std::strerror(errno));
				std::fclose(fp);
				return -1;
			}
			if (std::fclose(fp) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not close charsave file for character \"%s\" after writing (std::fclose: %s)", charname, std::strerror(errno));
				return -1;
			}
			eventlog(eventlog_level_info, __FUNCTION__, "character %s(*%s) converted to expansion", charname, account);
			return 0;
		}


		extern int d2char_delete(char const * account, char const * charname)
		{
			char		* file;

			ASSERT(account, -1);
			ASSERT(charname, -1);
			if (d2char_check_charname(charname) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad character name \"%s\"", charname);
				return -1;
			}
			if (d2char_check_acctname(account) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad account name \"%s\"", account);
				return -1;
			}

			/* charsave file */
			file = (char*)xmalloc(std::strlen(prefs_get_charinfo_dir()) + 1 + std::strlen(account) + 1 + std::strlen(charname) + 1);
			d2char_get_infofile_name(file, account, charname);
			if (std::remove(file) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "failed to delete charinfo file \"%s\" (std::remove: %s)", file, std::strerror(errno));
				xfree(file);
				return -1;
			}
			xfree(file);

			/* charinfo file */
			file = (char*)xmalloc(std::strlen(prefs_get_charsave_dir()) + 1 + std::strlen(charname) + 1);
			d2char_get_savefile_name(file, charname);
			if (std::remove(file) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "failed to delete charsave file \"%s\" (std::remove: %s)", file, std::strerror(errno));
			}
			xfree(file);

			/* bak charsave file */
			file = (char*)xmalloc(std::strlen(prefs_get_bak_charinfo_dir()) + 1 + std::strlen(account) + 1 + std::strlen(charname) + 1);
			d2char_get_bak_infofile_name(file, account, charname);
			if (access(file, F_OK) == 0) {
				if (std::remove(file) < 0) {
					eventlog(eventlog_level_error, __FUNCTION__, "failed to delete bak charinfo file \"%s\" (std::remove: %s)", file, std::strerror(errno));
				}
			}
			xfree(file);

			/* bak charinfo file */
			file = (char*)xmalloc(std::strlen(prefs_get_bak_charsave_dir()) + 1 + std::strlen(charname) + 1);
			d2char_get_bak_savefile_name(file, charname);
			if (access(file, F_OK) == 0) {
				if (std::remove(file) < 0) {
					eventlog(eventlog_level_error, __FUNCTION__, "failed to delete bak charsave file \"%s\" (std::remove: %s)", file, std::strerror(errno));
				}
			}
			xfree(file);

			eventlog(eventlog_level_info, __FUNCTION__, "character %s(*%s) deleted", charname, account);
			return 0;
		}


		extern int d2char_get_summary(char const * account, char const * charname, t_d2charinfo_summary * charinfo)
		{
			t_d2charinfo_file	data;

			ASSERT(account, -1);
			ASSERT(charname, -1);
			ASSERT(charinfo, -1);
			if (d2charinfo_load(account, charname, &data) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error loading character %s(*%s)", charname, account);
				return -1;
			}
			std::memcpy(charinfo, &data.summary, sizeof(data.summary));
			eventlog(eventlog_level_info, __FUNCTION__, "character %s difficulty %d expansion %d hardcore %d dead %d loaded", charname,
				d2charinfo_get_difficulty(charinfo), d2charinfo_get_expansion(charinfo),
				d2charinfo_get_hardcore(charinfo), d2charinfo_get_dead(charinfo));
			return 0;
		}


		extern int d2charinfo_load(char const * account, char const * charname, t_d2charinfo_file * data)
		{
			char			* file;
			int			size, ladder_time;

			if (d2char_check_charname(charname) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad character name \"%s\"", charname);
				return -1;
			}
			if (d2char_check_acctname(account) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad account name \"%s\"", account);
				return -1;
			}
			file = (char*)xmalloc(std::strlen(prefs_get_charinfo_dir()) + 1 + std::strlen(account) + 1 + std::strlen(charname) + 1);
			d2char_get_infofile_name(file, account, charname);
			size = sizeof(t_d2charinfo_file);
			if (file_read(file, data, (unsigned int*)&size) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error loading character file %s", file);
				xfree(file);
				return -1;
			}
			if (size != sizeof(t_d2charinfo_file)) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad charinfo file %s (length %d)", charname, size);
				xfree(file);
				return -1;
			}
			d2char_portrait_init(&data->portrait);
			if (d2charinfo_check(data) < 0) {
				xfree(file);
				return -1;
			}
			if (!(charstatus_get_ladder(bn_int_get(data->summary.charstatus)))) {
				bn_byte_set(&data->portrait.ladder, D2CHARINFO_PORTRAIT_PADBYTE);
				xfree(file);
				return 0;
			}
			ladder_time = prefs_get_ladder_start_time();
			if ((ladder_time > 0) && bn_int_get(data->header.create_time) < ladder_time) {
				char			buffer[MAX_SAVEFILE_SIZE];
				unsigned int		status_offset;
				unsigned char		status;
				unsigned int		charstatus;
				unsigned int		size;
				unsigned int		version;
				unsigned int		checksum;
				std::FILE			* fp;

				eventlog(eventlog_level_info, __FUNCTION__, "%s(*%s) was created in old ladder season, set to non-ladder", charname, account);
				if (!(fp = std::fopen(file, "wb"))) {
					eventlog(eventlog_level_error, __FUNCTION__, "charinfo file \"%s\" does not exist for account \"%s\"", file, account);
					xfree(file);
					return 0;
				}
				xfree(file);
				charstatus = bn_int_get(data->summary.charstatus);
				charstatus_set_ladder(charstatus, 0);
				bn_int_set(&data->summary.charstatus, charstatus);

				status = bn_byte_get(data->portrait.status);
				charstatus_set_ladder(status, 0);
				bn_byte_set(&data->portrait.status, status);
				bn_byte_set(&data->portrait.ladder, D2CHARINFO_PORTRAIT_PADBYTE);

				if (std::fwrite(data, 1, sizeof(*data), fp) != sizeof(*data)) {
					eventlog(eventlog_level_error, __FUNCTION__, "error writing charinfo file for character \"%s\" (std::fwrite: %s)", charname, std::strerror(errno));
					std::fclose(fp);
					return 0;
				}
				std::fclose(fp);

				file = (char*)xmalloc(std::strlen(prefs_get_charsave_dir()) + 1 + std::strlen(charname) + 1);
				d2char_get_savefile_name(file, charname);

				if (!(fp = std::fopen(file, "rb+"))) {
					eventlog(eventlog_level_error, __FUNCTION__, "could not open charsave file \"%s\" for reading and writing (std::fopen: %s)", file, std::strerror(errno));
					xfree(file);
					return 0;
				}
				xfree(file);
				size = std::fread(buffer, 1, sizeof(buffer), fp);
				if (!std::feof(fp)) {
					eventlog(eventlog_level_error, __FUNCTION__, "error reading charsave file for character \"%s\" (std::fread: %s)", charname, std::strerror(errno));
					std::fclose(fp);
					return 0;
				}
				version = bn_int_get((bn_basic*)(buffer + D2CHARSAVE_VERSION_OFFSET));
				if (version >= 0x5C) {
					status_offset = D2CHARSAVE_STATUS_OFFSET_109;
				}
				else {
					status_offset = D2CHARSAVE_STATUS_OFFSET;
				}
				status = bn_byte_get((bn_basic*)(buffer + status_offset));
				charstatus_set_ladder(status, 0);
				/* FIXME: shouldn't abuse bn_*_set()... what's the best way to do this? */
				bn_byte_set((bn_byte *)(buffer + status_offset), status);
				if (version >= 0x5C) {
					checksum = d2charsave_checksum((unsigned char*)buffer, size, D2CHARSAVE_CHECKSUM_OFFSET);
					bn_int_set((bn_int *)(buffer + D2CHARSAVE_CHECKSUM_OFFSET), checksum);
				}
				std::fseek(fp, 0, SEEK_SET);
				if (std::fwrite(buffer, 1, size, fp) != size) {
					eventlog(eventlog_level_error, __FUNCTION__, "error writing charsave file for character %s (std::fwrite: %s)", charname, std::strerror(errno));
					std::fclose(fp);
					return 0;
				}
				std::fclose(fp);
			}
			else {
				bn_byte_set(&data->portrait.ladder, 1);
				xfree(file);
			}
			return 0;
		}

		extern int d2charinfo_check(t_d2charinfo_file * data)
		{
			ASSERT(data, -1);
			if (bn_int_get(data->header.magicword) != D2CHARINFO_MAGICWORD) {
				eventlog(eventlog_level_error, __FUNCTION__, "info data check failed (header 0x%08X)", bn_int_get(data->header.magicword));
				return -1;
			}
			if (bn_int_get(data->header.version) != D2CHARINFO_VERSION) {
				eventlog(eventlog_level_error, __FUNCTION__, "info data check failed (version 0x%08X)", bn_int_get(data->header.version));
				return -1;
			}
			return 0;
		}


		extern int d2char_portrait_init(t_d2charinfo_portrait * portrait)
		{
			unsigned int		i;
			unsigned char	* p;

			p = (unsigned char *)portrait;
			for (i = 0; i < sizeof(t_d2charinfo_portrait); i++) {
				if (!p[i]) p[i] = D2CHARINFO_PORTRAIT_PADBYTE;
			}
			p[i - 1] = '\0';
			return 0;
		}


		extern int d2char_get_portrait(char const * account, char const * charname, t_d2charinfo_portrait * portrait)
		{
			t_d2charinfo_file	data;

			ASSERT(charname, -1);
			ASSERT(account, -1);
			ASSERT(portrait, -1);
			if (d2charinfo_load(account, charname, &data) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "error loading character %s(*%s)", charname, account);
				return -1;
			}
			std::strcpy((char *)portrait, (char *)&data.portrait);
			return 0;
		}


		extern int d2char_check_charname(char const * name)
		{
			unsigned int	i;
			unsigned char	ch;

			if (!name) return -1;
			if (!std::isalpha((int)name[0])) return -1;

			for (i = 1; i <= MAX_CHARNAME_LEN; i++) {
				ch = name[i];
				if (ch == '\0') break;
				if (std::isalpha(ch)) continue;
				if (ch == '-') continue;
				if (ch == '_') continue;
				if (ch == '.') continue;
				return -1;
			}
			if (i >= MIN_CHARNAME_LEN || i <= MAX_CHARNAME_LEN) return 0;
			return -1;
		}


		extern int d2char_check_acctname(char const * name)
		{
			unsigned int	i;
			unsigned char	ch;

			if (!name) return -1;
			if (!std::isalnum((int)name[0])) return -1;

			for (i = 1; i <= MAX_CHARNAME_LEN; i++) {
				ch = name[i];
				if (ch == '\0') break;
				if (std::isalnum(ch)) continue;
				if (std::strchr(prefs_get_d2cs_account_allowed_symbols(), ch)) continue;
				return -1;
			}
			if (i >= MIN_USERNAME_LEN || i <= MAX_USERNAME_LEN) return 0;
			return -1;
		}


		extern int d2char_get_savefile_name(char * filename, char const * charname)
		{
			char	tmpchar[MAX_CHARNAME_LEN];

			ASSERT(filename, -1);
			ASSERT(charname, -1);
			std::strncpy(tmpchar, charname, sizeof(tmpchar));
			tmpchar[sizeof(tmpchar)-1] = '\0';
			strtolower(tmpchar);
			std::sprintf(filename, "%s/%s", prefs_get_charsave_dir(), tmpchar);
			return 0;
		}


		extern int d2char_get_bak_savefile_name(char * filename, char const * charname)
		{
			char	tmpchar[MAX_CHARNAME_LEN];

			ASSERT(filename, -1);
			ASSERT(charname, -1);
			std::strncpy(tmpchar, charname, sizeof(tmpchar));
			tmpchar[sizeof(tmpchar)-1] = '\0';
			strtolower(tmpchar);
			std::sprintf(filename, "%s/%s", prefs_get_bak_charsave_dir(), tmpchar);
			return 0;
		}


		extern int d2char_get_infodir_name(char * filename, char const * account)
		{
			char	tmpacct[MAX_USERNAME_LEN];

			ASSERT(filename, -1);
			ASSERT(account, -1);

			std::strncpy(tmpacct, account, sizeof(tmpacct));
			tmpacct[sizeof(tmpacct)-1] = '\0';
			strtolower(tmpacct);
			std::sprintf(filename, "%s/%s", prefs_get_charinfo_dir(), tmpacct);
			return 0;
		}


		extern int d2char_get_infofile_name(char * filename, char const * account, char const * charname)
		{
			char	tmpchar[MAX_CHARNAME_LEN];
			char	tmpacct[MAX_USERNAME_LEN];

			ASSERT(filename, -1);
			ASSERT(account, -1);
			ASSERT(charname, -1);
			std::strncpy(tmpchar, charname, sizeof(tmpchar));
			tmpchar[sizeof(tmpchar)-1] = '\0';
			strtolower(tmpchar);

			std::strncpy(tmpacct, account, sizeof(tmpacct));
			tmpchar[sizeof(tmpacct)-1] = '\0';
			strtolower(tmpacct);
			std::sprintf(filename, "%s/%s/%s", prefs_get_charinfo_dir(), tmpacct, tmpchar);
			return 0;
		}


		extern int d2char_get_bak_infofile_name(char * filename, char const * account, char const * charname)
		{
			char	tmpchar[MAX_CHARNAME_LEN];
			char	tmpacct[MAX_USERNAME_LEN];

			ASSERT(filename, -1);
			ASSERT(account, -1);
			ASSERT(charname, -1);
			std::strncpy(tmpchar, charname, sizeof(tmpchar));
			tmpchar[sizeof(tmpchar)-1] = '\0';
			strtolower(tmpchar);

			std::strncpy(tmpacct, account, sizeof(tmpacct));
			tmpchar[sizeof(tmpacct)-1] = '\0';
			strtolower(tmpacct);
			std::sprintf(filename, "%s/%s/%s", prefs_get_bak_charinfo_dir(), tmpacct, tmpchar);
			return 0;
		}


		extern unsigned int d2charinfo_get_ladder(t_d2charinfo_summary const * charinfo)
		{
			ASSERT(charinfo, 0);
			return charstatus_get_ladder(bn_int_get(charinfo->charstatus));
		}

		extern unsigned int d2charinfo_get_expansion(t_d2charinfo_summary const * charinfo)
		{
			ASSERT(charinfo, 0);
			return charstatus_get_expansion(bn_int_get(charinfo->charstatus));
		}


		extern unsigned int d2charinfo_get_level(t_d2charinfo_summary const * charinfo)
		{
			ASSERT(charinfo, 0);
			return bn_int_get(charinfo->charlevel);
		}


		extern unsigned int d2charinfo_get_class(t_d2charinfo_summary const * charinfo)
		{
			ASSERT(charinfo, 0);
			return bn_int_get(charinfo->charclass);
		}


		extern unsigned int d2charinfo_get_hardcore(t_d2charinfo_summary const * charinfo)
		{
			ASSERT(charinfo, 0);
			return charstatus_get_hardcore(bn_int_get(charinfo->charstatus));
		}


		extern unsigned int d2charinfo_get_dead(t_d2charinfo_summary const * charinfo)
		{
			ASSERT(charinfo, 0);
			return charstatus_get_dead(bn_int_get(charinfo->charstatus));
		}


		extern unsigned int d2charinfo_get_difficulty(t_d2charinfo_summary const * charinfo)
		{
			unsigned int	difficulty;

			ASSERT(charinfo, 0);
			if (d2charinfo_get_expansion(charinfo)) {
				difficulty = charstatus_get_difficulty_expansion(bn_int_get(charinfo->charstatus));
			}
			else {
				difficulty = charstatus_get_difficulty(bn_int_get(charinfo->charstatus));
			}
			if (difficulty > 2) difficulty = 2;
			return difficulty;
		}

		/* those functions should move to util.c */
		extern int file_read(char const * filename, void * data, unsigned int * size)
		{
			std::FILE		* fp;
			unsigned int	n;

			ASSERT(filename, -1);
			ASSERT(data, -1);
			ASSERT(size, -1);
			if (!(fp = std::fopen(filename, "rb"))) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for reading (std::fopen: %s)", filename, std::strerror(errno));
				return -1;
			}

			std::fseek(fp, 0, SEEK_END); /* FIXME: check return value */
			n = std::ftell(fp);
			n = std::min(*size, n);
			std::rewind(fp); /* FIXME: check return value */

			if (std::fread(data, 1, n, fp) != n) {
				eventlog(eventlog_level_error, __FUNCTION__, "error reading file \"%s\" (std::fread: %s)", filename, std::strerror(errno));
				std::fclose(fp);
				return -1;
			}
			if (std::fclose(fp) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not close file \"%s\" after reading (std::fclose: %s)", filename, std::strerror(errno));
				return -1;
			}
			*size = n;
			return 0;
		}


		extern int file_write(char const * filename, void * data, unsigned int size)
		{
			std::FILE		* fp;

			ASSERT(filename, -1);
			ASSERT(data, -1);
			ASSERT(size, -1);
			if (!(fp = std::fopen(filename, "wb"))) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for writing (std::fopen: %s)", filename, std::strerror(errno));
				return -1;
			}
			if (std::fwrite(data, 1, size, fp) != size) {
				eventlog(eventlog_level_error, __FUNCTION__, "error writing file \"%s\" (std::fwrite: %s)", filename, std::strerror(errno));
				std::fclose(fp);
				return -1;
			}
			if (std::fclose(fp) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not close file \"%s\" after writing (std::fclose: %s)", filename, std::strerror(errno));
				return -1;
			}
			return 0;
		}

	}

}
