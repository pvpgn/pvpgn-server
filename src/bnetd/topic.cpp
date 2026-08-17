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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "common/setup_before.h"
#include "topic.h"

#include <chrono>
#include <fstream>
#include <memory>
#include <new>
#include <regex>
#include <string>
#include <unordered_map>
#include <utility>

#include <nonstd/optional.hpp>
#include <nlohmann/json.hpp>

#include "compat/strcasecmp.h"

#include "common/eventlog.h"
#include "common/field_sizes.h"

#include "channel.h"
#include "i18n.h"
#include "message.h"
#include "prefs.h"

#include "common/setup_after.h"


using nlohmann::json;
using nonstd::optional;
using nonstd::nullopt;

namespace pvpgn
{

	namespace bnetd
	{
		static std::unordered_map<std::string, std::string> topic_head;

		static bool is_topic_conf_loaded = false;
		static std::string topic_conf_filename;

		bool load_topic_conf(const std::string& filename)
		{
			if (is_topic_conf_loaded)
			{
				eventlog(eventlog_level_warn, __FUNCTION__, "topic conf already loaded");
				return true;
			}

			auto t0 = std::chrono::steady_clock::now();

			try
			{
				std::ifstream topicfile_stream(filename);
				if (!topicfile_stream.is_open())
				{
					eventlog(eventlog_level_error, __FUNCTION__, "couldn't open topic file \"{}\"", filename);
					return false;
				}

				json jconf;
				topicfile_stream >> jconf;

				for (auto& entry : jconf.at("topics").items())
				{
					try
					{
						auto success = topic_head.emplace(entry.key(), entry.value());
						if (!success.second)
						{
							eventlog(eventlog_level_warn, __FUNCTION__, "failed to load topic for channel \"{}\" (possible duplicate?)", entry.key());
						}
					}
					catch (const std::exception& e)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not load topic for channel \"{}\"", entry.key());
						continue;
					}
				}
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "failed to load {} ({})", filename, e.what());
				return false;
			}

			auto t1 = std::chrono::steady_clock::now();

			eventlog(eventlog_level_info, __FUNCTION__, "Successfully loaded {} channel topics in {} milliseconds", topic_head.size(), std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count());

			topic_conf_filename = filename;

			is_topic_conf_loaded = true;

			return is_topic_conf_loaded;
		}

		void unload_topic_conf()
		{
			topiclist_save();

			topic_head.clear();

			topic_conf_filename.clear();

			is_topic_conf_loaded = false;

			eventlog(eventlog_level_info, __FUNCTION__, "Successfully unloaded all channel topics");
		}

		void topiclist_save()
		{
			if (!is_topic_conf_loaded)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "topic conf not loaded");
				return;
			}

			try
			{
				std::ofstream topicfile_stream(topic_conf_filename);
				if (!topicfile_stream.is_open())
				{
					eventlog(eventlog_level_error, __FUNCTION__, "couldn't open topic file");
					return;
				}

				json jconf;
				jconf["topics"] = topic_head;

				topicfile_stream << jconf.dump(1, '\t');
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "failed to save topic_head to {} ({})", topic_conf_filename, e.what());
				return;
			}
		}

		nonstd::optional<std::string> channel_get_topic(const t_channel* channel)
		{
			if (channel == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return nonstd::nullopt;
			}

			const char* channelname = channel_get_name(channel);
			if (channelname == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel name");
				return nonstd::nullopt;
			}

			try
			{
				auto search = topic_head.find(channelname);
				if (search == topic_head.end())
				{
					return nonstd::nullopt;
				}

				return nonstd::optional<std::string>{search->second};
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "failed to get channel topic for channel \"{}\"", channelname);

				return nonstd::nullopt;
			}
		}

		bool channel_set_topic(t_channel* channel, const std::string& topic)
		{
			if (channel == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return false;
			}

			const char* channelname = channel_get_name(channel);
			if (channelname == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel name");
				return false;
			}

			try
			{
				auto success = topic_head.emplace(channelname, topic);
				if (success.second == false)
				{
					// topic was not inserted because a topic for the channel already exists in topic_head

					success.first->second = topic;
				}

				return true;
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "failed to set channel topic for channel \"{}\"", channelname);

				return false;
			}
		}

		bool channel_display_topic(t_channel* channel, t_connection* conn)
		{
			if (channel == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL channel");
				return false;
			}

			if (conn == nullptr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL conn");
				return false;
			}

			try
			{
				nonstd::optional<std::string> topic = channel_get_topic(channel);
				if (topic.has_value())
				{
					const std::string delimiter = "\\n";
					const std::size_t delimiter_len = delimiter.length();
					const std::string topicstr = topic.value();

					// support "\n" (not '\n') by individually sending substrings
					std::size_t old_newline_pos = 0;
					std::size_t curr_newline_pos = topicstr.find(delimiter, old_newline_pos);
					while (curr_newline_pos != std::string::npos)
					{
						if (old_newline_pos == curr_newline_pos)
						{
							old_newline_pos += delimiter_len;
							curr_newline_pos = topicstr.find(delimiter, old_newline_pos);
							continue;
						}

						std::string substr = topicstr.substr(old_newline_pos, curr_newline_pos - old_newline_pos);
						message_send_text(conn, message_type_info, conn, substr);

						old_newline_pos = curr_newline_pos + delimiter_len;
						curr_newline_pos = topicstr.find(delimiter, old_newline_pos);
					}

					message_send_text(conn, message_type_info, conn, topicstr.substr(old_newline_pos, std::string::npos));
				}

				return true;
			}
			catch (const std::exception& e)
			{
				message_send_text(conn, message_type_error, conn, localize(conn, "An error has occurred."));

				const char* channelname = channel_get_name(channel);
				eventlog(eventlog_level_error, __FUNCTION__, "failed to display channel topic for channel \"{}\"", channelname ? channelname : "?");

				return false;
			}
		}

	}

}