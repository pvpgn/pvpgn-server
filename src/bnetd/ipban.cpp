/*
 * Copyright (C) 2000  Gediminas (gugini@fortas.ktu.lt)
 * Copyright (C) 2002  Bartomiej Butyn (bartek@milc.com.pl)
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
#define IPBAN_INTERNAL_ACCESS
#include "ipban.h"

#include <cstdio>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <cctype>
#include <vector>

#include "compat/strsep.h"
#include "compat/strcasecmp.h"

#include "common/list.h"
#include "common/util.h"
#include "common/eventlog.h"
#include "common/xalloc.h"
#include "common/field_sizes.h"
#include "common/xstring.h"

#include "message.h"
#include "server.h"
#include "prefs.h"
#include "connection.h"

#include "helpfile.h"
#include "command.h"
#include "i18n.h"
#include "common/setup_after.h"

namespace pvpgn
{

	namespace bnetd
	{

		static int identify_ipban_function(const char * funcstr);
		static int ipban_func_del(t_connection * c, char const * cp);
		static int ipban_func_list(t_connection * c);
		static int ipban_func_check(t_connection * c, char const * cp);
		static int ipban_unload_entry(t_ipban_entry * e);
		static int ipban_identical_entry(t_ipban_entry * e1, t_ipban_entry * e2);
		static t_ipban_entry * ipban_str_to_ipban_entry(char const * cp);
		static char * ipban_entry_to_str(t_ipban_entry const * entry);
		static unsigned long ipban_str_to_ulong(char const * ipaddr);
		static int ipban_could_be_exact_ip_str(char const * str);
		static int ipban_could_be_ip_str(char const * ipstr);
		static void ipban_usage(t_connection * c);

		static t_list * ipbanlist_head = NULL;
		static std::time_t lastchecktime = 0;

		extern int ipbanlist_create(void)
		{
			ipbanlist_head = list_create();
			return 0;
		}


		extern int ipbanlist_destroy(void)
		{
			t_elem *		curr;
			t_ipban_entry *	entry;

			if (ipbanlist_head)
			{
				LIST_TRAVERSE(ipbanlist_head, curr)
				{
					entry = (t_ipban_entry*)elem_get_data(curr);
					if (!entry) /* should not happen */
					{
						eventlog(eventlog_level_error, __FUNCTION__, "ipbanlist contains NULL item");
						continue;
					}
					if (list_remove_elem(ipbanlist_head, &curr) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "could not remove item from list");
					ipban_unload_entry(entry);
				}
				if (list_destroy(ipbanlist_head) < 0)
					return -1;
				ipbanlist_head = NULL;
			}

			return 0;
		}


		extern int ipbanlist_load(char const * filename)
		{
			std::FILE *		fp;
			char *		buff;
			char *		ip;
			char *		timestr;
			unsigned int	currline;
			unsigned int	endtime;

			if (!filename)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
				return -1;
			}

			if (!(fp = std::fopen(filename, "r")))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not open banlist file \"%s\" for reading (std::fopen: %s)", filename, std::strerror(errno));
				return -1;
			}

			for (currline = 1; (buff = file_get_line(fp)); currline++)
			{
				ip = buff;

				/* eat whitespace in front */
				while (*ip == '\t' || *ip == ' ') ip++;
				if (*ip == '\0' || *ip == '#')
				{
					continue;
				}

				/* eat whitespace in back */
				while (ip[std::strlen(ip) - 1] == ' ' || ip[std::strlen(ip) - 1] == '\t')
					ip[std::strlen(ip) - 1] = '\0';

				if (std::strchr(ip, ' ') || std::strchr(ip, '\t'))
				{
					timestr = ip;
					while (*timestr != ' ' && *timestr != '\t') timestr++;
					*timestr = '\0';
					timestr++;
					while (*timestr == ' ' || *timestr == '\t') timestr++;
				}
				else
					timestr = NULL;

				if (!timestr)
					endtime = 0;
				else
				if (clockstr_to_seconds(timestr, &endtime) < 0)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not convert to seconds. Banning pernamently.");
					endtime = 0;
				}

				if (ipbanlist_add(NULL, ip, endtime) != 0)
				{
					eventlog(eventlog_level_warn, __FUNCTION__, "error in %.64s at line %u", filename, currline);
					continue;
				}

			}

			file_get_line(NULL); // clear file_get_line buffer
			if (std::fclose(fp) < 0)
				eventlog(eventlog_level_error, __FUNCTION__, "could not close banlist file \"%s\" after reading (std::fclose: %s)", filename, std::strerror(errno));

			return 0;
		}


		extern int ipbanlist_save(char const * filename)
		{
			t_elem const *	curr;
			t_ipban_entry *	entry;
			std::FILE *		fp;
			char *		ipstr;
			char		line[1024];

			if (!filename)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL filename");
				return -1;
			}

			if (!(fp = std::fopen(filename, "w")))
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not open banlist file \"%s\" for writing (std::fopen: %s)", filename, std::strerror(errno));
				return -1;
			}
			/*    if (ftruncate(fp,0)<0)
				{
				eventlog(eventlog_level_error,__FUNCTION__,"could not truncate banlist file \"%s\" (ftruncate: %s)",filename,std::strerror(errno));
				return -1;
				}*/

			LIST_TRAVERSE_CONST(ipbanlist_head, curr)
			{
				entry = (t_ipban_entry*)elem_get_data(curr);
				if (!entry)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "ipbanlist contains NULL element");
					continue;
				}
				if (!(ipstr = ipban_entry_to_str(entry)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "got NULL ipstr");
					continue;
				}
				if (entry->endtime == 0)
					std::sprintf(line, "%s\n", ipstr);
				else
					std::sprintf(line, "%s %ld\n", ipstr, entry->endtime);
				if (!(std::fwrite(line, std::strlen(line), 1, fp)))
					eventlog(eventlog_level_error, __FUNCTION__, "could not write to banlist file (write: %s)", std::strerror(errno));
				xfree(ipstr);
			}

			if (std::fclose(fp) < 0)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not close banlist file \"%s\" after writing (std::fclose: %s)", filename, std::strerror(errno));
				return -1;
			}

			return 0;
		}


		extern int ipbanlist_check(char const * ipaddr)
		{
			t_elem const *  curr;
			t_ipban_entry * entry;
			char *          whole;
			char const *    ip1;
			char const *    ip2;
			char const *    ip3;
			char const *    ip4;
			int		    counter;

			if (!ipaddr)
			{
				eventlog(eventlog_level_warn, __FUNCTION__, "got NULL ipaddr");
				return -1;
			}

			whole = xstrdup(ipaddr);

			eventlog(eventlog_level_debug, __FUNCTION__, "lastcheck: %u, now: %u, now-lc: %u.", (unsigned)lastchecktime, (unsigned)now, (unsigned)(now - lastchecktime));

			if (now - lastchecktime >= (signed)prefs_get_ipban_check_int()) /* unsigned; no need to check prefs < 0 */
			{
				ipbanlist_unload_expired();
				lastchecktime = now;
			}

			ip1 = std::strtok(whole, ".");
			ip2 = std::strtok(NULL, ".");
			ip3 = std::strtok(NULL, ".");
			ip4 = std::strtok(NULL, ".");

			if (!ip1 || !ip2 || !ip3 || !ip4)
			{
				eventlog(eventlog_level_warn, __FUNCTION__, "got bad IP address \"%s\"", ipaddr);
				xfree(whole);
				return -1;
			}

			eventlog(eventlog_level_debug, __FUNCTION__, "checking %s.%s.%s.%s", ip1, ip2, ip3, ip4);

			counter = 0;
			LIST_TRAVERSE_CONST(ipbanlist_head, curr)
			{
				entry = (t_ipban_entry*)elem_get_data(curr);
				if (!entry)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "ipbanlist contains NULL item");
					return -1;
				}
				counter++;
				switch (entry->type)
				{
				case ipban_type_exact:
					if (std::strcmp(entry->info1, ipaddr) == 0)
					{
						eventlog(eventlog_level_debug, __FUNCTION__, "address %s matched exact %s", ipaddr, entry->info1);
						xfree(whole);
						return counter;
					}
					eventlog(eventlog_level_debug, __FUNCTION__, "address %s does not match exact %s", ipaddr, entry->info1);
					continue;

				case ipban_type_wildcard:
					if (std::strcmp(entry->info1, "*") != 0 && std::strcmp(ip1, entry->info1) != 0)
					{
						eventlog(eventlog_level_debug, __FUNCTION__, "address %s does not match part 1 of wildcard %s.%s.%s.%s", ipaddr, entry->info1, entry->info2, entry->info3, entry->info4);
						continue;
					}
					if (std::strcmp(entry->info2, "*") != 0 && std::strcmp(ip2, entry->info2) != 0)
					{
						eventlog(eventlog_level_debug, __FUNCTION__, "address %s does not match part 2 of wildcard %s.%s.%s.%s", ipaddr, entry->info1, entry->info2, entry->info3, entry->info4);
						continue;
					}
					if (std::strcmp(entry->info3, "*") != 0 && std::strcmp(ip3, entry->info3) != 0)
					{
						eventlog(eventlog_level_debug, __FUNCTION__, "address %s does not match part 3 of wildcard %s.%s.%s.%s", ipaddr, entry->info1, entry->info2, entry->info3, entry->info4);
						continue;
					}
					if (std::strcmp(entry->info4, "*") != 0 && std::strcmp(ip4, entry->info4) != 0)
					{
						eventlog(eventlog_level_debug, __FUNCTION__, "address %s does not match part 4 of wildcard %s.%s.%s.%s", ipaddr, entry->info1, entry->info2, entry->info3, entry->info4);
						continue;
					}

					eventlog(eventlog_level_debug, __FUNCTION__, "address %s matched wildcard %s.%s.%s.%s", ipaddr, entry->info1, entry->info2, entry->info3, entry->info4);
					xfree(whole);
					return counter;

				case ipban_type_range:
					if ((ipban_str_to_ulong(ipaddr) >= ipban_str_to_ulong(entry->info1)) &&
						(ipban_str_to_ulong(ipaddr) <= ipban_str_to_ulong(entry->info2)))
					{
						eventlog(eventlog_level_debug, __FUNCTION__, "address %s matched range %s-%s", ipaddr, entry->info1, entry->info2);
						xfree(whole);
						return counter;
					}
					eventlog(eventlog_level_debug, __FUNCTION__, "address %s does not match range %s-%s", ipaddr, entry->info1, entry->info2);
					continue;

				case ipban_type_netmask:
				{
										   unsigned long	lip1;
										   unsigned long	lip2;
										   unsigned long	netmask;

										   if (!(lip1 = ipban_str_to_ulong(ipaddr)))
											   return -1;
										   if (!(lip2 = ipban_str_to_ulong(entry->info1)))
											   return -1;
										   if (!(netmask = ipban_str_to_ulong(entry->info2)))
											   return -1;

										   lip1 = lip1 & netmask;
										   lip2 = lip2 & netmask;
										   if (lip1 == lip2)
										   {
											   eventlog(eventlog_level_debug, __FUNCTION__, "address %s matched netmask %s/%s", ipaddr, entry->info1, entry->info2);
											   xfree(whole);
											   return counter;
										   }
										   eventlog(eventlog_level_debug, __FUNCTION__, "address %s does not match netmask %s/%s", ipaddr, entry->info1, entry->info2);
										   continue;
				}

				case ipban_type_prefix:
				{
										  unsigned long	lip1;
										  unsigned long	lip2;
										  int		prefix;

										  if (!(lip1 = ipban_str_to_ulong(ipaddr)))
											  return -1;
										  if (!(lip2 = ipban_str_to_ulong(entry->info1)))
											  return -1;
										  prefix = std::atoi(entry->info2);

										  lip1 = lip1 >> (32 - prefix);
										  lip2 = lip2 >> (32 - prefix);
										  if (lip1 == lip2)
										  {
											  eventlog(eventlog_level_debug, __FUNCTION__, "address %s matched prefix %s/%s", ipaddr, entry->info1, entry->info2);
											  xfree(whole);
											  return counter;
										  }
										  eventlog(eventlog_level_debug, __FUNCTION__, "address %s does not match prefix %s/%s", ipaddr, entry->info1, entry->info2);
										  continue;
				}
				default:  /* unknown type */
					eventlog(eventlog_level_warn, __FUNCTION__, "found bad ban type %d", (int)entry->type);
				}
			}

			xfree(whole);

			return 0;
		}


		extern int ipbanlist_add(t_connection * c, char const * cp, std::time_t endtime)
		{
			t_ipban_entry *	entry;
			std::string msgtemp;

			if (!(entry = ipban_str_to_ipban_entry(cp)))
			{
				if (c)
					message_send_text(c, message_type_error, c, localize(c, "Bad IP."));
				eventlog(eventlog_level_error, __FUNCTION__, "could not convert to t_ipban_entry: \"%s\"", cp);
				return -1;
			}

			entry->endtime = endtime;
			list_append_data(ipbanlist_head, entry);

			if (c)
			{

				if (endtime == 0)
				{
					msgtemp = localize(c, "{} banned permamently by {}.", cp, conn_get_username(c));
					eventlog(eventlog_level_info, __FUNCTION__, msgtemp.c_str());
					message_send_admins(c, message_type_info, msgtemp.c_str());
					msgtemp = localize(c, "{} banned permamently.", cp);
					message_send_text(c, message_type_info, c, msgtemp);
				}
				else
				{
					msgtemp = localize(c, "{} banned for {} by {}.", cp, seconds_to_timestr(entry->endtime - now), conn_get_username(c));
					eventlog(eventlog_level_info, __FUNCTION__, msgtemp.c_str());
					message_send_admins(c, message_type_info, msgtemp.c_str());
					msgtemp = localize(c, "{} banned for {}.", cp, seconds_to_timestr(entry->endtime - now));
					message_send_text(c, message_type_info, c, msgtemp);
				}
			}

			return 0;
		}


		extern int ipbanlist_unload_expired(void)
		{
			t_elem *		curr;
			t_ipban_entry * 	entry;
			char removed;

			removed = 0;
			LIST_TRAVERSE(ipbanlist_head, curr)
			{
				entry = (t_ipban_entry*)elem_get_data(curr);
				if (!entry)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "ipbanlist_contains NULL element");
					return -1;
				}
				if ((entry->endtime - now <= 0) && (entry->endtime != 0))
				{
					eventlog(eventlog_level_debug, __FUNCTION__, "removing item: %s", entry->info1);
					removed = 1;
					if (list_remove_elem(ipbanlist_head, &curr) < 0)
						eventlog(eventlog_level_error, __FUNCTION__, "could not remove item");
					else
						ipban_unload_entry(entry);
				}
			}
			if (removed == 1) ipbanlist_save(prefs_get_ipbanfile());
			return 0;
		}

		extern std::time_t ipbanlist_str_to_time_t(t_connection * c, char const * timestr)
		{

			unsigned int	bmin;
			char		minstr[MAX_TIME_STR];
			unsigned int	i;
			std::string msgtemp;

			for (i = 0; std::isdigit((int)timestr[i]) && i < sizeof(minstr)-1; i++)
				minstr[i] = timestr[i];
			minstr[i] = '\0';

			if (timestr[i] != '\0')
			{
				if (c)
				{
					msgtemp = "There was an error in std::time.";
					//if (std::strlen(minstr) < 1)
					//	message_send_text(c, message_type_info, c, localize(c, msgtemp));
					//else
					{
						msgtemp += " ";
						msgtemp += localize(c, "Banning only for: {} minutes.", minstr);
						message_send_text(c, message_type_info, c, msgtemp);
					}
				}
			}

			if (clockstr_to_seconds(minstr, &bmin) < 0) /* it thinks these are seconds but we treat them as minutes */
			{
				eventlog(eventlog_level_error, __FUNCTION__, "could not convert to minutes: \"%s\"", timestr);
				return -1;
			}
			if (bmin == 0)
				return 0;
			else
			{
				return now + bmin * 60;
			}
		}


		extern int handle_ipban_command(t_connection * c, char const * text)
		{
			char const *subcommand, *ipstr, *time;

			std::vector<std::string> args = split_command(text, 3);
			if (args[1].empty())
			{
				describe_command(c, args[0].c_str());
				return -1;
			}
			subcommand = args[1].c_str(); // subcommand
			ipstr = args[2].c_str(); // ip address
			time = args[3].c_str(); // username

			switch (identify_ipban_function(subcommand))
			{
			case IPBAN_FUNC_ADD:
				ipbanlist_add(c, ipstr, ipbanlist_str_to_time_t(c, time));
				ipbanlist_save(prefs_get_ipbanfile());
				break;
			case IPBAN_FUNC_DEL:
				ipban_func_del(c, ipstr);
				ipbanlist_save(prefs_get_ipbanfile());
				break;
			case IPBAN_FUNC_LIST:
				ipban_func_list(c);
				break;
			case IPBAN_FUNC_CHECK:
				ipban_func_check(c, ipstr);
				break; 
			default:
				describe_command(c, args[0].c_str());
			}

			return 0;
		}


		static int identify_ipban_function(const char * funcstr)
		{
			if (strcasecmp(funcstr, "add") == 0 || strcasecmp(funcstr, "a") == 0)
				return IPBAN_FUNC_ADD;
			if (strcasecmp(funcstr, "del") == 0 || strcasecmp(funcstr, "d") == 0)
				return IPBAN_FUNC_DEL;
			if (strcasecmp(funcstr, "list") == 0 || strcasecmp(funcstr, "l") == 0 || std::strcmp(funcstr, "") == 0)
				return IPBAN_FUNC_LIST;
			if (strcasecmp(funcstr, "check") == 0 || strcasecmp(funcstr, "c") == 0)
				return IPBAN_FUNC_CHECK;

			return IPBAN_FUNC_UNKNOWN;
		}


		static int ipban_func_del(t_connection * c, char const * cp)
		{
			t_ipban_entry *	to_delete;
			unsigned int	to_delete_nmbr;
			t_ipban_entry *	entry;
			t_elem *		curr;
			unsigned int	counter;
			char		tstr[MAX_MESSAGE_LEN];

			counter = 0;
			if (std::strchr(cp, '.') || std::strchr(cp, '/') || std::strchr(cp, '*'))
			{
				if (!(to_delete = ipban_str_to_ipban_entry(cp)))
				{
					message_send_text(c, message_type_error, c, localize(c, "Illegal IP entry."));
					return -1;
				}
				LIST_TRAVERSE(ipbanlist_head, curr)
				{
					entry = (t_ipban_entry*)elem_get_data(curr);
					if (!entry)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "ipbanlist contains NULL item");
						return -1;
					}
					if (ipban_identical_entry(to_delete, entry))
					{
						counter++;
						if (list_remove_elem(ipbanlist_head, &curr) < 0)
							eventlog(eventlog_level_error, __FUNCTION__, "could not remove item");
						else
							ipban_unload_entry(entry);
					}
				}

				ipban_unload_entry(to_delete);
				if (counter == 0)
				{
					message_send_text(c, message_type_error, c, localize(c, "No matching entry."));
					return -1;
				}
				else
				{
					if (counter == 1)
						std::sprintf(tstr, "Entry deleted.");
					else
						std::sprintf(tstr, "Deleted %u entries.", counter);
					message_send_text(c, message_type_info, c, tstr);
					return 0;
				}
			}

			to_delete_nmbr = std::atoi(cp);
			if (to_delete_nmbr <= 0)
			{
				message_send_text(c, message_type_error, c, localize(c, "Wrong entry number."));
				return -1;
			}
			LIST_TRAVERSE(ipbanlist_head, curr)
			{
				if (to_delete_nmbr == ++counter)
				{
					entry = (t_ipban_entry*)elem_get_data(curr);
					if (!entry)
					{
						eventlog(eventlog_level_error, __FUNCTION__, "ipbanlist contains NULL item");
						return -1;
					}
					if (list_remove_elem(ipbanlist_head, &curr)<0)
						eventlog(eventlog_level_error, __FUNCTION__, "could not remove item");
					else
					{
						ipban_unload_entry(entry);
						message_send_text(c, message_type_info, c, localize(c, "Entry deleted."));
					}
				}
			}

			if (to_delete_nmbr > counter)
			{
				std::sprintf(tstr, "There are only %u entries.", counter);
				message_send_text(c, message_type_error, c, tstr);
				return -1;
			}

			return 0;
		}


		static int ipban_func_list(t_connection * c)
		{
			t_elem const *	curr;
			t_ipban_entry * 	entry;
			char		tstr[MAX_MESSAGE_LEN];
			unsigned int	counter;
			char	 	timestr[50];
			char *		ipstr;

			counter = 0;
			message_send_text(c, message_type_info, c, localize(c, "Banned IPs:"));
			LIST_TRAVERSE_CONST(ipbanlist_head, curr)
			{
				entry = (t_ipban_entry*)elem_get_data(curr);
				if (!entry)
				{
					eventlog(eventlog_level_error, __FUNCTION__, "ipbanlist contains NULL item");
					return -1;
				}
				counter++;
				if (entry->endtime == 0)
					std::sprintf(timestr, localize(c, "(perm)").c_str());
				else
					std::sprintf(timestr, "(%.48s)", seconds_to_timestr(entry->endtime - now));

				if (!(ipstr = ipban_entry_to_str(entry)))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "could not convert entry to string");
					continue;
				}
				std::sprintf(tstr, "%u: %s %s", counter, ipstr, timestr);
				message_send_text(c, message_type_info, c, tstr);
				xfree(ipstr);
			}

			if (counter == 0)
				message_send_text(c, message_type_info, c, localize(c, "none"));
			return 0;
		}


		static int ipban_func_check(t_connection * c, char const * cp)
		{
			int		res;
			std::string msgtemp;

			res = ipbanlist_check(cp);
			switch (res)
			{
			case 0:
				message_send_text(c, message_type_info, c, localize(c, "IP not banned."));
				break;
			case -1:
				message_send_text(c, message_type_error, c, localize(c, "Error occured."));
				break;
			default:
				msgtemp = localize(c, "IP banned by rule #{}.", res);
				message_send_text(c, message_type_info, c, msgtemp);
			}

			return 0;
		}


		static int ipban_identical_entry(t_ipban_entry * e1, t_ipban_entry * e2)
		{
			if (e1->type != e2->type)
				return 0;

			switch (e2->type)
			{
			case ipban_type_exact:
				if (std::strcmp(e1->info1, e2->info1) == 0)
					return 1;
				break;
			case ipban_type_range:
				if (std::strcmp(e1->info1, e2->info1) == 0 &&
					std::strcmp(e1->info2, e2->info2) == 0)
					return 1;
				break;
			case ipban_type_wildcard:
				if (std::strcmp(e1->info1, e2->info1) == 0 &&
					std::strcmp(e1->info2, e2->info2) == 0 &&
					std::strcmp(e1->info3, e2->info3) == 0 &&
					std::strcmp(e1->info4, e2->info4) == 0)
					return 1;
				break;
			case ipban_type_netmask:
				if (std::strcmp(e1->info1, e2->info1) == 0 &&
					std::strcmp(e1->info2, e2->info2) == 0)
					return 1;
				break;
			case ipban_type_prefix:
				if (std::strcmp(e1->info1, e2->info1) == 0 &&
					std::strcmp(e1->info2, e2->info2) == 0)
					return 1;
				break;
			default:  /* unknown type */
				eventlog(eventlog_level_warn, __FUNCTION__, "found bad ban type %d", (int)e2->type);
			}

			return 0;
		}


		static int ipban_unload_entry(t_ipban_entry * e)
		{
			switch (e->type)
			{
			case ipban_type_exact:
			case ipban_type_wildcard:
				if (e->info1)
					xfree(e->info1);
				break;
			case ipban_type_range:
			case ipban_type_netmask:
			case ipban_type_prefix:
				if (e->info1)
					xfree(e->info1);
				if (e->info2)
					xfree(e->info2);
				break;
			default:  /* unknown type */
				eventlog(eventlog_level_warn, __FUNCTION__, "found bad ban type %d", (int)e->type);
				return -1;
			}
			xfree(e);
			return 0;
		}


		static t_ipban_entry * ipban_str_to_ipban_entry(char const * ipstr)
		{
			char *          matched;
			char *          whole;
			char *          cp;
			t_ipban_entry * entry;

			entry = (t_ipban_entry*)xmalloc(sizeof(t_ipban_entry));
			if (!ipstr)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "got NULL IP");
				xfree(entry);
				return NULL;
			}
			if (ipstr[0] == '\0')
			{
				eventlog(eventlog_level_warn, __FUNCTION__, "got empty IP string");
				xfree(entry);
				return NULL;
			}
			cp = xstrdup(ipstr);

			if (ipban_could_be_ip_str(cp) == 0)
			{
				eventlog(eventlog_level_debug, __FUNCTION__, "string: \"%.32s\" can not be valid IP", cp);
				xfree(entry);
				return NULL;
			}
			if ((matched = std::strchr(cp, '-'))) /* range */
			{
				entry->type = ipban_type_range;
				eventlog(eventlog_level_debug, __FUNCTION__, "entry: %s matched as ipban_type_range", cp);
				matched[0] = '\0';
				entry->info1 = xstrdup(cp); /* start of range */
				entry->info2 = xstrdup(&matched[1]); /* end of range */
				entry->info3 = NULL; /* clear unused elements so debugging is nicer */
				entry->info4 = NULL;
			}
			else
			if (std::strchr(cp, '*')) /* wildcard */
			{
				entry->type = ipban_type_wildcard;
				eventlog(eventlog_level_debug, __FUNCTION__, "entry: %s matched as ipban_type_wildcard", cp);

				/* only xfree() info1! */
				whole = xstrdup(cp);
				entry->info1 = std::strtok(whole, ".");
				entry->info2 = std::strtok(NULL, ".");
				entry->info3 = std::strtok(NULL, ".");
				entry->info4 = std::strtok(NULL, ".");
				if (!entry->info4) /* not enough dots */
				{
					eventlog(eventlog_level_error, __FUNCTION__, "wildcard entry \"%s\" does not contain all four octets", cp);
					xfree(entry->info1);
					xfree(entry);
					xfree(cp);
					return NULL;
				}
			}
			else
			if ((matched = std::strchr(cp, '/'))) /* netmask or prefix */
			{
				if (std::strchr(&matched[1], '.'))
				{
					entry->type = ipban_type_netmask;
					eventlog(eventlog_level_debug, __FUNCTION__, "entry: %s matched as ipban_type_netmask", cp);
				}
				else
				{
					entry->type = ipban_type_prefix;
					eventlog(eventlog_level_debug, __FUNCTION__, "entry: %s matched as ipban_type_prefix", cp);
				}

				matched[0] = '\0';
				entry->info1 = xstrdup(cp);
				entry->info2 = xstrdup(&matched[1]);
				entry->info3 = NULL; /* clear unused elements so debugging is nicer */
				entry->info4 = NULL;
			}
			else /* exact */
			{
				entry->type = ipban_type_exact;
				eventlog(eventlog_level_debug, __FUNCTION__, "entry: %s matched as ipban_type_exact", cp);

				entry->info1 = xstrdup(cp);
				entry->info2 = NULL; /* clear unused elements so debugging is nicer */
				entry->info3 = NULL;
				entry->info4 = NULL;
			}
			xfree(cp);

			return entry;
		}


		static char * ipban_entry_to_str(t_ipban_entry const * entry)
		{
			char 	tstr[MAX_MESSAGE_LEN];
			char * 	str;

			switch (entry->type)
			{
			case ipban_type_exact:
				std::sprintf(tstr, "%s", entry->info1);
				break;
			case ipban_type_wildcard:
				std::sprintf(tstr, "%s.%s.%s.%s", entry->info1, entry->info2, entry->info3, entry->info4);
				break;
			case ipban_type_range:
				std::sprintf(tstr, "%s-%s", entry->info1, entry->info2);
				break;
			case ipban_type_netmask:
			case ipban_type_prefix:
				std::sprintf(tstr, "%s/%s", entry->info1, entry->info2);
				break;

			default: /* unknown type */
				eventlog(eventlog_level_warn, __FUNCTION__, "found bad ban type %d", (int)entry->type);
				return NULL;
			}
			str = xstrdup(tstr);

			return str;
		}

		static unsigned long ipban_str_to_ulong(char const * ipaddr)
		{
			unsigned long	lip;
			char *    		ip1;
			char *    		ip2;
			char *    		ip3;
			char *    		ip4;
			char *		tipaddr;

			tipaddr = xstrdup(ipaddr);
			ip1 = std::strtok(tipaddr, ".");
			ip2 = std::strtok(NULL, ".");
			ip3 = std::strtok(NULL, ".");
			ip4 = std::strtok(NULL, ".");
			lip = (std::atoi(ip1) << 24) + (std::atoi(ip2) << 16) + (std::atoi(ip3) << 8) + (std::atoi(ip4));

			xfree(tipaddr);

			return lip;
		}


		static int ipban_could_be_exact_ip_str(char const * str)
		{
			char * 	ipstr;
			char *	s;
			char * 	ttok;
			int 	i;

			ipstr = xstrdup(str);

			s = ipstr;
			for (i = 0; i < 4; i++)
			{
				ttok = (char *)strsep(&ipstr, ".");
				if (!ttok || std::strlen(ttok)<1 || std::strlen(ttok)>3)
				{
					xfree(s);
					return 0;
				}
			}

			xfree(s);
			return 1;
		}


		static int ipban_could_be_ip_str(char const * str)
		{
			char *	matched;
			char *	ipstr;
			unsigned int 	i;

			if (std::strlen(str) < 7)
			{
				eventlog(eventlog_level_error, __FUNCTION__, "string too short");
				return 0;
			}
			for (i = 0; i < std::strlen(str); i++)
			if (!std::isdigit((int)str[i]) && str[i] != '.' && str[i] != '*' && str[i] != '/' && str[i] != '-')
			{
				eventlog(eventlog_level_debug, __FUNCTION__, "illegal character on position %i", i);
				return 0;
			}

			ipstr = xstrdup(str);
			if ((matched = std::strchr(ipstr, '-')))
			{
				matched[0] = '\0';
				if ((ipban_could_be_exact_ip_str(ipstr) == 0) ||
					(ipban_could_be_exact_ip_str(&matched[1]) == 0))
				{
					xfree(ipstr);
					return 0;
				}
			}
			else if ((matched = std::strchr(ipstr, '*')))
			{
				if (ipban_could_be_exact_ip_str(ipstr) == 0) /* FIXME: 123.123.1*.123 allowed */
				{
					xfree(ipstr);
					return 0;
				}
			}
			else if ((matched = std::strchr(ipstr, '/')))
			{
				matched[0] = '\0';
				if (std::strchr(&matched[1], '.'))
				{
					if ((ipban_could_be_exact_ip_str(ipstr) == 0) ||
						(ipban_could_be_exact_ip_str(&matched[1]) == 0))
					{
						xfree(ipstr);
						return 0;
					}
				}
				else
				{
					if (ipban_could_be_exact_ip_str(ipstr) == 0)
					{
						xfree(ipstr);
						return 0;
					}
					for (i = 1; i<std::strlen(&matched[1]); i++)
					if (!std::isdigit((int)matched[i]))
					{
						xfree(ipstr);
						return 0;
					}
					if (std::atoi(&matched[1])>32) /* can not be less than 0 because IP/-24 is matched as range */
					{
						xfree(ipstr);
						return 0;
					}
				}
			}
			else
			{
				if (ipban_could_be_exact_ip_str(ipstr) == 0)
				{
					xfree(ipstr);
					return 0;
				}
			}
			xfree(ipstr);

			return 1;
		}


	}

}
