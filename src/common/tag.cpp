/*
 * Copyright (C) 2004	Aaron
 * Copyright (C) 2004	CreepLord (creeplord@pvpgn.org)
 * Copyright (C) 2007,2008	Pelish (pelish@gmail.com)
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
#include "common/tag.h"

#include <cstring>
#include <cctype>

#include "compat/strcasecmp.h"

#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/xstring.h"
#include "common/setup_after.h"

namespace pvpgn
{

	/* fixme: have all functions call tag_str_to_uint() */
	extern t_clienttag clienttag_str_to_uint(char const * clienttag)
	{
		if (!clienttag)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL clienttag");
			return CLIENTTAG_UNKNOWN_UINT;
		}

		return tag_str_to_uint(clienttag);
	}

	/* fixme: have all fuctions call tag_uint_to_str() */
	extern char const * clienttag_uint_to_str(t_clienttag clienttag)
	{
		switch (clienttag)
		{
		case CLIENTTAG_BNCHATBOT_UINT:
			return CLIENTTAG_BNCHATBOT;
		case CLIENTTAG_STARCRAFT_UINT:
			return CLIENTTAG_STARCRAFT;
		case CLIENTTAG_BROODWARS_UINT:
			return CLIENTTAG_BROODWARS;
		case CLIENTTAG_SHAREWARE_UINT:
			return CLIENTTAG_SHAREWARE;
		case CLIENTTAG_DIABLORTL_UINT:
			return CLIENTTAG_DIABLORTL;
		case CLIENTTAG_DIABLOSHR_UINT:
			return CLIENTTAG_DIABLOSHR;
		case CLIENTTAG_WARCIIBNE_UINT:
			return CLIENTTAG_WARCIIBNE;
		case CLIENTTAG_DIABLO2DV_UINT:
			return CLIENTTAG_DIABLO2DV;
		case CLIENTTAG_STARJAPAN_UINT:
			return CLIENTTAG_STARJAPAN;
		case CLIENTTAG_DIABLO2ST_UINT:
			return CLIENTTAG_DIABLO2ST;
		case CLIENTTAG_DIABLO2XP_UINT:
			return CLIENTTAG_DIABLO2XP;
		case CLIENTTAG_WARCRAFT3_UINT:
			return CLIENTTAG_WARCRAFT3;
		case CLIENTTAG_WAR3XP_UINT:
			return CLIENTTAG_WAR3XP;
		case CLIENTTAG_IIRC_UINT:
			return CLIENTTAG_IIRC;
		case CLIENTTAG_WCHAT_UINT:
			return CLIENTTAG_WCHAT;
		case CLIENTTAG_TIBERNSUN_UINT:
			return CLIENTTAG_TIBERNSUN;
		case CLIENTTAG_TIBSUNXP_UINT:
			return CLIENTTAG_TIBSUNXP;
		case CLIENTTAG_REDALERT_UINT:
			return CLIENTTAG_REDALERT;
		case CLIENTTAG_REDALERT2_UINT:
			return CLIENTTAG_REDALERT2;
		case CLIENTTAG_DUNE2000_UINT:
			return CLIENTTAG_DUNE2000;
		case CLIENTTAG_NOX_UINT:
			return CLIENTTAG_NOX;
		case CLIENTTAG_NOXQUEST_UINT:
			return CLIENTTAG_NOXQUEST;
		case CLIENTTAG_RENEGADE_UINT:
			return CLIENTTAG_RENEGADE;
		case CLIENTTAG_RENGDFDS_UINT:
			return CLIENTTAG_RENGDFDS;
		case CLIENTTAG_YURISREV_UINT:
			return CLIENTTAG_YURISREV;
		case CLIENTTAG_EMPERORBD_UINT:
			return CLIENTTAG_EMPERORBD;
		case CLIENTTAG_LOFLORE3_UINT:
			return CLIENTTAG_LOFLORE3;
		case CLIENTTAG_WWOL_UINT:
			return CLIENTTAG_WWOL;
		default:
			return CLIENTTAG_UNKNOWN;
		}
	}

	/*****/
	/* make all letters in string upper case - used in command.cpp*/
	extern t_tag tag_case_str_to_uint(char const * tag_str)
	{
		unsigned int i, len;
		char temp_str[5];

		len = std::strlen(tag_str);
		if (len != 4)
			eventlog(eventlog_level_warn, __FUNCTION__, "got unusual sized clienttag '%s'", tag_str);

		for (i = 0; i < len && i < 4; i++)
			temp_str[i] = safe_toupper(tag_str[i]);

		temp_str[4] = '\0';

		return tag_str_to_uint(temp_str);
	}

	extern t_tag tag_str_to_uint(char const * tag_str)
	{
		t_tag	tag_uint;

		if (!tag_str) {
			eventlog(eventlog_level_error, __FUNCTION__, "got NULL tag");
			return 0; /* unknown */
		}

		tag_uint = tag_str[0] << 24;
		tag_uint |= tag_str[1] << 16;
		tag_uint |= tag_str[2] << 8;
		tag_uint |= tag_str[3];

		return tag_uint;
	}
	/* tag_uint_to_str()
	 *
	 * from calling function:
	 *
	 *    char tag_str[5]; // define first, then send into fuction
	 *    tag_uint_to_str(tag_str, tag_uint); // returns pointer to tag_str
	 *
	 * Nothing to malloc, nothing to free
	 */
	extern const char * tag_uint_to_str(char * tag_str, t_tag tag_uint)
	{
		if (!tag_uint) /* return "UNKN" if tag_uint = 0 */
			return TAG_UNKNOWN;

		tag_str[0] = ((unsigned char)(tag_uint >> 24));
		tag_str[1] = ((unsigned char)(tag_uint >> 16) & 0xff);
		tag_str[2] = ((unsigned char)(tag_uint >> 8) & 0xff);
		tag_str[3] = ((unsigned char)(tag_uint)& 0xff);
		tag_str[4] = '\0';
		return tag_str;
	}

	extern const char * tag_uint_to_revstr(char * tag_str, t_tag tag_uint)
	{
		if (!tag_uint) /* return "UNKN" if tag_uint = 0 */
			return TAG_UNKNOWN;

		tag_str[0] = ((unsigned char)(tag_uint)& 0xff);
		tag_str[1] = ((unsigned char)(tag_uint >> 8) & 0xff);
		tag_str[2] = ((unsigned char)(tag_uint >> 16) & 0xff);
		tag_str[3] = ((unsigned char)(tag_uint >> 24));
		tag_str[4] = '\0';
		return tag_str;
	}

	extern int tag_check_arch(t_tag tag_uint)
	{
		switch (tag_uint)
		{
		case ARCHTAG_WINX86_UINT:
		case ARCHTAG_MACPPC_UINT:
		case ARCHTAG_OSXPPC_UINT:
			return 1;
		default:
			return 0;
		}
	}

	extern int tag_check_client(t_tag tag_uint)
	{
		switch (tag_uint)
		{
		case CLIENTTAG_BNCHATBOT_UINT:
		case CLIENTTAG_STARCRAFT_UINT:
		case CLIENTTAG_BROODWARS_UINT:
		case CLIENTTAG_SHAREWARE_UINT:
		case CLIENTTAG_DIABLORTL_UINT:
		case CLIENTTAG_DIABLOSHR_UINT:
		case CLIENTTAG_WARCIIBNE_UINT:
		case CLIENTTAG_DIABLO2DV_UINT:
		case CLIENTTAG_STARJAPAN_UINT:
		case CLIENTTAG_DIABLO2ST_UINT:
		case CLIENTTAG_DIABLO2XP_UINT:
		case CLIENTTAG_WARCRAFT3_UINT:
		case CLIENTTAG_WAR3XP_UINT:
		case CLIENTTAG_IIRC_UINT:
		case CLIENTTAG_WCHAT_UINT:
		case CLIENTTAG_TIBERNSUN_UINT:
		case CLIENTTAG_TIBSUNXP_UINT:
		case CLIENTTAG_REDALERT_UINT:
		case CLIENTTAG_REDALERT2_UINT:
		case CLIENTTAG_DUNE2000_UINT:
		case CLIENTTAG_NOX_UINT:
		case CLIENTTAG_NOXQUEST_UINT:
		case CLIENTTAG_RENEGADE_UINT:
		case CLIENTTAG_RENGDFDS_UINT:
		case CLIENTTAG_YURISREV_UINT:
		case CLIENTTAG_EMPERORBD_UINT:
		case CLIENTTAG_LOFLORE3_UINT:
		case CLIENTTAG_WWOL_UINT:
			return 1;
		default:
			return 0;
		}
	}

	extern int tag_check_gamelang(t_tag gamelang)
	{
		switch (gamelang)
		{
		case GAMELANG_ENGLISH_UINT:	/* enUS */
		case GAMELANG_GERMAN_UINT:	/* deDE */
		case GAMELANG_CZECH_UINT:	/* csCZ */
		case GAMELANG_SPANISH_UINT:	/* esES */
		case GAMELANG_FRENCH_UINT:	/* frFR */
		case GAMELANG_ITALIAN_UINT:	/* itIT */
		case GAMELANG_JAPANESE_UINT:	/* jaJA */
		case GAMELANG_KOREAN_UINT:	/* koKR */
		case GAMELANG_POLISH_UINT:	/* plPL */
		case GAMELANG_RUSSIAN_UINT:	/* ruRU */
		case GAMELANG_CHINESE_S_UINT:	/* zhCN */
		case GAMELANG_CHINESE_T_UINT:	/* zhTW */
			return 1;
		default:
			return 0;
		}
	}

	extern int tag_check_wolv1(t_tag tag_uint)
	{
		switch (tag_uint) {
		case CLIENTTAG_WCHAT_UINT:
			return 1;
		default:
			return 0;
		}
	}

	extern int tag_check_wolv2(t_tag tag_uint)
	{
		switch (tag_uint) {
		case CLIENTTAG_TIBERNSUN_UINT:
		case CLIENTTAG_TIBSUNXP_UINT:
		case CLIENTTAG_REDALERT_UINT:
		case CLIENTTAG_REDALERT2_UINT:
		case CLIENTTAG_DUNE2000_UINT:
		case CLIENTTAG_NOX_UINT:
		case CLIENTTAG_NOXQUEST_UINT:
		case CLIENTTAG_RENEGADE_UINT:
		case CLIENTTAG_RENGDFDS_UINT:
		case CLIENTTAG_YURISREV_UINT:
		case CLIENTTAG_EMPERORBD_UINT:
		case CLIENTTAG_LOFLORE3_UINT:
		case CLIENTTAG_WWOL_UINT:
			return 1;
		default:
			return 0;
		}
	}

	extern char const * clienttag_get_title(t_clienttag clienttag)
	{
		switch (clienttag)
		{
		case CLIENTTAG_WAR3XP_UINT:
			return "Warcraft III Frozen Throne";
		case CLIENTTAG_WARCRAFT3_UINT:
			return "Warcraft III";
		case CLIENTTAG_DIABLO2XP_UINT:
			return "Diablo II Lord of Destruction";
		case CLIENTTAG_DIABLO2DV_UINT:
			return "Diablo II";
		case CLIENTTAG_STARJAPAN_UINT:
			return "Starcraft (Japan)";
		case CLIENTTAG_WARCIIBNE_UINT:
			return "Warcraft II";
		case CLIENTTAG_DIABLOSHR_UINT:
			return "Diablo I (Shareware)";
		case CLIENTTAG_DIABLORTL_UINT:
			return "Diablo I";
		case CLIENTTAG_SHAREWARE_UINT:
			return "Starcraft (Shareware)";
		case CLIENTTAG_BROODWARS_UINT:
			return "Starcraft: Brood War";
		case CLIENTTAG_STARCRAFT_UINT:
			return "Starcraft";
		case CLIENTTAG_BNCHATBOT_UINT:
			return "Chat";
		case CLIENTTAG_IIRC_UINT:
			return "Internet Relay Chat";
		case CLIENTTAG_WCHAT_UINT:
			return "Westwood Chat";
		case CLIENTTAG_TIBERNSUN_UINT:
			return "Tiberian Sun";
		case CLIENTTAG_TIBSUNXP_UINT:
			return "Tiberian Sun: Firestorm";
		case CLIENTTAG_REDALERT_UINT:
			return "Red Alert";
		case CLIENTTAG_REDALERT2_UINT:
			return "Red Alert 2";
		case CLIENTTAG_DUNE2000_UINT:
			return "Dune 2000";
		case CLIENTTAG_NOX_UINT:
			return "Nox";
		case CLIENTTAG_NOXQUEST_UINT:
			return "Nox Quest";
		case CLIENTTAG_RENEGADE_UINT:
			return "Renegade";
		case CLIENTTAG_RENGDFDS_UINT:
			return "Renegade Free Dedicated Server";
		case CLIENTTAG_YURISREV_UINT:
			return "Yuri's Revenge";
		case CLIENTTAG_EMPERORBD_UINT:
			return "Emepror: Battle for Dune";
		case CLIENTTAG_LOFLORE3_UINT:
			return "Lands of Lore 3";
		case CLIENTTAG_WWOL_UINT:
			return "Westwood Online";
		default:
			return "Unknown";
		}
	}

	extern int tag_check_in_list(t_clienttag clienttag, char const * list)
	{
		/* checks if a clienttag is in the list
		 * @clienttag : clienttag integer to check
		 * if it's allowed returns 0
		 * if it's not allowed returns -1
		 */
		char *tmp, *p, *q;

		/* by default allow all */
		if (!list)
			return 0;

		/* this shortcut check should make server as fast as before if
		 * the configuration is left in default mode */
		if (!strcasecmp(list, "all"))
			return 0;

		tmp = p = xstrdup(list);
		do {
			q = std::strchr(p, ',');
			if (q)
				*q = '\0';
			if (!strcasecmp(p, "all"))
				goto ok;
			if (std::strlen(p) != 4)
				continue;
			if (clienttag == tag_case_str_to_uint(p))
				goto ok;		/* client is in list */
			if (q)
				p = q + 1;
		} while (q);
		xfree((void *)tmp);

		return -1;			/* client is NOT in list */

	ok:
		xfree((void *)tmp);
		return 0;
	}

	extern t_clienttag tag_sku_to_uint(int sku)
	{
		/**
		 *  Here is table of Westwood Online SKUs and by this SKU returning CLIENTTAG
		 *  SKUs are from Autoupdate FTP and from windows registry
		 */

		if (!sku) {
			ERROR0("got NULL sku");
			return CLIENTTAG_UNKNOWN_UINT;
		}

		switch (sku) {
		case 1000:  /* Westwood Chat */
			return CLIENTTAG_WCHAT_UINT;
			/*    case 1002: Internet Registration  */
		case 1003:  /* Command & Conquer */
			return CLIENTTAG_WCHAT_UINT;
			//           return CLIENTTAG_CNCONQUER_UINT;
		case 1005:  /* Red Alert 1 v2.00 (Westwood Chat Version) */
		case 1006:
		case 1007:
		case 1008:
			return CLIENTTAG_WCHAT_UINT;
			//           return CLIENTTAG_REDALERT_UINT;
		case 1040:  /* C&C Sole Survivor */
			return CLIENTTAG_WCHAT_UINT;
			//           return CLIENTTAG_CNCSOLES_UINT;
		case 3072:  /* C&C Renegade */
		case 3074:
		case 3075:
		case 3078:
		case 3081:
		case 3082:
			//      case 16780288: /* renegade ladder players */
			return CLIENTTAG_RENEGADE_UINT;
		case 3584:  /* Dune 2000 */
		case 3586:
		case 3587:
		case 3589:
		case 3591:
			return CLIENTTAG_DUNE2000_UINT;
		case 4096:  /* Nox */
		case 4098:
		case 4099:
		case 4101:
		case 4102:
		case 4105:
			return CLIENTTAG_NOX_UINT;
		case 4608:  /* Tiberian Sun */
		case 4610:
		case 4611:
		case 4615:
			return CLIENTTAG_TIBERNSUN_UINT;
		case 5376:  /* Red Alert 1 v3.03 (4 Players Internet Version) */
		case 5378:
		case 5379:
			return CLIENTTAG_REDALERT_UINT;
		case 6400:  /* Lands of Lore 3 */
		case 6401:
		case 6402:
		case 6403:
		case 6405:
			return CLIENTTAG_LOFLORE3_UINT;
		case 7168:  /* Tiberian Sun: Firestorm */
		case 7170:
		case 7171:
		case 7175:
		case 7424:
		case 7426:
		case 7427:
		case 7431:
			return CLIENTTAG_TIBSUNXP_UINT;
		case 7936:  /* Emperor: Battle for Dune */
		case 7938:
		case 7939:
		case 7945:
		case 7946:
			return CLIENTTAG_EMPERORBD_UINT;
		case 8448:  /* Red Alert 2 */
		case 8450:
		case 8451:
		case 8457:
		case 8458:
		case 8960:
		case 8962:
		case 8963:
		case 8969:
		case 8970:
			return CLIENTTAG_REDALERT2_UINT;
		case 9472:  /* Nox Quest */
		case 9474:
		case 9475:
		case 9477:
		case 9478:
		case 9481:
			return CLIENTTAG_NOXQUEST_UINT;
		case 10496:  /* Yuri's Revenge */
		case 10498:
		case 10499:
		case 10505:
		case 10506:
			return CLIENTTAG_YURISREV_UINT;
		case 12288:  /* C&C Renegade Free Dedicated Server */
			return CLIENTTAG_RENGDFDS_UINT;
		case 32512:  /* Westwood Online API */
			return CLIENTTAG_WWOL_UINT;
		default:  /* Unknown Westwood Online game -> is anyone SKU that we havent??? */
			return CLIENTTAG_WWOL_UINT;
		}
	}

	extern t_clienttag tag_channeltype_to_uint(int channeltype)
	{
		/**
		 *  This is used to convert WOL channel type to t_clienttag
		 */

		if (!channeltype) {
			ERROR0("got NULL channeltype");
			return CLIENTTAG_UNKNOWN_UINT;
		}

		switch (channeltype) {
		case 0:   /* Westwood Chat channels */
		case 1:   /* Command & Conquer Win95 channels */
		case 2:   /* Red Alert Win95 channels */
		case 3:   /* Red Alert Win95 Counterstrike channels */
		case 4:   /* Red Alert Win95 The Aftermath channels */
		case 5:   /* Command & Conquer: Sole Survivor channels */
			return CLIENTTAG_WCHAT_UINT;
		case 12:  /* Command & Conquer: Renegade channels */
			return CLIENTTAG_RENEGADE_UINT;
		case 14:  /* Dune 2000 channels */
			return CLIENTTAG_DUNE2000_UINT;
		case 16:  /* Nox channels */
			return CLIENTTAG_NOX_UINT;
		case 18:  /* Tiberian Sun channels */
			return CLIENTTAG_TIBERNSUN_UINT;
		case 21:  /* Red Alert 1 v 3.03 channels */
			return CLIENTTAG_REDALERT_UINT;
		case 31:  /* Emperor: Battle for Dune channels */
			return CLIENTTAG_EMPERORBD_UINT;
		case 33:  /* Red Alert 2 channels */
			return CLIENTTAG_REDALERT2_UINT;
		case 37:  /* Nox Quest channels */
			return CLIENTTAG_NOXQUEST_UINT;
		case 38:  /* QuickMatch channels */
		case 39:
		case 40:
			return CLIENTTAG_WWOL_UINT;
		case 41:  /* Yuri's Revenge channels */
			return CLIENTTAG_YURISREV_UINT;
		default:
			return CLIENTTAG_WWOL_UINT;
		}
	}

	extern t_tag tag_wol_locale_to_uint(int locale)
	{
		switch (locale) {
		case tag_wol_locale_unknown:
			return GAMELANG_ENGLISH_UINT;
		case tag_wol_locale_other:
			return GAMELANG_ENGLISH_UINT;
		case tag_wol_locale_usa:
			return GAMELANG_ENGLISH_UINT;
			/*        case tag_wol_locale_canada:
						return enCA
						case tag_wol_locale_uk:
						return enGB */
		case tag_wol_locale_germany:
			return GAMELANG_GERMAN_UINT;
		case tag_wol_locale_france:
			return GAMELANG_FRENCH_UINT;
		case tag_wol_locale_spain:
			return GAMELANG_SPANISH_UINT;
			/*        case tag_wol_locale_netherlands:
						return nlNL
						case tag_wol_locale_belgium:
						return nlBE
						case tag_wol_locale_austria:
						return deAT
						case tag_wol_locale_switzerland:
						return deCH */
		case tag_wol_locale_italy:
			return GAMELANG_ITALIAN_UINT;
			/*        case tag_wol_locale_denmark:
						return daDK
						case tag_wol_locale_sweden:
						return svSE
						case tag_wol_locale_norway:
						return noNO
						case tag_wol_locale_finland:
						return fiFI
						case tag_wol_locale_israel:
						return arIL
						case tag_wol_locale_south_africa:
						return enZA*/
		case tag_wol_locale_japan:
			return GAMELANG_JAPANESE_UINT;
		case tag_wol_locale_south_korea:
			return GAMELANG_KOREAN_UINT;
		case tag_wol_locale_china:
			return GAMELANG_CHINESE_S_UINT;
			/*        case tag_wol_locale_singapore:
						return enSG
						case tag_wol_locale_taiwan:
						return zhTW
						case tag_wol_locale_malaysia:
						return msMY
						case tag_wol_locale_australia:
						return enAU
						case tag_wol_locale_new_zealand:
						return enNZ
						case tag_wol_locale_brazil:
						return ptBR
						case tag_wol_locale_thailand:
						return thTH
						case tag_wol_locale_argentina:
						return esAR
						case tag_wol_locale_philippines:
						return enPH
						case tag_wol_locale_greece:
						return elGR
						case tag_wol_locale_ireland:
						return enIE*/
		case tag_wol_locale_poland:
			return GAMELANG_POLISH_UINT;
			/*        case tag_wol_locale_portugal:
						return ptPT
						case tag_wol_locale_mexico:
						return esMX*/
		case tag_wol_locale_russia:
			return GAMELANG_RUSSIAN_UINT;
			/*        case tag_wol_locale_turkey:
						return trTR*/
		default:
			WARN1("%d is not defined", locale);
			return GAMELANG_ENGLISH_UINT;
		}
	}

	/* Convert clienttag to uppercase and check it in valid client list
	*   Return NULL of tag not found
	*/
	extern t_clienttag tag_validate_client(char const * client)
	{
		t_clienttag clienttag;
		if (!client || strlen(client) != 4)
			return NULL;

		// toupper
		clienttag = tag_case_str_to_uint(client);
		
		if (!tag_check_client(clienttag))
			return NULL;

		return clienttag;
	}

}
