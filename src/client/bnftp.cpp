/*
 * Copyright (C) 1999,2001  Ross Combs (rocombs@cs.nmsu.edu)
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
#include <ctime>
#include <cstring>
#include <cctype>
#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif
#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif

#include "compat/termios.h"
#include "compat/psock.h"
#include "common/packet.h"
#include "common/field_sizes.h"
#include "common/bnettime.h"
#include "common/hexdump.h"
#include "common/util.h"
#include "common/tag.h"
#include "common/bn_type.h"
#include "common/xalloc.h"
#include "common/file_protocol.h"
#include "common/init_protocol.h"
#ifdef CLIENTDEBUG
# include "common/eventlog.h"
#endif
#include "client.h"

#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_WS2TCPIP_H
# include <Ws2tcpip.h>
#endif

#include "common/setup_after.h"

using namespace pvpgn;
using namespace pvpgn::client;

namespace
{

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
			"    --client=WAR3               report client as Warcraft III\n"
			"    --client=W3XP               report client as Warcraft III: FT\n"
			"    --hexdump=FILE              do hex dump of packets into FILE\n");
		std::fprintf(stderr,
			"    --arch=IX86                 report architecture as Windows (x86)\n"
			"    --arch=PMAC                 report architecture as Macintosh\n"
			"    --arch=XMAC                 report architecture as Macintosh OSX\n"
			);
		std::fprintf(stderr,
			"    --startoffset=OFFSET        force offset to be OFFSET\n"
			"    --exists=ACTION             Ask/Overwrite/Backup/Resume if the file exists\n"
			"    --file=FILENAME             use FILENAME instead of asking\n"
			"    -h, --help, --usage         show this information and exit\n"
			"    -v, --version               print version number and exit\n");
		std::exit(EXIT_FAILURE);
	}

}

extern int main(int argc, char * argv[])
{
	int                a;
	int                sd;
	struct sockaddr_in saddr;
	t_packet *         packet;
	t_packet *         rpacket;
	t_packet *         fpacket;
	char const *       clienttag = NULL;
	char const *       archtag = NULL;
	char const *       servname = NULL;
	unsigned short     servport = 0;
	char const *       hexfile = NULL;
	char               text[MAX_MESSAGE_LEN];
	char const *       reqfile = NULL;
	struct hostent *   host;
	unsigned int       commpos;
	struct termios     in_attr_old;
	struct termios     in_attr_new;
	int                changed_in;
	unsigned int       currsize;
	unsigned int       filelen;
	unsigned int       startoffset;
	int                startoffsetoverride = 0;
#define EXIST_ACTION_UNSPEC    -1
#define EXIST_ACTION_ASK        0
#define EXIST_ACTION_OVERWRITE  1
#define EXIST_ACTION_BACKUP     2
#define EXIST_ACTION_RESUME     3
	int		           exist_action = EXIST_ACTION_UNSPEC;
	struct stat        exist_buf;
	char const *       filename;
	std::FILE *        fp;
	std::FILE *        hexstrm = NULL;
	int                fd_stdin;
	t_bnettime         bntime;
	std::time_t        tm;
	char               timestr[FILE_TIME_MAXLEN];
	unsigned int       screen_width, screen_height;
	int                munged;
	unsigned int       newproto = 0;

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
			std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], clienttag);
			usage(argv[0]);
		}
		clienttag = CLIENTTAG_DIABLO2DV;
	}
	else if (std::strcmp(argv[a], "--client=D2XP") == 0)
	{
		if (clienttag)
		{
			std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], clienttag);
			usage(argv[0]);
		}
		clienttag = CLIENTTAG_DIABLO2XP;
	}
	else if (std::strcmp(argv[a], "--client=WAR3") == 0)
	{
		if (clienttag)
		{
			std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], clienttag);
			usage(argv[0]);
		}
		clienttag = CLIENTTAG_WARCRAFT3;
		newproto = 1;
	}
	else if (std::strcmp(argv[a], "--client=W3XP") == 0)
	{
		if (clienttag)
		{
			std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], clienttag);
			usage(argv[0]);
		}
		clienttag = CLIENTTAG_WAR3XP;
		newproto = 1;
	}
	else if (std::strncmp(argv[a], "--client=", 9) == 0)
	{
		std::fprintf(stderr, "%s: unknown client tag \"%s\"\n", argv[0], &argv[a][9]);
		usage(argv[0]);
	}
	else if (std::strcmp(argv[a], "--arch=IX86") == 0)
	{
		if (archtag)
		{
			std::fprintf(stderr, "%s: architecture type was already specified as \"%s\"\n", argv[0], archtag);
			usage(argv[0]);
		}
		archtag = ARCHTAG_WINX86;
	}
	else if (std::strcmp(argv[a], "--arch=PMAC") == 0)
	{
		if (archtag)
		{
			std::fprintf(stderr, "%s: architecture type was already specified as \"%s\"\n", argv[0], archtag);
			usage(argv[0]);
		}
		archtag = ARCHTAG_MACPPC;
	}
	else if (std::strcmp(argv[a], "--arch=XMAC") == 0)
	{
		if (archtag)
		{
			std::fprintf(stderr, "%s: architecture type was already specified as \"%s\"\n", argv[0], archtag);
			usage(argv[0]);
		}
		archtag = ARCHTAG_OSXPPC;
	}
	else if (std::strncmp(argv[a], "--arch=", 9) == 0)
	{
		std::fprintf(stderr, "%s: unknown architecture tag \"%s\"\n", argv[0], &argv[a][9]);
		usage(argv[0]);
	}
	else if (std::strncmp(argv[a], "--hexdump=", 10) == 0)
	{
		if (hexfile)
		{
			std::fprintf(stderr, "%s: hexdump file was already specified as \"%s\"\n", argv[0], hexfile);
			usage(argv[0]);
		}
		hexfile = &argv[a][10];
	}
	else if (std::strncmp(argv[a], "--startoffset=", 14) == 0)
	{
		if (startoffsetoverride)
		{
			std::fprintf(stderr, "%s: startoffset was already specified as %u\n", argv[0], startoffset);
			usage(argv[0]);
		}
		if (str_to_uint(&argv[a][14], &startoffset) < 0)
		{
			std::fprintf(stderr, "%s: startoffset \"%s\" should be a positive integer\n", argv[0], &argv[a][14]);
			usage(argv[0]);
		}
		startoffsetoverride = 1;
	}
	else if (std::strncmp(argv[a], "--exists=", 9) == 0)
	{
		if (exist_action != EXIST_ACTION_UNSPEC)
		{
			std::fprintf(stderr, "%s: exists was already specified\n", argv[0]);
			usage(argv[0]);
		}
		if (argv[a][9] == 'o' || argv[a][9] == 'O')
			exist_action = EXIST_ACTION_OVERWRITE;
		else if (argv[a][9] == 'a' || argv[a][9] == 'A')
			exist_action = EXIST_ACTION_ASK;
		else if (argv[a][9] == 'b' || argv[a][9] == 'B')
			exist_action = EXIST_ACTION_BACKUP;
		else if (argv[a][9] == 'r' || argv[a][9] == 'R')
			exist_action = EXIST_ACTION_RESUME;
		else {
			std::fprintf(stderr, "%s: exists must begin with a,A,o,O,b,B,r or R", argv[0]);
			usage(argv[0]);
		}
	}
	else if (std::strncmp(argv[a], "--file=", 7) == 0)
	{
		if (reqfile)
		{
			std::fprintf(stderr, "%s: file was already specified as \"%s\"\n", argv[0], reqfile);
			usage(argv[0]);
		}
		reqfile = &argv[a][7];
	}
	else if (std::strcmp(argv[a], "-v") == 0 || std::strcmp(argv[a], "--version") == 0)
	{
		std::printf("version " PVPGN_VERSION "\n");
		return 0;
	}
	else if (std::strcmp(argv[a], "-h") == 0 || std::strcmp(argv[a], "--help") == 0 || std::strcmp(argv[a], "--usage") == 0)
		usage(argv[0]);
	else if (std::strcmp(argv[a], "--client") == 0 || std::strcmp(argv[a], "--hexdump") == 0)
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
	if (!clienttag)
		clienttag = CLIENTTAG_STARCRAFT;
	if (!archtag)
		archtag = ARCHTAG_WINX86;
	if (!servname)
		servname = BNETD_DEFAULT_HOST;
	if (exist_action == EXIST_ACTION_UNSPEC)
		exist_action = EXIST_ACTION_ASK;

	if (hexfile)
	{
		if (!(hexstrm = std::fopen(hexfile, "w")))
			std::fprintf(stderr, "%s: could not open file \"%s\" for writing the hexdump (std::fopen: %s)", argv[0], hexfile, std::strerror(errno));
		else
		{
			std::fprintf(hexstrm, "# dump generated by bnftp version " PVPGN_VERSION "\n");
		}
	}

	if (psock_init() < 0)
	{
		std::fprintf(stderr, "%s: could not inialialize socket functions\n", argv[0]);
		return EXIT_FAILURE;
	}

	if (!(host = gethostbyname(servname)))
	{
		std::fprintf(stderr, "%s: unknown host \"%s\"\n", argv[0], servname);
		return EXIT_FAILURE;
	}

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
		std::fprintf(stderr, "%s: could not get terminal attributes for stdin\n", argv[0]);
		changed_in = 0;
	}

	if (client_get_termsize(fd_stdin, &screen_width, &screen_height) < 0)
	{
		std::fprintf(stderr, "%s: could not determine screen size\n", argv[0]);
		if (changed_in)
			tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
		return EXIT_FAILURE;
	}

	if ((sd = psock_socket(PSOCK_PF_INET, PSOCK_SOCK_STREAM, PSOCK_IPPROTO_TCP)) < 0)
	{
		std::fprintf(stderr, "%s: could not create socket (psock_socket: %s)\n", argv[0], std::strerror(psock_errno()));
		if (changed_in)
			tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
		return EXIT_FAILURE;
	}

	std::memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = PSOCK_AF_INET;
	saddr.sin_port = htons(servport);
	std::memcpy(&saddr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
	if (psock_connect(sd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0)
	{
		std::fprintf(stderr, "%s: could not connect to server \"%s\" port %hu (psock_connect: %s)\n", argv[0], servname, servport, std::strerror(psock_errno()));
		if (changed_in)
			tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
		return EXIT_FAILURE;
	}

	char addrstr[INET_ADDRSTRLEN] = { 0 };
	inet_ntop(AF_INET, &(saddr.sin_addr), addrstr, sizeof(addrstr));
	std::printf("Connected to %s:%hu.\n", addrstr, servport);

#ifdef CLIENTDEBUG
	eventlog_set(stderr);
#endif

	if (!(packet = packet_create(packet_class_init)))
	{
		std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
		if (changed_in)
			tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
		return EXIT_FAILURE;
	}
	bn_byte_set(&packet->u.client_initconn.cclass, CLIENT_INITCONN_CLASS_FILE);
	if (hexstrm)
	{
		std::fprintf(hexstrm, "%d: send class=%s[0x%02hx] type=%s[0x%04hx] length=%u\n",
			sd,
			packet_get_class_str(packet), (unsigned int)packet_get_class(packet),
			packet_get_type_str(packet, packet_dir_from_client), packet_get_type(packet),
			packet_get_size(packet));
		hexdump(hexstrm, packet_get_raw_data(packet, 0), packet_get_size(packet));
	}
	client_blocksend_packet(sd, packet);
	packet_del_ref(packet);

	if (!(rpacket = packet_create(packet_class_file)))
	{
		std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
		if (changed_in)
			tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
		return EXIT_FAILURE;
	}

	if (!(fpacket = packet_create(packet_class_raw)))
	{
		std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
		if (changed_in)
			tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
		packet_del_ref(rpacket);
		return EXIT_FAILURE;
	}

	if (!reqfile) /* if not specified on the command line then prompt for it */
	{
		munged = 1;
		commpos = 0;
		text[0] = '\0';

		for (;;)
		{
			switch (client_get_comm("filename: ", text, sizeof(text), &commpos, 1, munged, screen_width))
			{
			case -1: /* cancel or error */
				std::printf("\n");
				if (changed_in)
					tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
				packet_del_ref(fpacket);
				packet_del_ref(rpacket);
				return EXIT_FAILURE;

			case 0: /* timeout */
				munged = 0;
				continue;

			case 1:
				munged = 0;
				if (text[0] == '\0')
					continue;
				std::printf("\n");
			}
			break;
		}
		reqfile = text;
	}

	if (stat(reqfile, &exist_buf) == 0) /* check if the file exists */
	{
		char text2[MAX_MESSAGE_LEN];

		munged = 1;
		commpos = 0;
		text2[0] = '\0';

		while (exist_action == EXIST_ACTION_ASK)
		{
			switch (client_get_comm("File exists [O]verwrite, [B]ackup or [R]esume?: ", text2, sizeof(text2), &commpos, 1, munged, screen_width))
			{
			case -1: /* cancel or error */
				std::printf("\n");
				if (changed_in)
					tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
				packet_del_ref(fpacket);
				packet_del_ref(rpacket);
				return EXIT_FAILURE;

			case 0: /* timeout */
				munged = 0;
				continue;

			case 1:
				munged = 0;
				if (text2[0] == '\0')
					continue;
				std::printf("\n");
				break;
			}

			switch (text2[0])
			{
			case 'o':
			case 'O':
				exist_action = EXIST_ACTION_OVERWRITE;
				break;
			case 'b':
			case 'B':
				exist_action = EXIST_ACTION_BACKUP;
				break;
			case 'r':
			case 'R':
				exist_action = EXIST_ACTION_RESUME;
				break;
			default:
				std::printf("Please answer with o,O,b,B,r or R.\n");
				munged = 1;
				continue;
			}
			break;
		}

		switch (exist_action)
		{
		case EXIST_ACTION_OVERWRITE:
			if (!startoffsetoverride)
				startoffset = 0;
			break;
		case EXIST_ACTION_BACKUP:
		{
									char *       bakfile;
									unsigned int bnr;
									int          renamed = 0;

									bakfile = (char*)xmalloc(std::strlen(reqfile) + 1 + 2 + 1); /* assuming we go up to bnr 99 we need reqfile+'.'+'99'+'\0' */
									for (bnr = 0; bnr < 100; bnr++)
									{
										std::sprintf(bakfile, "%s.%d", reqfile, bnr);
										if (stat(bakfile, &exist_buf) == 0)
											continue; /* backup exists */
										/* backup does not exist */
										if (std::rename(reqfile, bakfile) < 0) /* just std::rename the existing file to the backup */
											std::fprintf(stderr, "%s: could not create backup file \"%s\" (std::rename: %s)\n", argv[0], bakfile, std::strerror(errno));
										else
										{
											renamed = 1;
											std::printf("Renaming \"%s\" to \"%s\".\n", reqfile, bakfile);
										}
										break;
									}
									xfree(bakfile);
									if (!renamed)
									{
										std::fprintf(stderr, "%s: could not create backup for \"%s\".\n", argv[0], reqfile);
										if (changed_in)
											tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);
										packet_del_ref(fpacket);
										packet_del_ref(rpacket);
										return EXIT_FAILURE;
									}
									if (!startoffsetoverride)
										startoffset = 0;
		}
			break;
		case EXIST_ACTION_RESUME:
			if (!startoffsetoverride)
				startoffset = exist_buf.st_size;
			break;
		}
	}
	else
	if (!startoffsetoverride)
		startoffset = 0;

	if (changed_in)
		tcsetattr(fd_stdin, TCSAFLUSH, &in_attr_old);

	if (!(packet = packet_create(packet_class_file)))
	{
		std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
		packet_del_ref(fpacket);
		packet_del_ref(rpacket);
		return EXIT_FAILURE;
	}
	if (newproto) {
		/* first send ARCH/CTAG */
		packet_set_size(packet, sizeof(t_client_file_req2));
		packet_set_type(packet, CLIENT_FILE_REQ2);
		bn_int_tag_set(&packet->u.client_file_req2.archtag, archtag);
		bn_int_tag_set(&packet->u.client_file_req2.clienttag, clienttag);
		bn_long_set_a_b(&packet->u.client_file_req2.unknown1, 0, 0);
	}
	else {
		packet_set_size(packet, sizeof(t_client_file_req));
		packet_set_type(packet, CLIENT_FILE_REQ);
		bn_int_tag_set(&packet->u.client_file_req.archtag, archtag);
		bn_int_tag_set(&packet->u.client_file_req.clienttag, clienttag);
		bn_int_set(&packet->u.client_file_req.adid, 0);
		bn_int_set(&packet->u.client_file_req.extensiontag, 0);
		bn_int_set(&packet->u.client_file_req.startoffset, startoffset);
		bn_long_set_a_b(&packet->u.client_file_req.timestamp, 0x00000000, 0x00000000);
		packet_append_string(packet, reqfile);
	}
	if (hexstrm)
	{
		std::fprintf(hexstrm, "%d: send class=%s[0x%02hx] type=%s[0x%04hx] length=%u\n",
			sd,
			packet_get_class_str(packet), (unsigned int)packet_get_class(packet),
			packet_get_type_str(packet, packet_dir_from_client), packet_get_type(packet),
			packet_get_size(packet));
		hexdump(hexstrm, packet_get_raw_data(packet, 0), packet_get_size(packet));
	}
	if (newproto) std::printf("\nSending ARCH/CTAG info...");
	else std::printf("\nRequesting info...");
	std::fflush(stdout);
	client_blocksend_packet(sd, packet);
	packet_del_ref(packet);

	if (newproto) {
		/* received the stupid 4 bytes unknown "packet" */
		packet_set_size(fpacket, sizeof(t_server_file_unknown1));
		if (client_blockrecv_packet(sd, fpacket) < 0)
		{
			std::fprintf(stderr, "%s: server closed connection\n", argv[0]);
			packet_del_ref(fpacket);
			packet_del_ref(rpacket);
			return EXIT_FAILURE;
		}
		if (hexstrm)
		{
			std::fprintf(hexstrm, "%d: recv class=%s[0x%02hx] type=%s[0x%04hx] length=%u\n",
				sd,
				packet_get_class_str(fpacket), (unsigned int)packet_get_class(fpacket),
				packet_get_type_str(fpacket, packet_dir_from_server), packet_get_type(fpacket),
				packet_get_size(fpacket));
			hexdump(hexstrm, packet_get_raw_data(fpacket, 0), packet_get_size(fpacket));
		}

		/* send file info request */
		packet_set_size(fpacket, sizeof(t_client_file_req3));
		bn_int_set(&fpacket->u.client_file_req3.unknown1, 0);
		bn_long_set_a_b(&fpacket->u.client_file_req3.timestamp, 0, 0);
		bn_long_set_a_b(&fpacket->u.client_file_req3.unknown2, 0, 0);
		bn_long_set_a_b(&fpacket->u.client_file_req3.unknown3, 0, 0);
		bn_long_set_a_b(&fpacket->u.client_file_req3.unknown4, 0, 0);
		bn_long_set_a_b(&fpacket->u.client_file_req3.unknown5, 0, 0);
		bn_long_set_a_b(&fpacket->u.client_file_req3.unknown6, 0, 0);
		packet_append_string(fpacket, reqfile);
		std::printf("\nRequesting info...");
		std::fflush(stdout);
		client_blocksend_packet(sd, fpacket);
		if (hexstrm)
		{
			std::fprintf(hexstrm, "%d: send class=%s[0x%02hx] type=%s[0x%04hx] length=%u\n",
				sd,
				packet_get_class_str(fpacket), (unsigned int)packet_get_class(fpacket),
				packet_get_type_str(fpacket, packet_dir_from_client), packet_get_type(fpacket),
				packet_get_size(fpacket));
			hexdump(hexstrm, packet_get_raw_data(fpacket, 0), packet_get_size(fpacket));
		}
	}
	do
	{
		if (client_blockrecv_packet(sd, rpacket) < 0)
		{
			std::fprintf(stderr, "%s: server closed connection\n", argv[0]);
			packet_del_ref(fpacket);
			packet_del_ref(rpacket);
			return EXIT_FAILURE;
		}
		if (hexstrm)
		{
			std::fprintf(hexstrm, "%d: recv class=%s[0x%02hx] type=%s[0x%04hx] length=%u\n",
				sd,
				packet_get_class_str(rpacket), (unsigned int)packet_get_class(rpacket),
				packet_get_type_str(rpacket, packet_dir_from_server), packet_get_type(rpacket),
				packet_get_size(rpacket));
			hexdump(hexstrm, packet_get_raw_data(rpacket, 0), packet_get_size(rpacket));
		}
	} while (packet_get_type(rpacket) != SERVER_FILE_REPLY);

	filelen = bn_int_get(rpacket->u.server_file_reply.filelen);
	bn_long_to_bnettime(rpacket->u.server_file_reply.timestamp, &bntime);
	tm = bnettime_to_time(bntime);
	std::strftime(timestr, FILE_TIME_MAXLEN, FILE_TIME_FORMAT, std::localtime(&tm));
	filename = packet_get_str_const(rpacket, sizeof(t_server_file_reply), MAX_FILENAME_STR);

	if (exist_action == EXIST_ACTION_RESUME)
	{
		if (!(fp = std::fopen(reqfile, "ab")))
		{
			std::fprintf(stderr, "%s: could not open file \"%s\" for appending (std::fopen: %s)\n", argv[0], reqfile, std::strerror(errno));
			packet_del_ref(fpacket);
			packet_del_ref(rpacket);
			return EXIT_FAILURE;
		}
	}
	else
	{
		if (!(fp = std::fopen(reqfile, "wb")))
		{
			std::fprintf(stderr, "%s: could not open file \"%s\" for writing (std::fopen: %s)\n", argv[0], reqfile, std::strerror(errno));
			packet_del_ref(fpacket);
			packet_del_ref(rpacket);
			return EXIT_FAILURE;
		}
	}

	std::printf("\n name: \"");
	str_print_term(stdout, filename, 0, 0);
	std::printf("\"\n changed: %s\n length: %u bytes\n", timestr, filelen);
	std::fflush(stdout);

	if (startoffset > 0) {
		filelen -= startoffset; /* for resuming files */
		std::printf("Resuming at position %u (%u bytes remaining).\n", startoffset, filelen);
	}

	std::printf("\nSaving to \"%s\"...", reqfile);

	for (currsize = 0; currsize + MAX_PACKET_SIZE <= filelen; currsize += MAX_PACKET_SIZE)
	{
		std::printf(".");
		std::fflush(stdout);

		if (client_blockrecv_raw_packet(sd, fpacket, MAX_PACKET_SIZE) < 0)
		{
			std::printf("error\n");
			std::fprintf(stderr, "%s: server closed connection\n", argv[0]);
			if (std::fclose(fp) < 0)
				std::fprintf(stderr, "%s: could not close file \"%s\" after writing (std::fclose: %s)\n", argv[0], reqfile, std::strerror(errno));
			packet_del_ref(fpacket);
			packet_del_ref(rpacket);
			return EXIT_FAILURE;
		}
		if (hexstrm)
		{
			std::fprintf(hexstrm, "%d: recv class=%s[0x%02hx] type=%s[0x%04hx] length=%u\n",
				sd,
				packet_get_class_str(fpacket), (unsigned int)packet_get_class(fpacket),
				packet_get_type_str(fpacket, packet_dir_from_server), packet_get_type(fpacket),
				packet_get_size(fpacket));
			hexdump(hexstrm, packet_get_raw_data(fpacket, 0), packet_get_size(fpacket));
		}
		if (std::fwrite(packet_get_raw_data_const(fpacket, 0), 1, MAX_PACKET_SIZE, fp) < MAX_PACKET_SIZE)
		{
			std::printf("error\n");
			std::fprintf(stderr, "%s: could not write to file \"%s\" (std::fwrite: %s)\n", argv[0], reqfile, std::strerror(errno));
			if (std::fclose(fp) < 0)
				std::fprintf(stderr, "%s: could not close file \"%s\" after writing (std::fclose: %s)\n", argv[0], reqfile, std::strerror(errno));
			packet_del_ref(fpacket);
			packet_del_ref(rpacket);
			return EXIT_FAILURE;
		}
	}
	filelen -= currsize;
	if (filelen)
	{
		std::printf(".");
		std::fflush(stdout);

		if (client_blockrecv_raw_packet(sd, fpacket, filelen) < 0)
		{
			std::printf("error\n");
			std::fprintf(stderr, "%s: server closed connection\n", argv[0]);
			if (std::fclose(fp) < 0)
				std::fprintf(stderr, "%s: could not close file \"%s\" after writing (std::fclose: %s)\n", argv[0], reqfile, std::strerror(errno));
			packet_del_ref(fpacket);
			packet_del_ref(rpacket);
			return EXIT_FAILURE;
		}
		if (hexstrm)
		{
			std::fprintf(hexstrm, "%d: recv class=%s[0x%02hx] type=%s[0x%04hx] length=%u\n",
				sd,
				packet_get_class_str(fpacket), (unsigned int)packet_get_class(fpacket),
				packet_get_type_str(fpacket, packet_dir_from_server), packet_get_type(fpacket),
				packet_get_size(fpacket));
			hexdump(hexstrm, packet_get_raw_data(fpacket, 0), packet_get_size(fpacket));
		}
		if (std::fwrite(packet_get_raw_data_const(fpacket, 0), 1, filelen, fp) < filelen)
		{
			std::printf("error\n");
			std::fprintf(stderr, "%s: could not write to file \"%s\"\n", argv[0], reqfile);
			if (std::fclose(fp) < 0)
				std::fprintf(stderr, "%s: could not close file \"%s\" after writing (std::fclose: %s)\n", argv[0], reqfile, std::strerror(errno));
			packet_del_ref(fpacket);
			packet_del_ref(rpacket);
			return EXIT_FAILURE;
		}
	}

	packet_del_ref(fpacket);
	packet_del_ref(rpacket);

	if (hexstrm)
	{
		std::fprintf(hexstrm, "# end of dump\n");
		if (std::fclose(hexstrm) < 0)
			std::fprintf(stderr, "%s: could not close hexdump file \"%s\" after writing (std::fclose: %s)", argv[0], hexfile, std::strerror(errno));
	}

	if (std::fclose(fp) < 0)
	{
		std::fprintf(stderr, "%s: could not close file \"%s\" after writing (std::fclose: %s)\n", argv[0], reqfile, std::strerror(errno));
		return EXIT_FAILURE;
	}

	std::printf("done\n");
	return EXIT_SUCCESS;
}
