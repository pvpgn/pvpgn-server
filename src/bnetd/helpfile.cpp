/*
 * Copyright (C) 2000  Dizzy
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
#include "helpfile.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <map>

#include "compat/strcasecmp.h"
#include "common/eventlog.h"
#include "common/util.h"
#include "common/xalloc.h"
#include "common/xstring.h"

#include "message.h"
#include "command_groups.h"
#include "account_wrap.h"
#include "connection.h"
#include "i18n.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{
		static std::map<t_gamelang, std::FILE*> hfd_list;
		static std::FILE* hfd = NULL; /* helpfile descriptor */

		static int list_commands(t_connection *);
		static std::FILE* get_hfd(t_connection * c);

		static std::FILE* get_hfd(t_connection * c)
		{
			t_gamelang lang = conn_get_gamelang_localized(c);

			std::map<t_gamelang, std::FILE*>::iterator it = hfd_list.find(lang);
			if (it != hfd_list.end())
			{
				return it->second;
			}
			// return enUS if language is not specified in language list
			return hfd_list[languages[0]];
		}

		extern int helpfile_init(char const *filename)
		{
			if (!filename)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
				return -1;
			}
			const char * _filename;

			// iterate language list
			for (int i = 0; i < (sizeof(languages) / sizeof(*languages)); i++)
			{
				// get hfd of all localized help files
				_filename = i18n_filename(filename, languages[i]);
				if (!(hfd_list[languages[i]] = std::fopen(_filename, "r")))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not open help file \"%s\" for reading (std::fopen: %s)", _filename, std::strerror(errno));
					return -1;
				}
			}
			return 0;
		}


		extern int helpfile_unload(void)
		{
			// destroy file handles
			for (std::map<t_gamelang, std::FILE*>::iterator it = hfd_list.begin(); it != hfd_list.end(); ++it)
			{
				if (it->second != NULL)
				{
					if (std::fclose(it->second) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "could not close help file after reading (std::fclose: %s)", std::strerror(errno));
					it->second = NULL;
				}
			}
			// clear list
			hfd_list.clear();

			return 0;
		}


		extern int handle_help_command(t_connection * c, char const * text)
		{
			std::FILE* hfd = get_hfd(c);
			unsigned int i, j;
			char         cmd[MAX_COMMAND_LEN];

			if (hfd == NULL)
			{ /* an error ocured opening readonly the help file, helpfile_unload was called, or helpfile_init hasn't been called */
				message_send_text(c, message_type_error, c, localize(c, "Oops ! There is a problem with the help file. Please contact the administrator of the server."));
				return 0;
			}

			for (i = 0; text[i] != ' ' && text[i] != '\0'; i++); /* skip command */
			for (; text[i] == ' '; i++);
			if (text[i] == '/') /* skip / in front of command (if present) */
				i++;
			for (j = 0; text[i] != ' ' && text[i] != '\0'; i++) /* get comm */
			if (j < sizeof(cmd)-1) cmd[j++] = text[i];
			cmd[j] = '\0';

			/* just read the whole file and dump only the commands */
			if (cmd[0] == '\0')
			{
				list_commands(c);
				return 0;
			}

			describe_command(c, cmd);

			return 0;
		}


		static int list_commands(t_connection * c)
		{
			std::FILE* hfd = get_hfd(c);
			char * line;
			int    i;

			message_send_text(c, message_type_info, c, localize(c, "Chat commands : "));
			std::rewind(hfd);
			while ((line = file_get_line(hfd)) != NULL)
			{
				for (i = 0; line[i] == ' ' && line[i] != '\0'; i++); /* skip spaces in front of %command */
				if (line[i] == '%')  /* is this a command ? */
				{
					char *p, *buffer;
					int al;
					int skip;
					unsigned int length, position;

					/* ok. now we must see if there are any aliases */
					length = MAX_COMMAND_LEN + 1; position = 0;
					buffer = (char*)xmalloc(length + 1); /* initial memory allocation = pretty fair */
					p = line + i;
					do
					{
						al = 0;
						skip = 0;
						for (i = 1; p[i] != ' ' && p[i] != '\0' && p[i] != '#'; i++); /* skip command */
						if (p[i] == ' ') al = 1; /* we have something after the command.. must remember that */
						p[i] = '\0'; /* end the string at the end of the command */
						p[0] = '/'; /* change the leading character (% or space) read from the help file to / */
						if (!(command_get_group(p) & account_get_command_groups(conn_get_account(c)))) skip = 1;
						if (length < std::strlen(p) + position + 1)
							/* if we don't have enough space in the buffer then get some */
							length = std::strlen(p) + position + 1; /* the new length */
						buffer = (char*)xrealloc(buffer, length + 1);
						buffer[position++] = ' '; /* put a space before each alias */
						/* add the alias to the output string */
						std::strcpy(buffer + position, p); position += std::strlen(p);
						if (al)
						{
							for (; p[i + 1] == ' ' && p[i + 1] != '\0' && p[i + 1] != '#'; i++); /* skip spaces */
							if (p[i + 1] == '\0' || p[i + 1] == '#')
							{
								al = 0; continue;
							}
							p += i; /* jump to the next command */
						}
					} while (al);
					if (!skip) message_send_text(c, message_type_info, c, buffer); /* print out the buffer */
					xfree(buffer);
				}
			}
			file_get_line(NULL); // clear file_get_line buffer
			return 0;
		}


		extern int describe_command(t_connection * c, char const * cmd)
		{
			std::FILE* hfd = get_hfd(c);
			char * line;
			int    i;


			/* ok. the client requested help for a specific command */
			std::rewind(hfd);
			while ((line = file_get_line(hfd)) != NULL)
			{
				for (i = 0; line[i] == ' ' && line[i] != '\0'; i++); /* skip spaces in front of %command */
				if (line[i] == '%') /* is this a command ? */
				{
					char *p;
					int al;
					/* ok. now we must see if there are any aliases */
					p = line + i;
					do
					{
						al = 0;
						for (i = 1; p[i] != ' ' && p[i] != '\0' && p[i] != '#'; i++); /* skip command */
						if (p[i] == ' ') al = 1; /* we have something after the command.. must remember that */
						p[i] = '\0'; /* end the string at the end of the command */
						if (strcasecmp(cmd, p + 1) == 0) /* is this the command the user asked for help ? */
						{
							while ((line = file_get_line(hfd)) != NULL)
							{ /* write everything until we get another % or EOF */
								for (i = 0; line[i] == ' '; i++); /* skip spaces in front of a possible % */
								if (line[i] == '%')
								{
									break; /* we reached another command */
								}
								if (line[0] != '#' && line[i] != '\0')
								{ /* is this a whole line comment ? */
									/* truncate the line when a comment starts */
									for (; line[i] != '\0' && line[i] != '#'; i++);
									if (line[i] == '#') line[i] = '\0';

									// replace tabs with 3 spaces
									line = str_replace(line, "\t", "   ");
									// if text starts with slash then make it colored
									int j = 0; for (; line[j] == ' ' || line[j] == '\t'; j++);

									i18n_convert(c, line);

									if (line[j] == '/')
										message_send_text(c, message_type_error, c, line);
									else
										message_send_text(c, message_type_info, c, line);
								}
							}
							return 0;
						}
						if (al)
						{
							for (; p[i + 1] == ' ' && p[i + 1] != '\0'; i++); /* skip spaces */
							if (p[i + 1] == '\0')
							{
								al = 0; continue;
							}
							p += i; /* jump to the next command */
						}
					} while (al);
				}
			}
			file_get_line(NULL); // clear file_get_line buffer

			/* no description was found for this command. inform the user */
			message_send_text(c, message_type_error, c, localize(c, "No help available for that command"));
			return -1;
		}

	}

}
