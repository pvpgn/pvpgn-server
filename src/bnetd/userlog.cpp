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
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>

#include "compat/localtime_s.h"
#include "compat/strcasecmp.h"
#include "compat/pdir.h"
#include "compat/mkdir.h"
#include "common/util.h"
#include "common/eventlog.h"
#include "common/xstring.h"

#include "connection.h"
#include "message.h"
#include "account.h"
#include "account_wrap.h"
#include "command_groups.h"
#include "command.h"
#include "prefs.h"
#include "helpfile.h"
#include "i18n.h"
#include "userlog.h"

#include "common/setup_after.h"


namespace pvpgn
{

	namespace bnetd
	{
		// max output lines to send user from /log command result
		static const int userlog_max_output_lines = 50;

		static std::vector<std::string> userlog_commands;

		extern void userlog_init()
		{
			char *       temp;
			char const * tok;

			userlog_commands.clear();

			// fill command list that must be logged
			if (const char * cmdlist = prefs_get_log_command_list())
			{
				temp = xstrdup(cmdlist);
				tok = std::strtok(temp, ","); /* std::strtok modifies the string it is passed */
				while (tok) {
					userlog_commands.push_back(tok);
					tok = std::strtok(NULL, ",");
				}
				xfree(temp);
			}
		}

		// add new line at the end of log file
		extern void userlog_append(t_account * account, const char * text)
		{
			// is logging enabled?
			if (!prefs_get_log_commands())
				return;
			
			unsigned int groups = 0;
			const char * cglist = prefs_get_log_command_groups();
			size_t cglistlen = std::strlen(cglist);

			// convert string groups from config to integer
			for (std::size_t i = 0; i < cglistlen; i++)
			{
				if (cglist[i] == '1') groups |= 1;
				else if (cglist[i] == '2') groups |= 2;
				else if (cglist[i] == '3') groups |= 4;
				else if (cglist[i] == '4') groups |= 8;
				else if (cglist[i] == '5') groups |= 16;
				else if (cglist[i] == '6') groups |= 32;
				else if (cglist[i] == '7') groups |= 64;
				else if (cglist[i] == '8') groups |= 128;
			}

			// log only commands for admins/operators and users in "groups" defined in config
			if (!account_is_operator_or_admin(account, NULL) && !(account_get_command_groups(account) & groups))
				return;

			bool is_cmd_found = false;

			// if command list empty then log all commands
			if (userlog_commands.size() == 0)
				is_cmd_found = true;
			else
			{
				// get command name
				std::vector<std::string> args = split_command(text, 0);
				std::string cmd = args[0];

				// find command in defined command list
				for (std::vector<std::string>::iterator it = userlog_commands.begin(); it != userlog_commands.end(); ++it) {
					if (*it == cmd)
					{
						is_cmd_found = true;
						break;
					}
				}
			}
			if (!is_cmd_found)
				return;

			// get time string
			char        time_string[USEREVENT_TIME_MAXLEN];

			{
				struct std::tm tmnow = {};
				std::time_t now = (std::time_t)(-1);
				std::time(&now);
				if (now == (std::time_t)(-1) || pvpgn::localtime_s(&now, &tmnow) == nullptr)
					std::strcpy(time_string, "?");
				else
					std::strftime(time_string, USEREVENT_TIME_MAXLEN, USEREVENT_TIME_FORMAT, &tmnow);
			}


			std::string filename = userlog_filename(account_get_name(account), true);

			if (FILE *fp = fopen(filename.c_str(), "a"))
			{
				// append date and text
				std::fprintf(fp, "[%s] %s\n", time_string, text);
				std::fclose(fp);
			}
			else
			{
				ERROR1("could not write into user log file \"{}\"", filename);
			}
		}

		// read "count" lines from the end starting from "startline"
		extern std::map<long, char*> userlog_read(const char* username, long startline, const char* search_substr)
		{
			if (!username)
				throw std::runtime_error("username is a nullptr");

			FILE* fp = std::fopen(userlog_filename(username).c_str(), "r");
			if (!fp)
				throw std::runtime_error("Could not open userlog");

			// set position to the end of file
			std::fseek(fp, 0, SEEK_END);

			long pos = std::ftell(fp);
			int c = {}, prev_c = {};
			std::map<long, char*> lines;
			long linecount = 0;
			char line[MAX_MESSAGE_LEN + 1] = {};
			int linepos = 0;

			// read file in reverse, byte by byte
			do
			{
				pos--;
				std::fseek(fp, pos, SEEK_SET);
				c = std::fgetc(fp);

				// add char into line array
				if (c != '\n')
					line[linepos] = c;

				// end of line (or start of file)
				if ((c == '\n' && c != prev_c) || pos == -1)
				{
					// hack for large lines (instead we will receive cut line without start symbols)
					if (linepos == MAX_MESSAGE_LEN)
					{
						// return carriage to read whole line to(from) start
						pos = pos + MAX_MESSAGE_LEN;
						linepos = 0;
					}

					if (linepos > 0)
					{
						line[linepos] = '\0'; // set end of string
						strreverse(line);

						linepos = 0; // reset position inside line

						linecount++;
						if (linecount >= startline)
						{
							if (search_substr && std::strlen(search_substr) > 0)
							{
								if (find_substr(line, search_substr))
									lines[linecount] = xstrdup(line);
							}
							else
							{
								lines[linecount] = xstrdup(line);
							}
						}

						// limitation of results
						if (lines.size() >= userlog_max_output_lines)
							break;
					}
				}
				prev_c = c;
				if (c != '\n' && linepos < MAX_MESSAGE_LEN)
					linepos++;

			} while (c != EOF);

			std::fclose(fp);

			return lines;
		}

		// search "substring" in user log file from the end starting from "startline" and return "count" results
		extern std::map<long, char*> userlog_find_text(const char * username, const char * search_substr, long startline)
		{
			return userlog_read(username, startline, search_substr);
		}

		// return full path for user log filename
		// if (force_create_path == true) then create dirs in path if not exist
		extern std::string userlog_filename(const char * username, bool force_create_path)
		{
			// lowercase username
			std::string lusername = std::string(username); 
			std::transform(lusername.begin(), lusername.end(), lusername.begin(), ::tolower);
			// get first 3 symbols of account and use it in path
			// it will improve performance with large count of files
			std::string dir_prefix = lusername.substr(0, 3);

			std::string filepath = fmt::format("{}/{}", prefs_get_userlogdir(), dir_prefix);
			// create directories in path
			if (force_create_path)
			{
				struct stat statbuf;
				// create inside user dir
				if (stat(filepath.c_str(), &statbuf) == -1)
				{
					p_mkdir(filepath.c_str(), S_IRWXU | S_IRWXG | S_IRWXO);
					eventlog(eventlog_level_info, __FUNCTION__, "created user directory: {}", filepath);
				}
			}

			filepath += fmt::format("/{}.log", lusername.c_str());

			return filepath;
		}

		// handle command
		// /log read user startline
		// /log find user substr startline
		extern int handle_log_command(t_connection * c, char const *text)
		{
			const char *subcommand, *username;
			long startline = 0;
			std::map<long, char*> lines;

			// split command args
			std::vector<std::string> args = split_command(text, 4);
			if (args[1].empty() || args[2].empty() 
				|| (args[1].at(0) != 'r' && args[1].at(0) != 'f')) // check start symbols for subcommand
			{
				describe_command(c, args[0].c_str());
				return -1;
			}
			subcommand = args[1].c_str(); // sub command
			username = args[2].c_str(); // username

			if (!accountlist_find_account(username))
			{
				message_send_text(c, message_type_error, c, localize(c, "Invalid user."));
				return -1;
			}

			std::string title = localize(c, "{}'s log output", username);
			// read
			if (subcommand[0] == 'r')
			{
				if (!args[3].empty())
					startline = atoi(args[3].c_str());

				try
				{
					lines = userlog_read(username, startline);
				}
				catch (...)
				{
					message_send_text(c, message_type_error, c, "Could not read user log");
					return 0;
				}
			}
			// find
			else if (subcommand[0] == 'f')
			{
				if (args[3].empty())
				{
					describe_command(c, args[0].c_str());
					return -1;
				}
				const char * search = args[3].c_str();
				title += localize(c, " by occurrence \"{}\"", search);

				if (!args[4].empty())
					startline = atoi(args[4].c_str());

				lines = userlog_find_text(username, search, startline);
			}

			title += ":";
			message_send_text(c, message_type_info, c, title);

			int linelen = 0;
			int paddedlen = 0;
			std::string linenum;

			// send each log line to user
			for (std::map<long, char*>::reverse_iterator it = lines.rbegin(); it != lines.rend(); ++it)
			{
				int linelen = floor(log10(static_cast<double>(abs(it->first)))) + 1; // get length of integer (line number)
				if (linelen > paddedlen)
					paddedlen = linelen;

				linenum = std_to_string(it->first);
				// pad left to max line length
				linenum.insert(linenum.begin(), paddedlen - linenum.size(), '0');

				message_send_text(c, message_type_info, c, linenum + ": " + std::string(it->second));
			}

			return 0;
		}


	}
}
