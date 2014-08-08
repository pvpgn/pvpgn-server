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

#include "common/setup_before.h"
#include "output.h"

#include <cstdio>
#include <cerrno>
#include <cstring>

#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/tag.h"
#include "common/list.h"
#include "common/util.h"

#include "prefs.h"
#include "game.h"
#include "channel.h"
#include "connection.h"
#include "account.h"
#include "server.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		char * status_filename;

		int output_standard_writer(std::FILE * fp);

		/*
		 * Initialisation Output *
		 */

		extern void output_init(void)
		{
			eventlog(eventlog_level_info, __FUNCTION__, "initializing output file");

			if (prefs_get_XML_status_output())
				status_filename = buildpath(prefs_get_outputdir(), "server.xml");
			else
				status_filename = buildpath(prefs_get_outputdir(), "server.dat");

			return;
		}

		/*
		 * Write Functions *
		 */

		static int _glist_cb_xml(t_game *game, void *data)
		{
			char clienttag_str[5];

			std::fprintf((std::FILE*)data, "\t\t<game><id>%u</id><name>%s</name><clienttag>%s</clienttag></game>\n", game_get_id(game), game_get_name(game), tag_uint_to_str(clienttag_str, game_get_clienttag(game)));

			return 0;
		}

		static int _glist_cb_simple(t_game *game, void *data)
		{
			static int number;
			char clienttag_str[5];

			if (!data) {
				number = 1;
				return 0;
			}

			std::fprintf((std::FILE*)data, "game%d=%s,%u,%s\n", number, tag_uint_to_str(clienttag_str, game_get_clienttag(game)), game_get_id(game), game_get_name(game));
			number++;

			return 0;
		}

		int output_standard_writer(std::FILE * fp)
		{
			t_elem const	*curr;
			t_connection	*conn;
			t_channel const	*channel;
			t_game *game;

			char const		*channel_name;
			int			number;
			char		clienttag_str[5];
			int uptime = server_get_uptime();


			if (prefs_get_XML_status_output())
			{
				int seconds;
				int minutes;
				int hours;
				int days;

				days = (uptime / (60 * 60 * 24));
				hours = (uptime / (60 * 60)) % 24;
				minutes = (uptime / 60) % 60;
				seconds = uptime % 60;

				std::fprintf(fp, "<?xml version=\"1.0\"?>\n<status>\n");
				std::fprintf(fp, "\t\t<Version>%s</Version>\n", PVPGN_VERSION);
				std::fprintf(fp, "\t\t<Uptime>\n");
				std::fprintf(fp, "\t\t\t<Days>%d</Days>\n", days);
				std::fprintf(fp, "\t\t\t<Hours>%d</Hours>\n", hours);
				std::fprintf(fp, "\t\t\t<Minutes>%d</Minutes>\n", minutes);
				std::fprintf(fp, "\t\t\t<Seconds>%d</Seconds>\n", seconds);
				std::fprintf(fp, "\t\t</Uptime>\n");
				std::fprintf(fp, "\t\t<Users>\n");
				std::fprintf(fp, "\t\t<Number>%d</Number>\n", connlist_login_get_length());

				LIST_TRAVERSE_CONST(connlist(), curr)
				{
					conn = (t_connection*)elem_get_data(curr);
					if (conn_get_account(conn))
						std::fprintf(fp, "\t\t<user><name>%s</name><clienttag>%s</clienttag><version>%s</version>", conn_get_username(conn), tag_uint_to_str(clienttag_str, conn_get_clienttag(conn)), conn_get_clientver(conn));

					if ((game = conn_get_game(conn)))
						std::fprintf(fp, "<gameid>%u</gameid>", game_get_id(game));
					std::fprintf(fp, "</user>\n");
				}

				std::fprintf(fp, "\t\t</Users>\n");
				std::fprintf(fp, "\t\t<Games>\n");
				std::fprintf(fp, "\t\t<Number>%d</Number>\n", gamelist_get_length());

				gamelist_traverse(_glist_cb_xml, fp);

				std::fprintf(fp, "\t\t</Games>\n");
				std::fprintf(fp, "\t\t<Channels>\n");
				std::fprintf(fp, "\t\t<Number>%d</Number>\n", channellist_get_length());

				LIST_TRAVERSE_CONST(channellist(), curr)
				{
					channel = (t_channel*)elem_get_data(curr);
					channel_name = channel_get_name(channel);
					std::fprintf(fp, "\t\t<channel>%s</channel>\n", channel_name);
				}

				std::fprintf(fp, "\t\t</Channels>\n");
				std::fprintf(fp, "</status>\n");
				return 0;
			}
			else
			{
				std::fprintf(fp, "[STATUS]\nVersion=%s\nUptime=%s\nGames=%d\nUsers=%d\nChannels=%d\nUserAccounts=%d\n", PVPGN_VERSION, seconds_to_timestr(uptime), gamelist_get_length(), connlist_login_get_length(), channellist_get_length(), accountlist_get_length()); // Status
				std::fprintf(fp, "[CHANNELS]\n");
				number = 1;
				LIST_TRAVERSE_CONST(channellist(), curr)
				{
					channel = (t_channel*)elem_get_data(curr);
					channel_name = channel_get_name(channel);
					std::fprintf(fp, "channel%d=%s\n", number, channel_name);
					number++;
				}

				std::fprintf(fp, "[GAMES]\n");
				_glist_cb_simple(NULL, NULL);	/* init number */
				gamelist_traverse(_glist_cb_simple, fp);

				std::fprintf(fp, "[USERS]\n");
				number = 1;
				LIST_TRAVERSE_CONST(connlist(), curr)
				{
					conn = (t_connection*)elem_get_data(curr);
					if (conn_get_account(conn))
					{
						std::fprintf(fp, "user%d=%s,%s,%s", number, tag_uint_to_str(clienttag_str, conn_get_clienttag(conn)), conn_get_username(conn), conn_get_clientver(conn));

						if ((game = conn_get_game(conn)))
							std::fprintf(fp, ",%u", game_get_id(game));
						std::fprintf(fp, "\n");
						number++;
					}
				}

				return 0;
			}
		}

		extern int output_write_to_file(void)
		{
			std::FILE * fp;

			if (!status_filename)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
				return -1;
			}

			if (!(fp = std::fopen(status_filename, "w")))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for writing (std::fopen: %s)", status_filename, std::strerror(errno));
				return -1;
			}

			output_standard_writer(fp);
			std::fclose(fp);
			return 0;
		}

		extern void output_dispose_filename(void)
		{
			if (status_filename) xfree(status_filename);
		}

	}

}