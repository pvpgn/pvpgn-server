/*
 * Copyright (C) 2000 Alexey Belyaev (spider@omskart.ru)
 * Copyright (C) 2004 Dizzy
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
#define NEWS_INTERNAL_ACCESS
#include "news.h"

#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>

#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/tag.h"
#include "i18n.h"

#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static t_elist news_head;

		static int _news_parsetime(char *buff, struct std::tm *date, unsigned line)
		{
			char *p;

			date->tm_hour = 6;
			date->tm_min = 6;  // need to set non-zero values or else date is displayed wrong
			date->tm_sec = 6;
			date->tm_isdst = -1;

			if (!(p = std::strchr(buff, '/'))) return -1;
			*p = '\0';

			date->tm_mon = std::atoi(buff) - 1;
			if ((date->tm_mon<0) || (date->tm_mon>11)) {
				eventlog(eventlog_level_error, __FUNCTION__, "found invalid month (%i) in news date. (format: {MM/DD/YYYY}) on line %u", date->tm_mon, line);
			}

			buff = p + 1;
			if (!(p = std::strchr(buff, '/'))) return -1;
			*p = '\0';

			date->tm_mday = std::atoi(buff);
			if ((date->tm_mday<1) || (date->tm_mday>31)) {
				eventlog(eventlog_level_error, __FUNCTION__, "found invalid month day (%i) in news date. (format: {MM/DD/YYYY}) on line %u", date->tm_mday, line);
				return -1;
			}

			buff = p + 1;
			if (!(p = std::strchr(buff, '}'))) return -1;
			*p = '\0';

			date->tm_year = std::atoi(buff) - 1900;
			if (date->tm_year > 137) //limited due to 32bit t_time
			{
				eventlog(eventlog_level_error, __FUNCTION__, "found invalid year (%i) (>2037) in news date.  on line %u", date->tm_year + 1900, line);
				return -1;
			}

			return 0;
		}

		static void _news_insert_index(t_news_index *ni, const char *buff, unsigned len, int date_set)
		{
			t_elist *curr;
			t_news_index *cni;

			elist_for_each(curr, &news_head) {
				cni = elist_entry(curr, t_news_index, list);
				if (cni->date <= ni->date) break;
			}

			if (curr != &news_head && cni->date == ni->date) {
				if (date_set == 1)
					eventlog(eventlog_level_warn, __FUNCTION__, "found another news item for same date, trying to join both");

				if ((lstr_get_len(&cni->body) + len + 2) > 1023)
					eventlog(eventlog_level_error, __FUNCTION__, "failed in joining news, cause news too long - skipping");
				else {
					lstr_set_str(&cni->body, (char*)xrealloc(lstr_get_str(&cni->body), lstr_get_len(&cni->body) + len + 1 + 1));
					std::strcpy(lstr_get_str(&cni->body) + lstr_get_len(&cni->body), buff);
					*(lstr_get_str(&cni->body) + lstr_get_len(&cni->body) + len) = '\n';
					*(lstr_get_str(&cni->body) + lstr_get_len(&cni->body) + len + 1) = '\0';
					lstr_set_len(&cni->body, lstr_get_len(&cni->body) + len + 1);
				}
				xfree((void *)ni);
			}
			else {
				/* adding new index entry */
				lstr_set_str(&ni->body, (char*)xmalloc(len + 2));
				std::strcpy(lstr_get_str(&ni->body), buff);
				std::strcat(lstr_get_str(&ni->body), "\n");
				lstr_set_len(&ni->body, len + 1);
				elist_add_tail(curr, &ni->list);
			}
		}

		static void _news_insert_default(void)
		{
			const char * deftext = "No news today";
			t_news_index	*ni;

			ni = (t_news_index*)xmalloc(sizeof(t_news_index));
			ni->date = std::time(NULL);
			_news_insert_index(ni, deftext, std::strlen(deftext), 1);
		}

		extern int news_load(const char *filename)
		{
			std::FILE * 		fp;
			unsigned int	line;
			unsigned int	len;
			char		buff[256];
			struct std::tm		date;
			char		date_set;
			t_news_index	*ni;

			elist_init(&news_head);

			date_set = 0;

			if (!filename) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL fullname");
				return -1;
			}

			// FIXME: (HarpyWar) change news loading when user log on to send a localized version
			if ((fp = std::fopen(i18n_filename(filename, GAMELANG_ENGLISH_UINT), "rt")) == NULL) {
				eventlog(eventlog_level_warn, __FUNCTION__, "can't open news file");
				_news_insert_default();
				return 0;
			}

			for (line = 1; std::fgets(buff, sizeof(buff), fp); line++) {
				len = std::strlen(buff);
				while (len && (buff[len - 1] == '\n' || buff[len - 1] == '\r')) len--;
				if (!len) continue; /* empty line */
				buff[len] = '\0';

				if (buff[0] == '{') {
					if (_news_parsetime(buff + 1, &date, line)) {
						eventlog(eventlog_level_error, __FUNCTION__, "error parsing news date on line %u", line);
						return -1;
					}
					date_set = 1;
				}
				else {
					ni = (t_news_index*)xmalloc(sizeof(t_news_index));
					if (date_set)
						ni->date = std::mktime(&date);
					else {
						ni->date = std::time(NULL);
						eventlog(eventlog_level_warn, __FUNCTION__, "(first) news entry seems to be missing a timestamp, please check your news file on line %u", line);
					}
					_news_insert_index(ni, buff, len, date_set);
					date_set = 2;
				}
			}
			std::fclose(fp);

			if (elist_empty(&news_head)) {
				eventlog(eventlog_level_warn, __FUNCTION__, "no news configured");
				_news_insert_default();
			}

			return 0;
		}

		/* Free up all of the elements in the linked list */
		extern int news_unload(void)
		{
			t_elist *		curr, *save;
			t_news_index * 	ni;

			elist_for_each_safe(curr, &news_head, save)
			{
				ni = elist_entry(curr, t_news_index, list);
				elist_del(&ni->list);
				xfree((void *)lstr_get_str(&ni->body));
				xfree((void *)ni);
			}

			elist_init(&news_head);

			return 0;
		}

		extern unsigned int news_get_lastnews(void)
		{
			if (elist_empty(&news_head)) return 0;
			return ((elist_entry(news_head.next, t_news_index, list))->date);
		}

		extern unsigned int news_get_firstnews(void)
		{
			if (elist_empty(&news_head)) return 0;
			return ((elist_entry(news_head.prev, t_news_index, list))->date);
		}

		extern void news_traverse(t_news_cb cb, void *data)
		{
			t_elist *curr;
			t_news_index *cni;

			assert(cb);

			elist_for_each(curr, &news_head)
			{
				cni = elist_entry(curr, t_news_index, list);
				if (cb(cni->date, &cni->body, data)) break;
			}
		}

	}

}
