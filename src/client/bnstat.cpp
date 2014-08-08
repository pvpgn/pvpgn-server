/*
 * Copyright (C) 1998,1999,2000  Ross Combs (rocombs@cs.nmsu.edu)
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
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <ctime>

#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif

#include "compat/termios.h"
#include "compat/psock.h"
#include "compat/inet_ntoa.h"
#include "common/bnet_protocol.h"
#include "common/util.h"
#include "common/tag.h"
#include "common/packet.h"
#include "common/bn_type.h"
#include "common/bnettime.h"
/*
#include "common/init_protocol.h"
#include "common/udp_protocol.h"
#include "common/field_sizes.h"
#include "common/network.h"
#include "common/version.h"
#ifdef CLIENTDEBUG
# include "common/eventlog.h"
#endif
#include "udptest.h"
*/
#include "client_connect.h"
#include "client.h"
#include "common/setup_after.h"


#ifdef CLIENTDEBUG
# define dprintf printf
#else
# define dprintf if (0) printf
#endif

using namespace pvpgn;
using namespace pvpgn::client;


namespace
{

	char const * bnclass_get_str(unsigned int cclass)
	{
		switch (cclass)
		{
		case PLAYERINFO_DRTL_CLASS_WARRIOR:
			return "warrior";
		case PLAYERINFO_DRTL_CLASS_ROGUE:
			return "rogue";
		case PLAYERINFO_DRTL_CLASS_SORCERER:
			return "sorcerer";
		default:
			return "unknown";
		}
	}


	void usage(char const * progname)
	{
		std::fprintf(stderr, "usage: %s [<options>] [<servername> [<TCP portnumber>]]\n", progname);
		std::fprintf(stderr,
			"    -b, --client=SEXP           report client as Brood Wars\n"
			"    -d, --client=DRTL           report client as Diablo Retail\n"
			"    --client=DSHR               report client as Diablo Shareware\n"
			"    -s, --client=STAR           report client as Starcraft (default)\n");
		std::fprintf(stderr,
			"    --client=SSHR               report client as Starcraft Shareware\n"
			"    -w, --client=W2BN           report client as Warcraft II BNE\n"
			"    --client=D2DV               report client as Diablo II\n"
			"    --client=D2XP               report client as Diablo II: LoD\n"
			"    --client=WAR3               report client as Warcraft III\n");
		std::fprintf(stderr,
			"    -o NAME, --owner=NAME       report CD owner as NAME\n"
			"    -k KEY, --cdkey=KEY         report CD key as KEY\n"
			"    -i, --ignore-version        ignore version request (do not send game version, CD owner/key)\n"
			"    -p PLR, --player=PLR        print stats for player PLR\n"
			"    --bnetd                     also print BNETD-specific stats\n"
			"    --fsgs                      also print FSGS-specific stats\n"
			"    -h, --help, --usage         show this information and exit\n"
			"    -v, --version               print version number and exit\n");
		std::exit(EXIT_FAILURE);
	}

}

extern int main(int argc, char * argv[])
{
	int                a;
	int                sd;
	int                gotplayer = 0;
	struct sockaddr_in saddr;
	t_packet *         packet;
	t_packet *         rpacket;
	char const *       cdowner = NULL;
	char const *       cdkey = NULL;
	char const *       clienttag = NULL;
	char const *       servname = NULL;
	unsigned short     servport = 0;
	char               text[MAX_MESSAGE_LEN] = "";
	unsigned int       commpos;
	struct termios     in_attr_old;
	struct termios     in_attr_new;
	int                changed_in;
	unsigned int       count;
	unsigned int       sessionkey;
	unsigned int       sessionnum;
	unsigned int       val;
	int                fd_stdin = 0;
	int                use_bnetd = 0;
	int                use_fsgs = 0;
	unsigned int       screen_width, screen_height;
	int                munged = 0;
	int                ignoreversion = 0;

	if (argc < 1 || !argv || !argv[0])
	{
		std::fprintf(stderr, "bad arguments\n");
		return EXIT_FAILURE;
	}

	for (a = 1; a < argc; a++)
	if (servname && std::isdigit((int)argv[a][0]) && a + 1 >= argc)
	{
		if (str_to_ushort(argv[a], &servport) < 0)
		{
			std::fprintf(stderr, "%s: \"%s\" should be a positive integer\n", argv[0], argv[a]);
			usage(argv[0]);
		}
	}
	else if (!servname && argv[a][0] != '-' && a + 2 >= argc)
		servname = argv[a];
	else if (std::strcmp(argv[a], "-b") == 0 || std::strcmp(argv[a], "--client=SEXP") == 0)
	{
		if (clienttag)
		{
			std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], clienttag);
			usage(argv[0]);
		}
		clienttag = CLIENTTAG_BROODWARS;
	}
	else if (std::strcmp(argv[a], "-d") == 0 || std::strcmp(argv[a], "--client=DRTL") == 0)
	{
		if (clienttag)
		{
			std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], clienttag);
			usage(argv[0]);
		}
		clienttag = CLIENTTAG_DIABLORTL;
	}
	else if (std::strcmp(argv[a], "--client=DSHR") == 0)
	{
		if (clienttag)
		{
			std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], clienttag);
			usage(argv[0]);
		}
		clienttag = CLIENTTAG_DIABLOSHR;
	}
	else if (std::strcmp(argv[a], "-s") == 0 || std::strcmp(argv[a], "--client=STAR") == 0)
	{
		if (clienttag)
		{
			std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], clienttag);
			usage(argv[0]);
		}
		clienttag = CLIENTTAG_STARCRAFT;
	}
	else if (std::strcmp(argv[a], "--client=SSHR") == 0)
	{
		if (clienttag)
		{
			std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], clienttag);
			usage(argv[0]);
		}
		clienttag = CLIENTTAG_SHAREWARE;
	}
	else if (std::strcmp(argv[a], "-w") == 0 || std::strcmp(argv[a], "--client=W2BN") == 0)
	{
		if (clienttag)
		{
			std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], clienttag);
			usage(argv[0]);
		}
		clienttag = CLIENTTAG_WARCIIBNE;
	}
	else if (std::strcmp(argv[a], "--client=D2DV") == 0)
	{
		if (clienttag)
		{
			std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], clienttag
				);
			usage(argv[0]);
		}
		clienttag = CLIENTTAG_DIABLO2DV;
	}
	else if (std::strcmp(argv[a], "--client=D2XP") == 0)
	{
		if (clienttag)
		{
			std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], clienttag
				);
			usage(argv[0]);
		}
		clienttag = CLIENTTAG_DIABLO2XP;
	}
	else if (std::strcmp(argv[a], "--client=WAR3") == 0)
	{
		if (clienttag)
		{
			std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], clienttag
				);
			usage(argv[0]);
		}
		clienttag = CLIENTTAG_WARCRAFT3;
	}
	else if (std::strncmp(argv[a], "--client=", 9) == 0)
	{
		std::fprintf(stderr, "%s: unknown client tag \"%s\"\n", argv[0], &argv[a][9]);
		usage(argv[0]);
	}
	else if (std::strcmp(argv[a], "-o") == 0)
	{
		if (a + 1 >= argc)
		{
			std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
			usage(argv[0]);
		}
		if (cdowner)
		{
			std::fprintf(stderr, "%s: CD owner was already specified as \"%s\"\n", argv[0], cdowner);
			usage(argv[0]);
		}
		cdowner = argv[++a];
	}
	else if (std::strncmp(argv[a], "--owner=", 8) == 0)
	{
		if (cdowner)
		{
			std::fprintf(stderr, "%s: CD owner was already specified as \"%s\"\n", argv[0], cdowner);
			usage(argv[0]);
		}
		cdowner = &argv[a][8];
	}
	else if (std::strcmp(argv[a], "-k") == 0)
	{
		if (a + 1 >= argc)
		{
			std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
			usage(argv[0]);
		}
		if (cdkey)
		{
			std::fprintf(stderr, "%s: CD key was already specified as \"%s\"\n", argv[0], cdkey);
			usage(argv[0]);
		}
		cdkey = argv[++a];
	}
	else if (std::strncmp(argv[a], "--cdkey=", 8) == 0)
	{
		if (cdkey)
		{
			std::fprintf(stderr, "%s: CD key was already specified as \"%s\"\n", argv[0], cdkey);
			usage(argv[0]);
		}
		cdkey = &argv[a][8];
	}
	else if (std::strcmp(argv[a], "-p") == 0)
	{
		if (a + 1 >= argc)
		{
			std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
			usage(argv[0]);
		}
		if (gotplayer)
		{
			std::fprintf(stderr, "%s: player was already specified as \"%s\"\n", argv[0], text);
			usage(argv[0]);
		}
		a++;
		if (argv[a][0] == '\0')
		{
			std::fprintf(stderr, "%s: player name can not be empty\n", argv[0]);
			usage(argv[0]);
		}
		std::strncpy(text, argv[a], sizeof(text));
		text[sizeof(text)-1] = '\0';
		gotplayer = 1;
	}
	else if (std::strncmp(argv[a], "--player=", 9) == 0)
	{
		if (gotplayer)
		{
			std::fprintf(stderr, "%s: player was already specified as \"%s\"\n", argv[0], text);
			usage(argv[0]);
		}
		if (argv[a][9] == '\0')
		{
			std::fprintf(stderr, "%s: player name can not be empty\n", argv[0]);
			usage(argv[0]);
		}
		std::strncpy(text, &argv[a][9], sizeof(text));
		text[sizeof(text)-1] = '\0';
		gotplayer = 1;
	}
	else if (std::strcmp(argv[a], "--bnetd") == 0)
		use_bnetd = 1;
	else if (std::strcmp(argv[a], "--fsgs") == 0)
		use_fsgs = 1;
	else if (std::strcmp(argv[a], "-h") == 0 || std::strcmp(argv[a], "--help") == 0 || std::strcmp(argv[a], "--usage")
		== 0)
		usage(argv[0]);
	else if (std::strcmp(argv[a], "-i") == 0 || std::strcmp(argv[a], "--ignore-version") == 0)
		ignoreversion = 1;
	else if (std::strcmp(argv[a], "-v") == 0 || std::strcmp(argv[a], "--version") == 0)
	{
		std::printf("version "PVPGN_VERSION"\n");
		return EXIT_SUCCESS;
	}
	else if (std::strcmp(argv[a], "--client") == 0 || std::strcmp(argv[a], "--owner") == 0 ||
		std::strcmp(argv[a], "--cdkey") == 0 || std::strcmp(argv[a], "--player") == 0)
	{
		std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
		usage(argv[0]);
	}
	else
	{
		std::fprintf(stderr, "%s: unknown option \"%s\"\n", argv[0], argv[a]);
		usage(argv[0]);
	}

	if (servport == 0)
		servport = BNETD_SERV_PORT;
	if (!cdowner)
		cdowner = BNETD_DEFAULT_OWNER;
	if (!cdkey)
		cdkey = BNETD_DEFAULT_KEY;
	if (!clienttag)
		clienttag = CLIENTTAG_STARCRAFT;
	if (!servname)
		servname = BNETD_DEFAULT_HOST;

	if (gotplayer)
		changed_in = 0; /* no need to change terminal attributes */
	else
	{
		fd_stdin = fileno(stdin);
		if (tcgetattr(fd_stdin, &in_attr_old) >= 0)
		{
			in_attr_new = in_attr_old;
			in_attr_new.c_lflag &= ~(ECHO | ICANON); /* turn off ECHO and ICANON */
			in_attr_new.c_cc[VMIN] = 1; /* require reads to return at least one byte */
			in_attr_new.c_cc[VTIME] = 0; /* no timeout */
			tcsetattr(fd_stdin, TCSANOW, &in_attr_new);
			changed_in = 1;
		}
		else
		{
			std::fprintf(stderr, "%s: could not set terminal attributes for stdin\n", argv[0]);
			changed_in = 0;
		}

		if (client_get_termsize(fd_stdin, &screen_width, &screen_height) < 0)
		{
			std::fprintf(stderr, "%s: could not determine screen size, assuming 80x24\n", argv[0]);
			screen_width = 80;
			screen_height = 24;
		}
	}

	if ((sd = client_connect(argv[0], servname, servport, cdowner, cdkey, clienttag, ignoreversion, &saddr, &sessionkey, &sessionnum, ARCHTAG_WINX86, CLIENT_COUNTRYINFO_109_GAMELANG)) < 0)
	{
		std::fprintf(stderr, "%s: fatal error during handshake\n", argv[0]);
		if (changed_in)
			tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
		return EXIT_FAILURE;
	}

	/* reuse this same packet over and over */
	if (!(rpacket = packet_create(packet_class_bnet)))
	{
		std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
		psock_close(sd);
		if (changed_in)
			tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
		return EXIT_FAILURE;
	}

	if (!(packet = packet_create(packet_class_bnet)))
	{
		std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
		psock_close(sd);
		if (changed_in)
			tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
		return EXIT_FAILURE;
	}
	packet_set_size(packet, sizeof(t_client_fileinforeq));
	packet_set_type(packet, CLIENT_FILEINFOREQ);
	bn_int_set(&packet->u.client_fileinforeq.type, CLIENT_FILEINFOREQ_TYPE_TOS);
	bn_int_set(&packet->u.client_fileinforeq.unknown2, CLIENT_FILEINFOREQ_UNKNOWN2);
	packet_append_string(packet, CLIENT_FILEINFOREQ_FILE_TOSUSA);
	client_blocksend_packet(sd, packet);
	packet_del_ref(packet);
	do
	if (client_blockrecv_packet(sd, rpacket) < 0)
	{
		std::fprintf(stderr, "%s: server closed connection\n", argv[0]);
		psock_close(sd);
		if (changed_in)
			tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
		return EXIT_FAILURE;
	}
	while (packet_get_type(rpacket) != SERVER_FILEINFOREPLY);
	dprintf("Got FILEINFOREPLY\n");

	if (!gotplayer)
	{
		munged = 1;
		commpos = 0;
		text[0] = '\0';
	}

	for (;;)
	{
		unsigned int i;

		if (!gotplayer)
		{
			switch (client_get_comm("player: ", text, sizeof(text), &commpos, 1, munged, screen_width))
			{
			case -1: /* cancel or error */
				break;

			case 0: /* timeout */
				munged = 0;
				continue;

			case 1:
				munged = 0;
				if (text[0] == '\0')
					continue;
			}

			if (text[0] == '\0')
				break;
			std::printf("\r");
			for (i = 0; i < std::strlen(text) && i < screen_width; i++)
				std::printf(" ");
			std::printf("\r");
		}

		if (!(packet = packet_create(packet_class_bnet)))
		{
			std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
			psock_close(sd);
			if (changed_in)
				tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
			return EXIT_FAILURE;
		}
		packet_set_size(packet, sizeof(t_client_statsreq));
		packet_set_type(packet, CLIENT_STATSREQ);
		bn_int_set(&packet->u.client_statsreq.name_count, 1);
		bn_int_set(&packet->u.client_statsreq.requestid, 0);
		packet_append_string(packet, text);
		count = 0;

		if (use_bnetd)
		{
			packet_append_string(packet, "BNET\\acct\\username");
			packet_append_string(packet, "BNET\\acct\\userid");
			packet_append_string(packet, "BNET\\acct\\lastlogin_clienttag");
			packet_append_string(packet, "BNET\\acct\\lastlogin_connection");
			packet_append_string(packet, "BNET\\acct\\lastlogin_time");
			packet_append_string(packet, "BNET\\acct\\firstlogin_clienttag");
			packet_append_string(packet, "BNET\\acct\\firstlogin_connection");
			packet_append_string(packet, "BNET\\acct\\firstlogin_time");
			count += 8;
		}
		if (use_fsgs)
		{
			packet_append_string(packet, "FSGS\\Created");
			count += 1;
		}

		packet_append_string(packet, "profile\\sex");
		packet_append_string(packet, "profile\\age");
		packet_append_string(packet, "profile\\location");
		packet_append_string(packet, "profile\\description");
		count += 4;

		if (std::strcmp(clienttag, "STAR") == 0)
		{
			packet_append_string(packet, "Record\\STAR\\0\\last game");
			packet_append_string(packet, "Record\\STAR\\0\\last game result");
			packet_append_string(packet, "Record\\STAR\\0\\wins");
			packet_append_string(packet, "Record\\STAR\\0\\losses");
			packet_append_string(packet, "Record\\STAR\\0\\disconnects");
			packet_append_string(packet, "Record\\STAR\\0\\draws");
			count += 6;

			packet_append_string(packet, "Record\\STAR\\1\\last game");
			packet_append_string(packet, "Record\\STAR\\1\\last game result");
			packet_append_string(packet, "Record\\STAR\\1\\rating");
			packet_append_string(packet, "Record\\STAR\\1\\active rating");
			packet_append_string(packet, "Record\\STAR\\1\\high rating");
			packet_append_string(packet, "Record\\STAR\\1\\rank");
			packet_append_string(packet, "Record\\STAR\\1\\active rank");
			packet_append_string(packet, "Record\\STAR\\1\\high rank");
			packet_append_string(packet, "Record\\STAR\\1\\wins");
			packet_append_string(packet, "Record\\STAR\\1\\losses");
			packet_append_string(packet, "Record\\STAR\\1\\disconnects");
			packet_append_string(packet, "Record\\STAR\\1\\draws");
			count += 12;
		}
		else if (std::strcmp(clienttag, "SEXP") == 0)
		{
			packet_append_string(packet, "Record\\SEXP\\0\\last game");
			packet_append_string(packet, "Record\\SEXP\\0\\last game result");
			packet_append_string(packet, "Record\\SEXP\\0\\wins");
			packet_append_string(packet, "Record\\SEXP\\0\\losses");
			packet_append_string(packet, "Record\\SEXP\\0\\disconnects");
			packet_append_string(packet, "Record\\SEXP\\0\\draws");
			count += 6;

			packet_append_string(packet, "Record\\SEXP\\1\\last game");
			packet_append_string(packet, "Record\\SEXP\\1\\last game result");
			packet_append_string(packet, "Record\\SEXP\\1\\rating");
			packet_append_string(packet, "Record\\SEXP\\1\\active rating");
			packet_append_string(packet, "Record\\SEXP\\1\\high rating");
			packet_append_string(packet, "Record\\SEXP\\1\\rank");
			packet_append_string(packet, "Record\\SEXP\\1\\active rank");
			packet_append_string(packet, "Record\\SEXP\\1\\high rank");
			packet_append_string(packet, "Record\\SEXP\\1\\wins");
			packet_append_string(packet, "Record\\SEXP\\1\\losses");
			packet_append_string(packet, "Record\\SEXP\\1\\disconnects");
			packet_append_string(packet, "Record\\SEXP\\1\\draws");
			count += 12;
		}
		else if (std::strcmp(clienttag, "SSHR") == 0)
		{
			packet_append_string(packet, "Record\\SSHR\\0\\last game");
			packet_append_string(packet, "Record\\SSHR\\0\\last game result");
			packet_append_string(packet, "Record\\SSHR\\0\\wins");
			packet_append_string(packet, "Record\\SSHR\\0\\losses");
			packet_append_string(packet, "Record\\SSHR\\0\\disconnects");
			packet_append_string(packet, "Record\\SSHR\\0\\draws");
			count += 6;
		}
		else if (std::strcmp(clienttag, "DSHR") == 0 ||
			std::strcmp(clienttag, "DRTL") == 0)
		{
			if (use_bnetd)
			{
				packet_append_string(packet, "BNET\\Record\\DRTL\\0\\level");
				packet_append_string(packet, "BNET\\Record\\DRTL\\0\\class");
				packet_append_string(packet, "BNET\\Record\\DRTL\\0\\strength");
				packet_append_string(packet, "BNET\\Record\\DRTL\\0\\magic");
				packet_append_string(packet, "BNET\\Record\\DRTL\\0\\dexterity");
				packet_append_string(packet, "BNET\\Record\\DRTL\\0\\vitality");
				packet_append_string(packet, "BNET\\Record\\DRTL\\0\\gold");
				packet_append_string(packet, "BNET\\Record\\DRTL\\0\\diablo kills");
				count += 8;
			}
		}
		else if (std::strcmp(clienttag, "W2BN") == 0)
		{
			packet_append_string(packet, "Record\\W2BN\\0\\last game");
			packet_append_string(packet, "Record\\W2BN\\0\\last game result");
			packet_append_string(packet, "Record\\W2BN\\0\\wins");
			packet_append_string(packet, "Record\\W2BN\\0\\losses");
			packet_append_string(packet, "Record\\W2BN\\0\\disconnects");
			packet_append_string(packet, "Record\\W2BN\\0\\draws");
			count += 6;

			packet_append_string(packet, "Record\\W2BN\\1\\last game");
			packet_append_string(packet, "Record\\W2BN\\1\\last game result");
			packet_append_string(packet, "Record\\W2BN\\1\\rating");
			packet_append_string(packet, "Record\\W2BN\\1\\active rating");
			packet_append_string(packet, "Record\\W2BN\\1\\high rating");
			packet_append_string(packet, "Record\\W2BN\\1\\rank");
			packet_append_string(packet, "Record\\W2BN\\1\\active rank");
			packet_append_string(packet, "Record\\W2BN\\1\\high rank");
			packet_append_string(packet, "Record\\W2BN\\1\\wins");
			packet_append_string(packet, "Record\\W2BN\\1\\losses");
			packet_append_string(packet, "Record\\W2BN\\1\\disconnects");
			packet_append_string(packet, "Record\\W2BN\\1\\draws");
			count += 12;

			packet_append_string(packet, "Record\\W2BN\\3\\last game");
			packet_append_string(packet, "Record\\W2BN\\3\\last game result");
			packet_append_string(packet, "Record\\W2BN\\3\\rating");
			packet_append_string(packet, "Record\\W2BN\\3\\active rating");
			packet_append_string(packet, "Record\\W2BN\\3\\high rating");
			packet_append_string(packet, "Record\\W2BN\\3\\rank");
			packet_append_string(packet, "Record\\W2BN\\3\\active rank");
			packet_append_string(packet, "Record\\W2BN\\3\\high rank");
			packet_append_string(packet, "Record\\W2BN\\3\\wins");
			packet_append_string(packet, "Record\\W2BN\\3\\losses");
			packet_append_string(packet, "Record\\W2BN\\3\\disconnects");
			packet_append_string(packet, "Record\\W2BN\\3\\draws");
			count += 12;
		}
		bn_int_set(&packet->u.client_statsreq.key_count, count);
		client_blocksend_packet(sd, packet);
		packet_del_ref(packet);

		do
		if (client_blockrecv_packet(sd, rpacket) < 0)
		{
			std::fprintf(stderr, "%s: server closed connection\n", argv[0]);
			if (changed_in)
				tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
			return EXIT_FAILURE;
		}
		while (packet_get_type(rpacket) != SERVER_STATSREPLY);
		dprintf("Got STATSREPLY\n");

		{
			unsigned int   names, keys;
			unsigned int   j;
			unsigned int   strpos;
			char const *   temp;
			unsigned int   uid;
			struct in_addr laddr;
			std::time_t         tm;
			char           timestr[STAT_TIME_MAXLEN];

			names = bn_int_get(rpacket->u.server_statsreply.name_count);
			keys = bn_int_get(rpacket->u.server_statsreply.key_count);

			std::printf("----\n");

			strpos = sizeof(t_server_statsreply);
			for (i = 0; i < names; i++)
			{
				j = 0;
				if (use_bnetd)
				{
					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (temp[0] == '\0')
						std::printf("                        Username: UNKNOWN\n");
					else
						std::printf("                        Username: %s\n", temp);

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &uid) < 0 || uid < 1)
						std::printf("                         Account: UNKNOWN\n");
					else
						std::printf("                         Account: "UID_FORMAT"\n", uid);

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					std::printf("               Last login client: %s\n", temp);

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &val) < 0)
						std::printf("                 Last login host: UNKNOWN\n");
					else
					{
						laddr.s_addr = htonl(val);
						std::printf("                 Last login host: %s\n", inet_ntoa(laddr));
					}

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &val) < 0 || val == 0)
						std::printf("                 Last login time: NEVER\n");
					else
					{
						tm = (std::time_t)val;
						std::strftime(timestr, STAT_TIME_MAXLEN, STAT_TIME_FORMAT, std::localtime(&tm));
						std::printf("                 Last login time: %s\n", timestr);
					}

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					std::printf("              First login client: %s\n", temp);

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &val) < 0)
						std::printf("                First login host: UNKNOWN\n");
					else
					{
						laddr.s_addr = htonl(val);
						std::printf("                First login host: %s\n", inet_ntoa(laddr));
					}

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &val) < 0 || val == 0)
						std::printf("                First login time: NEVER\n");
					else
					{
						tm = (std::time_t)val;
						std::strftime(timestr, STAT_TIME_MAXLEN, STAT_TIME_FORMAT, std::localtime(&tm));
						std::printf("                First login time: %s\n", timestr);
					}
				}
				else
					std::printf("                        Username: %s\n", text);
				if (use_fsgs)
				{
					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &val) < 0 || val == 0)
						std::printf("                   Creation time: NEVER\n");
					else
					{
						tm = (std::time_t)val;
						std::strftime(timestr, STAT_TIME_MAXLEN, STAT_TIME_FORMAT, std::localtime(&tm));
						std::printf("                   Creation time: %s\n", timestr);
					}
				}

				if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
					strpos += std::strlen(temp) + 1;
				else
					temp = "";
				j++;
				std::printf("                             Sex: %s\n", temp);

				if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
					strpos += std::strlen(temp) + 1;
				else
					temp = "";
				j++;
				std::printf("                             Age: %s\n", temp);

				if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
					strpos += std::strlen(temp) + 1;
				else
					temp = "";
				j++;
				std::printf("                        Location: %s\n", temp);

				if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
					strpos += std::strlen(temp) + 1;
				else
					temp = "";
				j++;
				std::printf("                     Description: %s\n", temp);

				if (std::strcmp(clienttag, "STAR") == 0 ||
					std::strcmp(clienttag, "SSHR") == 0 ||
					std::strcmp(clienttag, "SEXP") == 0 ||
					std::strcmp(clienttag, "W2BN") == 0)
				{
					t_bnettime   bntime;
					unsigned int wins, losses, disconnects, draws;

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (bnettime_set_str(&bntime, temp) < 0)
						std::strcpy(timestr, "NEVER");
					else
					{
						tm = bnettime_to_time(bntime);
						std::strftime(timestr, STAT_TIME_MAXLEN, STAT_TIME_FORMAT, std::localtime(&tm));
					}
					std::printf("                  Last game time: %s\n", timestr);

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					std::printf("                Last game result: %s\n", temp);

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &wins) < 0)
						wins = 0;
					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &losses) < 0)
						losses = 0;
					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &disconnects) < 0)
						disconnects = 0;
					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &draws) < 0)
						draws = 0;
					std::printf("                          Record: (%u/%u/%u) %u draws\n", wins, losses, disconnects, draws);
				}

				if (std::strcmp(clienttag, "STAR") == 0 ||
					std::strcmp(clienttag, "SEXP") == 0 ||
					std::strcmp(clienttag, "W2BN") == 0)
				{
					t_bnettime   bntime;
					unsigned int wins, losses, disconnects, draws;
					unsigned int rating, rank;
					unsigned int active_rating, active_rank;
					unsigned int high_rating, high_rank;

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (bnettime_set_str(&bntime, temp) < 0)
						std::strcpy(timestr, "NEVER");
					else
					{
						tm = bnettime_to_time(bntime);
						std::strftime(timestr, STAT_TIME_MAXLEN, STAT_TIME_FORMAT, std::localtime(&tm));
					}
					std::printf("  Last standard ladder game time: %s\n", timestr);

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					std::printf("Last standard ladder game result: %s\n", temp);

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &rating) < 0)
						rating = 0;
					if (rating)
						std::printf("  Current standard ladder rating: %u\n", rating);
					else
						std::printf("  Current standard ladder rating: UNRATED\n");

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &active_rating) < 0)
						active_rating = 0;
					if (active_rating)
						std::printf("   Active standard ladder rating: %u\n", active_rating);
					else
						std::printf("   Active standard ladder rating: UNRATED\n");

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &high_rating) < 0)
						high_rating = 0;
					if (high_rating)
						std::printf("     High standard ladder rating: %u\n", high_rating);
					else
						std::printf("     High standard ladder rating: UNRATED\n");

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &rank) < 0)
						rank = 0;
					if (rank)
						std::printf("    Current standard ladder rank: #%u\n", rank);
					else
						std::printf("    Current standard ladder rank: UNRANKED\n");

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &active_rank) < 0)
						active_rank = 0;
					if (active_rank)
						std::printf("     Active standard ladder rank: #%u\n", active_rank);
					else
						std::printf("     Active standard ladder rank: UNRANKED\n");

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &high_rank) < 0)
						high_rank = 0;
					if (high_rank)
						std::printf("       High standard ladder rank: #%u\n", high_rank);
					else
						std::printf("       High standard ladder rank: UNRANKED\n");

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &wins) < 0)
						wins = 0;
					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &losses) < 0)
						losses = 0;
					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &disconnects) < 0)
						disconnects = 0;
					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &draws) < 0)
						draws = 0;
					std::printf("          Standard ladder record: (%u/%u/%u) %u draws\n", wins, losses, disconnects, draws);
				}

				if (std::strcmp(clienttag, "W2BN") == 0)
				{
					t_bnettime   bntime;
					unsigned int wins, losses, disconnects, draws;
					unsigned int rating, rank;
					unsigned int active_rating, active_rank;
					unsigned int high_rating, high_rank;

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (bnettime_set_str(&bntime, temp) < 0)
						std::strcpy(timestr, "NEVER");
					else
					{
						tm = bnettime_to_time(bntime);
						std::strftime(timestr, STAT_TIME_MAXLEN, STAT_TIME_FORMAT, std::localtime(&tm));
					}
					std::printf("   Last Ironman ladder game time: %s\n", timestr);

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					std::printf(" Last Ironman ladder game result: %s\n", temp);

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &rating) < 0)
						rating = 0;
					if (rating)
						std::printf("   Current Ironman ladder rating: %u\n", rating);
					else
						std::printf("   Current Ironman ladder rating: UNRATED\n");

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &active_rating) < 0)
						active_rating = 0;
					if (active_rating)
						std::printf("    Active Ironman ladder rating: %u\n", active_rating);
					else
						std::printf("    Active Ironman ladder rating: UNRATED\n");

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &high_rating) < 0)
						high_rating = 0;
					if (high_rating)
						std::printf("      High Ironman ladder rating: %u\n", high_rating);
					else
						std::printf("      High Ironman ladder rating: UNRATED\n");

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &rank) < 0)
						rank = 0;
					if (rank)
						std::printf("     Current Ironman ladder rank: #%u\n", rank);
					else
						std::printf("     Current Ironman ladder rank: UNRANKED\n");

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &active_rank) < 0)
						active_rank = 0;
					if (active_rank)
						std::printf("      Active Ironman ladder rank: #%u\n", active_rank);
					else
						std::printf("      Active Ironman ladder rank: UNRANKED\n");

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &high_rank) < 0)
						high_rank = 0;
					if (high_rank)
						std::printf("        High Ironman ladder rank: #%u\n", high_rank);
					else
						std::printf("        High Ironman ladder rank: UNRANKED\n");

					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &wins) < 0)
						wins = 0;
					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &losses) < 0)
						losses = 0;
					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &disconnects) < 0)
						disconnects = 0;
					if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
						strpos += std::strlen(temp) + 1;
					else
						temp = "";
					j++;
					if (str_to_uint(temp, &draws) < 0)
						draws = 0;
					std::printf("           Ironman ladder record: (%u/%u/%u) %u draws\n", wins, losses, disconnects, draws);
				}

				if (std::strcmp(clienttag, "DSHR") == 0 ||
					std::strcmp(clienttag, "DRTL") == 0)
				{
					unsigned int level;
					unsigned int chclass;
					unsigned int diablo_kills;
					unsigned int strength;
					unsigned int magic;
					unsigned int dexterity;
					unsigned int vitality;
					unsigned int gold;

					if (use_bnetd)
					{
						if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
							strpos += std::strlen(temp) + 1;
						else
							temp = "";
						j++;
						if (str_to_uint(temp, &level)<0)
							level = 0;
						if (level>0)
							std::printf("                           Level: %u\n", level);
						else
							std::printf("                           Level: NONE\n");

						if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
							strpos += std::strlen(temp) + 1;
						else
							temp = "";
						j++;
						if (str_to_uint(temp, &chclass) < 0)
							chclass = 99;
						std::printf("                           Class: %s\n", bnclass_get_str(chclass));

						if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
							strpos += std::strlen(temp) + 1;
						else
							temp = "";
						j++;
						if (str_to_uint(temp, &strength) < 0)
							strength = 0;

						if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
							strpos += std::strlen(temp) + 1;
						else
							temp = "";
						j++;
						if (str_to_uint(temp, &magic) < 0)
							magic = 0;

						if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
							strpos += std::strlen(temp) + 1;
						else
							temp = "";
						j++;
						if (str_to_uint(temp, &dexterity) < 0)
							dexterity = 0;

						if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
							strpos += std::strlen(temp) + 1;
						else
							temp = "";
						j++;
						if (str_to_uint(temp, &vitality) < 0)
							vitality = 0;

						if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
							strpos += std::strlen(temp) + 1;
						else
							temp = "";
						j++;
						if (str_to_uint(temp, &gold) < 0)
							gold = 0;

						std::printf("                           Stats: %u str  %u mag  %u dex  %u vit  %u gld\n",
							strength,
							magic,
							dexterity,
							vitality,
							gold);

						if (j < keys && (temp = packet_get_str_const(rpacket, strpos, 256)))
							strpos += std::strlen(temp) + 1;
						else
							temp = "";
						j++;
						if (str_to_uint(temp, &diablo_kills) < 0)
							diablo_kills = 0;
						std::printf("                    Diablo Kills: %u\n", diablo_kills);
					}
				}
			}
			std::printf("----\n");
		}

		if (gotplayer)
			break;
		commpos = 0;
		text[0] = '\0';
	}

	psock_close(sd);
	if (changed_in)
		tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
	return EXIT_SUCCESS;
}
