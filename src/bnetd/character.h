/*
 * Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_CHARACTER_TYPES
#define INCLUDED_CHARACTER_TYPES

#ifdef CHARACTER_INTERNAL_ACCESS

#ifdef JUST_NEED_TYPES
# include "compat/uint.h"
#else
# define JUST_NEED_TYPES
# include "compat/uint.h"
# undef JUST_NEED_TYPES
#endif

#endif

namespace pvpgn
{

	namespace bnetd
	{

		typedef enum
		{
			character_class_none,
			character_class_amazon,
			character_class_sorceress,
			character_class_necromancer,
			character_class_paladin,
			character_class_barbarian,
			character_class_druid,
			character_class_assassin
		} t_character_class;

		typedef enum
		{
			character_expansion_none,
			character_expansion_classic,
			character_expansion_lod
		} t_character_expansion;

		typedef struct character
#ifdef CHARACTER_INTERNAL_ACCESS
		{
			char const * name; /* max 15 chars */
			char const * realmname;
			char const * guildname; /* max 3 chars */

			/* stored in Battle.net format for now */
			t_uint8      helmgfx;
			t_uint8      bodygfx;
			t_uint8      leggfx;
			t_uint8      lhandweapon;
			t_uint8      lhandgfx;
			t_uint8      rhandweapon;
			t_uint8      rhandgfx;
			t_uint8      chclass;
			t_uint8      level;
			t_uint8      status;
			t_uint8      title;
			t_uint8      emblembgc;
			t_uint8      emblemfgc;
			t_uint8      emblemnum;

			/* not sure what these represent */
			t_uint32     unknown1;
			t_uint32     unknown2;
			t_uint32     unknown3;
			t_uint32     unknown4;
			t_uint8      unknownb1;
			t_uint8      unknownb2;
			t_uint8      unknownb3;
			t_uint8      unknownb4;
			t_uint8      unknownb5;
			t_uint8      unknownb6;
			t_uint8      unknownb7;
			t_uint8      unknownb8;
			t_uint8      unknownb9;
			t_uint8      unknownb10;
			t_uint8      unknownb11;
			t_uint8      unknownb13;
			t_uint8      unknownb14;

			/* Keep some generic "data", basically the blob that is sent between client and server, in an array */
			t_uint8      data[64];
			t_uint8      datalen;
		}
#endif
		t_character;

	}

}

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_CHARACTER_PROTOS
#define INCLUDED_CHARACTER_PROTOS

#define JUST_NEED_TYPES
#include "compat/uint.h"
#include "account.h"
#undef JUST_NEED_TYPES

namespace pvpgn
{

	namespace bnetd
	{

		extern int character_create(t_account * account, t_clienttag clienttag, char const * realmname, char const * name, t_character_class chclass, t_character_expansion expansion);
		extern char const * character_get_name(t_character const * ch);
		extern char const * character_get_realmname(t_character const * ch);
		extern char const * character_get_playerinfo(t_character const * ch);
		extern char const * character_get_guildname(t_character const * ch);
		extern t_character_class character_get_class(t_character const * ch);
		extern int character_verify_charlist(t_character const * ch, char const * charlist);

		extern int characterlist_create(char const * dirname);
		extern int characterlist_destroy(void);
		extern t_character * characterlist_find_character(char const * realmname, char const * charname);

	}

}

#endif
#endif
