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

#ifndef INCLUDED_TOPIC_TYPES
#define INCLUDED_TOPIC_TYPES

#include "connection.h"

namespace pvpgn
{

	namespace bnetd
	{

		typedef struct s_topic
#ifdef TOPIC_INTERNAL_ACCESS
		{
			char *   channel_name;
			char *   topic;
			int      save;
		}
#endif
		t_topic;

	}

}

#define DO_SAVE_TOPIC 1
#define NO_SAVE_TOPIC 0

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_TOPIC_PROTOS
#define INCLUDED_TOPIC_PROTOS

namespace pvpgn
{

	namespace bnetd
	{

		int    topiclist_load(char const * topicfile);
		int    topiclist_unload(void);
		int    channel_set_topic(char const * channel_name, char const * topic_text, int do_save);
		char * channel_get_topic(char const * channel_name);
		int   channel_display_topic(t_connection * c, const char * channel_name);

	}

}

#endif
#endif
