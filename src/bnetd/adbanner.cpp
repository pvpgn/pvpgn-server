/*
 * Copyright (C) 1999  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2005 Dizzy
 * Copyright (C) 2014 HarpyWar
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
#include "adbanner.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include "i18n.h"
#include <nlohmann/json.hpp>

#include "common/bn_type.h"
#include "common/tag.h"
#include "compat/strcasecmp.h"
#include "common/eventlog.h"
#include "connection.h"
#include "common/setup_after.h"


using json = nlohmann::json;


namespace pvpgn
{
	namespace bnetd
	{
		AdBannerSelector AdBannerList;
		std::default_random_engine engine{};

		bool AdBannerSelector::is_loaded() const noexcept
		{
			return this->m_loaded;
		}

		void AdBannerSelector::load(const std::string& filename)
		{
			if (this->is_loaded())
				throw std::runtime_error("An ad banner file is already loaded");

			std::ifstream file_stream(filename, std::ios::in);
			if (!file_stream.is_open())
				throw std::runtime_error("Could not open " + filename);

			json j;
			try
			{
				file_stream >> j;
			}
			catch (const std::invalid_argument& e)
			{
				throw std::runtime_error("Invalid JSON file: " + std::string(e.what()));
			}

			const std::map<std::string, std::string> extension_map = {
				{ "pcx", EXTENSIONTAG_PCX },
				{ "smk", EXTENSIONTAG_SMK },
				{ "mng", EXTENSIONTAG_MNG },
				{ "png", EXTENSIONTAG_MNG }
			};

			for (const auto& ad : j["ads"])
			{
				try
				{
					if (ad["filename"].get<std::string>().find('/') != std::string::npos
						|| ad["filename"].get<std::string>().find('\\') != std::string::npos)
						throw std::runtime_error("Paths are not supported (" + ad["filename"].get<std::string>() + ")");

					const std::size_t ext = ad["filename"].get<std::string>().find('.');
					if (ext == std::string::npos)
						throw std::runtime_error("Filename must contain an extension");

					const std::string file_extension(ad["filename"].get<std::string>().substr(ext + 1));
					bn_int extensiontag = {};
					for (const auto& m : extension_map)
					{
						if (strcasecmp(file_extension.c_str(), m.first.c_str()) == 0)
						{
							bn_int_tag_set(&extensiontag, m.second.c_str());
							break;
						}
					}
					if (extensiontag == 0)
					{
						throw std::runtime_error("Unsupported file extension (" + file_extension + ")");
					}

					t_clienttag ctag = 0;
					if (strcasecmp(ad["client"].get<std::string>().c_str(), "NULL") != 0)
					{
						ctag = clienttag_str_to_uint(ad["client"].get<std::string>().c_str());

						if (std::strcmp(clienttag_uint_to_str(ctag), "UNKN") == 0)
							throw std::runtime_error("Unknown client tag (" + ad["client"].get<std::string>() + ")");
					}

					t_gamelang ltag = 0;
					if (strcasecmp(ad["lang"].get<std::string>().c_str(), "NULL") != 0)
					{
						ltag = tag_str_to_uint(ad["lang"].get<std::string>().c_str());

						bool found;
						language_find_by_tag(ltag, found);
						if (!found)
							throw std::runtime_error("Unknown language (" + ad["lang"].get<std::string>() + ")");
					}

					this->m_banners.push_back(AdBanner(m_banners.size()+1, extensiontag, ad["filename"].get<std::string>(),
						ad["url"].get<std::string>(), ctag, ltag));
				}
				catch (const std::runtime_error& e)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "Could not load ad: {}", e.what());
					continue;
				}
			}

			this->m_loaded = true;
		}

		void AdBannerSelector::unload() noexcept
		{
			this->m_banners.clear();
			this->m_loaded = false;
		}

		std::size_t AdBannerSelector::size() const noexcept
		{
			return this->m_banners.size();
		}

		const AdBanner* const AdBannerSelector::pick(t_clienttag client_tag, t_gamelang client_lang, std::size_t prev_ad_id)
		{
			try
			{
				switch (this->m_banners.size())
				{
				case 0:
					return nullptr;
				case 1:
					return &this->m_banners.at(0);
				default:
				{
					bn_int ext;
					std::vector<AdBanner*> candidates = {};

					for (auto& ad : this->m_banners)
					{
						if ((ad.get_client() == client_tag || ad.get_client() == 0)
							&& (ad.get_language() == client_lang || ad.get_language() == 0))
						{

							bn_int_set(&ext, ad.get_extension_tag());
							// Warcraft 3
							if (client_tag == CLIENTTAG_WAR3XP_UINT || client_tag == CLIENTTAG_WARCRAFT3_UINT)
							{
								// ignore all formats except MNG for Warcraft 3, cause it's only one supported format
								if (bn_int_tag_eq(ext, EXTENSIONTAG_MNG) != 0)
								{
									continue;
								}
							}
							// Starcraft, Warcraft 2, Diablo, Diablo 2
							else
							{
								// ignore MNG, cause it's not supported format for other games
								if (bn_int_tag_eq(ext, EXTENSIONTAG_MNG) == 0)
								{
									continue;
								}
							}

							candidates.push_back(&ad);
						}
					}

					if (candidates.empty())
					{
						return nullptr;
					}


					unsigned int idx = 0;
					if (client_tag == CLIENTTAG_WAR3XP_UINT || client_tag == CLIENTTAG_WARCRAFT3_UINT)
					{
						// Warcraft 3 client always send prev_ad_id = 0, because of that we use random selection instead of sequence
						std::uniform_int_distribution<std::size_t> random(0, candidates.size() - 1);
						idx = random(engine);
					}
					else
					{
						// if prev_id in middle (for first and last idx=0)
						if (prev_ad_id != 0 && prev_ad_id != candidates.at(candidates.size() - 1)->get_id())
						{
							bool prev_found = false;
							for (unsigned int i = 0; i < candidates.size(); i++)
							{
								if (prev_found)
								{
									idx = i;
									break;
								}
								if (prev_ad_id == candidates.at(i)->get_id())
								{
									prev_found = true;
								}
							}
						}
					}

					/*
					char lang[5] = {};
					eventlog(eventlog_level_trace, __FUNCTION__, "Pick ad idx={} candidates={} id=0x{:08} prev_id=0x{:08} filename=\"{}\" link=\"{}\" client=\"{}\" lang=\"{}\"",
						idx, candidates.size(), ad->get_id(), prev_ad_id, ad->get_filename(), ad->get_url(), client_tag ? clienttag_uint_to_str(client_tag) : "NULL",
						client_lang ? tag_uint_to_str(lang, client_lang) : "NULL");
					*/

					const AdBanner* const ad = candidates.at(idx);
					return ad;
				}
				}
			}
			catch (const std::exception& e)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "error picking ad banner ({})", e.what());
				return nullptr;
			}
		}

		const AdBanner* const AdBannerSelector::find(t_clienttag client_tag, t_gamelang client_lang, std::size_t ad_id)
		{
			auto result = std::find_if(m_banners.begin(), m_banners.end(),
				[client_tag, client_lang, ad_id](const AdBanner& a) -> bool
			{
				return (a.get_client() == client_tag || a.get_client() == 0)
					&& (a.get_language() == client_lang || a.get_language() == 0)
					&& a.get_id() == ad_id;
			});

			return result != m_banners.end() ? &*result : nullptr;
		}

		/***************************************************************************************************/

		AdBanner::AdBanner(std::size_t id, bn_int extensiontag, const std::string& filename, const std::string& url, t_clienttag clienttag, t_gamelang language) 
			: m_id(id),
			m_extensiontag(bn_int_get(extensiontag)),
			m_filename(filename),
			m_url(url),
			m_client(clienttag),
			m_language(language)
		{
			char lang[5] = {};
			eventlog(eventlog_level_info, __FUNCTION__, "Created ad id=0x{:08} filename=\"{}\" link=\"{}\" client=\"{}\" lang=\"{}\"", 
				id, filename.c_str(), url.c_str(), clienttag ? clienttag_uint_to_str(clienttag) : "NULL", 
				language ? tag_uint_to_str(lang, language) : "NULL");
		}

		bool AdBanner::is_empty() const
		{
			return this->m_client == 0
				&& this->m_extensiontag == 0
				&& this->m_filename.empty()
				&& this->m_id == 0
				&& this->m_language == 0
				&& this->m_url.empty();
		}

		/* id starts from 1 */
		std::size_t AdBanner::get_id() const noexcept
		{
			return this->m_id;
		}

		std::string AdBanner::get_filename() const noexcept
		{
			return this->m_filename;
		}

		unsigned int AdBanner::get_extension_tag() const noexcept
		{
			return this->m_extensiontag;
		}

		std::string AdBanner::get_url() const noexcept
		{
			return this->m_url;
		}

		t_clienttag AdBanner::get_client() const noexcept
		{
			return this->m_client;
		}

		t_gamelang AdBanner::get_language() const noexcept
		{
			return this->m_language;
		}
	}
}
