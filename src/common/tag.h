/*
 * Copyright (C) 1998  Mark Baysinger (mbaysing@ucsd.edu)
 * Copyright (C) 1998,1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 2004 CreepLord (creeplord@pvpgn.org)
 * Copyright (C) 2007  Pelish (pelish@gmail.com)
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
#ifndef INCLUDED_TAG_TYPES
#define INCLUDED_TAG_TYPES

#ifdef JUST_NEED_TYPES
#include "compat/uint.h"
#else
#define JUST_NEED_TYPES
#include "compat/uint.h"
#undef JUST_NEED_TYPES
#endif

namespace pvpgn
{

	typedef t_uint32	t_tag;
	typedef t_tag		t_archtag;
	typedef t_tag		t_clienttag;
	typedef t_tag		t_gamelang;

}

typedef enum {
	tag_wol_locale_unknown = 0,
	tag_wol_locale_other = 1,
	tag_wol_locale_usa = 2,
	tag_wol_locale_canada = 3,
	tag_wol_locale_uk = 4,
	tag_wol_locale_germany = 5,
	tag_wol_locale_france = 6,
	tag_wol_locale_spain = 7,
	tag_wol_locale_netherlands = 8,
	tag_wol_locale_belgium = 9,
	tag_wol_locale_austria = 10,
	tag_wol_locale_switzerland = 11,
	tag_wol_locale_italy = 12,
	tag_wol_locale_denmark = 13,
	tag_wol_locale_sweden = 14,
	tag_wol_locale_norway = 15,
	tag_wol_locale_finland = 16,
	tag_wol_locale_israel = 17,
	tag_wol_locale_south_africa = 18,
	tag_wol_locale_japan = 19,
	tag_wol_locale_south_korea = 20,
	tag_wol_locale_china = 21,
	tag_wol_locale_singapore = 22,
	tag_wol_locale_taiwan = 23,
	tag_wol_locale_malaysia = 24,
	tag_wol_locale_australia = 25,
	tag_wol_locale_new_zealand = 26,
	tag_wol_locale_brazil = 27,
	tag_wol_locale_thailand = 28,
	tag_wol_locale_argentina = 29,
	tag_wol_locale_philippines = 30,
	tag_wol_locale_greece = 31,
	tag_wol_locale_ireland = 32,
	tag_wol_locale_poland = 33,
	tag_wol_locale_portugal = 34,
	tag_wol_locale_mexico = 35,
	tag_wol_locale_russia = 36,
	tag_wol_locale_turkey = 37
} t_tag_wol_locale;

#endif

/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_TAG_PROTOS
#define INCLUDED_TAG_PROTOS

/* Software tags */
#define CLIENTTAG_BNCHATBOT 		"CHAT" /* CHAT bot */
#define CLIENTTAG_BNCHATBOT_UINT 	0x43484154
#define CLIENTTAG_STARCRAFT 		"STAR" /* Starcraft (original) */
#define CLIENTTAG_STARCRAFT_UINT 	0x53544152
#define CLIENTTAG_BROODWARS 		"SEXP" /* Starcraft EXpansion Pack */
#define CLIENTTAG_BROODWARS_UINT 	0x53455850
#define CLIENTTAG_SHAREWARE 		"SSHR" /* Starcraft Shareware */
#define CLIENTTAG_SHAREWARE_UINT	0x53534852
#define CLIENTTAG_DIABLORTL 		"DRTL" /* Diablo ReTaiL */
#define CLIENTTAG_DIABLORTL_UINT	0x4452544C
#define CLIENTTAG_DIABLOSHR 		"DSHR" /* Diablo SHaReware */
#define CLIENTTAG_DIABLOSHR_UINT	0x44534852
#define CLIENTTAG_WARCIIBNE 		"W2BN" /* WarCraft II Battle.net Edition */
#define CLIENTTAG_WARCIIBNE_UINT	0x5732424E
#define CLIENTTAG_DIABLO2DV 		"D2DV" /* Diablo II Diablo's Victory */
#define CLIENTTAG_DIABLO2DV_UINT	0x44324456
#define CLIENTTAG_STARJAPAN 		"JSTR" /* Starcraft (Japan) */
#define CLIENTTAG_STARJAPAN_UINT	0x4A535452
#define CLIENTTAG_DIABLO2ST 		"D2ST" /* Diablo II Stress Test */
#define CLIENTTAG_DIABLO2ST_UINT	0x44325354
#define CLIENTTAG_DIABLO2XP 		"D2XP" /* Diablo II Extension Pack */
#define CLIENTTAG_DIABLO2XP_UINT	0x44325850
/* FIXME: according to FSGS:
	SJPN==Starcraft (Japanese)
	SSJP==Starcraft (Japanese,Spawn)
	*/
#define CLIENTTAG_WARCRAFT3 		"WAR3" /* WarCraft III */
#define CLIENTTAG_WARCRAFT3_UINT	0x57415233
#define CLIENTTAG_WAR3XP    		"W3XP" /* WarCraft III Expansion */
#define CLIENTTAG_WAR3XP_UINT		0x57335850

#define CLIENTTAG_IIRC              "IIRC"/* IRC */
#define CLIENTTAG_IIRC_UINT         0x49495243

/* Westwood Online tags: */
#define CLIENTTAG_WCHAT             "WCHT" /* Westwood Chat */
#define CLIENTTAG_WCHAT_UINT        0x57434854
#define CLIENTTAG_TIBERNSUN         "TSUN" /* Tiberian Sun */
#define CLIENTTAG_TIBERNSUN_UINT    0x5453554E
#define CLIENTTAG_TIBSUNXP          "TSXP" /* Tiberian Sun Extension Pack */
#define CLIENTTAG_TIBSUNXP_UINT     0x54535850
#define CLIENTTAG_REDALERT          "RALT" /* Red Alert 1 */
#define CLIENTTAG_REDALERT_UINT     0x52414C54
#define CLIENTTAG_REDALERT2         "RAL2" /* Red Alert 2 */
#define CLIENTTAG_REDALERT2_UINT    0x52414C32
#define CLIENTTAG_DUNE2000          "DN2K" /* Dune 2000 */
#define CLIENTTAG_DUNE2000_UINT     0x444E324B
#define CLIENTTAG_NOX               "NOXX" /* NOX */
#define CLIENTTAG_NOX_UINT          0x4E4F5858
#define CLIENTTAG_NOXQUEST          "NOXQ" /* NOX Quest*/
#define CLIENTTAG_NOXQUEST_UINT     0x4E4F5851
#define CLIENTTAG_RENEGADE          "RNGD" /* Renegade */
#define CLIENTTAG_RENEGADE_UINT     0x524E4744
#define CLIENTTAG_RENGDFDS          "RFDS" /* Renegade Free Dedicated Server */
#define CLIENTTAG_RENGDFDS_UINT     0x52464453
#define CLIENTTAG_YURISREV          "YURI" /* Yuri's Revenge */
#define CLIENTTAG_YURISREV_UINT     0x59555249
#define CLIENTTAG_EMPERORBD         "EBFD" /* Emperor: Battle for Dune */
#define CLIENTTAG_EMPERORBD_UINT    0x45424644
#define CLIENTTAG_LOFLORE3          "LOR3" /* Lands of Lore 3 */
#define CLIENTTAG_LOFLORE3_UINT     0x4C4F5233
#define CLIENTTAG_WWOL              "WWOL" /* Other Westwood Online games */
#define CLIENTTAG_WWOL_UINT         0x57574F4C

/* BNETD-specific software tags - we try to use lowercase to avoid collisions  */
#define CLIENTTAG_FREECRAFT "free" /* FreeCraft http://www.freecraft.com/ */

#define CLIENTTAG_UNKNOWN		"UNKN"
#define CLIENTTAG_UNKNOWN_UINT		0x554E4B4E

/* Architecture tags */
#define ARCHTAG_WINX86       "IX86" /* MS Windows on Intel x86 */
#define ARCHTAG_MACPPC       "PMAC" /* MacOS   on PowerPC   */
#define ARCHTAG_OSXPPC       "XMAC" /* MacOS X on PowerPC   */

#define ARCHTAG_WINX86_UINT  0x49583836		/* IX86 */
#define ARCHTAG_MACPPC_UINT  0x504D4143		/* PMAC */
#define ARCHTAG_OSXPPC_UINT  0x584D4143		/* XMAC */

/* game languages */
#define GAMELANG_ENGLISH_UINT	0x656E5553	/* enUS */
#define GAMELANG_GERMAN_UINT	0x64654445	/* deDE */
#define GAMELANG_CZECH_UINT	0x6373435A	/* csCZ */
#define GAMELANG_SPANISH_UINT	0x65734553	/* esES */
#define GAMELANG_FRENCH_UINT	0x66724652	/* frFR */
#define GAMELANG_ITALIAN_UINT	0x69744954	/* itIT */
#define GAMELANG_JAPANESE_UINT	0x6A614A41	/* jaJA */
#define GAMELANG_KOREAN_UINT	0x6B6F4B52	/* koKR */
#define GAMELANG_POLISH_UINT	0x706C504C	/* plPL */
#define GAMELANG_RUSSIAN_UINT	0x72755255	/* ruRU */
#define GAMELANG_CHINESE_S_UINT	0x7A68434E	/* zhCN */
#define GAMELANG_CHINESE_T_UINT	0x7A685457	/* zhTW */

#define TAG_UNKNOWN_UINT	0x554E4B4E	/* UNKN */
#define TAG_UNKNOWN		"UNKN"

/* Server tag */
#define BNETTAG "bnet" /* Battle.net */

/* Filetype tags (note these are "backwards") */
#define EXTENSIONTAG_PCX "xcp."
#define EXTENSIONTAG_SMK "kms."
#define EXTENSIONTAG_MNG "gnm."

namespace pvpgn
{

	extern t_clienttag clienttag_str_to_uint(char const * clienttag);
	extern char const * clienttag_uint_to_str(t_clienttag clienttag);
	extern char const * clienttag_get_title(t_clienttag clienttag);

	extern t_tag	tag_str_to_uint(char const * tag_str);
	extern t_tag	tag_case_str_to_uint(char const * tag_str);
	extern const char * tag_uint_to_str(char * tag_str, t_tag tag_uint);
	extern const char * tag_uint_to_revstr(char * tag_str, t_tag tag_uint);
	extern int	tag_check_arch(t_tag tag_uint);
	extern int	tag_check_client(t_tag tag_uint);
	extern int	tag_check_gamelang(t_tag tag_uint);
	extern int	tag_check_wolv1(t_tag tag_uint);
	extern int	tag_check_wolv2(t_tag tag_uint);
	extern int	tag_check_in_list(t_clienttag clienttag, const char * list);
	extern t_clienttag tag_sku_to_uint(int sku);
	extern t_clienttag tag_channeltype_to_uint(int channeltype);
	extern t_tag tag_wol_locale_to_uint(int locale);
	extern t_clienttag tag_validate_client(char const * client);
}

#endif
#endif
