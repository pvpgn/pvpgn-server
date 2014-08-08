/*
 * Copyright (C) 2000  Onlyer (onlyer@263.net)
 * Copyright (C) 2002  Ross Combs (rocombs@cs.nmsu.edu)
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
#define ALIAS_COMMAND_INTERNAL_ACCESS
#include "common/setup_before.h"

#include <cerrno>
#include <cstring>
#include <cctype>

#include "alias_command.h"
#include "compat/strcasecmp.h"
#include "common/field_sizes.h"
#include "common/util.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "common/xalloc.h"
#include "message.h"
#include "connection.h"
#include "i18n.h"
#include "common/setup_after.h"


namespace pvpgn
{

	namespace bnetd
	{

		static t_list * aliaslist_head = NULL;

#define MAX_ALIAS_LEN 32


		static int list_aliases(t_connection * c);
		static char * replace_args(char const * in, unsigned int * offsets, int numargs, char const * text);
		static int do_alias(t_connection * c, char const * cmd, char const * args);


		static int list_aliases(t_connection * c)
		{
			t_elem const *  elem1;
			t_elem const *  elem2;
			t_alias const * alias;
			t_output *      output;
			char            temp[MAX_MESSAGE_LEN];

			message_send_text(c, message_type_info, c, localize(c, "Alias list:"));
			LIST_TRAVERSE_CONST(aliaslist_head, elem1)
			{
				if (!(alias = (t_alias*)elem_get_data(elem1)))
					continue;

				std::sprintf(temp, "@%.128s", alias->alias);
				message_send_text(c, message_type_info, c, temp);
				LIST_TRAVERSE_CONST(alias->output, elem2)
				{
					if (!(output = (t_output*)elem_get_data(elem2)))
						continue;

					/*
					 * FIXME: need a more user-friendly way to express this... maybe
					 * add a help line to the file format?
					 */
					if (output->min == -1)
						std::sprintf(temp, "[*]%.128s", output->line);
					else if (output->max == -1)
						std::sprintf(temp, "[%d+]%.128s", output->min, output->line);
					else if (output->min == output->max)
						std::sprintf(temp, "[%d]%.128s", output->min, output->line);
					else
						std::sprintf(temp, "[%d-%d]%.128s", output->min, output->max, output->line);
					message_send_text(c, message_type_info, c, temp);
				}
			}
			return 0;
		}


		static char * replace_args(char const * in, unsigned int * offsets, int numargs, char const * text)
		{
			char *         out;
			unsigned int   inpos;
			unsigned int   outpos;
			unsigned int   off1;
			unsigned int   off2;
			unsigned int   size;

			off1 = off2 = 0;

			out = xstrdup(in);
			size = std::strlen(out);

			for (inpos = outpos = 0; inpos < std::strlen(in); inpos++)
			{
				if (in[inpos] != '$')
				{
					out[outpos] = in[inpos];
					outpos++;
					continue;
				}

				inpos++;

				if (in[inpos] == '*')
				{
					off1 = offsets[0];
					off2 = std::strlen(text);
				}
				else if (in[inpos] == '{')
				{
					int arg1, arg2;
					char symbol;

					if (std::sscanf(&in[inpos], "{%d-%d}", &arg1, &arg2) != 2)
					{
						if (std::sscanf(&in[inpos], "{%d%c", &arg2, &symbol) != 2)
						{
							while (in[inpos] != '\0' && in[inpos] != '}')
								inpos++;
							continue;
						}
						else
						{
							if (symbol == '}')
							{
								if (arg2 < 0)
								{
									arg1 = 0;
									arg2 = -arg2;
								}
								else
								{
									arg1 = arg2;
								}
							}
							else if (symbol == '-')
							{
								arg1 = arg2;
								arg2 = numargs - 1;
							}
							else
							{
								while (in[inpos] != '\0' && in[inpos] != '}') inpos++;
								continue;
							}

						}
					}

					if (arg2 >= numargs)
						arg2 = numargs - 1;
					if (arg1 > arg2)
					{
						while (in[inpos] != '\0' && in[inpos] != '}')
							inpos++;
						continue;
					}
					off1 = offsets[arg1];
					if (arg2 + 1 == numargs)
						off2 = std::strlen(text);
					else
						off2 = offsets[arg2 + 1] - 1;

					while (in[inpos] != '\0' && in[inpos] != '}')
						inpos++;
				}
				else if (std::isdigit((int)in[inpos]))
				{
					int arg;

					if (in[inpos] == '0') arg = 0;
					else if (in[inpos] == '1') arg = 1;
					else if (in[inpos] == '2') arg = 2;
					else if (in[inpos] == '3') arg = 3;
					else if (in[inpos] == '4') arg = 4;
					else if (in[inpos] == '5') arg = 5;
					else if (in[inpos] == '6') arg = 6;
					else if (in[inpos] == '7') arg = 7;
					else if (in[inpos] == '8') arg = 8;
					else arg = 9;

					if (arg >= numargs)
						continue;
					for (off1 = off2 = offsets[arg]; text[off2] != '\0' && text[off2] != ' ' && text[off2] != '\t'; off2++);
				}

				{
					char * newout;

					newout = (char*)xmalloc(size + (off2 - off1) + 1); /* curr + new + nul */
					size = size + (off2 - off1) + 1;
					std::memmove(newout, out, outpos);
					xfree(out);
					out = newout;

					while (off1 < off2)
						out[outpos++] = text[off1++];
				}
			}
			out[outpos] = '\0';

			return out;
		}

		int count_args(char const * text)
		{
			int args = 1;
			char * pointer = (char *)text;
			int not_ws = 1;

			for (; *pointer != '\0'; pointer++)
			{
				if ((not_ws) && (*pointer == ' ')) not_ws = 0;
				if ((not_ws == 0) && (*pointer != ' ')) { not_ws = 1; args++; }
			}

			return args;
		}

		void get_offsets(char const * text, unsigned int * offsets)
		{
			char * pointer = (char *)text;
			int counter = 1;
			int not_ws = 1;

			offsets[0] = 0;
			for (; *pointer != '\0'; pointer++)
			{
				if ((not_ws) && (*pointer == ' ')) not_ws = 0;
				if ((not_ws == 0) && (*pointer != ' '))
				{
					not_ws = 1;
					offsets[counter] = (unsigned int)(pointer - text);
					counter++;
				}
			}

		}

		static int do_alias(t_connection * c, char const * cmd, char const * text)
		{
			t_elem const *  elem1;
			t_elem const *  elem2;
			t_alias const * alias;
			t_output *      output;
			unsigned int *  offsets;
			int    numargs;
			int match = -1;

			numargs = count_args(text) - 1;
			offsets = (unsigned int*)xmalloc(sizeof(unsigned int)*(numargs + 1));
			get_offsets(text, offsets);

			LIST_TRAVERSE_CONST(aliaslist_head, elem1)
			{
				if (!(alias = (t_alias*)elem_get_data(elem1)))
					continue;

				if (std::strstr(alias->alias, cmd) == NULL)
					continue;

				LIST_TRAVERSE_CONST(alias->output, elem2)
				{
					if (!(output = (t_output*)elem_get_data(elem2)))
						continue;

					if (!output->line)
						continue;

					if ((output->min == -1) ||
						((output->min <= numargs) && (output->max == -1)) ||
						((output->min <= numargs) && (output->max >= numargs)))

						//TODO: initialize offsets

					{
						char * msgtmp;
						char * tmp2;
						const char * cmd = "%C";
						match = 1;

						if ((msgtmp = replace_args(output->line, offsets, numargs + 1, text)))
						{
							if (msgtmp[0] != '\0')
							{
								if ((msgtmp[0] == '/') && (msgtmp[1] != '/')) // to make sure we don't get endless aliasing loop
								{
									tmp2 = (char*)xmalloc(std::strlen(msgtmp) + 3);
									std::sprintf(tmp2, "%s%s", cmd, msgtmp);
									xfree((void *)msgtmp);
									msgtmp = tmp2;

								}
								if (std::strlen(msgtmp) > MAX_MESSAGE_LEN)
								{
									msgtmp[MAX_MESSAGE_LEN] = '\0';
									eventlog(eventlog_level_info, __FUNCTION__, "message line after alias expansion was too long, truncating it");
								}
								message_send_formatted(c, msgtmp);
							}
							xfree((void *)msgtmp); /* avoid warning */
						}
						else
							eventlog(eventlog_level_error, __FUNCTION__, "could not perform argument replacement");
					}
				}
			}
			xfree(offsets);
			return match;
		}


		extern int aliasfile_load(char const * filename)
		{
			std::FILE *       afp;
			char *       buff;
			char *       temp;
			unsigned int line;
			unsigned int pos;
			int          inalias;
			t_alias *    alias = NULL;


			if (!filename)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
				return -1;
			}
			if (!(afp = std::fopen(filename, "r")))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "unable to open alias file \"%s\" for reading (std::fopen: %s)", filename, std::strerror(errno));
				return -1;
			}

			aliaslist_head = list_create();

			inalias = 0;
			for (line = 1; (buff = file_get_line(afp)); line++)
			{
				for (pos = 0; buff[pos] == '\t' || buff[pos] == ' '; pos++);
				if (buff[pos] == '\0' || buff[pos] == '#')
				{
					continue;
				}
				if (!(temp = std::strrchr(buff, '"'))) /* FIXME: assumes comments don't contain " */
					temp = buff;
				if ((temp = std::strrchr(temp, '#')))
				{
					unsigned int len;
					unsigned int endpos;

					*temp = '\0';
					len = std::strlen(buff) + 1;
					for (endpos = len - 1; buff[endpos] == '\t' || buff[endpos] == ' '; endpos--);
					buff[endpos + 1] = '\0';
				}

				switch (inalias)
				{
				case 0:
					if (buff[pos] != '@') /* not start of alias */
					{
						eventlog(eventlog_level_error, __FUNCTION__, "expected start of alias stanza on line %u of alias file \"%s\" but found \"%s\"", line, filename, &buff[pos]);
						break;
					}
					inalias = 1;
					break;

				case 1:
				{
						  if (buff[pos] == '\0') break;

						  inalias = 2;
						  alias = (t_alias*)xmalloc(sizeof(t_alias));
						  alias->output = 0;
						  alias->alias = xstrdup(&buff[pos]);
				}
					break;

				case 2:
				{
						  char * dummy;
						  char * out = NULL;
						  int min, max;
						  t_output * output;

						  min = max = 0;
						  if (buff[pos] != '[')
						  {
							  eventlog(eventlog_level_error, __FUNCTION__, "expected output entry on line %u of alias file \"%s\" but found \"%s\"", line, filename, &buff[pos]);
							  break;
						  }

						  if ((dummy = std::strchr(&buff[pos], ']')))
						  {
							  if (dummy[1] != '\0') out = xstrdup(&dummy[1]);
						  }

						  if (buff[pos + 1] == '*')
						  {
							  min = max = -1;
						  }
						  else if (buff[pos + 1] == '+')
						  {
							  min = 1;
							  max = -1;
						  }
						  else if (std::sscanf(&buff[pos], "[%i", &min) == 1)
						  {
							  if (*(dummy - 1) == '+')
								  max = -1;
							  else max = min;
						  }
						  else
						  {
							  eventlog(eventlog_level_error, __FUNCTION__, "error parsing min/max value for alias output");
						  }

						  if (out != NULL)
						  {
							  output = (t_output*)xmalloc(sizeof(t_output));
							  output->min = min;
							  output->max = max;
							  output->line = out;
							  alias->output = list_create();
							  list_append_data(alias->output, output);
							  inalias = 3;
						  }
				}
					break;
				case 3:
				{
						  if (buff[pos] == '@')
						  {
							  inalias = 1;
							  if (alias != NULL) list_append_data(aliaslist_head, alias);
							  alias = NULL;
							  break;
						  }
						  else if (buff[pos] == '[')
						  {
							  char * dummy;
							  char * out = NULL;
							  int min, max;
							  t_output * output;

							  min = max = 0;
							  if ((dummy = std::strchr(&buff[pos], ']')))
							  {
								  if (dummy[1] != '\0') out = xstrdup(&dummy[1]);
							  }


							  if (buff[pos + 1] == '*')
							  {
								  min = max = -1;
							  }
							  else if (buff[pos + 1] == '+')
							  {
								  min = 1;
								  max = -1;
							  }
							  else if (std::sscanf(&buff[pos], "[%i", &min) == 1)
							  {
								  if (*(dummy - 1) == '+')
									  max = -1;
								  else max = min;
							  }
							  else
							  {
								  eventlog(eventlog_level_error, __FUNCTION__, "error parsing min/max value for alias output");
							  }

							  if (out != NULL)
							  {
								  output = (t_output*)xmalloc(sizeof(t_output));
								  output->min = min;
								  output->max = max;
								  output->line = out;

								  list_append_data(alias->output, output);
							  }
						  }
						  else
							  eventlog(eventlog_level_error, __FUNCTION__, "expected output entry or next alias stanza on line %u of file \"%s\"i but found \"%s\"", line, filename, &buff[pos]);
						  break;
				}
				}
			}
			if (alias != NULL) list_append_data(aliaslist_head, alias);

			file_get_line(NULL); // clear file_get_line buffer
			std::fclose(afp);
			eventlog(eventlog_level_info, __FUNCTION__, "done loading aliases");
			return 0;
		}


		extern int aliasfile_unload(void)
		{
			t_elem       *  elem1;
			t_elem       *  elem2;
			t_alias const * alias;
			t_output *      output;

			if (aliaslist_head)
			{
				LIST_TRAVERSE(aliaslist_head, elem1)
				{
					if (!(alias = (t_alias*)elem_get_data(elem1))) /* should not happen */
					{
						eventlog(eventlog_level_error, __FUNCTION__, "alias list contains NULL item");
						continue;
					}

					if (list_remove_elem(aliaslist_head, &elem1) < 0)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "could not remove alias");
						continue;
					}
					if (alias->output)
					{
						LIST_TRAVERSE(alias->output, elem2)
						{
							if (!(output = (t_output*)elem_get_data(elem2)))
							{
								eventlog(eventlog_level_error, __FUNCTION__, "output list contains NULL item");
								continue;
							}

							if (list_remove_elem(alias->output, &elem2) < 0)
							{
								eventlog(eventlog_level_error, __FUNCTION__, "could not remove output");
								continue;
							}
							xfree((void *)output->line); /* avoid warning */
							xfree((void *)output);
						}
						list_destroy(alias->output);
					}
					if (alias->alias) xfree((void *)alias->alias);
					xfree((void *)alias);
				}

				if (list_destroy(aliaslist_head) < 0)
					return -1;
				aliaslist_head = NULL;
			}

			return 0;
		}


		extern int handle_alias_command(t_connection * c, char const * text)
		{
			unsigned int i, j;
			char         cmd[MAX_COMMAND_LEN];

			for (i = j = 0; text[i] != ' ' && text[i] != '\0'; i++) /* get command */
			if (j < sizeof(cmd)-1) cmd[j++] = text[i];
			cmd[j] = '\0';

			if (cmd[2] == '\0')
				return list_aliases(c);

			if (do_alias(c, cmd, text) < 0)
			{
				message_send_text(c, message_type_info, c, localize(c, "No such alias.  Use // to show the list."));
				return -1;
			}
			return 0;
		}

	}

}
