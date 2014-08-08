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

#ifndef INCLUDED_NEWS_TYPES
#define INCLUDED_NEWS_TYPES

#include <ctime>
#include "common/elist.h"
#include "common/lstr.h"

namespace pvpgn
{

	namespace bnetd
	{

		typedef struct news_index
#ifdef NEWS_INTERNAL_ACCESS
		{
			std::time_t 		date;
			t_lstr		body;
			t_elist		list;
		}
#endif
		t_news_index;

		typedef int(*t_news_cb)(std::time_t, t_lstr *, void *);

	}

}


#endif

/*****/
#ifndef JUST_NEED_TYPES
#ifndef NEWS_INTERNAL_ACCESS
#define NEWS_INTERNAL_ACCESS

namespace pvpgn
{

	namespace bnetd
	{

		extern int news_load(const char *filename);
		extern int news_unload(void);

		extern unsigned int news_get_firstnews(void);
		extern unsigned int news_get_lastnews(void);

		extern void news_traverse(t_news_cb cb, void *data);

	}

}

#endif
#endif
