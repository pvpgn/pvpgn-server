/*
* Copyright (C) 2014  HarpyWar (harpywar@gmail.com)
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

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <sstream>
#include <iostream>
#include <fstream>
#include <vector>
#include <utility>
#include <map>
#include <string.h>

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#include "compat/strcasecmp.h"
#include "compat/snprintf.h"

#include "common/token.h"

#include "common/list.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/xstring.h"
#include "common/util.h"
#include "common/tag.h"
#include "common/pugixml.h"
#include "common/format.h"

#include "account.h"
#include "connection.h"
#include "message.h"
#include "helpfile.h"
#include "channel.h"
#include "prefs.h"
#include "account_wrap.h"
#include "command.h"
#include "i18n.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{
		const char * commonfile = "common.xml"; // filename template, actually file name is "common-{lang}.xml"

		/* Array with string translations, each string has array with pair language=translation
			{ 
				original = { 
					{ language = translate }, 
					... 
				}, 
				... 
			} 
		*/
		std::map<std::string, std::map<t_gamelang, std::string> > translations = std::map<std::string, std::map<t_gamelang, std::string> >();

		const char * _find_string(char const * text, t_gamelang gamelang);

		extern const t_gamelang languages[12] = {
			GAMELANG_ENGLISH_UINT,	/* enUS */
			GAMELANG_GERMAN_UINT,	/* deDE */
			GAMELANG_CZECH_UINT,	/* csCZ */
			GAMELANG_SPANISH_UINT,	/* esES */
			GAMELANG_FRENCH_UINT,	/* frFR */
			GAMELANG_ITALIAN_UINT,	/* itIT */
			GAMELANG_JAPANESE_UINT,	/* jaJA */
			GAMELANG_KOREAN_UINT,	/* koKR */
			GAMELANG_POLISH_UINT,	/* plPL */
			GAMELANG_RUSSIAN_UINT,	/* ruRU */
			GAMELANG_CHINESE_S_UINT,	/* zhCN */
			GAMELANG_CHINESE_T_UINT	/* zhTW */
		};


		// http://msdn.microsoft.com/en-us/goglobal/bb896001.aspx
		extern const char * countries[][2] =
		{
			/* English (uses if other not found) */
			{ "USA", "enUS" },

			/* Polish */
			{ "POL", "plPL" },

			/* Korean */
			{ "KOR", "koKR" },

			/* French */
			{ "FRA", "frFR" },

			/* Bulgarian */
			{ "BGR", "bgBG" },

			/* Italian */
			{ "ITA", "itIT" },

			/* Japanese */
			{ "JPN", "jpJA" },

			/* Czech */
			{ "CZE", "csCZ" },

			/* Dutch */
			{ "NLD", "nlNL" },

			/* Portuguese */
			{ "BRA", "ptBR" },
			{ "PRT", "ptBR" },

			/* Swedish */
			{ "SWE", "svSE" },
			{ "FIN", "svSE" },

			/* German */
			{ "DEU", "deDE" },
			{ "AUT", "deDE" },
			{ "LIE", "deDE" },
			{ "LUX", "deDE" },

			/* Russian */
			{ "RUS", "ruRU" },
			{ "UZB", "ruRU" },
			{ "TTT", "ruRU" },
			{ "UKR", "ruRU" },
			{ "AZE", "ruRU" },
			{ "ARM", "ruRU" },

			/* Chinese */
			{ "CHN", "chCN" },
			{ "SGP", "chCN" },
			{ "HKG", "chTW" },
			{ "MCO", "chTW" },
			{ "TWN", "chTW" },

			/* Spanish */
			{ "ESP", "esES" },
			{ "ARG", "esES" },
			{ "BOL", "esES" },
			{ "CHL", "esES" },
			{ "COL", "esES" },
			{ "CRI", "esES" },
			{ "DOM", "esES" },
			{ "ECU", "esES" },
			{ "SLV", "esES" },
			{ "GTM", "esES" },
			{ "HND", "esES" },
			{ "MEX", "esES" },
			{ "NIC", "esES" },
			{ "PAN", "esES" },
			{ "PRY", "esES" },
			{ "PER", "esES" },
			{ "PRI", "esES" },
			{ "URY", "esES" },
			{ "VEN", "esES" },
		};

		int convert_utf8_to_windows1251(const char* utf8, char* windows1251, size_t n);
		int convert_windows1252_to_utf8(char * windows1252);


		extern int i18n_reload(void)
		{
			translations.clear();
			i18n_load();

			return 0;
		}

		extern int i18n_load(void)
		{
			std::string lang_filename;
			pugi::xml_document doc;
			std::string original, translate;

			// iterate language list
			for (int i = 0; i < (sizeof(languages) / sizeof(*languages)); i++)
			{
				lang_filename = i18n_filename(prefs_get_localizefile(), languages[i]);
				if (FILE *f = fopen(lang_filename.c_str(), "r")) {
					fclose(f);

					if (!doc.load_file(lang_filename.c_str()))
					{
						ERROR1("could not parse localization file \"%s\"", lang_filename.c_str());
						continue;
					}
				}
				else {
					// file not exists, ignore it
					continue;
				}

				// root node
				pugi::xml_node nodes = doc.child("root").child("items");

				// load xml strings to map
				for (pugi::xml_node node = nodes.child("item"); node; node = node.next_sibling("item"))
				{
					original = node.child_value("original");
					if (original[0] == '\0')
						continue;

					//std::map<const char *, std::map<t_gamelang, const char *> >::iterator it = translations.find(original);
					// if not found then init
					//if (it == translations.end())
					//	translations[original] = std::map<t_gamelang, const char *>();
					

					// check if translate string has a reference to another translation
					if (pugi::xml_attribute attr = node.child("translate").attribute("refid"))
					{
						if (pugi::xml_node n = nodes.find_child_by_attribute("id", attr.value()))
							translate = n.child_value("translate");
						else
						{
							translate = original;
							//WARN2("could not find translate reference refid=\"%s\", use original string (%s)", attr.value(), lang_filename.c_str());
						}
					}
					else
					{
						translate = node.child_value("translate");
						if (translate[0] == '\0')
						{
							translate = original;
							//WARN2("empty translate for \"%s\", use original string (%s)", original.c_str(), lang_filename.c_str());
						}
					}
					translations[original][languages[i]] = translate;
				}

				INFO1("localization file loaded \"%s\"", lang_filename.c_str());
			}

			return 0;
		}

		extern std::string _localize(t_connection * c, const char * func, const char * fmt, const fmt::ArgList &args)
		{
			const char *format = fmt;
			std::string output(fmt);
			t_gamelang lang;

			if (!c) {
				eventlog(eventlog_level_error, __FUNCTION__, "got bad connection");
				return format;
			}
			try
			{
				if (lang = conn_get_gamelang_localized(c))
				if (!(format = _find_string(fmt, lang)))
					format = fmt; // if not found use original

				output = fmt::format(format, args);

				char* p = (char*)calloc(30, sizeof(char));
				strcpy(p, "Hello world");

				char tmp[MAX_MESSAGE_LEN];
				strcpy(tmp, output.c_str());

				i18n_convert(c, tmp);
				output = tmp;
			}
			catch (const std::exception& e)
			{
				ERROR2("Can't format translation string \"%s\" (%s)", fmt, e.what());
			}

			return output;
		}


		/* Find localized text for the given language */
		const char * _find_string(char const * text, t_gamelang gamelang)
		{
			std::map<std::string, std::map<t_gamelang, std::string> >::iterator it = translations.find(text);
			if (it != translations.end())
			{
				if (!it->second[gamelang].empty())
					return it->second[gamelang].c_str();
			}
			return NULL;
		}

		/* Add a locale tag into filename
		example: motd.txt -> motd-ruRU.txt */
		extern const char * i18n_filename(const char * filename, t_tag gamelang)
		{
			// get language string
			char lang_str[sizeof(t_tag)+1];
			std::memset(lang_str, 0, sizeof(lang_str));
			tag_uint_to_str(lang_str, gamelang);

			struct stat sfile;
			const char * _filename;
			
			_filename = buildpath(buildpath(prefs_get_i18ndir(), lang_str), filename);
			// if localized file not found
			if (stat(_filename, &sfile) < 0)
			{
				// use default file
				_filename = buildpath(prefs_get_i18ndir(), filename);
			}

			return _filename;
		}

		/* Return language tag by code */
		extern t_gamelang lang_find_by_country(const char * code)
		{
			if (!code || code[0] == '\0')
				return tag_str_to_uint(countries[0][0]);

			for (int i = 0; i < (sizeof(countries) / sizeof(*countries)); i++)
			if (strcasecmp(code, countries[i][0]) == 0)
				return tag_str_to_uint(countries[i][1]);

			return tag_str_to_uint(countries[0][1]); // default
		}

		extern t_gamelang conn_get_gamelang_localized(t_connection * c)
		{
			t_gamelang lang = conn_get_gamelang(c);

			// force localize by user country
			if (prefs_get_localize_by_country())
			if (const char * country = conn_get_country(c))
				lang = lang_find_by_country(country);

			// if user set own language
			if (t_account * a = conn_get_account(c))
			if (const char * l = account_get_userlang(a))
				lang = tag_str_to_uint(l);

			// FIXME: Russian text displays not correctly directly in game in non-russian Starcraft
			//        but most Russians use English Starcraft.
			//        Instead of next hack we can add "if (conn_get_game(c) {}" in i18n_convert() and return transliterated text
			//
			// Return English text in game for Russian
			if (t_clienttag clienttag = conn_get_clienttag(c))
			if (lang == GAMELANG_RUSSIAN_UINT && conn_get_game(c) &&
				(clienttag == CLIENTTAG_STARCRAFT_UINT || clienttag == CLIENTTAG_BROODWARS_UINT || clienttag == CLIENTTAG_STARJAPAN_UINT || clienttag == CLIENTTAG_SHAREWARE_UINT ||
				clienttag == CLIENTTAG_DIABLORTL_UINT || clienttag == CLIENTTAG_DIABLOSHR_UINT || clienttag == CLIENTTAG_WARCIIBNE_UINT))
				lang = GAMELANG_ENGLISH_UINT;


			return lang;
		}


		/* Set custom user language (command) */
		extern int handle_language_command(t_connection * c, char const *text)
		{
			// split command args
			std::vector<std::string> args = split_command(text, 3);
			if (args[1].empty())
			{
				// display command help
				describe_command(c, args[0].c_str());


				std::string out = "     ";
				char lang_str[5];
				// display available language list
				for (int i = 0; i < (sizeof(languages) / sizeof(*languages)); i++)
				{
					tag_uint_to_str(lang_str, languages[i]);

					// select with brackets current user language
					if (languages[i] == conn_get_gamelang_localized(c))
						out += "[" + std::string(lang_str) + "]";
					else
						out += lang_str;
					if (i < (sizeof(languages) / sizeof(*languages)) - 1)
						out += ", ";
				}
				message_send_text(c, message_type_info, c, out.c_str());

				return -1;
			}

			const char * userlang = args[1].c_str();

			// validate given language
			if (!tag_check_gamelang(tag_str_to_uint(userlang)))
			{
				message_send_text(c, message_type_error, c, localize(c, "Bad language code."));
				return -1;
			}

			if (t_account * account = conn_get_account(c))
			{
				account_set_userlang(account, userlang);
				message_send_text(c, message_type_error, c, localize(c, "Set your language to {}", userlang));
			}
			return 0;
		}



		/* Convert text with user native encoding */
		extern char * i18n_convert(t_connection * c, char * buf)
		{
			t_gamelang gamelang = conn_get_gamelang_localized(c);
			t_clienttag clienttag = conn_get_clienttag(c);

			switch (gamelang)
			{
				case GAMELANG_RUSSIAN_UINT:
					// Starcraft, Diablo 1, Warcraft 2
					if (clienttag == CLIENTTAG_STARCRAFT_UINT || clienttag == CLIENTTAG_BROODWARS_UINT || clienttag == CLIENTTAG_STARJAPAN_UINT || clienttag == CLIENTTAG_SHAREWARE_UINT ||
						clienttag == CLIENTTAG_DIABLORTL_UINT || clienttag == CLIENTTAG_DIABLOSHR_UINT || clienttag == CLIENTTAG_WARCIIBNE_UINT)
					{
						convert_utf8_to_windows1251(buf, buf, MAX_MESSAGE_LEN);
						convert_windows1252_to_utf8(buf);
					}
					break;

				default:
					break;
				}
				return buf;
		}


/*
* Text conversion for Russian text
*/
#ifndef REGION_CONVERT_RUSSIAN

		typedef struct ConvLetter {
			char    win1251;
			int             unicode;
		} Letter;

		static Letter g_letters[] = { { 0x82, 0x201A }, { 0x83, 0x0453 }, { 0x84, 0x201E }, { 0x85, 0x2026 }, { 0x86, 0x2020 }, { 0x87, 0x2021 }, { 0x88, 0x20AC }, { 0x89, 0x2030 }, { 0x8A, 0x0409 }, { 0x8B, 0x2039 }, { 0x8C, 0x040A }, { 0x8D, 0x040C }, { 0x8E, 0x040B }, { 0x8F, 0x040F }, { 0x90, 0x0452 }, { 0x91, 0x2018 }, { 0x92, 0x2019 }, { 0x93, 0x201C }, { 0x94, 0x201D }, { 0x95, 0x2022 }, { 0x96, 0x2013 }, { 0x97, 0x2014 }, { 0x99, 0x2122 }, { 0x9A, 0x0459 }, { 0x9B, 0x203A }, { 0x9C, 0x045A }, { 0x9D, 0x045C }, { 0x9E, 0x045B }, { 0x9F, 0x045F }, { 0xA0, 0x00A0 }, { 0xA1, 0x040E }, { 0xA2, 0x045E }, { 0xA3, 0x0408 }, { 0xA4, 0x00A4 }, { 0xA5, 0x0490 }, { 0xA6, 0x00A6 }, { 0xA7, 0x00A7 }, { 0xA8, 0x0401 }, { 0xA9, 0x00A9 }, { 0xAA, 0x0404 }, { 0xAB, 0x00AB }, { 0xAC, 0x00AC }, { 0xAD, 0x00AD }, { 0xAE, 0x00AE }, { 0xAF, 0x0407 }, { 0xB0, 0x00B0 }, { 0xB1, 0x00B1 }, { 0xB2, 0x0406 }, { 0xB3, 0x0456 }, { 0xB4, 0x0491 }, { 0xB5, 0x00B5 }, { 0xB6, 0x00B6 }, { 0xB7, 0x00B7 }, { 0xB8, 0x0451 }, { 0xB9, 0x2116 }, { 0xBA, 0x0454 }, { 0xBB, 0x00BB }, { 0xBC, 0x0458 }, { 0xBD, 0x0405 }, { 0xBE, 0x0455 }, { 0xBF, 0x0457 } };
		
		// https://code.google.com/p/convert-utf8-to-cp1251/
		int convert_utf8_to_windows1251(const char* utf8, char* windows1251, size_t n)
		{
			int i = 0;
			int j = 0;
			for (; i < (int)n && utf8[i] != 0; ++i) {
				char prefix = utf8[i];
				char suffix = utf8[i + 1];
				if ((prefix & 0x80) == 0) {
					windows1251[j] = (char)prefix;
					++j;
				}
				else if ((~prefix) & 0x20) {
					int first5bit = prefix & 0x1F;
					first5bit <<= 6;
					int sec6bit = suffix & 0x3F;
					int unicode_char = first5bit + sec6bit;

					if (unicode_char >= 0x410 && unicode_char <= 0x44F) {
						windows1251[j] = (char)(unicode_char - 0x350);
					}
					else if (unicode_char >= 0x80 && unicode_char <= 0xFF) {
						windows1251[j] = (char)(unicode_char);
					}
					else if (unicode_char >= 0x402 && unicode_char <= 0x403) {
						windows1251[j] = (char)(unicode_char - 0x382);
					}
					else {
						int count = sizeof(g_letters) / sizeof(Letter);
						for (int k = 0; k < count; ++k) {
							if (unicode_char == g_letters[k].unicode) {
								windows1251[j] = g_letters[k].win1251;
								goto NEXT_LETTER;
							}
						}
						// can't convert this char
						return 0;
					}
				NEXT_LETTER:
					++i;
					++j;
				}
				else {
					// can't convert this chars
					return 0;
				}
			}
			windows1251[j] = 0;
			return 1;
		}

		int convert_windows1252_to_utf8(char * windows1252)
		{
			int i = 0, j = 0;
			unsigned char c, temp[512];

			while (windows1252[i] != 0)
			{
				c = windows1252[i];

				if (c < 128)
				{
					temp[j] = c;
					j++;
				}
				else
				{
					temp[j] = (c >> 6) | 192;
					temp[j + 1] = (c & 63) | 128;
					j += 2;
				}
				i++;
			}
			temp[j] = 0;
			memcpy(windows1252, temp, j + 1);

			return 0;
		}
#endif






	}
}
