/*
 * Copyright (C) 1999  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_ADBANNER_H
#define INCLUDED_ADBANNER_H

#include <string>
#include <map>

#include "common/tag.h"
#include "common/scoped_ptr.h"
#include "common/bn_type.h"

namespace pvpgn
{

	namespace bnetd
	{

		class AdBanner
		{
		public:
			AdBanner(unsigned id_, bn_int extag, unsigned delay_, unsigned next_, const std::string& fname, const std::string& link_, t_clienttag client_, t_gamelang lang_);
			~AdBanner() throw();

			unsigned getId() const;
			unsigned getNextId() const;
			unsigned getExtensionTag() const;
			char const * getFilename() const;
			char const * getLink() const;
			t_clienttag getClient() const;
			t_gamelang getGameLang() const;

		private:
			unsigned id;
			unsigned extensiontag;
			unsigned delay; /* in seconds */
			unsigned next; /* adid or 0 */
			const std::string filename;
			const std::string link;
			t_clienttag client;
			t_gamelang lang;
		};

		class AdBannerComponent
		{
		public:
			explicit AdBannerComponent(const std::string& fname);
			~AdBannerComponent() throw();

			typedef std::pair<t_clienttag, t_gamelang> AdKey;
			const AdBanner* pick(t_clienttag ctag, t_gamelang lang, unsigned prev_id) const;
			const AdBanner* find(t_clienttag ctag, t_gamelang lang, unsigned id) const;

		private:
			typedef std::map<unsigned, AdBanner> AdIdMap;
			typedef std::map<AdKey, AdIdMap> AdCtagMap;
			typedef std::map<unsigned, AdIdMap::const_iterator> AdIdRefMap;
			typedef std::map<AdKey, AdIdRefMap> AdCtagRefMap;

			AdCtagMap adlist;
			AdCtagRefMap adlist_init;
			AdCtagRefMap adlist_start;
			AdCtagRefMap adlist_norm;

			const AdBanner* pick(AdKey adKey, unsigned prev_id) const;
			const AdBanner* find(AdKey adKey, unsigned id) const;
			const AdBanner* finder(AdKey adKey, unsigned id) const;
			const AdBanner* findRandom(const AdCtagRefMap& where, AdKey adKey) const;
			const AdBanner* randomFinder(const AdCtagRefMap& where, AdKey adKey) const;
			void insert(AdCtagRefMap& where, const std::string& fname, unsigned id, unsigned delay, const std::string& link, unsigned next_id, const std::string& client, const std::string& lang);
		};

		extern scoped_ptr<AdBannerComponent> adbannerlist;

	}

}

#endif
