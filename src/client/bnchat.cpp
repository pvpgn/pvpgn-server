/*
 * Copyright (C) 1998,1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
 * Copyright (C) 1999  Oleg Drokin (green@ccssu.ccssu.crimea.ua)
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
#include <cstring>
#include <cstdio>
#include <cctype>
#include <cstdarg>
#include <cstdlib>

#ifdef WIN32
# include <conio.h>
#endif
#ifdef HAVE_TERMIOS_H
# include <termios.h>
#endif
#ifdef HAVE_SIGACTION
# include <signal.h>
#endif

#include "compat/psock.h"
#include "compat/vsnprintf.h"
#include "compat/termios.h"
#include "compat/strcasecmp.h"
#include "common/field_sizes.h"
#include "common/bnet_protocol.h"
#include "common/packet.h"
#include "common/tag.h"
#include "common/file_protocol.h"
#include "common/bn_type.h"
#include "common/util.h"
#include "common/init_protocol.h"
#include "common/bnethash.h"
#include "common/bnethashconv.h"
#include "common/xalloc.h"
#include "common/network.h"
#include "common/hexdump.h"
#include "common/version.h"
#ifdef CLIENTDEBUG
# include "common/eventlog.h"
#endif
#include "ansi_term.h"
#include "client_connect.h"
#include "client.h"
#include "common/setup_after.h"


#ifdef CLIENTDEBUG
# define dprintf printf
#else
# define dprintf if (0) printf
#endif

#define CHANNEL_BNCHATBOT "Chat"
#define CHANNEL_STARCRAFT "Starcraft"
#define CHANNEL_BROODWARS "Brood War"
#define CHANNEL_SHAREWARE "Starcraft Shareware"
#define CHANNEL_DIABLORTL "Diablo Retail"
#define CHANNEL_DIABLOSHR "Diablo Shareware" /* FIXME: is this one right? */
#define CHANNEL_WARCIIBNE "War2BNE"
#define CHANNEL_DIABLO2DV "Diablo II"
#define CHANNEL_DIABLO2XP "Diablo II"
#define CHANNEL_WARCRAFT3 "W3"
#define CHANNEL_WAR3XP    "W3"

using namespace pvpgn;
using namespace pvpgn::client;

namespace
{

	volatile int handle_winch = 0;

	typedef enum {
		mode_chat,
		mode_command,
		mode_waitstat,
		mode_waitgames,
		mode_waitladder,
		mode_gamename,
		mode_gamepass,
		mode_gamewait,
		mode_gamestop,
		mode_claninvite,
		mode_clancreateinvite
	} t_mode;

	typedef struct _client_state
	{
		int 		useansi;
		int 		sd;
		struct sockaddr_in	saddr;
		unsigned int	sessionkey;
		unsigned int	sessionnum;
		unsigned int	currsize;
		unsigned int	commpos;
		struct termios	in_attr_old;
		struct termios	in_attr_new;
		int			changed_in;
		int			fd_stdin;
		unsigned int	screen_width, screen_height;
		int			munged;
		t_mode		mode;
		char		text[MAX_MESSAGE_LEN];
	} t_client_state;

	typedef struct _user_info
	{
		char const *	clienttag;
		char const *	archtag;
		char const *	gamelang;
		char		player[MAX_MESSAGE_LEN];
		char const *	cdowner;
		char const *	cdkey;
		char const *	channel;
		char		curr_gamename[MAX_GAMENAME_LEN];
		char		curr_gamepass[MAX_GAMEPASS_LEN];
		int			count, clantag;
		char const *	inviter;
		int			ignoreversion;

	} t_user_info;


	char const * mflags_get_str(unsigned int flags);
	char const * cflags_get_str(unsigned int flags);
	char const * mode_get_prompt(t_mode mode);
	int print_file(struct sockaddr_in * saddr, char const * filename, char const * progname, char const * clienttag);
	void usage(char const * progname);
#ifdef HAVE_SIGACTION
	void winch_sig_handle(int unused);
#endif


#ifdef HAVE_SIGACTION
	void winch_sig_handle(int unused)
	{
		handle_winch = 1;
	}
#endif


	char const * mflags_get_str(unsigned int flags)
	{
		static char buffer[32];

		buffer[0] = buffer[1] = '\0';
		if (flags&MF_BLIZZARD)
			std::strcat(buffer, ",Blizzard");
		if (flags&MF_GAVEL)
			std::strcat(buffer, ",Gavel");
		if (flags&MF_VOICE)
			std::strcat(buffer, ",Megaphone");
		if (flags&MF_BNET)
			std::strcat(buffer, ",BNET");
		if (flags&MF_PLUG)
			std::strcat(buffer, ",Plug");
		if (flags&MF_X)
			std::strcat(buffer, ",X");
		if (flags&MF_SHADES)
			std::strcat(buffer, ",Shades");
		if (flags&MF_PGLPLAY)
			std::strcat(buffer, ",PGL_Player");
		if (flags&MF_PGLOFFL)
			std::strcat(buffer, ",PGL_Official");
		buffer[0] = '[';
		std::strcat(buffer, "]");

		return buffer;
	}


	char const * cflags_get_str(unsigned int flags)
	{
		static char buffer[32];

		buffer[0] = buffer[1] = '\0';
		if (flags&CF_PUBLIC)
			std::strcat(buffer, ",Public");
		if (flags&CF_MODERATED)
			std::strcat(buffer, ",Moderated");
		if (flags&CF_RESTRICTED)
			std::strcat(buffer, ",Restricted");
		if (flags&CF_THEVOID)
			std::strcat(buffer, ",The Void");
		if (flags&CF_SYSTEM)
			std::strcat(buffer, ",System");
		if (flags&CF_OFFICIAL)
			std::strcat(buffer, ",Official");
		buffer[0] = '[';
		std::strcat(buffer, "]");

		return buffer;
	}


	char const * mode_get_prompt(t_mode mode)
	{
		switch (mode)
		{
		case mode_chat:
			return "] ";
		case mode_command:
			return "command> ";
		case mode_waitstat:
		case mode_waitgames:
		case mode_waitladder:
		case mode_gamewait:
			return "*please wait* ";
		case mode_gamename:
			return "Name: ";
		case mode_gamepass:
			return "Password: ";
		case mode_gamestop:
			return "[Return to kill game] ";
		default:
			return "? ";
		}
	}


	int print_file(struct sockaddr_in * saddr, char const * filename, char const * progname, char const * clienttag)
	{
		int          sd;
		t_packet *   ipacket;
		t_packet *   packet;
		t_packet *   rpacket;
		t_packet *   fpacket;
		unsigned int currsize;
		unsigned int filelen;

		if (!saddr || !filename || !progname)
			return -1;

		if ((sd = psock_socket(PSOCK_PF_INET, PSOCK_SOCK_STREAM, PSOCK_IPPROTO_TCP)) < 0)
		{
			std::fprintf(stderr, "%s: could not create socket (psock_socket: %s)\n", progname, std::strerror(psock_errno()));
			return -1;
		}

		if (psock_connect(sd, (struct sockaddr *)saddr, sizeof(*saddr)) < 0)
		{
			std::fprintf(stderr, "%s: could not connect to server (psock_connect: %s)\n", progname, std::strerror(psock_errno()));
			return -1;
		}

		if (!(ipacket = packet_create(packet_class_init)))
		{
			std::fprintf(stderr, "%s: could not create packet\n", progname);
			return -1;
		}
		bn_byte_set(&ipacket->u.client_initconn.cclass, CLIENT_INITCONN_CLASS_FILE);
		client_blocksend_packet(sd, ipacket);
		packet_del_ref(ipacket);

		if (!(rpacket = packet_create(packet_class_file)))
		{
			std::fprintf(stderr, "%s: could not create packet\n", progname);
			return -1;
		}

		if (!(fpacket = packet_create(packet_class_raw)))
		{
			std::fprintf(stderr, "%s: could not create packet\n", progname);
			return -1;
		}

		if (!(packet = packet_create(packet_class_file)))
		{
			std::fprintf(stderr, "%s: could not create packet\n", progname);
			return -1;
		}
		packet_set_size(packet, sizeof(t_client_file_req));
		packet_set_type(packet, CLIENT_FILE_REQ);
		bn_int_tag_set(&packet->u.client_file_req.archtag, ARCHTAG_WINX86);
		bn_int_tag_set(&packet->u.client_file_req.clienttag, clienttag);
		bn_int_set(&packet->u.client_file_req.adid, 0);
		bn_int_set(&packet->u.client_file_req.extensiontag, 0);
		bn_int_set(&packet->u.client_file_req.startoffset, 0);
		bn_long_set_a_b(&packet->u.client_file_req.timestamp, 0x00000000, 0x00000000);
		packet_append_string(packet, filename);
		client_blocksend_packet(sd, packet);
		packet_del_ref(packet);

		do
		if (client_blockrecv_packet(sd, rpacket) < 0)
		{
			std::fprintf(stderr, "%s: server closed file connection\n", progname);
			packet_del_ref(fpacket);
			packet_del_ref(rpacket);
			return -1;
		}
		while (packet_get_type(rpacket) != SERVER_FILE_REPLY);

		filelen = bn_int_get(rpacket->u.server_file_reply.filelen);
		packet_del_ref(rpacket);

		for (currsize = 0; currsize + MAX_PACKET_SIZE <= filelen; currsize += MAX_PACKET_SIZE)
		{
			if (client_blockrecv_raw_packet(sd, fpacket, MAX_PACKET_SIZE) < 0)
			{
				std::fflush(stdout);
				std::fprintf(stderr, "%s: server closed file connection\n", progname);
				packet_del_ref(fpacket);
				return -1;
			}
			str_print_term(stdout, (const char*)packet_get_raw_data_const(fpacket, 0), MAX_PACKET_SIZE, 1);
		}
		filelen -= currsize;
		if (filelen)
		{
			if (client_blockrecv_raw_packet(sd, fpacket, filelen) < 0)
			{
				std::fflush(stdout);
				std::fprintf(stderr, "%s: server closed file connection\n", progname);
				packet_del_ref(fpacket);
				return -1;
			}
			str_print_term(stdout, (const char*)packet_get_raw_data_const(fpacket, 0), filelen, 1);
		}
		std::fflush(stdout);

		psock_close(sd);

		packet_del_ref(fpacket);

		return 0;
	}


	void usage(char const * progname)
	{
		std::fprintf(stderr, "usage: %s [<options>] [<servername> [<TCP portnumber>]]\n", progname);
		std::fprintf(stderr,
			"    -a, --ansi-color            use ANSI colors\n"
			"    -n, --new-account           create a new account\n"
			"    -c, --change-password       change account password\n"
			"    --client=CHAT               report client as a chat bot\n"
			"    -b, --client=SEXP           report client as Brood Wars\n"
			"    -d, --client=DRTL           report client as Diablo Retail\n"
			"    --client=DSHR               report client as Diablo Shareware\n");
		std::fprintf(stderr,
			"    -s, --client=STAR           report client as Starcraft (default)\n"
			"    --client=SSHR               report client as Starcraft Shareware\n"
			"    -w, --client=W2BN           report client as Warcraft II BNE\n"
			"    --client=D2DV               report client as Diablo II\n"
			"    --client=D2XP               report client as Diablo II: LoD\n"
			"    --client=WAR3               report client as Warcraft III\n"
			"    --client=W3XP               report client as Warcraft III Frozen Throne\n"
			"    --arch=IX86                 report architecture as Intel x86 (default)\n"
			"    --arch=PMAC                 report architecture as PowerPC MacOS\n"
			"    --arch=XMAC                 report architecture as PowerPC MacOSX\n");
		std::fprintf(stderr,
			"    -o NAME, --owner=NAME       report CD owner as NAME\n"
			"    -k KEY, --cdkey=KEY         report CD key as KEY\n"
			"    -l LANG --lang=LANG         report language as LANG (default \"enUS\")\n"
			"    -i, --ignore-version        ignore version request (do not send game version, CD owner/key)\n"
			"    -h, --help, --usage         show this information and exit\n"
			"    -v, --version               print version number and exit\n");
		std::exit(EXIT_FAILURE);
	}

	int read_commandline(int argc, char * * argv,
		char const * * servname, unsigned short * servport,
		char const * * clienttag,
		char const * * archtag,
		int * changepass, int * newacct,
		char const * * channel,
		char const * * cdowner,
		char const * * cdkey,
		char const * * gamelang,
		int * ignoreversion,
		int * useansi)
	{
		int a;

		if (argc < 1 || !argv || !argv[0])
		{
			std::fprintf(stderr, "bad arguments\n");
			return EXIT_FAILURE;
		}

		for (a = 1; a < argc; a++)
		if (*servname && std::isdigit((int)argv[a][0]) && a + 1 >= argc)
		{
			if (str_to_ushort(argv[a], servport) < 0)
			{
				std::fprintf(stderr, "%s: \"%s\" should be a positive integer\n", argv[0], argv[a]);
				usage(argv[0]);
			}
		}
		else if (!(*servname) && argv[a][0] != '-' && a + 2 >= argc)
			*servname = argv[a];
		else if (std::strcmp(argv[a], "-a") == 0 || std::strcmp(argv[a], "--use-ansi") == 0)
			*useansi = 1;
		else if (std::strcmp(argv[a], "-n") == 0 || std::strcmp(argv[a], "--new-account") == 0)
		{
			if (*changepass)
			{
				std::fprintf(stderr, "%s: can not create new account when changing passwords\n", argv[0]);
				usage(argv[0]);
			}
			*newacct = 1;
		}
		else if (std::strcmp(argv[a], "-c") == 0 || std::strcmp(argv[a], "--change-password") == 0)
		{
			if (*newacct)
			{
				std::fprintf(stderr, "%s: can not change passwords when creating a new account\n", argv[0]);
				usage(argv[0]);
			}
			*changepass = 1;
		}
		else if (std::strcmp(argv[a], "--client=CHAT") == 0)
		{
			if (*clienttag)
			{
				std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], *clienttag);
				usage(argv[0]);
			}
			*clienttag = CLIENTTAG_BNCHATBOT;
			*channel = CHANNEL_BNCHATBOT;
		}
		else if (std::strcmp(argv[a], "-b") == 0 || std::strcmp(argv[a], "--client=SEXP") == 0)
		{
			if (*clienttag)
			{
				std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], *clienttag);
				usage(argv[0]);
			}
			*clienttag = CLIENTTAG_BROODWARS;
			*channel = CHANNEL_BROODWARS;
		}
		else if (std::strcmp(argv[a], "-d") == 0 || std::strcmp(argv[a], "--client=DRTL") == 0)
		{
			if (*clienttag)
			{
				std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], *clienttag);
				usage(argv[0]);
			}
			*clienttag = CLIENTTAG_DIABLORTL;
			*channel = CHANNEL_DIABLORTL;
		}
		else if (std::strcmp(argv[a], "--client=DSHR") == 0)
		{
			if (*clienttag)
			{
				std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], *clienttag);
				usage(argv[0]);
			}
			*clienttag = CLIENTTAG_DIABLOSHR;
			*channel = CHANNEL_DIABLOSHR;
		}
		else if (std::strcmp(argv[a], "-s") == 0 || std::strcmp(argv[a], "--client=STAR") == 0)
		{
			if (*clienttag)
			{
				std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], *clienttag);
				usage(argv[0]);
			}
			*clienttag = CLIENTTAG_STARCRAFT;
			*channel = CHANNEL_STARCRAFT;
		}
		else if (std::strcmp(argv[a], "--client=SSHR") == 0)
		{
			if (*clienttag)
			{
				std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], *clienttag);
				usage(argv[0]);
			}
			*clienttag = CLIENTTAG_SHAREWARE;
			*channel = CHANNEL_SHAREWARE;
		}
		else if (std::strcmp(argv[a], "-w") == 0 || std::strcmp(argv[a], "--client=W2BN") == 0)
		{
			if (*clienttag)
			{
				std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], *clienttag);
				usage(argv[0]);
			}
			*clienttag = CLIENTTAG_WARCIIBNE;
			*channel = CHANNEL_WARCIIBNE;
		}
		else if (std::strcmp(argv[a], "--client=D2DV") == 0)
		{
			if (*clienttag)
			{
				std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], *clienttag);
				usage(argv[0]);
			}
			*clienttag = CLIENTTAG_DIABLO2DV;
			*channel = CHANNEL_DIABLO2DV;
		}
		else if (std::strcmp(argv[a], "--client=D2XP") == 0)
		{
			if (*clienttag)
			{
				std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], *clienttag);
				usage(argv[0]);
			}
			*clienttag = CLIENTTAG_DIABLO2XP;
			*channel = CHANNEL_DIABLO2XP;
		}
		else if (std::strcmp(argv[a], "--client=WAR3") == 0)
		{
			if (*clienttag)
			{
				std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], *clienttag);
				usage(argv[0]);
			}
			*clienttag = CLIENTTAG_WARCRAFT3;
			*channel = CHANNEL_WARCRAFT3;
		}
		else if (std::strcmp(argv[a], "--client=W3XP") == 0)
		{
			if (*clienttag)
			{
				std::fprintf(stderr, "%s: client type was already specified as \"%s\"\n", argv[0], *clienttag);
				usage(argv[0]);
			}
			*clienttag = CLIENTTAG_WAR3XP;
			*channel = CHANNEL_WAR3XP;
		}
		else if (std::strncmp(argv[a], "--client=", 9) == 0)
		{
			std::fprintf(stderr, "%s: unknown client tag \"%s\"\n", argv[0], &argv[a][9]);
			usage(argv[0]);
		}
		else if (std::strcmp(argv[a], "--arch=IX86") == 0)
		{
			*archtag = ARCHTAG_WINX86;
		}
		else if (std::strcmp(argv[a], "--arch=PMAC") == 0)
		{
			*archtag = ARCHTAG_MACPPC;
		}
		else if (std::strcmp(argv[a], "--arch=XMAC") == 0)
		{
			*archtag = ARCHTAG_OSXPPC;
		}
		else if (std::strncmp(argv[a], "--arch=", 7) == 0)
		{
			std::fprintf(stderr, "%s: unknown architecture tag \"%s\"\n", argv[0], &argv[a][7]);
			usage(argv[0]);
		}
		else if (std::strcmp(argv[a], "-o") == 0)
		{
			if (a + 1 >= argc)
			{
				std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
				usage(argv[0]);
			}
			if (*cdowner)
			{
				std::fprintf(stderr, "%s: CD owner was already specified as \"%s\"\n", argv[0], *cdowner);
				usage(argv[0]);
			}
			*cdowner = argv[++a];
		}
		else if (std::strncmp(argv[a], "--owner=", 8) == 0)
		{
			if (*cdowner)
			{
				std::fprintf(stderr, "%s: CD owner was already specified as \"%s\"\n", argv[0], *cdowner);
				usage(argv[0]);
			}
			*cdowner = &argv[a][8];
		}
		else if (std::strcmp(argv[a], "-k") == 0)
		{
			if (a + 1 >= argc)
			{
				std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
				usage(argv[0]);
			}
			if (*cdkey)
			{
				std::fprintf(stderr, "%s: CD key was already specified as \"%s\"\n", argv[0], *cdkey);
				usage(argv[0]);
			}
			*cdkey = argv[++a];
		}
		else if (std::strncmp(argv[a], "--cdkey=", 8) == 0)
		{
			if (*cdkey)
			{
				std::fprintf(stderr, "%s: CD key was already specified as \"%s\"\n", argv[0], *cdkey);
				usage(argv[0]);
			}
			*cdkey = &argv[a][8];
		}
		else if (std::strcmp(argv[a], "-l") == 0)
		{
			if (a + 1 >= argc)
			{
				std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
				usage(argv[0]);
			}
			if (std::strlen(argv[a + 1]) != 4)
			{
				std::fprintf(stderr, "%s: language has to be 4 characters long\n", argv[0]);
				usage(argv[0]);
			}
			*gamelang = argv[++a];
		}
		else if (std::strncmp(argv[a], "--lang=", 7) == 0)
		{
			if (std::strlen(argv[a] + 7) != 4)
			{
				std::fprintf(stderr, "%s: language has to be 4 characters long\n", argv[0]);
				usage(argv[0]);
			}
			*gamelang = &argv[a][7];
		}
		else if (std::strcmp(argv[a], "-v") == 0 || std::strcmp(argv[a], "--version") == 0)
		{
			std::printf("version "PVPGN_VERSION"\n");
			return EXIT_SUCCESS;
		}
		else if (std::strcmp(argv[a], "-h") == 0 || std::strcmp(argv[a], "--help") == 0 || std::strcmp(argv[a], "--usage") == 0)
			usage(argv[0]);
		else if (std::strcmp(argv[a], "--client") == 0 || std::strcmp(argv[a], "--owner") == 0 || std::strcmp(argv[a], "--cdkey") == 0)
		{
			std::fprintf(stderr, "%s: option \"%s\" requires an argument\n", argv[0], argv[a]);
			usage(argv[0]);
		}
		else if (std::strcmp(argv[a], "-i") == 0 || std::strcmp(argv[a], "--ignore-version") == 0)
		{
			*ignoreversion = 1;
		}
		else
		{
			std::fprintf(stderr, "%s: unknown option \"%s\"\n", argv[0], argv[a]);
			usage(argv[0]);
		}

		if (*servport == 0)
			*servport = BNETD_SERV_PORT;
		if (!(*cdowner))
			*cdowner = BNETD_DEFAULT_OWNER;
		if (!(*cdkey))
			*cdkey = BNETD_DEFAULT_KEY;
		if (!(*clienttag))
		{
			*clienttag = CLIENTTAG_STARCRAFT;
			*channel = CHANNEL_STARCRAFT;
		}
		if (!(*servname))
			*servname = BNETD_DEFAULT_HOST;

		return 0;
	}

	void munge(t_client_state * client)
	{
		if (!client->munged)
		{
			std::printf("\r");
			for (unsigned i = 0; i < std::strlen(mode_get_prompt(client->mode)); i++)
				std::printf(" ");
			for (unsigned i = 0; i < std::strlen(client->text) && i < client->screen_width - std::strlen(mode_get_prompt(client->mode)); i++)
				std::printf(" ");
			std::printf("\r");
			client->munged = 1;
		}
	}

	void ansi_printf(t_client_state * client, int color, char const * fmt, ...)
	{
		std::va_list		args;
		char		buffer[2048];

		if (!(client))
			return;

		if (!(fmt))
			return;

		if (client->useansi)
			ansi_text_color_fore(color);

		va_start(args, fmt);
		vsnprintf(buffer, 2048, fmt, args);
		va_end(args);

		str_print_term(stdout, buffer, 0, 1);

		if (client->useansi)
			ansi_text_reset();

		std::fflush(stdout);

	}

}

extern int main(int argc, char * argv[])
{
	int                newacct = 0;
	int                changepass = 0;
	t_client_state	client;
	t_user_info		user;
	t_packet *         packet;
	t_packet *         rpacket;
	char const *       servname = NULL;
	unsigned short     servport = 0;
	char const * *     channellist;
	unsigned int       statsmatch = 24; /* any random number that is rare in uninitialized fields */

	std::memset(&user, 0, sizeof(t_user_info));
	std::memset(&client, 0, sizeof(t_client_state));

	/* default values */
	user.archtag = ARCHTAG_WINX86;
	user.gamelang = CLIENT_COUNTRYINFO_109_GAMELANG;
	user.ignoreversion = 0;

	read_commandline(argc, argv, &servname, &servport, &user.clienttag, &user.archtag, &changepass,
		&newacct, &user.channel, &user.cdowner, &user.cdkey, &user.gamelang, &user.ignoreversion, &client.useansi);

	client.fd_stdin = fileno(stdin);
	if (tcgetattr(client.fd_stdin, &client.in_attr_old) >= 0)
	{
		client.in_attr_new = client.in_attr_old;
		client.in_attr_new.c_lflag &= ~(ECHO | ICANON); /* turn off ECHO and ICANON */
		client.in_attr_new.c_cc[VMIN] = 0; /* don't require reads to return data */
		client.in_attr_new.c_cc[VTIME] = 1; /* timeout = .1 seconds */
		tcsetattr(client.fd_stdin, TCSANOW, &client.in_attr_new);
		client.changed_in = 1;
	}
	else
	{
		std::fprintf(stderr, "%s: could not set terminal attributes for stdin\n", argv[0]);
		client.changed_in = 0;
	}

#ifdef HAVE_SIGACTION
	{
		struct sigaction winch_action;

		winch_action.sa_handler = winch_sig_handle;
		sigemptyset(&winch_action.sa_mask);
		winch_action.sa_flags = SA_RESTART;

		sigaction(SIGWINCH, &winch_action, NULL);
	}
#endif


	if (client_get_termsize(client.fd_stdin, &client.screen_width, &client.screen_height) < 0)
	{
		std::fprintf(stderr, "%s: could not determine screen size\n", argv[0]);
		if (client.changed_in)
			tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
		return EXIT_FAILURE;
	}

	if (client.useansi)
	{
		ansi_text_reset();
		ansi_screen_clear();
		ansi_cursor_move_home();
		std::fflush(stdout);
	}

	if ((client.sd = client_connect(argv[0],
		servname, servport, user.cdowner, user.cdkey, user.clienttag, user.ignoreversion,
		&client.saddr, &client.sessionkey, &client.sessionnum, user.archtag, user.gamelang)) < 0)
	{
		std::fprintf(stderr, "%s: fatal error during handshake\n", argv[0]);
		if (client.changed_in)
			tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
		return EXIT_FAILURE;
	}

	/* reuse this same packet over and over */
	if (!(rpacket = packet_create(packet_class_bnet)))
	{
		std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
		psock_close(client.sd);
		if (client.changed_in)
			tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
		return EXIT_FAILURE;
	}

	if (!(packet = packet_create(packet_class_bnet)))
	{
		std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
		psock_close(client.sd);
		if (client.changed_in)
			tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
		return EXIT_FAILURE;
	}
	packet_set_size(packet, sizeof(t_client_fileinforeq));
	packet_set_type(packet, CLIENT_FILEINFOREQ);
	bn_int_set(&packet->u.client_fileinforeq.type, CLIENT_FILEINFOREQ_TYPE_TOS);
	bn_int_set(&packet->u.client_fileinforeq.unknown2, CLIENT_FILEINFOREQ_UNKNOWN2);

	if ((user.clienttag == CLIENTTAG_DIABLO2DV) || (user.clienttag == CLIENTTAG_DIABLO2XP))
		packet_append_string(packet, CLIENT_FILEINFOREQ_FILE_TOSUNICODEUSA);
	else
		packet_append_string(packet, CLIENT_FILEINFOREQ_FILE_TOSUSA);

	client_blocksend_packet(client.sd, packet);
	packet_del_ref(packet);
	do
	if (client_blockrecv_packet(client.sd, rpacket) < 0)
	{
		std::fprintf(stderr, "%s: server closed connection\n", argv[0]);
		psock_close(client.sd);
		if (client.changed_in)
			tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
		return EXIT_FAILURE;
	}
	while (packet_get_type(rpacket) != SERVER_FILEINFOREPLY);

	/* real client would also send statsreq on past logins here */

	std::printf("----\n");
	if (newacct)
	{
		if (client.useansi)
			ansi_text_color_fore(ansi_text_color_red);
		std::printf("###### Terms Of Service ######\n");
		print_file(&client.saddr, packet_get_str_const(rpacket, sizeof(t_server_fileinforeply), 1024), argv[0], user.clienttag);
		std::printf("##############################\n\n");
		if (client.useansi)
			ansi_text_reset();
	}

	for (;;)
	{
		char         password[MAX_MESSAGE_LEN];
		unsigned int i;
		int          status;

		if (newacct)
		{
			char   passwordvrfy[MAX_MESSAGE_LEN];
			t_hash passhash1;

			std::printf("Enter information for your new account\n");
			client.munged = 1;
			client.commpos = 0;
			user.player[0] = '\0';
			while ((status = client_get_comm("Username: ", user.player, MAX_MESSAGE_LEN, &client.commpos,
				1, client.munged, client.screen_width)) == 0)
			if (handle_winch)
			{
				client_get_termsize(client.fd_stdin, &client.screen_width, &client.screen_height);
				std::printf(" \r");
				client.munged = 1;
				handle_winch = 0;
			}
			else
				client.munged = 0;
			std::printf("\n");
			if (status < 0)
				continue;
			if (std::strchr(user.player, ' ') || std::strchr(user.player, '\t') ||
				std::strchr(user.player, '\r') || std::strchr(user.player, '\n'))
			{
				std::printf("Spaces are not allowed in usernames. Try again.\n");
				continue;
			}
			/* we could use std::strcspn() but it doesn't exist everywhere */
			if (std::strchr(user.player, '#') ||
				std::strchr(user.player, '%') ||
				std::strchr(user.player, '&') ||
				std::strchr(user.player, '*') ||
				std::strchr(user.player, '\\') ||
				std::strchr(user.player, '"') ||
				std::strchr(user.player, ',') ||
				std::strchr(user.player, '<') ||
				std::strchr(user.player, '/') ||
				std::strchr(user.player, '>') ||
				std::strchr(user.player, '?'))
			{
				std::printf("The special characters #%%&*\\\",</>? are not allowed in usernames. Try again.\n");
			}
			if (std::strlen(user.player) >= MAX_USERNAME_LEN)
			{
				std::printf("Usernames must not be more than %u characters long. Try again.\n", MAX_USERNAME_LEN - 1);
				continue;
			}

			client.munged = 1;
			client.commpos = 0;
			password[0] = '\0';
			while ((status = client_get_comm("Password: ", password, MAX_MESSAGE_LEN, &client.commpos, 0,
				client.munged, client.screen_width)) == 0)
			if (handle_winch)
			{
				client_get_termsize(client.fd_stdin, &client.screen_width, &client.screen_height);
				std::printf(" \r");
				client.munged = 1;
				handle_winch = 0;
			}
			else
				client.munged = 0;
			std::printf("\n");
			if (status<0)
				continue;
			if (std::strlen(password)>MAX_USERPASS_LEN)
			{
				std::printf("password must not be more than %u characters long. Try again.\n", MAX_USERPASS_LEN);
				continue;
			}
			for (i = 0; i < std::strlen(password); i++)
			if (std::isupper((int)password[i]))
				password[i] = std::tolower((int)password[i]);

			client.munged = 1;
			client.commpos = 0;
			passwordvrfy[0] = '\0';
			while ((status = client_get_comm("Retype password: ", passwordvrfy, MAX_MESSAGE_LEN, &client.commpos, 0,
				client.munged, client.screen_width)) == 0)
			if (handle_winch)
			{
				client_get_termsize(client.fd_stdin, &client.screen_width, &client.screen_height);
				std::printf(" \r");
				client.munged = 1;
				handle_winch = 0;
			}
			else
				client.munged = 0;
			std::printf("\n");
			if (status < 0)
				continue;
			for (i = 0; i < std::strlen(passwordvrfy); i++)
				passwordvrfy[i] = std::tolower((int)passwordvrfy[i]);

			if (std::strcmp(password, passwordvrfy) != 0)
			{
				std::printf("Passwords do not match. Try again.\n");
				continue;
			}

			bnet_hash(&passhash1, std::strlen(password), password); /* do the single hash */

			if (!(packet = packet_create(packet_class_bnet)))
			{
				std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
				psock_close(client.sd);
				if (client.changed_in)
					tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
				return EXIT_FAILURE;
			}
			packet_set_size(packet, sizeof(t_client_createacctreq1));
			packet_set_type(packet, CLIENT_CREATEACCTREQ1);
			hash_to_bnhash((t_hash const *)&passhash1, packet->u.client_createacctreq1.password_hash1); /* avoid warning */
			packet_append_string(packet, user.player);
			client_blocksend_packet(client.sd, packet);
			packet_del_ref(packet);

			do
			if (client_blockrecv_packet(client.sd, rpacket) < 0)
			{
				std::fprintf(stderr, "%s: server closed connection\n", argv[0]);
				psock_close(client.sd);
				if (client.changed_in)
					tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
				return EXIT_FAILURE;
			}
			while (packet_get_type(rpacket) != SERVER_CREATEACCTREPLY1);
			dprintf("Got CREATEACCTREPLY1\n");
			if (bn_int_get(rpacket->u.server_createacctreply1.result) == SERVER_CREATEACCTREPLY1_RESULT_NO)
			{
				std::printf("Could not create an account under that name. Try another one.\n");
				continue;
			}
			std::printf("Account created.\n");
		}
		else if (changepass)
		{
			char         passwordprev[MAX_MESSAGE_LEN];
			char         passwordvrfy[MAX_MESSAGE_LEN];
			struct
			{
				bn_int ticks;
				bn_int sessionkey;
				bn_int passhash1[5];
			}            temp;
			t_hash       oldpasshash1;
			t_hash       oldpasshash2;
			t_hash       newpasshash1;
			unsigned int ticks;

			std::printf("Enter your old and new login information\n");

			client.munged = 1;
			client.commpos = 0;
			user.player[0] = '\0';
			while ((status = client_get_comm("Username: ", user.player, MAX_MESSAGE_LEN, &client.commpos, 1,
				client.munged, client.screen_width)) == 0)
			if (handle_winch)
			{
				client_get_termsize(client.fd_stdin, &client.screen_width, &client.screen_height);
				std::printf(" \r");
				client.munged = 1;
				handle_winch = 0;
			}
			else
				client.munged = 0;
			std::printf("\n");
			if (status < 0)
				continue;
			if (std::strchr(user.player, ' '))
			{
				std::printf("Spaces not allowed in username. Try again.\n");
				continue;
			}
			if (std::strlen(user.player) >= MAX_USERNAME_LEN)
			{
				std::printf("Usernames must not be more than %u characters long. Try again.\n", MAX_USERNAME_LEN - 1);
				continue;
			}

			client.munged = 1;
			client.commpos = 0;
			passwordprev[0] = '\0';
			while ((status = client_get_comm("Old password: ", passwordprev, MAX_MESSAGE_LEN, &client.commpos, 0,
				client.munged, client.screen_width)) == 0)
			if (handle_winch)
			{
				client_get_termsize(client.fd_stdin, &client.screen_width, &client.screen_height);
				std::printf(" \r");
				client.munged = 1;
				handle_winch = 0;
			}
			else
				client.munged = 0;
			std::printf("\n");
			if (status < 0)
				continue;
			for (i = 0; i < std::strlen(passwordprev); i++)
				passwordprev[i] = std::tolower((int)passwordprev[i]);

			client.munged = 1;
			client.commpos = 0;
			password[0] = '\0';
			while ((status = client_get_comm("New password: ", password, MAX_MESSAGE_LEN, &client.commpos, 0,
				client.munged, client.screen_width)) == 0)
			if (handle_winch)
			{
				client_get_termsize(client.fd_stdin, &client.screen_width, &client.screen_height);
				std::printf(" \r");
				client.munged = 1;
				handle_winch = 0;
			}
			else
				client.munged = 0;
			std::printf("\n");
			if (status < 0)
				continue;
			for (i = 0; i < std::strlen(password); i++)
				password[i] = std::tolower((int)password[i]);

			client.munged = 1;
			client.commpos = 0;
			passwordvrfy[0] = '\0';
			while ((status = client_get_comm("Retype new password: ", passwordvrfy, MAX_MESSAGE_LEN, &client.commpos, 0,
				client.munged, client.screen_width)) == 0)
			if (handle_winch)
			{
				client_get_termsize(client.fd_stdin, &client.screen_width, &client.screen_height);
				std::printf(" \r");
				client.munged = 1;
				handle_winch = 0;
			}
			else
				client.munged = 0;
			std::printf("\n");
			if (status < 0)
				continue;
			for (i = 0; i < std::strlen(passwordvrfy); i++)
				passwordvrfy[i] = std::tolower((int)passwordvrfy[i]);

			if (std::strcmp(password, passwordvrfy) != 0)
			{
				std::printf("New passwords do not match. Try again.\n");
				continue;
			}

			ticks = 0; /* FIXME: what to use here? */
			bn_int_set(&temp.ticks, ticks);
			bn_int_set(&temp.sessionkey, client.sessionkey);
			bnet_hash(&oldpasshash1, std::strlen(passwordprev), passwordprev); /* do the single hash for old */
			hash_to_bnhash((t_hash const *)&oldpasshash1, temp.passhash1); /* avoid warning */
			bnet_hash(&oldpasshash2, sizeof(temp), &temp); /* do the double hash for old */
			bnet_hash(&newpasshash1, std::strlen(password), password); /* do the single hash for new */

			if (!(packet = packet_create(packet_class_bnet)))
			{
				std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
				psock_close(client.sd);
				if (client.changed_in)
					tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
				return EXIT_FAILURE;
			}
			packet_set_size(packet, sizeof(t_client_changepassreq));
			packet_set_type(packet, CLIENT_CHANGEPASSREQ);
			bn_int_set(&packet->u.client_changepassreq.ticks, ticks);
			bn_int_set(&packet->u.client_changepassreq.sessionkey, client.sessionkey);
			hash_to_bnhash((t_hash const *)&oldpasshash2, packet->u.client_changepassreq.oldpassword_hash2); /* avoid warning */
			hash_to_bnhash((t_hash const *)&newpasshash1, packet->u.client_changepassreq.newpassword_hash1); /* avoid warning */
			packet_append_string(packet, user.player);
			client_blocksend_packet(client.sd, packet);
			packet_del_ref(packet);

			do
			if (client_blockrecv_packet(client.sd, rpacket) < 0)
			{
				std::fprintf(stderr, "%s: server closed connection\n", argv[0]);
				psock_close(client.sd);
				if (client.changed_in)
					tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
				return EXIT_FAILURE;
			}
			while (packet_get_type(rpacket) != SERVER_CHANGEPASSACK);
			dprintf("Got CHANGEPASSACK\n");
			if (bn_int_get(rpacket->u.server_changepassack.message) == SERVER_CHANGEPASSACK_MESSAGE_FAIL)
			{
				std::printf("Could not change password. Try again.\n");
				continue;
			}
			std::printf("Password changed.\n");
		}
		else
		{
			std::printf("Enter your login information\n");

			client.munged = 1;
			client.commpos = 0;
			user.player[0] = '\0';
			while ((status = client_get_comm("Username: ", user.player, MAX_MESSAGE_LEN, &client.commpos, 1,
				client.munged, client.screen_width)) == 0)
			if (handle_winch)
			{
				client_get_termsize(client.fd_stdin, &client.screen_width, &client.screen_height);
				std::printf(" \r");
				client.munged = 1;
				handle_winch = 0;
			}
			else
				client.munged = 0;
			std::printf("\n");
			if (status < 0)
				continue;
			if (std::strchr(user.player, ' '))
			{
				std::printf("Spaces not allowed in username. Try again.\n");
				continue;
			}
			if (std::strlen(user.player) >= MAX_USERNAME_LEN)
			{
				std::printf("Usernames must not be more than %u characters long. Try again.\n", MAX_USERNAME_LEN - 1);
				continue;
			}

			client.munged = 1;
			client.commpos = 0;
			password[0] = '\0';
			while ((status = client_get_comm("Password: ", password, MAX_MESSAGE_LEN, &client.commpos, 0,
				client.munged, client.screen_width)) == 0)
			if (handle_winch)
			{
				client_get_termsize(client.fd_stdin, &client.screen_width, &client.screen_height);
				std::printf(" \r");
				client.munged = 1;
				handle_winch = 0;
			}
			else
				client.munged = 0;
			std::printf("\n");
			if (status < 0)
				continue;
			for (i = 0; i < std::strlen(password); i++)
				password[i] = std::tolower((int)password[i]);
		}

		/* now login */
		{
			struct
			{
				bn_int ticks;
				bn_int sessionkey;
				bn_int passhash1[5];
			}            temp;
			t_hash       passhash1;
			t_hash       passhash2;
			unsigned int ticks;

			ticks = 0; /* FIXME: what to use here? */
			bn_int_set(&temp.ticks, ticks);
			bn_int_set(&temp.sessionkey, client.sessionkey);
			bnet_hash(&passhash1, std::strlen(password), password); /* do the single hash */
			hash_to_bnhash((t_hash const *)&passhash1, temp.passhash1); /* avoid warning */
			bnet_hash(&passhash2, sizeof(temp), &temp); /* do the double hash */

			if (!(packet = packet_create(packet_class_bnet)))
			{
				std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
				psock_close(client.sd);
				if (client.changed_in)
					tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
				return EXIT_FAILURE;
			}
			packet_set_size(packet, sizeof(t_client_loginreq1));
			packet_set_type(packet, CLIENT_LOGINREQ1);
			bn_int_set(&packet->u.client_loginreq1.ticks, ticks);
			bn_int_set(&packet->u.client_loginreq1.sessionkey, client.sessionkey);
			hash_to_bnhash((t_hash const *)&passhash2, packet->u.client_loginreq1.password_hash2); /* avoid warning */
			packet_append_string(packet, user.player);
			client_blocksend_packet(client.sd, packet);
			packet_del_ref(packet);
		}

		do
		if (client_blockrecv_packet(client.sd, rpacket) < 0)
		{
			std::fprintf(stderr, "%s: server closed connection\n", argv[0]);
			psock_close(client.sd);
			if (client.changed_in)
				tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
			return EXIT_FAILURE;
		}
		while (packet_get_type(rpacket) != SERVER_LOGINREPLY1);
		if (bn_int_get(rpacket->u.server_loginreply1.message) == SERVER_LOGINREPLY1_MESSAGE_SUCCESS)
			break;
		std::fprintf(stderr, "Login incorrect.\n");
	}

	std::fprintf(stderr, "Logged in.\n");
	std::printf("----\n");

	if (newacct && (std::strcmp(user.clienttag, CLIENTTAG_DIABLORTL) == 0 ||
		std::strcmp(user.clienttag, CLIENTTAG_DIABLOSHR) == 0))
	{
		if (!(packet = packet_create(packet_class_bnet)))
		{
			std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
			if (client.changed_in)
				tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
		}
		else
		{
			packet_set_size(packet, sizeof(t_client_playerinforeq));
			packet_set_type(packet, CLIENT_PLAYERINFOREQ);
			packet_append_string(packet, user.player);
			packet_append_string(packet, "LTRD 1 2 0 20 25 15 20 100 0"); /* FIXME: don't hardcode */
			client_blocksend_packet(client.sd, packet);
			packet_del_ref(packet);
		}
	}

	if (!(packet = packet_create(packet_class_bnet)))
	{
		std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
		psock_close(client.sd);
		if (client.changed_in)
			tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
		return EXIT_FAILURE;
	}
	packet_set_size(packet, sizeof(t_client_progident2));
	packet_set_type(packet, CLIENT_PROGIDENT2);
	bn_int_tag_set(&packet->u.client_progident2.clienttag, user.clienttag);
	client_blocksend_packet(client.sd, packet);
	packet_del_ref(packet);

	do
	if (client_blockrecv_packet(client.sd, rpacket) < 0)
	{
		std::fprintf(stderr, "%s: server closed connection\n", argv[0]);
		psock_close(client.sd);
		if (client.changed_in)
			tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
		return EXIT_FAILURE;
	}
	while (packet_get_type(rpacket) != SERVER_CHANNELLIST);

	{
		unsigned int i;
		unsigned int chann_off;
		char const * chann;

		channellist = (const char**)xmalloc(sizeof(char*)* 1);
		for (i = 0, chann_off = sizeof(t_server_channellist);
			(chann = packet_get_str_const(rpacket, chann_off, 128));
			i++, chann_off += std::strlen(chann) + 1)
		{
			if (chann[0] == '\0') break;  /* channel list ends with a "" */

			channellist = (const char**)xrealloc(channellist, sizeof(char*)*(i + 2));
			channellist[i] = xstrdup(chann);
		}
		channellist[i] = NULL;
	}

	if (!(packet = packet_create(packet_class_bnet)))
	{
		std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
		psock_close(client.sd);
		if (client.changed_in)
			tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
		return EXIT_FAILURE;
	}
	packet_set_size(packet, sizeof(t_client_joinchannel));
	packet_set_type(packet, CLIENT_JOINCHANNEL);
	bn_int_set(&packet->u.client_joinchannel.channelflag, CLIENT_JOINCHANNEL_GENERIC);
	packet_append_string(packet, user.channel);
	client_blocksend_packet(client.sd, packet);
	packet_del_ref(packet);

	if (psock_ctl(client.sd, PSOCK_NONBLOCK) < 0)
		std::fprintf(stderr, "%s: could not set TCP socket to non-blocking mode (psock_ctl: %s)\n", argv[0], std::strerror(psock_errno()));

	client.mode = mode_chat;

	{
		unsigned int   i;
		int            highest_fd;
		t_psock_fd_set rfds;
#ifdef WIN32
		static struct timeval tv;

		tv.tv_sec = 50 / 1000;
		tv.tv_usec = 50 % 1000;
#endif

		PSOCK_FD_ZERO(&rfds);

#ifndef WIN32
		highest_fd = client.fd_stdin;
		if (client.sd > highest_fd)
#endif
			highest_fd = client.sd;

		client.currsize = 0;

		client.munged = 1; /* == need to draw prompt */
		client.commpos = 0;
		client.text[0] = '\0';

		for (;;)
		{


			if (handle_winch)
			{
				client_get_termsize(client.fd_stdin, &client.screen_width, &client.screen_height);
				handle_winch = 0;
				std::printf(" \r");
				client.munged = 1;
			}

			if (client.munged)
			{
				std::printf("%s%s", mode_get_prompt(client.mode), client.text + ((client.screen_width <= std::strlen(mode_get_prompt(client.mode)) + client.commpos) ? std::strlen(mode_get_prompt(client.mode)) + client.commpos + 1 - client.screen_width : 0));
				std::fflush(stdout);
				client.munged = 0;
			}
			PSOCK_FD_ZERO(&rfds);
#ifndef WIN32
			PSOCK_FD_SET(client.fd_stdin, &rfds);
#endif
			PSOCK_FD_SET(client.sd, &rfds);
			errno = 0;

#ifndef WIN32
			if (psock_select(highest_fd + 1, &rfds, NULL, NULL, NULL) < 0)
#else
			if (psock_select(highest_fd + 1, &rfds, NULL, NULL, &tv) < 0)
#endif
			{
				if (psock_errno() != PSOCK_EINTR)
				{
					munge(&client);
					std::printf("Select failed (select: %s)\n", std::strerror(psock_errno()));
				}
				continue;
			}
#ifndef WIN32
			if (PSOCK_FD_ISSET(client.fd_stdin, &rfds)) /* got keyboard data */
#else
			if (kbhit())
#endif
			{
				client.munged = 0;

				switch (client_get_comm(mode_get_prompt(client.mode), client.text, MAX_MESSAGE_LEN, &client.commpos, 1,
					0, client.screen_width))
				{
				case -1: /* cancel */
					munge(&client);
					if (client.mode == mode_command)
						client.mode = mode_chat;
					else
						client.mode = mode_command;
					client.commpos = 0;
					client.text[0] = '\0';
					break;

				case 0: /* timeout */
					break;

				case 1:
					switch (client.mode)
					{
					case mode_claninvite:

						std::printf("\n");
						client.munged = 1;

						if ((client.text[0] != '\0') && strcasecmp(client.text, "yes") && strcasecmp(client.text, "no"))
						{
							std::printf("Do you want to accept invitation (yes/no) ? [yes] ");
							break;
						}

						if (!(packet = packet_create(packet_class_bnet)))
						{
							std::printf("Packet creation failed.\n");
						}
						else
						{
							char result;

							packet_set_size(packet, sizeof(t_client_clan_invitereply));
							packet_set_type(packet, CLIENT_CLAN_INVITEREPLY);
							bn_int_set(&packet->u.client_clan_invitereply.count, user.count);
							bn_int_set(&packet->u.client_clan_invitereply.clantag, user.clantag);
							packet_append_string(packet, user.inviter);

							if (!strcasecmp(client.text, "no"))
								result = CLAN_RESPONSE_DECLINED;
							else
								result = CLAN_RESPONSE_ACCEPT;

							packet_append_data(packet, &result, 1);

							client_blocksend_packet(client.sd, packet);
							packet_del_ref(packet);

							if (user.inviter)
								xfree((void *)user.inviter);
							user.inviter = NULL;

						}

					case mode_clancreateinvite:

						std::printf("\n");
						client.munged = 1;

						if ((client.text[0] != '\0') && strcasecmp(client.text, "yes") && strcasecmp(client.text, "no"))
						{
							std::printf("Do you want to accept invitation (yes/no) ? [yes] ");
							break;
						}

						if (!(packet = packet_create(packet_class_bnet)))
						{
							std::printf("Packet creation failed.\n");
						}
						else
						{
							char result;

							packet_set_size(packet, sizeof(t_client_clan_createinvitereply));
							packet_set_type(packet, CLIENT_CLAN_CREATEINVITEREPLY);
							bn_int_set(&packet->u.client_clan_createinvitereply.count, user.count);
							bn_int_set(&packet->u.client_clan_createinvitereply.clantag, user.clantag);
							packet_append_string(packet, user.inviter);

							if (!strcasecmp(client.text, "no"))
								result = CLAN_RESPONSE_DECLINED;
							else
								result = CLAN_RESPONSE_ACCEPT;

							packet_append_data(packet, &result, 1);

							client_blocksend_packet(client.sd, packet);
							packet_del_ref(packet);

							if (user.inviter)
								xfree((void *)user.inviter);
							user.inviter = NULL;

						}


						client.mode = mode_chat;
						break;

					case mode_gamename:
						if (client.text[0] == '\0')
						{
							munge(&client);
							std::printf("Games must have a name.\n");
							break;
						}
						std::printf("\n");
						client.munged = 1;
						std::strncpy(user.curr_gamename, client.text, sizeof(user.curr_gamename));
						user.curr_gamename[sizeof(user.curr_gamename) - 1] = '\0';
						client.mode = mode_gamepass;
						break;
					case mode_gamepass:
						std::printf("\n");
						client.munged = 1;
						std::strncpy(user.curr_gamepass, client.text, sizeof(user.curr_gamepass));
						user.curr_gamepass[sizeof(user.curr_gamepass) - 1] = '\0';

						if (!(packet = packet_create(packet_class_bnet)))
						{
							std::printf("Packet creation failed.\n");
							client.mode = mode_command;
						}
						else
						{
							packet_set_size(packet, sizeof(t_client_startgame4));
							packet_set_type(packet, CLIENT_STARTGAME4);
							bn_short_set(&packet->u.client_startgame4.status, CLIENT_STARTGAME4_STATUS_INIT);
							bn_short_set(&packet->u.client_startgame4.flag, 0x0000);
							bn_int_set(&packet->u.client_startgame4.unknown2, CLIENT_STARTGAME4_UNKNOWN2);
							bn_short_set(&packet->u.client_startgame4.gametype, CLIENT_GAMELISTREQ_MELEE);
							bn_short_set(&packet->u.client_startgame4.option, CLIENT_STARTGAME4_OPTION_MELEE_NORMAL);
							bn_int_set(&packet->u.client_startgame4.unknown4, CLIENT_STARTGAME4_UNKNOWN4);
							bn_int_set(&packet->u.client_startgame4.unknown5, CLIENT_STARTGAME4_UNKNOWN5);
							packet_append_string(packet, user.curr_gamename);
							packet_append_string(packet, user.curr_gamepass);
							packet_append_string(packet, ",,,,1,3,1,3e37a84c,7,Player\rAshrigo\r");
							client_blocksend_packet(client.sd, packet);
							packet_del_ref(packet);
							client.mode = mode_gamewait;
						}
						break;
					case mode_gamestop:
						std::printf("\n");
						client.munged = 1;

						if (!(packet = packet_create(packet_class_bnet)))
							std::printf("Packet creation failed.\n");
						else
						{
							packet_set_size(packet, sizeof(t_client_closegame));
							packet_set_type(packet, CLIENT_CLOSEGAME);
							client_blocksend_packet(client.sd, packet);
							packet_del_ref(packet);
							std::printf("Game closed.\n");
							client.mode = mode_command;
						}
						break;
					case mode_command:
						if (client.text[0] == '\0')
							break;
						std::printf("\n");
						client.munged = 1;
						if (strstart(client.text, "channel") == 0)
						{
							std::printf("Available channels:\n");
							if (client.useansi)
								ansi_text_color_fore(ansi_text_color_yellow);
							for (i = 0; channellist[i]; i++)
								std::printf(" %s\n", channellist[i]);
							if (client.useansi)
								ansi_text_reset();
						}
						else if (strstart(client.text, "create") == 0)
						{
							std::printf("Enter new game information\n");
							client.mode = mode_gamename;
						}
						else if (strstart(client.text, "join") == 0)
						{
							std::printf("Not implemented yet.\n");
						}
						else if (strstart(client.text, "ladder") == 0)
						{
							std::printf("Not implemented yet.\n");
						}
						else if (strstart(client.text, "stats") == 0)
						{
							std::printf("Not implemented yet.\n");
						}
						else if (strstart(client.text, "help") == 0 || std::strcmp(client.text, "?") == 0)
						{
							std::printf("Available commands:\n"
								" channel        - join or create a channel\n"
								" create         - create a new game\n"
								" join           - list current games\n"
								" ladder         - list ladder rankings\n"
								" help           - show this text\n"
								" info <PLAYER>  - print a player's profile\n"
								" chinfo         - modify your profile\n"
								" quit           - exit bnchat\n"
								" stats <PLAYER> - print a player's game record\n"
								" invite <PLAYER> - invite a player to your clan\n"
								"Use the escape key to toggle between chat and command modes.\n");
						}
						else if (strstart(client.text, "invite") == 0)
						{
							for (i = 6; client.text[i] == ' ' || client.text[i] == '\t'; i++);
							if (client.text[i] == '\0')
							{
								ansi_printf(&client, ansi_text_color_red, "You must specify the player.\n");
							}
							else
							{
								if (!(packet = packet_create(packet_class_bnet)))
									std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
								else
								{
									std::printf("Inviting:  %s\n", &client.text[i]);
									packet_set_size(packet, sizeof(t_client_clan_invitereq));
									packet_set_type(packet, CLIENT_CLAN_INVITEREQ);
									bn_int_set(&packet->u.client_clan_invitereq.count, 1);
									packet_append_string(packet, &client.text[i]);
									client_blocksend_packet(client.sd, packet);
									packet_del_ref(packet);
								}
							}
						}
						else if (strstart(client.text, "info") == 0)
						{
							for (i = 4; client.text[i] == ' ' || client.text[i] == '\t'; i++);
							if (client.text[i] == '\0')
							{
								ansi_printf(&client, ansi_text_color_red, "You must specify the player.\n");
							}
							else
							{
								if (!(packet = packet_create(packet_class_bnet)))
									std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
								else
								{
									std::printf("Profile info for %s:\n", &client.text[i]);
									packet_set_size(packet, sizeof(t_client_statsreq));
									packet_set_type(packet, CLIENT_STATSREQ);
									bn_int_set(&packet->u.client_statsreq.name_count, 1);
									bn_int_set(&packet->u.client_statsreq.key_count, 4);
									statsmatch = (unsigned int)std::time(NULL);
									bn_int_set(&packet->u.client_statsreq.requestid, statsmatch);
									packet_append_string(packet, &client.text[i]);
#if 0
									packet_append_string(packet, "BNET\\acct\\username");
									packet_append_string(packet, "BNET\\acct\\userid");
#endif
									packet_append_string(packet, "profile\\sex");
									packet_append_string(packet, "profile\\age");
									packet_append_string(packet, "profile\\location");
									packet_append_string(packet, "profile\\description");
									client_blocksend_packet(client.sd, packet);
									packet_del_ref(packet);

									client.mode = mode_waitstat;
								}
							}
						}
						else if (strstart(client.text, "chinfo") == 0)
						{
							std::printf("Not implemented yet.\n");
						}
						else if (strstart(client.text, "quit") == 0)
						{
							psock_close(client.sd);
							if (client.changed_in)
								tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
							return EXIT_SUCCESS;
						}
						else
						{
							ansi_printf(&client, ansi_text_color_red, "Unknown local command \"%s\".\n", client.text);
						}
						break;

					case mode_chat:
						if (client.text[0] == '\0')
							break;
						if (client.text[0] == '/')
						{
							munge(&client);
						}
						else
						{
							ansi_printf(&client, ansi_text_color_blue, "\r<%s>", user.player);
							std::printf(" ");
							str_print_term(stdout, client.text, 0, 0);
							std::printf("\n");
							client.munged = 1;
						}

						if (!(packet = packet_create(packet_class_bnet)))
						{
							std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
							psock_close(client.sd);
							if (client.changed_in)
								tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
							return EXIT_FAILURE;
						}
						packet_set_size(packet, sizeof(t_client_message));
						packet_set_type(packet, CLIENT_MESSAGE);
						packet_append_string(packet, client.text);
						client_blocksend_packet(client.sd, packet);
						packet_del_ref(packet);
						break;

					default:
						/* one of the wait states; erase what they typed */
						munge(&client);
						break;
					}

					client.commpos = 0;
					client.text[0] = '\0';
					break;
				}
			}

			if (PSOCK_FD_ISSET(client.sd, &rfds)) /* got network data */
			{
				/* rpacket is from server, packet is from client */
				switch (net_recv_packet(client.sd, rpacket, &client.currsize))
				{
				case 0: /* nothing */
					break;

				case 1:
					switch (packet_get_type(rpacket))
					{
					case SERVER_ECHOREQ: /* might as well support it */
						if (packet_get_size(rpacket) < sizeof(t_server_echoreq))
						{
							munge(&client);
							std::printf("Got bad SERVER_ECHOREQ packet (expected %lu bytes, got %u)\n", sizeof(t_server_echoreq), packet_get_size(rpacket));
							break;
						}

						if (!(packet = packet_create(packet_class_bnet)))
						{
							munge(&client);
							std::fprintf(stderr, "%s: could not create packet\n", argv[0]);
							psock_close(client.sd);
							if (client.changed_in)
								tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
							return EXIT_FAILURE;
						}
						packet_set_size(packet, sizeof(t_client_echoreply));
						packet_set_type(packet, CLIENT_ECHOREPLY);
						bn_int_set(&packet->u.client_echoreply.ticks, bn_int_get(rpacket->u.server_echoreq.ticks));
						client_blocksend_packet(client.sd, packet);
						packet_del_ref(packet);

						break;

					case SERVER_STARTGAME4_ACK:
						if (client.mode == mode_gamewait)
						{
							munge(&client);

							if (bn_int_get(rpacket->u.server_startgame4_ack.reply) == SERVER_STARTGAME4_ACK_OK)
							{
								std::printf("Game created.\n");
								client.mode = mode_gamestop;
							}
							else
							{
								std::printf("Game could not be created, try another name.\n");
								client.mode = mode_gamename;
							}
						}
						break;

					case SERVER_CLAN_INVITEREQ:
						if (packet_get_size(rpacket) < sizeof(t_server_clan_invitereq))
						{
							munge(&client);
							std::printf("Got bad SERVER_CLAN_INVITEREQ packet (expected %lu bytes, got %u)\n", sizeof(t_server_clan_invitereq), packet_get_size(rpacket));
							break;
						}

						{
							char const * clan;
							char const * inviter;
							int offset;

							offset = sizeof(t_server_clan_invitereq);
							if (!(clan = packet_get_str_const(rpacket, offset, CLAN_NAME_MAX)))
							{
								munge(&client);
								std::printf("Got SERVER_CLAN_INVITEREQ with bad or missing clanname\n");
								break;
							}
							offset += std::strlen(clan) + 1;
							if (!(inviter = packet_get_str_const(rpacket, offset, MAX_USERNAME_LEN)))
							{
								munge(&client);
								std::printf("Got SERVER_CLAN_INVITEREQ with bad or missing inviter\n");
								break;
							}
							user.count = bn_int_get(rpacket->u.server_clan_invitereq.count);
							user.clantag = bn_int_get(rpacket->u.server_clan_invitereq.clantag);
							if (user.inviter)
								xfree((void *)user.inviter);
							user.inviter = xstrdup(inviter);

							munge(&client);
							std::printf("%s invited you to Clan %s\n", inviter, clan);
							std::printf("Do you want to accept invitation (yes/no) ? [yes] ");

							client.mode = mode_claninvite;
							break;
						}

					case SERVER_CLAN_INVITEREPLY:
						if (packet_get_size(rpacket) < sizeof(t_server_clan_invitereply))
						{
							munge(&client);
							std::printf("Got bad SERVER_CLAN_INVITEREPLY packet (expected %lu bytes, got %u)\n", sizeof(t_server_clan_invitereply), packet_get_size(rpacket));
							break;
						}

						{
							char result = bn_byte_get(rpacket->u.server_clan_invitereply.result);
							std::printf("Recieved result: %i\n", result);
							break;
						}

					case SERVER_CLAN_CREATEINVITEREQ:
						if (packet_get_size(rpacket) < sizeof(t_server_clan_createinvitereq))
						{
							munge(&client);
							std::printf("Got bad SERVER_CLAN_CREATEINVITEREQ packet (expected %lu bytes, got %u)\n", sizeof(t_server_clan_createinvitereq), packet_get_size(rpacket));
							break;
						}

						{
							char const * clan;
							char const * inviter;
							char invited_count;
							int offset;

							offset = sizeof(t_server_clan_createinvitereq);
							if (!(clan = packet_get_str_const(rpacket, offset, CLAN_NAME_MAX)))
							{
								munge(&client);
								std::printf("Got SERVER_CLAN_CREATEINVITEREQ with bad or missing clanname\n");
								break;
							}
							offset += std::strlen(clan) + 1;
							if (!(inviter = packet_get_str_const(rpacket, offset, MAX_USERNAME_LEN)))
							{
								munge(&client);
								std::printf("Got SERVER_CLAN_CREATEINVITEREQ with bad or missing inviter\n");
								break;
							}
							offset += std::strlen(inviter) + 1;
							if (packet_get_size(rpacket) < offset + 4)
							{
								munge(&client);
								std::printf("Got SERVER_CLAN_CREATEINVITEREQ with missing invited count\n");
								break;
							}
							invited_count = *((char *)packet_get_data_const(rpacket, offset, 1));
							offset++;

							user.count = bn_int_get(rpacket->u.server_clan_invitereq.count);
							user.clantag = bn_int_get(rpacket->u.server_clan_invitereq.clantag);
							if (user.inviter)
								xfree((void *)user.inviter);
							user.inviter = xstrdup(inviter);

							munge(&client);
							std::printf("%s invited you to Clan %s along with %i other players\n", inviter, clan, invited_count);
							std::printf("Do you want to accept invitation (yes/no) ? [yes] ");

							client.mode = mode_clancreateinvite;
							break;
						}


					case SERVER_CLANMEMBERUPDATE:
						if (packet_get_size(rpacket) < sizeof(t_server_clanmemberupdate))
						{
							munge(&client);
							std::printf("Got bad SERVER_CLANMEMBERUPDATE packet (expected %lu bytes, got %u)\n", sizeof(t_server_clanmemberupdate), packet_get_size(rpacket));
							break;
						}

						if (client.mode == mode_claninvite)
							break;

						{
							char const * member;
							char rank, online;
							char const * rank_p;
							char const * online_p;
							char const * append_str;
							int offset;
							char const * rank_str;
							char const * online_str;

							offset = sizeof(t_server_clanmemberupdate);
							if (!(member = packet_get_str_const(rpacket, offset, MAX_USERNAME_LEN)))
							{
								munge(&client);
								std::printf("Got SERVER_CLANMEMBERUPDATE with bad or missing member\n");
								break;
							}
							offset += std::strlen(member) + 1;
							if (!(rank_p = (char *)packet_get_data_const(rpacket, offset, 1)))
							{
								munge(&client);
								std::printf("Got SERVER_CLANMEMBERUPDATE with bad or missing rank\n");
								break;
							}
							rank = *rank_p;
							offset += 1;
							if (!(online_p = (char *)packet_get_data_const(rpacket, offset, 1)))
							{
								munge(&client);
								std::printf("Got SERVER_CLAN_MEMBERUPDATE with bad or missing online status\n");
								break;
							}
							online = *online_p;
							offset += 1;
							if (!(append_str = packet_get_str_const(rpacket, offset, MAX_USERNAME_LEN)))
							{
								munge(&client);
								std::printf("Got SERVER_CLANMEMBERUPDATE with bad or missing append_str\n");
								break;
							}

							switch (rank)
							{
							case SERVER_CLAN_MEMBER_NEW:
								rank_str = "New clan member";
								break;
							case SERVER_CLAN_MEMBER_PEON:
								rank_str = "Peon";
								break;
							case SERVER_CLAN_MEMBER_GRUNT:
								rank_str = "Grunt";
								break;
							case SERVER_CLAN_MEMBER_SHAMAN:
								rank_str = "Shaman";
								break;
							case SERVER_CLAN_MEMBER_CHIEFTAIN:
								rank_str = "Chieftain";
								break;
							default:
								rank_str = "";

							}

							switch (online)
							{
							case SERVER_CLAN_MEMBER_OFFLINE:
								online_str = "offline";
								break;
							case SERVER_CLAN_MEMBER_ONLINE:
								online_str = "online";
								break;
							case SERVER_CLAN_MEMBER_CHANNEL:
								online_str = "in channel";
								break;
							case SERVER_CLAN_MEMBER_GAME:
								online_str = "in game";
								break;
							case SERVER_CLAN_MEMBER_PRIVATE_GAME:
								online_str = "in private game";
								break;
							default:
								online_str = "";
							}

							munge(&client);
							std::printf("%s %s  is now %s %s\n", rank_str, member, online_str, append_str);
						}


					case SERVER_STATSREPLY:
						if (client.mode == mode_waitstat)
						{
							unsigned int names, keys;
							unsigned int match;
							unsigned int strpos;
							char const * temp;

							munge(&client);

							names = bn_int_get(rpacket->u.server_statsreply.name_count);
							keys = bn_int_get(rpacket->u.server_statsreply.key_count);
							match = bn_int_get(rpacket->u.server_statsreply.requestid);

							if (names != 1 || keys != 4 || match != statsmatch)
								std::printf("mangled reply (name_count=%u key_count=%u unknown1=%u)\n",
								names, keys, match);

							strpos = sizeof(t_server_statsreply);
							if ((temp = packet_get_str_const(rpacket, strpos, 256)))
							{
								std::printf(" Sex: ");
								ansi_printf(&client, ansi_text_color_yellow, "%s\n", temp);
								strpos += std::strlen(temp) + 1;
							}
							if ((temp = packet_get_str_const(rpacket, strpos, 256)))
							{
								std::printf(" Age: ");
								ansi_printf(&client, ansi_text_color_yellow, "%s\n", temp);
								strpos += std::strlen(temp) + 1;
							}
							if ((temp = packet_get_str_const(rpacket, strpos, 256)))
							{
								std::printf(" Location: ");
								ansi_printf(&client, ansi_text_color_yellow, "%s\n", temp);
								strpos += std::strlen(temp) + 1;
							}
							if ((temp = packet_get_str_const(rpacket, strpos, 256)))
							{
								char   msgtemp[1024];
								char * tok;

								std::printf(" Description: \n");
								if (client.useansi)
									ansi_text_color_fore(ansi_text_color_yellow);
								std::strncpy(msgtemp, temp, sizeof(msgtemp));
								msgtemp[sizeof(msgtemp)-1] = '\0';
								for (tok = std::strtok(msgtemp, "\r\n"); tok; tok = std::strtok(NULL, "\r\n"))
									std::printf("  %s\n", tok);
								if (client.useansi)
									ansi_text_reset();
								strpos += std::strlen(temp) + 1;
							}

							if (match == statsmatch)
								client.mode = mode_command;
						}
						break;

					case SERVER_PLAYERINFOREPLY: /* hmm. we didn't ask for one... */
						break;

					case SERVER_MESSAGE:
						if (packet_get_size(rpacket) < sizeof(t_server_message))
						{
							munge(&client);
							std::printf("Got bad SERVER_MESSAGE packet (expected %lu bytes, got %u)", sizeof(t_server_message), packet_get_size(rpacket));
							break;
						}

						{
							char const * speaker;
							char const * message;

							if (!(speaker = packet_get_str_const(rpacket, sizeof(t_server_message), 32)))
							{
								munge(&client);
								std::printf("Got SERVER_MESSAGE packet with bad or missing speaker\n");
								break;
							}
							if (!(message = packet_get_str_const(rpacket, sizeof(t_server_message)+std::strlen(speaker) + 1, 1024)))
							{
								munge(&client);
								std::printf("Got SERVER_MESSAGE packet with bad or missing message (speaker=\"%s\" start=%lu len=%u)\n", speaker, sizeof(t_server_message)+std::strlen(speaker) + 1, packet_get_size(rpacket));
								break;
							}

							switch (bn_int_get(rpacket->u.server_message.type))
							{
							case SERVER_MESSAGE_TYPE_JOIN:
								munge(&client);
								ansi_printf(&client, ansi_text_color_green, "\"%s\" %s enters\n", speaker,
									mflags_get_str(bn_int_get(rpacket->u.server_message.flags)));
								break;
							case SERVER_MESSAGE_TYPE_CHANNEL:
								munge(&client);
								ansi_printf(&client, ansi_text_color_green, "Joining channel :\"%s\" %s\n", message,
									cflags_get_str(bn_int_get(rpacket->u.server_message.flags)));
								break;
							case SERVER_MESSAGE_TYPE_ADDUSER:
								munge(&client);
								ansi_printf(&client, ansi_text_color_green, "\"%s\" %s is here\n", speaker,
									mflags_get_str(bn_int_get(rpacket->u.server_message.flags)));
								break;
							case SERVER_MESSAGE_TYPE_USERFLAGS:
								break;
							case SERVER_MESSAGE_TYPE_PART:
								munge(&client);
								ansi_printf(&client, ansi_text_color_green, "\"%s\" %s leaves\n", speaker,
									mflags_get_str(bn_int_get(rpacket->u.server_message.flags)));
								break;
							case SERVER_MESSAGE_TYPE_WHISPER:
								munge(&client);
								ansi_printf(&client, ansi_text_color_blue, "<From: %s>", speaker);
								std::printf(" ");
								str_print_term(stdout, message, 0, 0);
								std::printf("\n");
								break;
							case SERVER_MESSAGE_TYPE_WHISPERACK:
								munge(&client);
								ansi_printf(&client, ansi_text_color_blue, "<To: %s>", speaker);
								std::printf(" ");
								str_print_term(stdout, message, 0, 0);
								std::printf("\n");
								break;
							case SERVER_MESSAGE_TYPE_BROADCAST:
								munge(&client);
								ansi_printf(&client, ansi_text_color_yellow, "<Broadcast %s> %s\n", speaker, message);
								break;
							case SERVER_MESSAGE_TYPE_ERROR:
								munge(&client);
								ansi_printf(&client, ansi_text_color_red, "<Error> %s\n", message);
								break;
							case SERVER_MESSAGE_TYPE_INFO:
								munge(&client);
								ansi_printf(&client, ansi_text_color_yellow, "<Info> %s\n", message);
								break;
							case SERVER_MESSAGE_TYPE_EMOTE:
								munge(&client);
								ansi_printf(&client, ansi_text_color_yellow, "<%s %s>\n", speaker, message);
								break;
							default:
							case SERVER_MESSAGE_TYPE_TALK:
								munge(&client);
								ansi_printf(&client, ansi_text_color_yellow, "<%s>", speaker);
								std::printf(" ");
								str_print_term(stdout, message, 0, 0);
								std::printf("\n");
							}
						}
						break;

					default:
						munge(&client);
						std::printf("Unsupported server packet type 0x%04x\n", packet_get_type(rpacket));
						hexdump(stdout, packet_get_data_const(rpacket, 0, packet_get_size(rpacket)), packet_get_size(rpacket));
						std::printf("\n");
					}

					client.currsize = 0;
					break;

				case -1: /* error (probably connection closed) */
				default:
					munge(&client);
					std::printf("----\nConnection closed by server.\n");
					psock_close(client.sd);
					if (client.changed_in)
						tcsetattr(client.fd_stdin, TCSAFLUSH, &client.in_attr_old);
					return EXIT_SUCCESS;
				}
			}
		}
	}

	/* not reached */
}
