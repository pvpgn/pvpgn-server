/*
 * Copyright (C) 1999  Mark Baysinger (mbaysing@ucsd.edu)
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
#include "common/setup_before.h"
#define TRACKER_INTERNAL_ACCESS

#include <cstdio>
#include <cstring>
#include <ctime>
#include <cstdlib>

#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "compat/stdfileno.h"
#include "compat/psock.h"
#include "compat/inet_ntoa.h"
#include "compat/pgetpid.h"
#include "common/tracker.h"
#include "common/eventlog.h"
#include "common/list.h"
#include "common/xalloc.h"
#include "common/util.h"
#include "common/version.h"
#include "common/bn_type.h"
#include "common/setup_after.h"

using namespace pvpgn;

namespace {

	/******************************************************************************
	 * TYPES
	 *****************************************************************************/
	typedef struct
	{
		struct in_addr address;
		std::time_t         updated;
		t_trackpacket  info;
	} t_server;

	typedef struct
	{
		int            foreground;
		int            debug;
		int            XML_mode;
		unsigned int   expire;
		unsigned int   update;
		unsigned short port;
		char const *   outfile;
		char const *   pidfile;
		char const *   logfile;
		char const *   process;
	} t_prefs;


	/******************************************************************************
	 * STATIC FUNCTION PROTOTYPES
	 *****************************************************************************/
	int server_process(int sockfd);
	void usage(char const * progname);
	void getprefs(int argc, char * argv[]);
	void fixup_str(bn_byte * str, unsigned int size);


	/******************************************************************************
	 * GLOBAL VARIABLES
	 *****************************************************************************/
	t_prefs prefs;
	t_list *           serverlist_head;
}


extern int main(int argc, char * argv[])
{
	int sockfd;
	int result;

	if (argc < 1 || !argv || !argv[0])
	{
		std::fprintf(stderr, "bad arguments\n");
		return EXIT_FAILURE;
	}

	getprefs(argc, argv);

	if (!prefs.debug)
		eventlog_del_level("debug");
	if (prefs.logfile)
	{
		eventlog_set(stderr);
		if (eventlog_open(prefs.logfile) < 0)
		{
			eventlog(eventlog_level_fatal, __FUNCTION__, "could not use file \"%s\" for the eventlog (exiting)", prefs.logfile);
			return EXIT_FAILURE;
		}
	}

#ifdef DO_DAEMONIZE
	if (!prefs.foreground)
	{
		switch (fork())
		{
		case -1:
			eventlog(eventlog_level_error, __FUNCTION__, "could not fork (fork: %s)\n", std::strerror(errno));
			return EXIT_FAILURE;
		case 0: /* child */
			break;
		default: /* parent */
			return EXIT_SUCCESS;
		}

		close(STDINFD);
		close(STDOUTFD);
		close(STDERRFD);

# ifdef HAVE_SETPGID
		if (setpgid(0, 0) < 0)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not create new process group (setpgid: %s)\n", std::strerror(errno));
			return EXIT_FAILURE;
		}
# else
#  ifdef HAVE_SETPGRP
#   ifdef SETPGRP_VOID
		if (setpgrp() < 0)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not create new process group (setpgrp: %s)\n", std::strerror(errno));
			return EXIT_FAILURE;
		}
#   else
		if (setpgrp(0, 0) < 0)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not create new process group (setpgrp: %s)\n", std::strerror(errno));
			return EXIT_FAILURE;
		}
#   endif
#  else
#   ifdef HAVE_SETSID
		if (setsid() < 0)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not create new process group (setsid: %s)\n", std::strerror(errno));
			return EXIT_FAILURE;
		}
#   else
#    error "One of setpgid(), setpgrp(), or setsid() is required"
#   endif
#  endif
# endif
	}
#endif

	if (prefs.pidfile)
	{
#ifdef HAVE_GETPID
		std::FILE * fp;

		if (!(fp = std::fopen(prefs.pidfile, "w")))
		{
			eventlog(eventlog_level_error, __FUNCTION__, "unable to open pid file \"%s\" for writing (std::fopen: %s)", prefs.pidfile, std::strerror(errno));
			prefs.pidfile = NULL;
		}
		else
		{
			std::fprintf(fp, "%u", (unsigned int)getpid());
			if (std::fclose(fp) < 0)
				eventlog(eventlog_level_error, __FUNCTION__, "could not close pid file \"%s\" after writing (std::fclose: %s)", prefs.pidfile, std::strerror(errno));
		}
#else
		eventlog(eventlog_level_warn, __FUNCTION__, "no getpid() system call, do not use the -P or the --pidfile option");
		prefs.pidfile = NULL;
#endif
	}

#ifdef HAVE_GETPID
	eventlog(eventlog_level_info, __FUNCTION__, "bntrackd version "PVPGN_VERSION" process %u", (unsigned int)getpid());
#else
	eventlog(eventlog_level_info, __FUNCTION__, "bntrackd version "PVPGN_VERSION);
#endif

	if (psock_init() < 0)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "could not initialize socket functions");
		return EXIT_FAILURE;
	}

	/* create the socket */
	if ((sockfd = psock_socket(PSOCK_PF_INET, PSOCK_SOCK_DGRAM, PSOCK_IPPROTO_UDP)) < 0)
	{
		eventlog(eventlog_level_error, __FUNCTION__, "could not create UDP listen socket (psock_socket: %s)\n", std::strerror(psock_errno()));
		return EXIT_FAILURE;
	}

	{
		struct sockaddr_in servaddr;

		/* bind the socket to correct port and interface */
		std::memset(&servaddr, 0, sizeof(servaddr));
		servaddr.sin_family = PSOCK_AF_INET;
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		servaddr.sin_port = htons(prefs.port);
		if (psock_bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
		{
			eventlog(eventlog_level_error, __FUNCTION__, "could not bind to UDP port %hu (psock_bind: %s)\n", prefs.port, std::strerror(psock_errno()));
			return EXIT_FAILURE;
		}
	}

	if (!(serverlist_head = list_create()))
	{
		eventlog(eventlog_level_error, __FUNCTION__, "could not create server list");
		return EXIT_FAILURE;;
	}


	result = server_process(sockfd);


	if (serverlist_head != NULL){
		list_destroy(serverlist_head);
	}

	if (result < 0)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

namespace {

	int server_process(int sockfd)
	{
		t_elem *           curr;
		t_server *         server;
		struct sockaddr_in cliaddr;
		t_psock_fd_set     rfds;
		struct timeval     tv;
		std::time_t             last;
		std::FILE *             outfile;
		psock_t_socklen    len;
		t_trackpacket      packet;

		/* the main loop */
		last = std::time(NULL) - prefs.update;
		for (;;)
		{
			/* time to dump our list to disk and call the process command */
			/* (I'm making the assumption that this won't take very long.) */
			if (last + (signed)prefs.update < std::time(NULL))
			{
				last = std::time(NULL);

				if (!(outfile = std::fopen(prefs.outfile, "w")))
				{
					eventlog(eventlog_level_error, __FUNCTION__, "unable to open file \"%s\" for writing (std::fopen: %s)", prefs.outfile, std::strerror(errno));
					continue;
				}

				LIST_TRAVERSE(serverlist_head, curr)
				{
					server = (t_server*)elem_get_data(curr);

					if (server->updated + (signed)prefs.expire < last)
					{
						list_remove_elem(serverlist_head, &curr);
						xfree(server);
					}
					else
					{
						if (prefs.XML_mode == 1)
						{
							std::fprintf(outfile, "<server>\n\t<address>%s</address>\n", inet_ntoa(server->address));
							std::fprintf(outfile, "\t<port>%hu</port>\n", bn_short_nget(server->info.port));
							std::fprintf(outfile, "\t<location>%s</location>\n", server->info.server_location);
							std::fprintf(outfile, "\t<software>%s</software>\n", server->info.software);
							std::fprintf(outfile, "\t<version>%s</version>\n", server->info.version);
							std::fprintf(outfile, "\t<users>%lu</users>\n", bn_int_nget(server->info.users));
							std::fprintf(outfile, "\t<channels>%lu</channels>\n", bn_int_nget(server->info.channels));
							std::fprintf(outfile, "\t<games>%lu</games>\n", bn_int_nget(server->info.games));
							std::fprintf(outfile, "\t<description>%s</description>\n", server->info.server_desc);
							std::fprintf(outfile, "\t<platform>%s</platform>\n", server->info.platform);
							std::fprintf(outfile, "\t<url>%s</url>\n", server->info.server_url);
							std::fprintf(outfile, "\t<contact_name>%s</contact_name>\n", server->info.contact_name);
							std::fprintf(outfile, "\t<contact_email>%s</contact_email>\n", server->info.contact_email);
							std::fprintf(outfile, "\t<uptime>%lu</uptime>\n", bn_int_nget(server->info.uptime));
							std::fprintf(outfile, "\t<total_games>%lu</total_games>\n", bn_int_nget(server->info.total_games));
							std::fprintf(outfile, "\t<logins>%lu</logins>\n", bn_int_nget(server->info.total_logins));
							std::fprintf(outfile, "</server>\n");
						}
						else
						{
							std::fprintf(outfile, "%s\n##\n", inet_ntoa(server->address));
							std::fprintf(outfile, "%hu\n##\n", bn_short_nget(server->info.port));
							std::fprintf(outfile, "%s\n##\n", server->info.server_location);
							std::fprintf(outfile, "%s\n##\n", server->info.software);
							std::fprintf(outfile, "%s\n##\n", server->info.version);
							std::fprintf(outfile, "%lu\n##\n", bn_int_nget(server->info.users));
							std::fprintf(outfile, "%lu\n##\n", bn_int_nget(server->info.channels));
							std::fprintf(outfile, "%lu\n##\n", bn_int_nget(server->info.games));
							std::fprintf(outfile, "%s\n##\n", server->info.server_desc);
							std::fprintf(outfile, "%s\n##\n", server->info.platform);
							std::fprintf(outfile, "%s\n##\n", server->info.server_url);
							std::fprintf(outfile, "%s\n##\n", server->info.contact_name);
							std::fprintf(outfile, "%s\n##\n", server->info.contact_email);
							std::fprintf(outfile, "%lu\n##\n", bn_int_nget(server->info.uptime));
							std::fprintf(outfile, "%lu\n##\n", bn_int_nget(server->info.total_games));
							std::fprintf(outfile, "%lu\n##\n", bn_int_nget(server->info.total_logins));
							std::fprintf(outfile, "###\n");
						}
					}
				}
				if (std::fclose(outfile) < 0)
					eventlog(eventlog_level_error, __FUNCTION__, "could not close output file \"%s\" after writing (std::fclose: %s)", prefs.outfile, std::strerror(errno));

				if (prefs.process[0] != '\0')
					std::system(prefs.process);
			}

			/* select socket to operate on */
			PSOCK_FD_ZERO(&rfds);
			PSOCK_FD_SET(sockfd, &rfds);
			tv.tv_sec = BNTRACKD_GRANULARITY;
			tv.tv_usec = 0;
			switch (psock_select(sockfd + 1, &rfds, NULL, NULL, &tv))
			{
			case -1: /* error */
				if (
#ifdef PSOCK_EINTR
					errno != PSOCK_EINTR &&
#endif
					1)
					eventlog(eventlog_level_error, __FUNCTION__, "select failed (select: %s)", std::strerror(errno));
			case 0: /* timeout and no sockets ready */
				continue;
			}

			/* New tracking packet */
			if (PSOCK_FD_ISSET(sockfd, &rfds))
			{

				len = sizeof(cliaddr);
				if (psock_recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&cliaddr, &len) >= 0)
				{

					if (bn_short_nget(packet.packet_version) >= TRACK_VERSION)
					{
						bn_byte_set(&packet.software[sizeof(packet.software) - 1], '\0');
						fixup_str(packet.software, sizeof(packet.software));
						bn_byte_set(&packet.version[sizeof(packet.version) - 1], '\0');
						fixup_str(packet.version, sizeof(packet.version));
						bn_byte_set(&packet.platform[sizeof(packet.platform) - 1], '\0');
						fixup_str(packet.platform, sizeof(packet.platform));
						bn_byte_set(&packet.server_desc[sizeof(packet.server_desc) - 1], '\0');
						fixup_str(packet.server_desc, sizeof(packet.server_desc));
						bn_byte_set(&packet.server_location[sizeof(packet.server_location) - 1], '\0');
						fixup_str(packet.server_location, sizeof(packet.server_location));
						bn_byte_set(&packet.server_url[sizeof(packet.server_url) - 1], '\0');
						fixup_str(packet.server_url, sizeof(packet.server_url));
						bn_byte_set(&packet.contact_name[sizeof(packet.contact_name) - 1], '\0');
						fixup_str(packet.contact_name, sizeof(packet.contact_name));
						bn_byte_set(&packet.contact_email[sizeof(packet.contact_email) - 1], '\0');
						fixup_str(packet.contact_email, sizeof(packet.contact_email));

						/* Find this server's slot */
						LIST_TRAVERSE(serverlist_head, curr)
						{
							server = (t_server*)elem_get_data(curr);

							if (!std::memcmp(&server->address, &cliaddr.sin_addr, sizeof(struct in_addr)))
							{
								if (bn_int_nget(packet.flags)&TF_SHUTDOWN)
								{
									list_remove_elem(serverlist_head, &curr);
									xfree(server);
								}
								else
								{
									/* update in place */
									server->info = packet;
									server->updated = std::time(NULL);
								}
								break;
							}
						}

						/* Not found? Make a new slot */
						if (!(bn_int_nget(packet.flags)&TF_SHUTDOWN) && !curr)
						{
							server = (t_server*)xmalloc(sizeof(t_server));
							server->address = cliaddr.sin_addr;
							server->info = packet;
							server->updated = std::time(NULL);

							list_append_data(serverlist_head, server);
						}

						eventlog(eventlog_level_debug, __FUNCTION__,
							"Packet received from %s:"
							" packet_version=%u"
							" flags=0x%08lx"
							" port=%hu"
							" software=\"%s\""
							" version=\"%s\""
							" platform=\"%s\""
							" server_desc=\"%s\""
							" server_location=\"%s\""
							" server_url=\"%s\""
							" contact_name=\"%s\""
							" contact_email=\"%s\""
							" uptime=%lu"
							" total_games=%lu"
							" total_logins=%lu",
							inet_ntoa(cliaddr.sin_addr),
							bn_short_nget(packet.packet_version),
							bn_int_nget(packet.flags),
							bn_short_nget(packet.port),
							packet.software,
							packet.version,
							packet.platform,
							packet.server_desc,
							packet.server_location,
							packet.server_url,
							packet.contact_name,
							packet.contact_email,
							bn_int_nget(packet.uptime),
							bn_int_nget(packet.total_games),
							bn_int_nget(packet.total_logins));
					}

				}
			}

		}
	}


	void usage(char const * progname)
	{
		std::fprintf(stderr, "usage: %s [<options>]\n", progname);
		std::fprintf(stderr,
			"  -c COMMAND, --command=COMMAND  execute COMMAND update\n"
			"  -d, --debug                    turn on debug mode\n"
			"  -e SECS, --expire SECS         forget a list entry after SEC seconds\n"
#ifdef DO_DAEMONIZE
			"  -f, --foreground               don't daemonize\n"
#else
			"  -f, --foreground               don't daemonize (default)\n"
#endif
			"  -l FILE, --logfile=FILE        write event messages to FILE\n"
			"  -o FILE, --outfile=FILE        write server list to FILE\n");
		std::fprintf(stderr,
			"  -p PORT, --port=PORT           listen for announcments on UDP port PORT\n"
			"  -P FILE, --pidfile=FILE        write pid to FILE\n"
			"  -u SECS, --update SECS         write output file every SEC seconds\n"
			"  -x, --XML                      write output file in XML format\n"
			"  -h, --help, --usage            show this information and exit\n"
			"  -v, --version                  print version number and exit\n");
		std::exit(EXIT_FAILURE);
	}


	void getprefs(int argc, char * argv[])
	{
		int a;

		prefs.foreground = 0;
		prefs.debug = 0;
		prefs.expire = 0;
		prefs.update = 0;
		prefs.port = 0;
		prefs.XML_mode = 0;
		prefs.outfile = NULL;
		prefs.pidfile = NULL;
		prefs.process = NULL;
		prefs.logfile = NULL;

		for (a = 1; a < argc; a++)
		if (std::strncmp(argv[a], "--command=", 10) == 0)
		{
			if (prefs.process)
			{
				std::fprintf(stderr, "%s: processing command was already specified as \"%s\"\n", argv[0], prefs.process);
				usage(argv[0]);
			}
			prefs.process = &argv[a][10];
		}
		else if (std::strcmp(argv[a], "-c") == 0)
		{
			if (a + 1 >= argc)
			{
				std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
				usage(argv[0]);
			}
			if (prefs.process)
			{
				std::fprintf(stderr, "%s: processing command was already specified as \"%s\"\n", argv[0], prefs.process);
				usage(argv[0]);
			}
			a++;
			prefs.process = argv[a];
		}
		else if (std::strcmp(argv[a], "-d") == 0 || std::strcmp(argv[a], "--debug") == 0)
			prefs.debug = 1;
		else if (std::strncmp(argv[a], "--expire=", 9) == 0)
		{
			if (prefs.expire)
			{
				std::fprintf(stderr, "%s: expiration period was already specified as \"%u\"\n", argv[0], prefs.expire);
				usage(argv[0]);
			}
			if (str_to_uint(&argv[a][9], &prefs.expire) < 0)
			{
				std::fprintf(stderr, "%s: \"%s\" should be a positive integer\n", argv[0], &argv[a][9]);
				usage(argv[0]);
			}
		}
		else if (std::strcmp(argv[a], "-e") == 0)
		{
			if (a + 1 >= argc)
			{
				std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
				usage(argv[0]);
			}
			if (prefs.expire)
			{
				std::fprintf(stderr, "%s: expiration period was already specified as \"%u\"\n", argv[0], prefs.expire);
				usage(argv[0]);
			}
			a++;
			if (str_to_uint(argv[a], &prefs.expire) < 0)
			{
				std::fprintf(stderr, "%s: \"%s\" should be a positive integer\n", argv[0], argv[a]);
				usage(argv[0]);
			}
		}
		else if (std::strcmp(argv[a], "-f") == 0 || std::strcmp(argv[a], "--foreground") == 0)
			prefs.foreground = 1;
		else if (std::strcmp(argv[a], "-x") == 0 || std::strcmp(argv[a], "--XML") == 0)
			prefs.XML_mode = 1;
		else if (std::strncmp(argv[a], "--logfile=", 10) == 0)
		{
			if (prefs.logfile)
			{
				std::fprintf(stderr, "%s: eventlog file was already specified as \"%s\"\n", argv[0], prefs.logfile);
				usage(argv[0]);
			}
			prefs.logfile = &argv[a][10];
		}
		else if (std::strcmp(argv[a], "-l") == 0)
		{
			if (a + 1 >= argc)
			{
				std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
				usage(argv[0]);
			}
			if (prefs.logfile)
			{
				std::fprintf(stderr, "%s: eventlog file was already specified as \"%s\"\n", argv[0], prefs.logfile);
				usage(argv[0]);
			}
			a++;
			prefs.logfile = argv[a];
		}
		else if (std::strncmp(argv[a], "--outfile=", 10) == 0)
		{
			if (prefs.outfile)
			{
				std::fprintf(stderr, "%s: output file was already specified as \"%s\"\n", argv[0], prefs.outfile);
				usage(argv[0]);
			}
			prefs.outfile = &argv[a][10];
		}
		else if (std::strcmp(argv[a], "-o") == 0)
		{
			if (a + 1 >= argc)
			{
				std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
				usage(argv[0]);
			}
			if (prefs.outfile)
			{
				std::fprintf(stderr, "%s: output file was already specified as \"%s\"\n", argv[0], prefs.outfile);
				usage(argv[0]);
			}
			a++;
			prefs.outfile = argv[a];
		}
		else if (std::strncmp(argv[a], "--pidfile=", 10) == 0)
		{
			if (prefs.pidfile)
			{
				std::fprintf(stderr, "%s: pid file was already specified as \"%s\"\n", argv[0], prefs.pidfile);
				usage(argv[0]);
			}
			prefs.pidfile = &argv[a][10];
		}
		else if (std::strncmp(argv[a], "--port=", 7) == 0)
		{
			if (prefs.port)
			{
				std::fprintf(stderr, "%s: port number was already specified as \"%hu\"\n", argv[0], prefs.port);
				usage(argv[0]);
			}
			if (str_to_ushort(&argv[a][7], &prefs.port) < 0)
			{
				std::fprintf(stderr, "%s: \"%s\" should be a positive integer\n", argv[0], &argv[a][7]);
				usage(argv[0]);
			}
		}
		else if (std::strcmp(argv[a], "-p") == 0)
		{
			if (a + 1 >= argc)
			{
				std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
				usage(argv[0]);
			}
			if (prefs.port)
			{
				std::fprintf(stderr, "%s: port number was already specified as \"%hu\"\n", argv[0], prefs.port);
				usage(argv[0]);
			}
			a++;
			if (str_to_ushort(argv[a], &prefs.port) < 0)
			{
				std::fprintf(stderr, "%s: \"%s\" should be a positive integer\n", argv[0], argv[a]);
				usage(argv[0]);
			}
		}
		else if (std::strcmp(argv[a], "-P") == 0)
		{
			if (a + 1 >= argc)
			{
				std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
				usage(argv[0]);
			}
			if (prefs.pidfile)
			{
				std::fprintf(stderr, "%s: pid file was already specified as \"%s\"\n", argv[0], prefs.pidfile);
				usage(argv[0]);
			}
			a++;
			prefs.pidfile = argv[a];
		}
		else if (std::strncmp(argv[a], "--update=", 9) == 0)
		{
			if (prefs.update)
			{
				std::fprintf(stderr, "%s: update period was already specified as \"%u\"\n", argv[0], prefs.expire);
				usage(argv[0]);
			}
			if (str_to_uint(&argv[a][9], &prefs.update) < 0)
			{
				std::fprintf(stderr, "%s: \"%s\" should be a positive integer\n", argv[0], &argv[a][9]);
				usage(argv[0]);
			}
		}
		else if (std::strcmp(argv[a], "-u") == 0)
		{
			if (a + 1 >= argc)
			{
				std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
				usage(argv[0]);
			}
			if (prefs.update)
			{
				std::fprintf(stderr, "%s: update period was already specified as \"%u\"\n", argv[0], prefs.expire);
				usage(argv[0]);
			}
			a++;
			if (str_to_uint(argv[a], &prefs.update) < 0)
			{
				std::fprintf(stderr, "%s: \"%s\" should be a positive integer\n", argv[0], argv[a]);
				usage(argv[0]);
			}
		}
		else if (std::strcmp(argv[a], "-h") == 0 || std::strcmp(argv[a], "--help") == 0 || std::strcmp(argv[a], "--usage")
			== 0)
			usage(argv[0]);
		else if (std::strcmp(argv[a], "-v") == 0 || std::strcmp(argv[a], "--version") == 0)
		{
			std::printf("version "PVPGN_VERSION"\n");
			std::exit(0);
		}
		else if (std::strcmp(argv[a], "--command") == 0 || std::strcmp(argv[a], "--expire") == 0 ||
			std::strcmp(argv[a], "--logfile") == 0 || std::strcmp(argv[a], "--outfile") == 0 ||
			std::strcmp(argv[a], "--port") == 0 || std::strcmp(argv[a], "--pidfile") == 0 ||
			std::strcmp(argv[a], "--update") == 0)
		{
			std::fprintf(stderr, "%s: option \"%s\" requires and argument.\n", argv[0], argv[a]);
			usage(argv[0]);
		}
		else
		{
			std::fprintf(stderr, "%s: unknown option \"%s\"\n", argv[0], argv[a]);
			usage(argv[0]);
		}

		if (!prefs.process)
			prefs.process = BNTRACKD_PROCESS;
		if (prefs.expire == 0)
			prefs.expire = BNTRACKD_EXPIRE;
		if (!prefs.logfile)
			prefs.logfile = BNTRACKD_LOGFILE;
		if (!prefs.outfile)
			prefs.outfile = BNTRACKD_OUTFILE;
		if (prefs.port == 0)
			prefs.port = BNTRACKD_SERVER_PORT;
		if (!prefs.pidfile)
			prefs.pidfile = BNTRACKD_PIDFILE;
		if (prefs.expire == 0)
			prefs.update = BNTRACKD_UPDATE;

		if (prefs.logfile[0] == '\0')
			prefs.logfile = NULL;
		if (prefs.pidfile[0] == '\0')
			prefs.pidfile = NULL;
	}


	void fixup_str(bn_byte * str, unsigned int size)
	{
		bn_byte         prev;
		unsigned int i;

		bn_byte_set(&prev, '\0');

		for (i = 0; i < size, bn_byte_get(str[i]) != '\0'; bn_byte_set(&prev, bn_byte_get(str[i])), i++)
		if (bn_byte_get(prev) == '#' && bn_byte_get(str[i]) == '#')
			bn_byte_set(&str[i], '%');
	}

}
