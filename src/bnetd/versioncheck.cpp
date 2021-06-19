/*
 * Copyright (C) 2000 Onlyer (onlyer@263.net)
 * Copyright (C) 2001 Ross Combs (ross@bnetd.org)
 * Copyright (C) 2002 Gianluigi Tiesi (sherpya@netfarm.it)
 * Copyright (C) 2004 CreepLord (creeplord@pvpgn.org)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "common/setup_before.h"
#include "versioncheck.h"

#include <cctype>
#include <cerrno>
#include <chrono>
#include <climits>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <regex>
#include <sstream>
#include <string>
#include <tuple>
#include <unordered_map>

#include <fmt/format.h>
#include <nlohmann/json.hpp>

#include "common/eventlog.h"
#include "common/field_sizes.h"
#include "common/hash_tuple.hpp"
#include "common/proginfo.h"
#include "compat/strcasecmp.h"
#include "common/tag.h"
#include "common/token.h"
#include "common/util.h"

#include "prefs.h"
#include "common/setup_after.h"


using json = nlohmann::json;


namespace pvpgn
{

	namespace bnetd
	{
		std::unordered_map<std::tuple<std::uint32_t, std::uint32_t, t_tag, t_tag>, VersionCheck, hash_tuple::hash<std::tuple<std::uint32_t, std::uint32_t, t_tag, t_tag>>> vc_entries;
		std::unordered_map<std::tuple<t_tag, t_tag, std::uint32_t>, std::tuple<std::string, std::string>, hash_tuple::hash<std::tuple<t_tag, t_tag, std::uint32_t>>> cr_entries;

		bool versioncheck_conf_is_loaded = false;

		// bool validate_file_metadata_format(const std::string& metadata);


		bool load_versioncheck_conf(const std::string& filename)
		{
			if (versioncheck_conf_is_loaded)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Could not load {}, a versioncheck configuration file is already loaded", filename);
				return false;
			}

			auto t0 = std::chrono::steady_clock::now();

			std::ifstream file_stream(filename, std::ios::in);
			if (!file_stream.is_open())
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Could not open file \"{}\" for reading", filename);
				return false;
			}

			json jconf;
			try
			{
				file_stream >> jconf;
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "Could not parse {}: {}", filename, e.what());
				return false;
			}

			vc_entries.reserve(185);

			for (auto jclient : jconf.items())
			{
				t_tag client = tag_str_to_uint(jclient.key().c_str());
				if (!tag_check_client(client))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "Invalid client: {}", jclient.key());
					continue;
				}

				for (auto jarchitecture : jclient.value().items())
				{
					t_tag architecture = tag_str_to_uint(jarchitecture.key().c_str());
					if (!tag_check_arch(architecture))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "Invalid architecture: {}", jarchitecture.key());
						continue;
					}

					for (auto jversion_id : jarchitecture.value().items())
					{
						std::uint32_t version_id;
						try
						{
							version_id = std::stoul(jversion_id.key(), nullptr, 0);
						}
						catch (const std::exception& e)
						{
							eventlog(eventlog_level_error, __FUNCTION__, "Invalid version id \"{}\": {}", jversion_id.key(), e.what());
							continue;
						}

						// FIXME: do error checking for these
						std::string checkrevision_filename = jversion_id.value()["checkRevisionFile"].get<std::string>();
						std::string checkrevision_equation = jversion_id.value()["equation"].get<std::string>();

						cr_entries.emplace(
							std::piecewise_construct,
							std::forward_as_tuple(architecture, client, version_id), 
							std::forward_as_tuple(checkrevision_filename, checkrevision_equation)
						);

						for (auto jentry : jversion_id.value()["entries"])
						{
							try
							{
								VersionCheck entry(jentry["title"].get<std::string>(),
									version_id, 
									jentry["version"].get<std::string>(),
									jentry["hash"].get<std::string>(), 
									architecture,
									client,
									jentry["versionTag"].get<std::string>()
								);
								
								vc_entries.insert({ std::make_tuple(entry.m_version_id, entry.m_game_version, entry.m_architecture, entry.m_client), entry });
							}
							catch (const std::exception& e)
							{
								eventlog(eventlog_level_error, __FUNCTION__, "Failed to load versioncheck entry: {}", e.what());
								continue;
							}
						}
					}
				}
			}

			auto t1 = std::chrono::steady_clock::now();

			eventlog(eventlog_level_info, __FUNCTION__, "Successfully loaded {} versioncheck entries in {} microseconds", vc_entries.size(), std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count());

			versioncheck_conf_is_loaded = true;

			return true;
		}

		void unload_versioncheck_conf()
		{
			if (versioncheck_conf_is_loaded)
			{
				vc_entries.clear();

				cr_entries.clear();

				versioncheck_conf_is_loaded = false;

				eventlog(eventlog_level_info, __FUNCTION__, "Successfully unloaded all version check entries");
			}
		}

		std::tuple<std::string, std::string> select_checkrevision(t_tag architecture, t_tag client, std::uint32_t version_id)
		{
			static const std::tuple<std::string, std::string> default_checkrevision { "ver-IX86-1.mpq", "A=42 B=42 C=42 4 A=A^S B=B^B C=C^C A=A^S" };

			auto key = cr_entries.find(std::make_tuple(architecture, client, version_id));
			if (key != cr_entries.end())
			{
				return key->second;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "Could not find corresponding CheckRevision entry, returning default CheckRevision");

			return default_checkrevision;
		}

		// could use C++17 std::optional here
		const VersionCheck* select_versioncheck(t_tag architecture, t_tag client, std::uint32_t version_id,
			std::uint32_t checkrevision_version, std::uint32_t checkrevision_checksum)
		{
			auto it = vc_entries.find(std::make_tuple(version_id, checkrevision_version, architecture, client));
			if (it == vc_entries.end())
			{
				return nullptr;
			}

			if (it->second.m_checksum != checkrevision_checksum
				&& !prefs_get_allow_bad_version())
			{
				return nullptr;
			}

			return &(it->second);
		}

		VersionCheck::VersionCheck(const std::string& title, std::uint32_t version_id, const std::string& game_version,
			const std::string& checksum, t_tag architecture, t_tag client, const std::string& version_tag)
		: m_version_id(version_id), m_architecture(architecture), m_client(client)
		{
			if (verstr_to_vernum(game_version.c_str(), &this->m_game_version) < 0)
			{
				throw std::runtime_error("Invalid version \"" + game_version + "\" in entry \"" + title + "\"");
			}

			this->m_checksum = std::stoul(checksum, nullptr, 0);

			// FIXME: check for uniqueness
			this->m_version_tag = version_tag;
		}

		std::string VersionCheck::get_version_tag() const
		{
			return this->m_version_tag;
		}

		/*
		// This function only validates the format of the metadata string
		// There is no need to validate the information in the string because validating the checksum and file version is sufficient
		bool validate_file_metadata_format(const std::string& metadata)
		{
			if (metadata.empty())
			{
				return false;
			}

			try
			{
				std::smatch tokens;
				if (std::regex_match(metadata, tokens, std::regex{ R"(([[:print:]]+\.exe) (\d\d/\d\d/\d\d \d\d:\d\d:\d\d) (\d+))" }) == false)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got invalid file metadata \"{}\"", metadata);
					return false;
				}
			}
			catch (const std::regex_error& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "{} (code {})", e.what(), e.code());
				return false;
			}

			return true;
		}
		*/

	}

}
