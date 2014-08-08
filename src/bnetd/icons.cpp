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
#include <vector>
#include <string.h>

#include "compat/strcasecmp.h"
#include "compat/snprintf.h"

#include "common/token.h"

#include "common/list.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/xstring.h"
#include "common/util.h"
#include "common/tag.h"

#include "account.h"
#include "connection.h"
#include "icons.h"
#include "account_wrap.h"

#include "message.h"
#include "helpfile.h"
#include "channel.h"
#include "command.h"
#include "i18n.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static t_list * icon_head = NULL;
		// TODO: wrapper to get value
		static int enable_custom_icons = 0;


		static int skip_comments(char *buff);
		static t_icon_var_info * _read_option(char *str, unsigned lineno);
		static char * _find_attr_key(char * clienttag);



		/* Set usericon (command) */
		extern int handle_icon_command(t_connection * c, char const *text)
		{
			t_account * account;
			t_connection * user_c;
			char const * channel_name;
			char const * subcommand, *username, *iconname;
			char const *usericon, *iconalias, *iconcode;
			t_clienttag clienttag;
			char clienttag_str[5];
			std::string output_icons = "";
			int count = 1; // 1 icon in stash ("default" icon)
			bool is_selected = false;

			std::string	msgtemp;

			if (!(conn_get_channel(c))) {
				message_send_text(c, message_type_error, c, localize(c, "This command can only be used inside a channel."));
				return -1;
			}
			else {
				channel_name = channel_get_name(conn_get_channel(c));
			}

			// get current clienttag
			clienttag = conn_get_clienttag(c);
			if (!clienttag || clienttag == CLIENTTAG_BNCHATBOT_UINT)
			{
				message_send_text(c, message_type_error, c, localize(c, "This command can only be used from the game."));
				return -1;
			}


			// split command args
			std::vector<std::string> args = split_command(text, 3);

			if (!(account_is_operator_or_admin(conn_get_account(c), channel_name)))
			{
				/* Simple command syntax for users */

				iconname = args[1].c_str();  // icon code

				account = conn_get_account(c);

				// get current user icon
				if (usericon = account_get_user_icon(account, clienttag))
					usericon = strreverse(xstrdup(usericon));

				bool is_found = false;
				// get user stash
				if (char const * iconstash = account_get_user_iconstash(account, clienttag))
				{
					std::string s(iconstash);
					std::istringstream iss(s);
					do
					{
						std::string _icon;
						iss >> _icon;
						if (_icon.empty()) continue;

						// output icon_alias instead of icon_code
						if (iconalias = customicons_stash_find(clienttag, _icon.c_str(), true))
							_icon = std::string(iconalias);

						if (!(iconcode = customicons_stash_find(clienttag, _icon.c_str())))
							iconcode = iconalias = _icon.c_str(); // icon was added earlier to user and then removed from server stash


						// set selected icon in brackets
						if (usericon && strcasecmp(usericon, iconcode) == 0)
						{
							_icon = "[" + _icon + "]";
							is_selected = true;
						}

						if (!output_icons.empty())
							output_icons += ", ";
						output_icons += _icon;

						// if iconname parameter passed then find it in user stash
						if (iconname[0] != '\0')
						{
							if (strcasecmp(iconname, iconalias) == 0 || strcasecmp(iconname, iconcode) == 0)
								is_found = true;
						}

						count++;
					} while (iss);
				}

				if (iconname[0] != '\0')
				{
					// unset value
					if (strcasecmp(iconname, "default") == 0)
					{
						msgtemp = localize(c, "Set default icon.");
						usericon = NULL;
					}
					// set usericon (reversed)
					else
					{
						// if icon not found in server stash
						if (!(iconcode = customicons_stash_find(clienttag, iconname)))
						{
							// set icon code from args
							std::transform(args[1].begin(), args[1].end(), args[1].begin(), ::toupper); // to upper
							iconcode = args[1].c_str();
						}
						if (!is_found || strlen(iconcode) != 4)
						{
							message_send_text(c, message_type_error, c, localize(c, "Bad icon."));
							return -1;
						}
						msgtemp = localize(c, "Set new icon is succeed.");
						usericon = strreverse((char*)iconcode);
					}
					account_set_user_icon(account, clienttag, usericon);
					message_send_text(c, message_type_info, c, msgtemp);

					// if user online then force him to rejoin channel
					if (c)
					{
						conn_update_w3_playerinfo(c);
						channel_rejoin(c);
					}
					return 0;
				}

				// display icon list in user stash
				msgtemp = localize(c, "You have {} icons in stash:", count);
				message_send_text(c, message_type_info, c, msgtemp);

				output_icons = ((is_selected || usericon) ? "default" : "[default]") + std::string((count > 1) ? ", " : "") + output_icons;

				msgtemp = "   " + output_icons;
				message_send_text(c, message_type_info, c, msgtemp);

				return 0;
			}

			/* Complex command syntax for operator and admins */

			if (args[1].empty())
			{
				describe_command(c, args[0].c_str());
				return -1;
			}
			subcommand = args[1].c_str(); // sub command
			username = args[2].c_str(); // username
			iconname = args[3].c_str();  // icon alias or code

			// display icons from the server stash
			// subcommand = list
			if (subcommand[0] == 'l' && username[0] == '\0')
			{
				message_send_text(c, message_type_info, c, localize(c, "Available icons in server stash:"));
				std::string output_icons = customicons_stash_get_list(clienttag, true);
				msgtemp = "   " + output_icons;
				message_send_text(c, message_type_info, c, msgtemp);
				return 0;
			}


			// find user account
			if (!(account = accountlist_find_account(username)))
			{
				message_send_text(c, message_type_error, c, localize(c, "Invalid user."));
				return 0;
			}
			user_c = account_get_conn(account);


			switch (std::tolower(subcommand[0]))
			{
				// set
				case 's':
					if (iconname[0] == '\0')
					{
						describe_command(c, args[0].c_str());
						return -1;
					}

					// unset value
					if (strcasecmp(iconname, "default") == 0)
					{
						msgtemp = localize(c, "Set default icon for {}", account_get_name(account));
						usericon = NULL;
					}
					// set usericon (reversed)
					else
					{
						// find icon in server stash
						if (!(iconcode = customicons_stash_find(clienttag, iconname)))
						{
							message_send_text(c, message_type_error, c, localize(c, "That icon doesn't exist in server stash."));
							return -1;
						}
						msgtemp = localize(c, "Set new icon is succeed for {}", account_get_name(account));
						usericon = strreverse((char*)iconcode);
					}
					account_set_user_icon(account, clienttag, usericon);
					message_send_text(c, message_type_info, c, msgtemp);

					// if user online then force him to rejoin channel
					if (user_c)
					{
						conn_update_w3_playerinfo(user_c);
						channel_rejoin(user_c);
					}

					return 0;
					break;

				// list
				case 'l':
					// get current user icon
					if (usericon = account_get_user_icon(account, clienttag))
						usericon = strreverse(xstrdup(usericon));

					// get user stash
					if (char const * iconstash = account_get_user_iconstash(account, clienttag))
					{
						std::string s(iconstash);
						std::istringstream iss(s);
						do
						{
							std::string _icon;
							iss >> _icon;
							if (_icon.empty()) continue;

							if (!(iconcode = customicons_stash_find(clienttag, _icon.c_str())))
								iconcode = _icon.c_str(); // icon was added earlier to user and then removed from server stash

							// output icon_alias instead of icon_code
							if (iconalias = customicons_stash_find(clienttag, _icon.c_str(), true))
								_icon = std::string(iconalias); // output key instead of code

							// set selected icon in brackets
							if (usericon && strcasecmp(usericon, iconcode) == 0) 
							{
								_icon = "[" + _icon + "]";
								is_selected = true;
							}

							if (!output_icons.empty())
								output_icons += ", ";

							output_icons += _icon;

							count++;
						} while (iss);
					}

					// display icon list in user stash
					msgtemp = localize(c, "{} has {} icons in stash : ", account_get_name(account), count);
					message_send_text(c, message_type_info, c, msgtemp);

					output_icons = ((is_selected || usericon) ? "default" : "[default]") + std::string((count > 1) ? ", " : "") + output_icons;

					msgtemp = "   " + output_icons;
					message_send_text(c, message_type_info, c, msgtemp);


					return 0;
					break;

				// add
				case 'a':
					if (iconname[0] == '\0')
					{
						describe_command(c, args[0].c_str());
						return -1;
					}
					// find icon in server stash
					if (!(iconcode = customicons_stash_find(clienttag, iconname)))
					{
						message_send_text(c, message_type_error, c, localize(c, "That icon doesn't exist in server stash."));
						return -1;
					}

					// find icon in user stash
					if (char const * iconstash = account_get_user_iconstash(account, clienttag))
					{
						std::string s(iconstash);
						std::istringstream iss(s);
						do
						{
							std::string _icon;
							iss >> _icon;
							if (_icon.empty()) continue;

							if (!output_icons.empty())
								output_icons += " ";
							output_icons += _icon;

							if (strcasecmp(_icon.c_str(), iconcode) == 0)
							{
								message_send_text(c, message_type_error, c, localize(c, "User already has that icon in stash."));
								return 0;
							}
						} while (iss);
					}
					// append new icon
					if (!output_icons.empty())
						output_icons += " ";
					output_icons += std::string(iconcode);

					// save stash
					account_set_user_iconstash(account, clienttag, output_icons.c_str());

					msgtemp = localize(c, "Add new icon to {}'s stash.", account_get_name(account));
					message_send_text(c, message_type_info, c, msgtemp);

					return 0;
					break;

				// del
				case 'd':
					if (iconname[0] == '\0')
					{
						describe_command(c, args[0].c_str());
						return -1;
					}
					bool is_found = false;
					if (char const * iconstash = account_get_user_iconstash(account, clienttag))
					{
						// get current user icon
						if (usericon = account_get_user_icon(account, clienttag))
							usericon = strreverse(xstrdup(usericon));

						std::string s(iconstash);
						std::istringstream iss(s);
						do
						{
							std::string _icon;
							iss >> _icon;
							if (_icon.empty()) continue;

							iconalias = customicons_stash_find(clienttag, _icon.c_str(), true);
							if (!(iconcode = customicons_stash_find(clienttag, _icon.c_str())))
								iconcode = iconalias = _icon.c_str(); // icon was added earlier to user and then removed from server stash

							// exclude deleted icon (allow to delete icon by code or alias)
							if (strcasecmp(iconname, iconalias) == 0 || strcasecmp(iconname, iconcode) == 0)
							{
								// also unset current user icon if it equals with given
								if (usericon && strcasecmp(usericon, iconcode) == 0)
									account_set_user_icon(account, clienttag, NULL);

								is_found = true;
								continue;
							}

							if (!output_icons.empty())
								output_icons += " ";
							output_icons += _icon;
						} while (iss);
					}
					if (!is_found)
					{
						message_send_text(c, message_type_error, c, localize(c, "That icon doesn't exist in user stash."));
						return -1;
					}
					// save stash
					account_set_user_iconstash(account, clienttag, output_icons.c_str());

					msgtemp = localize(c, "Delete icon from {}'s stash.", account_get_name(account));
					message_send_text(c, message_type_info, c, msgtemp);

					return 0;
					break;
			}

			return 0;
		}



		/* Search usericon in available list by icon code or alias, for given clienttag
		*   return icon code if found, and NULL if not found
		*   if (return_alias == true) then aias is returned
		*/
		extern char const * customicons_stash_find(t_clienttag clienttag, char const * code, bool return_alias)
		{
			t_elem *		curr;
			t_elem *		curr_var;
			t_icon_var_info *		var;
			t_iconset_info * iconset;
			t_icon_var_info *		item;

			if (!code || !clienttag)
				return NULL;

			char clienttag_str[5];
			tag_uint_to_str(clienttag_str, clienttag);

			if (icon_head) {
				LIST_TRAVERSE(icon_head, curr)
				{
					if (!(iconset = (t_iconset_info*)elem_get_data(curr))) {
						eventlog(eventlog_level_error, __FUNCTION__, "icon list contains NULL item");
						continue;
					}

					// find a needed tag
					if (std::strcmp(iconset->clienttag, clienttag_str) != 0)
						continue;

					LIST_TRAVERSE(iconset->iconstash, curr_var)
					{
						if (!(var = (t_icon_var_info*)elem_get_data(curr_var)))
						{
							eventlog(eventlog_level_error, __FUNCTION__, "vars list contains NULL item");
							continue;
						}

						if (strcasecmp(var->key, code) == 0 || strcasecmp(var->value, code) == 0)
						{
							char const * val = (return_alias)
								? xstrdup(var->key)
								: xstrdup(var->value);
							return val;
						}
					}
				}
			}
			return NULL;
		}


		/* Return comma delimeted icons from stash
		*/
		extern std::string customicons_stash_get_list(t_clienttag clienttag, bool return_alias)
		{
			t_elem *		curr;
			t_elem *		curr_var;
			t_icon_var_info *		var;
			t_iconset_info * iconset;
			t_icon_var_info *		item;

			std::string output = "";

			if (!clienttag)
				return output;

			char clienttag_str[5];
			tag_uint_to_str(clienttag_str, clienttag);

			if (icon_head) {
				LIST_TRAVERSE(icon_head, curr)
				{
					if (!(iconset = (t_iconset_info*)elem_get_data(curr))) {
						eventlog(eventlog_level_error, __FUNCTION__, "icon list contains NULL item");
						continue;
					}

					// find a needed tag
					if (std::strcmp(iconset->clienttag, clienttag_str) != 0)
						continue;

					LIST_TRAVERSE(iconset->iconstash, curr_var)
					{
						if (!(var = (t_icon_var_info*)elem_get_data(curr_var)))
						{
							eventlog(eventlog_level_error, __FUNCTION__, "vars list contains NULL item");
							continue;
						}


						char const * val = (return_alias) 
							? xstrdup(var->key)
							: xstrdup(var->value);

						if (!output.empty())
							output += ", ";
						output += std::string(val);
					}
					break;
				}
			}
			return output;
		}


		extern int prefs_get_custom_icons()
		{
			return enable_custom_icons;
		}


		/* Format stats text, with attributes from a storage, and output text to a user */
		extern const char * customicons_get_stats_text(t_account * account, t_clienttag clienttag)
		{
			const char *value;
			const char *text;
			char clienttag_str[5], tmp[64];
			t_icon_info * icon;
			t_iconset_info * iconset;
			t_icon_var_info * var;
			t_elem *		curr;
			t_elem *		curr_var;

			tag_uint_to_str(clienttag_str, clienttag);

			text = NULL;

			if (icon_head) {
				LIST_TRAVERSE(icon_head, curr)
				{
					if (!(iconset = (t_iconset_info*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "icon list contains NULL item");
						continue;
					}

					// find a needed tag
					if (std::strcmp(iconset->clienttag, clienttag_str) != 0)
						continue;

					if (!iconset->stats)
						return NULL;

					text = xstrdup(iconset->stats);

					LIST_TRAVERSE(iconset->vars, curr_var)
					{
						if (!(var = (t_icon_var_info*)elem_get_data(curr_var)))
						{
							eventlog(eventlog_level_error, __FUNCTION__, "vars list contains NULL item");
							continue;
						}

						// replace NULL attributes to "0"
						if (!(value = account_get_strattr(account, var->value)))
							value = "0";

						// replace in a text
						snprintf(tmp, sizeof(tmp), "{{%s}}", var->key);
						text = str_replace((char*)text, tmp, (char*)value);

						// also replace {var}->rank
						if (icon = customicons_get_icon_by_rating(atoi(value), clienttag_str))
						{
							snprintf(tmp, sizeof(tmp), "{{%s->rank}}", var->key);
							text = str_replace((char*)text, tmp, icon->rank);
						}
					}
				}
			}

			return text;
		}


		/* find icon code by rating for the clienttag */
		extern t_icon_info * customicons_get_icon_by_account(t_account * account, t_clienttag clienttag)
		{
			char * attr_key;
			int rating;
			char clienttag_str[5];
			t_icon_info * icon;
			t_icon_info * uicon;
			const char * usericon;

			tag_uint_to_str(clienttag_str, clienttag);

			// get attribute field name from a storage
			if (!(attr_key = _find_attr_key((char*)clienttag_str)))
			{
				eventlog(eventlog_level_trace, __FUNCTION__, "could not find attr_key in iconset for tag %s", clienttag_str);
				return NULL;
			}

			rating = account_get_numattr(account, attr_key);

			icon = customicons_get_icon_by_rating(rating, clienttag_str);
			return icon;
		}


		extern int customicons_unload(void)
		{
			t_elem *		curr;
			t_iconset_info *		iconset;

			if (icon_head) {
				LIST_TRAVERSE(icon_head, curr)
				{
					if (!(iconset = (t_iconset_info*)elem_get_data(curr))) {
						eventlog(eventlog_level_error, __FUNCTION__, "icon list contains NULL item");
						continue;
					}

					if (list_remove_elem(icon_head, &curr) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "could not remove item from list");

					xfree(iconset);
				}

				if (list_destroy(icon_head) < 0)
					return -1;
				icon_head = NULL;
			}
			return 0;
		}


		/*****/
		extern int customicons_load(char const * filename)
		{
			std::FILE * fp;
			unsigned int line, pos, counter = 0;
			bool end_of_iconset = false;
			char *buff, *value;
			char *rating, *rank, *icon;
			t_icon_var_info * option;

			icon_head = list_create();
			unsigned int fpos = 0;

			if (!filename) {
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
				return -1;
			}

			if (!(fp = std::fopen(filename, "r"))) {
				eventlog(eventlog_level_error, __FUNCTION__, "could not open file \"%s\" for reading (std::fopen: %s)", filename, std::strerror(errno));
				return -1;
			}

			/* 1) parse root config options */
			for (line = 1; (buff = file_get_line(fp)); line++)
			{
				if (skip_comments(buff) > 0)
				{
					continue;
				}
				// read root options
				if (option = _read_option(buff, line))
				{
					if (std::strcmp(option->key, "custom_icons") == 0)
						if (std::strcmp(option->value, "true") == 0)
							enable_custom_icons = 1;
						else
							enable_custom_icons = 0;
				}


				/* 2) parse clienttags */
				if (std::strcmp(buff, "[W3XP]") == 0 || std::strcmp(buff, "[WAR3]") == 0 || std::strcmp(buff, "[STAR]") == 0 || 
					std::strcmp(buff, "[SEXP]") == 0 || std::strcmp(buff, "[JSTR]") == 0 || std::strcmp(buff, "[SSHR]") == 0 || 
					std::strcmp(buff, "[W2BN]") == 0 || std::strcmp(buff, "[DRTL]") == 0 || std::strcmp(buff, "[DSHR]") == 0)
				{
					if (skip_comments(buff) > 0)
					{
						continue;
					}
					value = std::strtok(buff, " []"); // extract clienttag

					// new iconset for a clienttag
					t_iconset_info * icon_set = (t_iconset_info*)xmalloc(sizeof(t_iconset_info));
					icon_set->clienttag = xstrdup(value);
					icon_set->attr_key = NULL;
					icon_set->icon_info = list_create();
					icon_set->iconstash = list_create();
					icon_set->vars = list_create();
					icon_set->stats = NULL;


					/* 3) parse inner options under a clienttag */
					for (; (buff = file_get_line(fp)); line++)
					{
						if (std::strcmp(buff, "[W3XP]") == 0 || std::strcmp(buff, "[WAR3]") == 0 || std::strcmp(buff, "[STAR]") == 0 ||
							std::strcmp(buff, "[SEXP]") == 0 || std::strcmp(buff, "[JSTR]") == 0 || std::strcmp(buff, "[SSHR]") == 0 ||
							std::strcmp(buff, "[W2BN]") == 0 || std::strcmp(buff, "[DRTL]") == 0 || std::strcmp(buff, "[DSHR]") == 0)
						{
							// return position in stream
							fseek(fp, fpos, SEEK_SET);
							break;
						}
						if (skip_comments(buff) > 0)
						{
							continue;
						}
						// fill variables list
						if (option = _read_option(buff, line))
						{
							// set attr_key with a first variable
							if (!icon_set->attr_key)
								icon_set->attr_key = xstrdup(option->value);

							// add to variables
							list_append_data(icon_set->vars, option);

							continue;
						}

						/* 3) parse icons section */
						if (std::strcmp(buff, "[icons]") == 0)
						{
							counter = 0;
							for (; (buff = file_get_line(fp)); line++)
							{
								if (skip_comments(buff) > 0)
								{
									continue;
								}
								// end if icons
								if (std::strcmp(buff, "[/icons]") == 0) {
									break;
								}

								pos = 0;
								if (!(rating = next_token(buff, &pos)))
								{
									eventlog(eventlog_level_error, __FUNCTION__, "missing value in line %u in file \"%s\"", line, filename);
									continue;
								}
								if (!(rank = next_token(buff, &pos)))
								{
									eventlog(eventlog_level_error, __FUNCTION__, "missing rank in line %u in file \"%s\"", line, filename);
									continue;
								}
								if (!(icon = next_token(buff, &pos)))
								{
									eventlog(eventlog_level_error, __FUNCTION__, "missing icon in line %u in file \"%s\"", line, filename);
									continue;
								}
								counter++;

								t_icon_info * icon_info = (t_icon_info*)xmalloc(sizeof(t_icon_info));
								icon_info->rating = atoi(rating);
								icon_info->rank = xstrdup(rank);
								icon_info->icon_code = xstrdup(strreverse(icon)); // save reversed icon code
								list_prepend_data(icon_set->icon_info, icon_info);
							}
						}

						/* 3) parse stats section */
						if (std::strcmp(buff, "[stats]") == 0)
						{
							std::string tmp;
							for (; (buff = file_get_line(fp)); line++)
							{
								// end of stats
								if (std::strcmp(buff, "[/stats]") == 0) {
									// put whole text of stats after read
									icon_set->stats = xstrdup(tmp.c_str());
									break;
								}
								tmp = tmp + buff + "\n";
							}
						}


						/* 4) parse usericons section */
						if (std::strcmp(buff, "[iconstash]") == 0)
						{
							char *key, *value;
							counter = 0;
							for (; (buff = file_get_line(fp)); line++)
							{
								if (skip_comments(buff) > 0)
								{
									continue;
								}
								// end if usericons
								if (std::strcmp(buff, "[/iconstash]") == 0) {
									break;
								}

								pos = 0;
								if (!(key = next_token(buff, &pos)))
									continue;
								if (!(value = next_token(buff, &pos)))
									value = key;
								counter++;

								t_icon_var_info * icon_item = (t_icon_var_info*)xmalloc(sizeof(t_icon_var_info));
								icon_item->key = xstrdup(key);
								icon_item->value = xstrdup(value); 
								list_append_data(icon_set->iconstash, icon_item);
							}
						}

						// remember file position
						fpos = ftell(fp);
					}

					if (!icon_set->attr_key)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "attr_key is null for iconset %s", icon_set->clienttag);
						continue;
					}
					list_append_data(icon_head, icon_set);

					eventlog(eventlog_level_trace, __FUNCTION__, "loaded %u custom icons for %s", counter, icon_set->clienttag);
				}
			}

			return 0;
		}

		static int skip_comments(char *buff)
		{
			char *temp;
			int pos;

			for (pos = 0; buff[pos] == '\t' || buff[pos] == ' '; pos++);
			if (buff[pos] == '\0' || buff[pos] == '#') {
				return 1;
			}
			if ((temp = std::strrchr(buff, '#'))) {
				unsigned int len;
				unsigned int endpos;

				*temp = '\0';
				len = std::strlen(buff) + 1;
				for (endpos = len - 1; buff[endpos] == '\t' || buff[endpos] == ' '; endpos--);
				buff[endpos + 1] = '\0';
			}
			return 0;
		}

		/* Read an option like "key = value" from a str line, and split in into a pair: key, value */
		static t_icon_var_info * _read_option(char *str, unsigned lineno)
		{
			char *cp, prev, *directive;
			t_icon_var_info * icon_var = (t_icon_var_info*)xmalloc(sizeof(t_icon_var_info));

			directive = str;
			str = str_skip_word(str + 1);
			if (*str) *(str++) = '\0';

			str = str_skip_space(str);
			if (*str != '=') {
				return NULL;
			}

			str = str_skip_space(str + 1);
			if (!*str) {
				eventlog(eventlog_level_error, __FUNCTION__, "missing value at line %u", lineno);
				return NULL;
			}

			if (*str == '"') {
				for (cp = ++str, prev = '\0'; *cp; cp++) {
					switch (*cp) {
					case '"':
						if (prev != '\\') break;
						prev = '"';
						continue;
					case '\\':
						if (prev == '\\') prev = '\0';
						else prev = '\\';
						continue;
					default:
						prev = *cp;
						continue;
					}
					break;
				}

				if (*cp != '"') {
					eventlog(eventlog_level_error, __FUNCTION__, "missing end quota at line %u", lineno);
					return NULL;
				}

				*cp = '\0';
				cp = str_skip_space(cp + 1);
				if (*cp) {
					eventlog(eventlog_level_error, __FUNCTION__, "extra characters in value after ending quote at line %u", lineno);
					return NULL;
				}
			}
			else {
				cp = str_skip_word(str);
				if (*cp) {
					*cp = '\0';
					cp = str_skip_space(cp + 1);
					if (*cp) {
						eventlog(eventlog_level_error, __FUNCTION__, "extra characters after the value at line %u", lineno);
						return NULL;
					}
				}
			}

			//if (!find && std::strcmp(find, directive) == 0)
			//	return str_skip_space(str);
			//else
			//	return NULL;

			icon_var->key = xstrdup(directive);
			icon_var->value = xstrdup(str_skip_space(str));

			return icon_var;
		}




		/* Get custom icon by rating for clienttag */
		extern t_icon_info * customicons_get_icon_by_rating(int rating, char * clienttag)
		{
			t_elem *		curr;
			t_elem *		curr_icon;
			t_iconset_info *		iconset;
			t_icon_info *		icon;

			if (!clienttag)
				return NULL;

			if (icon_head) {
				LIST_TRAVERSE(icon_head, curr)
				{
					if (!(iconset = (t_iconset_info*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "icon list contains NULL item");
						continue;
					}

					// find a needed tag
					if (std::strcmp(iconset->clienttag, clienttag) != 0)
						continue;

					// iterate all icons for the tag
					LIST_TRAVERSE(iconset->icon_info, curr_icon)
					{
						if (!(icon = (t_icon_info*)elem_get_data(curr_icon)))
						{
							eventlog(eventlog_level_error, __FUNCTION__, "icon list contains NULL item");
							continue;
						}

						if (rating >= icon->rating)
							return icon;
					}
					// if nothing found then return last icon
					return icon;
				}
			}
			return NULL;
		}


		/* Get attr_key for clienttag */
		static char * _find_attr_key(char * clienttag)
		{
			t_elem *		curr;
			t_elem *		curr_icon;
			t_iconset_info *		iconset;

			if (icon_head) {
				LIST_TRAVERSE(icon_head, curr)
				{
					if (!(iconset = (t_iconset_info*)elem_get_data(curr)))
					{
						eventlog(eventlog_level_error, __FUNCTION__, "icon list contains NULL item");
						continue;
					}

					// find a needed tag
					if (std::strcmp(iconset->clienttag, clienttag) == 0)
						return iconset->attr_key;
				}
			}
			return NULL;
		}




	}
}
