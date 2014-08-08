/*
 * Copyright (C) 2003,2004 Dizzy
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
#include "file_cdb.h"

#include <cstdio>
#include <cstring>

#include "common/eventlog.h"
#include "tinycdb/cdb.h"

#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		/* cdb file storage API functions */

		static int cdb_read_attrs(const char *filename, t_read_attr_func cb, void *data);
		static t_attr * cdb_read_attr(const char *filename, const char *key);
		static int cdb_write_attrs(const char *filename, const t_hlist *attributes);

		/* file_engine struct populated with the functions above */

		t_file_engine file_cdb = {
			cdb_read_attr,
			cdb_read_attrs,
			cdb_write_attrs
		};

		/* start of actual cdb file storage code */

		//#define CDB_ON_DEMAND	1

		static int cdb_write_attrs(const char *filename, const t_hlist *attributes)
		{
			std::FILE      *cdbfile;
			t_hlist	   *curr;
			t_attr         *attr;
			struct cdb_make cdbm;

			if ((cdbfile = std::fopen(filename, "w+b")) == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "unable to open file \"%s\" for writing ", filename);
				return -1;
			}

			cdb_make_start(&cdbm, cdbfile);

			hlist_for_each(curr, attributes) {
				attr = hlist_entry(curr, t_attr, link);

				if (attr_get_key(attr) && attr_get_val(attr)) {
					if (std::strncmp("BNET\\CharacterDefault\\", attr_get_key(attr), 20) == 0) {
						eventlog(eventlog_level_debug, __FUNCTION__, "skipping attribute key=\"%s\"", attr_get_key(attr));
					}
					else {
						eventlog(eventlog_level_debug, __FUNCTION__, "saving attribute key=\"%s\" val=\"%s\"", attr_get_key(attr), attr_get_val(attr));
						if (cdb_make_add(&cdbm, attr_get_key(attr), std::strlen(attr_get_key(attr)), attr_get_val(attr), std::strlen(attr_get_val(attr))) < 0)
						{
							eventlog(eventlog_level_error, __FUNCTION__, "got error on cdb_make_add ('%s' = '%s')", attr_get_key(attr), attr_get_val(attr));
							cdb_make_finish(&cdbm); /* try to bail out nicely */
							std::fclose(cdbfile);
							return -1;
						}
					}
				}
				else eventlog(eventlog_level_error, __FUNCTION__, "could not save attribute key=\"%s\"", attr_get_key(attr));

				attr_clear_dirty(attr);
			}

			if (cdb_make_finish(&cdbm) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got error on cdb_make_finish");
				std::fclose(cdbfile);
				return -1;
			}

			if (std::fclose(cdbfile) < 0) {
				eventlog(eventlog_level_error, __FUNCTION__, "got error on std::fclose()");
				return -1;
			}

			return 0;
		}

#ifndef CDB_ON_DEMAND
		/* code adapted from tinycdb-0.73/cdb.c */

		static int fget(std::FILE * fd, unsigned char *b, cdbi_t len, cdbi_t *posp, cdbi_t limit)
		{
			if (posp && limit - *posp < len) {
				eventlog(eventlog_level_error, __FUNCTION__, "invalid cdb database format");
				return -1;
			}

			if (std::fread(b, 1, len, fd) != len) {
				if (std::ferror(fd)) {
					eventlog(eventlog_level_error, __FUNCTION__, "got error reading from db file");
					return -1;
				}
				eventlog(eventlog_level_error, __FUNCTION__, "unable to read from cdb file, incomplete file");
				return -1;
			}

			if (posp) *posp += len;
			return 0;
		}

		static const char * fcpy(std::FILE *fd, cdbi_t len, cdbi_t *posp, cdbi_t limit, unsigned char * buf)
		{
			static char *str;
			static unsigned strl;
			unsigned int res = 0, no = 0;

			if (strl < len + 1) {
				char *tmp;

				tmp = (char*)xmalloc(len + 1);
				if (str) xfree((void*)str);
				str = tmp;
				strl = len + 1;
			}

			while (len - res > 0) {
				if (len > 2048) no = 2048;
				else no = len;

				if (fget(fd, buf, no, posp, limit)) return NULL;
				std::memmove(str + res, buf, no);
				res += no;
			}

			if (res > strl - 1) {
				eventlog(eventlog_level_error, __FUNCTION__, "BUG, this should not happen");
				return NULL;
			}

			str[res] = '\0';
			return str;
		}

		static int cdb_read_attrs(const char *filename, t_read_attr_func cb, void *data)
		{
			cdbi_t eod, klen, vlen;
			cdbi_t pos = 0;
			const char *key;
			const char *val;
			unsigned char buf[2048];
			std::FILE *f;

			if ((f = std::fopen(filename, "rb")) == NULL) {
				eventlog(eventlog_level_error, __FUNCTION__, "got error opening file '%s'", filename);
				return -1;
			}

			if (fget(f, buf, 2048, &pos, 2048)) goto err_fd;
			eod = cdb_unpack(buf);
			while (pos < eod) {
				if (fget(f, buf, 8, &pos, eod)) goto err_fd;
				klen = cdb_unpack(buf);
				vlen = cdb_unpack(buf + 4);
				if ((key = fcpy(f, klen, &pos, eod, buf)) == NULL) {
					eventlog(eventlog_level_error, __FUNCTION__, "error reading attribute key");
					goto err_fd;
				}

				key = xstrdup(key);

				if ((val = fcpy(f, vlen, &pos, eod, buf)) == NULL) {
					eventlog(eventlog_level_error, __FUNCTION__, "error reading attribute val");
					goto err_key;
				}

				//	eventlog(eventlog_level_trace, __FUNCTION__, "read atribute : '%s' -> '%s'", key, val);
				if (cb(key, val, data))
					eventlog(eventlog_level_error, __FUNCTION__, "got error from callback on account file '%s'", filename);
				xfree((void *)key);
			}

			std::fclose(f);
			return 0;

		err_key:
			xfree((void *)key);

		err_fd:
			std::fclose(f);
			return -1;
		}
#else /* CDB_ON_DEMAND */
		static int cdb_read_attrs(const char *filename, t_read_attr_func cb, void *data)
		{
			return 0;
		}
#endif

		static t_attr * cdb_read_attr(const char *filename, const char *key)
		{
#ifdef CDB_ON_DEMAND
			std::FILE	*cdbfile;
			t_attr	*attr;
			char	*val;
			unsigned	vlen = 1;

			//    eventlog(eventlog_level_trace, __FUNCTION__, "reading key '%s'", key);
			if ((cdbfile = std::fopen(filename, "rb")) == NULL) {
				//	eventlog(eventlog_level_debug, __FUNCTION__, "unable to open file \"%s\" for reading ",filename);
				return NULL;
			}

			if (cdb_seek(cdbfile, key, std::strlen(key), &vlen) <= 0) {
				//	eventlog(eventlog_level_debug, __FUNCTION__, "could not find key '%s'", key);
			std:; std::fclose(cdbfile);
				return NULL;
			}

			/* FIXME: use attr_* API */
			attr = xmalloc(sizeof(t_attr));
			attr->key = xstrdup(key);
			val = xmalloc(vlen + 1);

			cdb_bread(cdbfile, val, vlen);
			std::fclose(cdbfile);

			val[vlen] = '\0';

			attr->val = val;
			attr->dirty = 0;
			//    eventlog(eventlog_level_trace, __FUNCTION__, "read key '%s' value '%s'", attr->key, attr->val);
			return attr;
#else
			return NULL;
#endif
		}

	}

}
