/*
 * Copyright (C) 1998,1999  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
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
#include "file.h"

#include <cstring>
#include <cerrno>

#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/bnettime.h"
#include "common/packet.h"
#include "common/util.h"
#include "common/bn_type.h"
#include "common/tag.h"

#include "prefs.h"
#include "connection.h"
#include "i18n.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static char const * file_get_info(t_connection * c, char const * rawname, unsigned int * len, bn_long * modtime);

		/* Requested files aliases */
		const char * requestfiles[] = {
			"termsofservice-", "termsofservice.txt",
			"newaccount-", "newaccount.txt", // used in warcraft 3 after agree with TOS
			"chathelp-war3-", "chathelp-war3.txt",
			"matchmaking-war3-", "matchmaking-war3.dat", // FIXME: (HarpyWar) this file should be in files, not in i18n
			"tos_", "termsofservice.txt",
			"tos-unicode_", "termsofservice.txt",
			NULL, NULL };


		static const char * file_find_localized(t_connection * c, const char *rawname)
		{
			const char ** pattern, **alias;

			for (pattern = requestfiles, alias = requestfiles + 1; *pattern; pattern += 2, alias += 2)
			{
				// Check if there is an alias file available for this kind of file
				if (!std::strncmp(rawname, *pattern, std::strlen(*pattern)))
				{
					t_gamelang lang;
					if (lang = conn_get_gamelang_localized(c))
						return i18n_filename(*alias, lang);

					// FIXME: when file is transferring by bnftp protofol client doesn't provide a language
					// but we can extract it from the filename, so do it in next code

					// if there is no country tag in the file (just in case to prevent crash from invalid filename)
					if ((strlen(*pattern) + 4) > strlen(rawname))
						return NULL;

					// get language tag from the file name (like "termsofservice-ruRU.txt")
					// (it used in War3)
					char langstr[5];
					strncpy(langstr, rawname + std::strlen(*pattern), 4);
					langstr[4] = 0;
					lang = tag_str_to_uint(langstr);

					// if language is invalid then try find it by country (like "tos_USA.txt")
					// (it used in D1, SC, War2)
					if (!tag_check_gamelang(lang))
					{
						strncpy(langstr, rawname + std::strlen(*pattern), 3);
						langstr[3] = 0;
						lang = lang_find_by_country(langstr);
					}
					return i18n_filename(*alias, lang);
				}
			}
			// if not found return source file
			return NULL;
		}

		static char const * file_get_info(t_connection * c, char const * rawname, unsigned int * len, bn_long * modtime)
		{
			const char *filename;
			t_bnettime   bt;
			struct stat sfile;

			if (!rawname) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL rawname");
				return NULL;
			}

			if (!len) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL len");
				return NULL;
			}

			if (!modtime) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL modtime");
				return NULL;
			}

			if (std::strchr(rawname, '/') || std::strchr(rawname, '\\')) {
				eventlog(eventlog_level_warn, __FUNCTION__, "got rawname containing '/' or '\\' \"%s\"", rawname);
				return NULL;
			}


			filename = file_find_localized(c, rawname);
			// if localized file not found in "i18n"
			if (!filename || stat(filename, &sfile) < 0)
			{
				// try find it in "files"
				filename = buildpath(prefs_get_filedir(), rawname);
				if (stat(filename, &sfile) < 0) { /* try again */
					/* FIXME: check for lower-case version of filename */
					return NULL;
				}
			}

			*len = (unsigned int)sfile.st_size;
			bt = time_to_bnettime(sfile.st_mtime, 0);
			bnettime_to_bn_long(bt, modtime);

			return filename;
		}


		extern int file_to_mod_time(t_connection * c, char const * rawname, bn_long * modtime)
		{
			char const * filename;
			unsigned int len;

			if (!rawname)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL rawname");
				return -1;
			}
			if (!modtime)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL modtime");
				return -1;
			}

			if (!(filename = file_get_info(c, rawname, &len, modtime)))
				return -1;

			xfree((void *)filename); /* avoid warning */

			return 0;
		}


		/* Send a file.  If the file doesn't exist we still need to respond
		 * to the file request.  This will set filelen to 0 and send the server
		 * reply message and the client will be happy and not hang.
		 */
		extern int file_send(t_connection * c, char const * rawname, unsigned int adid, unsigned int etag, unsigned int startoffset, int need_header)
		{
			char const * filename;
			t_packet *   rpacket;
			std::FILE *       fp;
			unsigned int filelen;
			int          nbytes;

			if (!c)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL connection");
				return -1;
			}
			if (!rawname)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL rawname");
				return -1;
			}

			if (!(rpacket = packet_create(packet_class_file)))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not create file packet");
				return -1;
			}
			packet_set_size(rpacket, sizeof(t_server_file_reply));
			packet_set_type(rpacket, SERVER_FILE_REPLY);

			if ((filename = file_get_info(c, rawname, &filelen, &rpacket->u.server_file_reply.timestamp)))
			{
				if (!(fp = std::fopen(filename, "rb")))
				{
					/* FIXME: check for lower-case version of filename */
					eventlog(eventlog_level_error, __FUNCTION__, "stat() succeeded yet could not open file \"%s\" for reading (std::fopen: %s)", filename, std::strerror(errno));
					filelen = 0;
				}
			}
			else
			{
				fp = NULL;
				filelen = 0;
				bn_long_set_a_b(&rpacket->u.server_file_reply.timestamp, 0, 0);
			}

			if (fp)
			{
				if (startoffset < filelen) {
					std::fseek(fp, startoffset, SEEK_SET);
				}
				else {
					eventlog(eventlog_level_warn, __FUNCTION__, "[%d] startoffset is beyond end of file (%u>%u)", conn_get_socket(c), startoffset, filelen);
					/* Keep the real filesize. Battle.net does it the same way ... */
					std::fclose(fp);
					fp = NULL;
				}
			}

			if (need_header)
			{
				/* send the header from the server with the rawname and length. */
				bn_int_set(&rpacket->u.server_file_reply.filelen, filelen);
				bn_int_set(&rpacket->u.server_file_reply.adid, adid);
				bn_int_set(&rpacket->u.server_file_reply.extensiontag, etag);
				/* rpacket->u.server_file_reply.timestamp is set above */
				packet_append_string(rpacket, rawname);
				conn_push_outqueue(c, rpacket);
			}
			packet_del_ref(rpacket);

			/* Now send the data. Since it may be longer than a packet; we use
			 * the raw packet class.
			 */
			if (!fp)
			{
				eventlog(eventlog_level_warn, __FUNCTION__, "[%d] sending no data for file \"%s\" (\"%s\")", conn_get_socket(c), rawname, filename);
				return -1;
			}

			eventlog(eventlog_level_info, __FUNCTION__, "[%d] sending file \"%s\" (\"%s\") of length %d", conn_get_socket(c), rawname, filename, filelen);
			for (;;)
			{
				if (!(rpacket = packet_create(packet_class_raw)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not create raw packet");
					if (std::fclose(fp) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "could not close file \"%s\" after reading (std::fclose: %s)", filename, std::strerror(errno));
					return -1;
				}
				if ((nbytes = std::fread(packet_get_raw_data_build(rpacket, 0), 1, MAX_PACKET_SIZE, fp))<(int)MAX_PACKET_SIZE)
				{
					if (nbytes>0) /* send out last portion */
					{
						packet_set_size(rpacket, nbytes);
						conn_push_outqueue(c, rpacket);
					}
					packet_del_ref(rpacket);
					if (std::ferror(fp))
						eventlog(eventlog_level_error, __FUNCTION__, "read failed before EOF on file \"%s\" (std::fread: %s)", rawname, std::strerror(errno));
					break;
				}
				packet_set_size(rpacket, nbytes);
				conn_push_outqueue(c, rpacket);
				packet_del_ref(rpacket);
			}

			if (std::fclose(fp) < 0)
				eventlog(eventlog_level_error, __FUNCTION__, "could not close file \"%s\" after reading (std::fclose: %s)", rawname, std::strerror(errno));
			return 0;
		}

	}

}
