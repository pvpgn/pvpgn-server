/*
 * Copyright (C) 1999  Rob Crittenden (rcrit@greyoak.com)
 * Copyright (C) 1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_LADDER_TYPES
#define INCLUDED_LADDER_TYPES

#include "account.h"
#include "common/tag.h"
#include <vector>
#include <list>
#include <map>
#include <string>
#include <utility>

#define W3_XPCALC_MAXLEVEL	65

namespace pvpgn
{

	namespace bnetd
	{


		typedef enum
		{
			ladder_sort_highestrated,
			ladder_sort_mostwins,
			ladder_sort_mostgames,
			ladder_sort_default
		} t_ladder_sort;

		extern const char * bin_ladder_sort_str[];

		typedef enum
		{
			ladder_time_active,
			ladder_time_current,
			ladder_time_default
		} t_ladder_time;

		extern const char * bin_ladder_time_str[];

		typedef enum
		{
			ladder_id_none = 0,
			ladder_id_normal = 1,
			ladder_id_ironman = 3,
			ladder_id_solo = 5,
			ladder_id_team = 6,
			ladder_id_ffa = 7,
			ladder_id_ateam = 8
		} t_ladder_id;

		const extern char * ladder_id_str[];
		const extern char * bin_ladder_id_str[];

		typedef enum
		{
			ladder_option_none = 0,
			ladder_option_disconnectisloss = 1
		} t_ladder_option;

		typedef enum { referenceTypeAccount, referenceTypeTeam, referenceTypeClan } t_referenceType;

		class LadderKey
		{
		public:
			LadderKey(t_ladder_id ladderId_, t_clienttag clienttag_, t_ladder_sort ladderSort_, t_ladder_time ladderTime_);
			~LadderKey() throw ();
			bool operator== (const LadderKey& right) const;
			bool operator< (const LadderKey& right) const;
			t_ladder_id getLadderId() const;
			t_clienttag getClienttag() const;
			t_ladder_sort getLadderSort() const;
			t_ladder_time getLadderTime() const;
			LadderKey getOpposingKey() const;
		private:
			t_ladder_id ladderId;
			t_clienttag clienttag;
			t_ladder_sort ladderSort;
			t_ladder_time ladderTime;
		};

		class LadderReferencedObject
		{
		public:
			explicit LadderReferencedObject(t_account *account_);
			explicit LadderReferencedObject(t_team *team_);
			~LadderReferencedObject() throw ();
			bool getData(const LadderKey& ladderKey_, unsigned int& uid, unsigned int& primary_, unsigned int& secondary_, unsigned int& tertiary_) const;
			unsigned int getRank(const LadderKey& ladderKey_) const;
			bool setRank(const LadderKey& ladderKey_, unsigned int rank_) const;
			t_account* getAccount() const;
			t_team* getTeam() const;
			void activate(const LadderKey& ladderKey_) const;
			const t_referenceType getReferenceType() const;
		private:

			t_referenceType referenceType;
			t_account * account;
			t_team * team;
		};

		class LadderEntry
		{
		public:
			LadderEntry(unsigned int uid_, unsigned int primary_, unsigned int secondary_, unsigned int tertiary_, LadderReferencedObject referencedObject_);
			~LadderEntry() throw ();
			unsigned int getUid() const;
			unsigned int getPrimary() const;
			unsigned int getSecondary() const;
			unsigned int getTertiary() const;
			unsigned int getRank() const;
			const LadderReferencedObject& getReferencedObject() const;
			bool setRank(unsigned int rank_, const LadderKey& ladderKey_);
			void update(unsigned int primary_, unsigned int secondary_, unsigned int tertiary_);
			std::string status() const;
			bool operator== (const LadderEntry& right) const;
			bool operator< (const LadderEntry& right) const;
		private:
			unsigned int uid;
			unsigned int primary;
			unsigned int secondary;
			unsigned int tertiary;
			unsigned int rank;
			LadderReferencedObject referencedObject;
		};

		class LadderList
		{
		public:
			explicit LadderList(LadderKey ladderkey_, t_referenceType referenceType_);
			~LadderList() throw ();
			bool load();
			bool save();
			void addEntry(unsigned int uid_, unsigned int primary_, unsigned int secondary_, unsigned int tertiary_, const LadderReferencedObject& referencedObject_);
			void updateEntry(unsigned int uid_, unsigned int primary_, unsigned int secondary_, unsigned int tertiary_, const LadderReferencedObject& referencedObject_);
			bool delEntry(unsigned int uid_);
			const LadderKey& getLadderKey() const;
			const t_referenceType getReferenceType() const;
			void sortAndUpdate();
			const LadderReferencedObject* getReferencedObject(unsigned int rank) const;
			unsigned int getRank(unsigned int uid_) const;
			void activateFrom(const LadderList * currentLadder_);
			void writeStatusfile() const;

		private:
			typedef std::vector<LadderEntry> LList;
			LadderKey ladderKey;
			LList ladder;
			bool dirty;
			bool saved;
			std::string ladderFilename;
			t_referenceType referenceType;
			bool loadBinary();
			bool saveBinary();
			void readdata(std::ifstream  &fp, unsigned int &data);
			void readdata(std::ifstream  &fp, unsigned int data[], unsigned int membercount);
			void writedata(std::ofstream &fp, unsigned int &data);
			void writedata(std::ofstream &fp, const unsigned int &data);
			void writedata(std::ofstream &fp, unsigned int data[], unsigned int membercount);

		};

		class Ladders
		{
		public:
			Ladders();
			~Ladders() throw ();
			LadderList* getLadderList(const LadderKey& ladderKey_);
			void load();
			void update();
			void activate();
			void save();
			void status() const;
		private:
			void rebuild(std::list<LadderList*>& laddersToRebuild);
			typedef std::map<LadderKey, LadderList> KeyLadderMap;
			KeyLadderMap ladderMap;
		};


		extern Ladders ladders;


#ifdef LADDER_INTERNAL_ACCESS

		typedef struct
		{
			int startxp, neededxp, lossfactor, mingames;
		} t_xplevel_entry;

		typedef struct
		{
			int higher_winxp, higher_lossxp, lower_winxp, lower_lossxp;
		} t_xpcalc_entry;
#endif


	}

}

#endif

/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_LADDER_PROTOS
#define INCLUDED_LADDER_PROTOS

#define JUST_NEED_TYPES
#include "account.h"
#include "game.h"
#include "ladder_calc.h"
#include "common/tag.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern int ladder_update(t_clienttag clienttag, t_ladder_id id, unsigned int count, t_account * * players, t_game_result * results, t_ladder_info * info);
		extern int ladder_update_wol(t_clienttag clienttag, t_ladder_id id, t_account * * players, t_game_result * results);

		extern int ladder_init_account(t_account * account, t_clienttag clienttag, t_ladder_id id);
		extern int ladder_init_account_wol(t_account * account, t_clienttag clienttag, t_ladder_id id);

		extern int ladder_check_map(char const * mapname, t_game_maptype maptype, t_clienttag clienttag);


		extern int  ladder_createxptable(const char *xplevelfile, const char *xpcalcfile);
		extern void ladder_destroyxptable(void);

		extern int ladder_war3_xpdiff(unsigned int winnerlevel, unsigned int looserlevel, int *, int *);
		extern int ladder_war3_updatelevel(unsigned int oldlevel, int xp);
		extern int ladder_war3_get_min_xp(unsigned int level);
		extern int war3_get_maxleveldiff(void);


		extern void ladder_reload_conf(void);
		/* reloads relevant parameters from bnetd.conf (xml/std mode for ladder) */
	}

}

#endif
#endif
