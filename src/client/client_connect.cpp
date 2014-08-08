/*
 * Copyright (C) 1998,1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
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
#include "client_connect.h"

#include <cstring>
#include <cstdio>
#include <cerrno>
#include <ctime>

#include "compat/gethostname.h"
#include "compat/inet_ntoa.h"
#include "common/tag.h"
#include "common/packet.h"
#include "common/init_protocol.h"
#include "common/bnet_protocol.h"
#include "common/bn_type.h"
#include "common/bnethash.h"
#include "common/bnethashconv.h"
#include "common/bnettime.h"
#ifdef CLIENTDEBUG
# include "common/eventlog.h"
#endif
#include "udptest.h"
#include "client.h"
#include "common/setup_after.h"


#define COMPNAMELEN 128

#ifdef CLIENTDEBUG
# define dprintf printf
#else
# define dprintf if (0) printf
#endif


namespace {

	int key_interpret(char const * cdkey, unsigned int * productid, unsigned int * keyvalue1, unsigned int * keyvalue2)
	{
		/* FIXME: implement... have example code from Eurijk! */
		*productid = 0;
		*keyvalue1 = 0;
		*keyvalue2 = 0;
		return 0;
	}


	int get_defversioninfo(char const * progname, char const * clienttag, unsigned int * versionid, unsigned int * gameversion, char const * * exeinfo, unsigned int * checksum)
	{
		if (std::strcmp(clienttag, CLIENTTAG_DIABLORTL) == 0)
		{
			*versionid = CLIENT_VERSIONID_DRTL;
			*gameversion = CLIENT_GAMEVERSION_DRTL;
			*exeinfo = CLIENT_EXEINFO_DRTL;
			*checksum = CLIENT_CHECKSUM_DRTL;
			return 0;
		}

		if (std::strcmp(clienttag, CLIENTTAG_STARCRAFT) == 0)
		{
			*versionid = CLIENT_VERSIONID_STAR;
			*gameversion = CLIENT_GAMEVERSION_STAR;
			*exeinfo = CLIENT_EXEINFO_STAR;
			*checksum = CLIENT_CHECKSUM_STAR;
			return 0;
		}

		if (std::strcmp(clienttag, CLIENTTAG_SHAREWARE) == 0)
		{
			*versionid = CLIENT_VERSIONID_SSHR;
			*gameversion = CLIENT_GAMEVERSION_SSHR;
			*exeinfo = CLIENT_EXEINFO_SSHR;
			*checksum = CLIENT_CHECKSUM_SSHR;
			return 0;
		}

		if (std::strcmp(clienttag, CLIENTTAG_BROODWARS) == 0)
		{
			*versionid = CLIENT_VERSIONID_SEXP;
			*gameversion = CLIENT_GAMEVERSION_SEXP;
			*exeinfo = CLIENT_EXEINFO_SEXP;
			*checksum = CLIENT_CHECKSUM_SEXP;
			return 0;
		}

		if (std::strcmp(clienttag, CLIENTTAG_WARCIIBNE) == 0)
		{
			*versionid = CLIENT_VERSIONID_W2BN;
			*gameversion = CLIENT_GAMEVERSION_W2BN;
			*exeinfo = CLIENT_EXEINFO_W2BN;
			*checksum = CLIENT_CHECKSUM_W2BN;
			return 0;
		}

		if (std::strcmp(clienttag, CLIENTTAG_DIABLO2DV) == 0)
		{
			*versionid = CLIENT_VERSIONID_D2DV;
			*gameversion = CLIENT_GAMEVERSION_D2DV;
			*exeinfo = CLIENT_EXEINFO_D2DV;
			*checksum = CLIENT_CHECKSUM_D2DV;
			return 0;
		}

		if (std::strcmp(clienttag, CLIENTTAG_DIABLO2XP) == 0)
		{
			*versionid = CLIENT_VERSIONID_D2XP;
			*gameversion = CLIENT_GAMEVERSION_D2XP;
			*exeinfo = CLIENT_EXEINFO_D2XP;
			*checksum = CLIENT_CHECKSUM_D2XP;
			return 0;
		}

		if (std::strcmp(clienttag, CLIENTTAG_WARCRAFT3) == 0 ||
			std::strcmp(clienttag, CLIENTTAG_WAR3XP) == 0)
		{
			*versionid = CLIENT_VERSIONID_WAR3;
			*gameversion = CLIENT_GAMEVERSION_WAR3;
			*exeinfo = CLIENT_EXEINFO_WAR3;
			*checksum = CLIENT_CHECKSUM_WAR3;
			return 0;
		}

		*versionid = 0;
		*gameversion = 0;
		*exeinfo = "";
		*checksum = 0;

		if (std::strcmp(clienttag, CLIENTTAG_BNCHATBOT) == 0)
			return 0;

		std::fprintf(stderr, "%s: unsupported clienttag \"%s\"\n", progname, clienttag);
		// aaron: dunno what we should return in case of this.. but returning nothing was definetly wrong
		return -1;
	}

}

namespace pvpgn
{

	namespace client
	{

		extern int client_connect(char const * progname, char const * servname, unsigned short servport, char const * cdowner, char const * cdkey, char const * clienttag, int ignoreversion, struct sockaddr_in * saddr, unsigned int * sessionkey, unsigned int * sessionnum, char const * archtag, char const * gamelang)
		{
			struct hostent * host;
			char const *     username;
			int              sd;
			t_packet *       packet;
			t_packet *       rpacket;
			char             compname[COMPNAMELEN];
			int              lsock;
			unsigned short   lsock_port;
			unsigned int     versionid;
			unsigned int     gameversion;
			char const *     exeinfo;
			unsigned int     checksum;

			if (!progname)
			{
				std::fprintf(stderr, "got NULL progname\n");
				return -1;
			}
			if (!saddr)
			{
				std::fprintf(stderr, "%s: got NULL saddr\n", progname);
				return -1;
			}
			if (!sessionkey)
			{
				std::fprintf(stderr, "%s: got NULL sessionkey\n", progname);
				return -1;
			}
			if (!sessionnum)
			{
				std::fprintf(stderr, "%s: got NULL sessionnum\n", progname);
				return -1;
			}

			if (psock_init() < 0)
			{
				std::fprintf(stderr, "%s: could not inialialize socket functions\n", progname);
				return -1;
			}

			if (!(host = gethostbyname(servname)))
			{
				std::fprintf(stderr, "%s: unknown host \"%s\"\n", progname, servname);
				return -1;
			}
			if (host->h_addrtype != PSOCK_AF_INET)
				std::fprintf(stderr, "%s: host \"%s\" is not in IPv4 address family\n", progname, servname);

			if (gethostname(compname, COMPNAMELEN) < 0)
			{
				std::fprintf(stderr, "%s: could not get host name (gethostname: %s)\n", progname, std::strerror(errno));
				return -1;
			}
#ifdef HAVE_GETLOGIN
			if (!(username = getlogin()))
#endif
			{
				username = "unknown";
				dprintf("%s: could not get login name, using \"%s\" (getlogin: %s)\n", progname, username, std::strerror(errno));
			}

			if ((sd = psock_socket(PSOCK_PF_INET, PSOCK_SOCK_STREAM, PSOCK_IPPROTO_TCP)) < 0)
			{
				std::fprintf(stderr, "%s: could not create socket (psock_socket: %s)\n", progname, std::strerror(psock_errno()));
				return -1;
			}

			std::memset(saddr, 0, sizeof(*saddr));
			saddr->sin_family = PSOCK_AF_INET;
			saddr->sin_port = htons(servport);
			std::memcpy(&saddr->sin_addr.s_addr, host->h_addr_list[0], host->h_length);
			if (psock_connect(sd, (struct sockaddr *)saddr, sizeof(*saddr)) < 0)
			{
				std::fprintf(stderr, "%s: could not connect to server \"%s\" port %hu (psock_connect: %s)\n", progname, servname, servport, std::strerror(psock_errno()));
				goto error_sd;
			}

			std::printf("Connected to %s:%hu.\n", inet_ntoa(saddr->sin_addr), servport);

#ifdef CLIENTDEBUG
			eventlog_set(stderr);
#endif

			if ((lsock = client_udptest_setup(progname, &lsock_port)) >= 0)
				dprintf("Got UDP data on port %hu\n", lsock_port);

			if (!(packet = packet_create(packet_class_init)))
			{
				std::fprintf(stderr, "%s: could not create packet\n", progname);
				goto error_lsock;
			}
			bn_byte_set(&packet->u.client_initconn.cclass, CLIENT_INITCONN_CLASS_BNET);
			client_blocksend_packet(sd, packet);
			packet_del_ref(packet);

			/* reuse this same packet over and over */
			if (!(rpacket = packet_create(packet_class_bnet)))
			{
				std::fprintf(stderr, "%s: could not create packet\n", progname);
				goto error_lsock;
			}

			get_defversioninfo(progname, clienttag, &versionid, &gameversion, &exeinfo, &checksum);

			if (std::strcmp(clienttag, CLIENTTAG_DIABLOSHR) == 0 ||
				std::strcmp(clienttag, CLIENTTAG_DIABLORTL) == 0)
			{
				if (!(packet = packet_create(packet_class_bnet)))
				{
					std::fprintf(stderr, "%s: could not create packet\n", progname);
					goto error_rpacket;
				}
				packet_set_size(packet, sizeof(t_client_unknown_1b));
				packet_set_type(packet, CLIENT_UNKNOWN_1B);
				bn_short_set(&packet->u.client_unknown_1b.unknown1, CLIENT_UNKNOWN_1B_UNKNOWN3);
				bn_short_nset(&packet->u.client_unknown_1b.port, lsock_port);
				bn_int_nset(&packet->u.client_unknown_1b.ip, 0); /* FIXME */
				bn_int_set(&packet->u.client_unknown_1b.unknown2, CLIENT_UNKNOWN_1B_UNKNOWN3);
				bn_int_set(&packet->u.client_unknown_1b.unknown3, CLIENT_UNKNOWN_1B_UNKNOWN3);
				client_blocksend_packet(sd, packet);
				packet_del_ref(packet);
			}

			if (!(packet = packet_create(packet_class_bnet)))
			{
				std::fprintf(stderr, "%s: could not create packet\n", progname);
				goto error_rpacket;
			}
			packet_set_size(packet, sizeof(t_client_countryinfo_109));
			packet_set_type(packet, CLIENT_COUNTRYINFO_109);
			bn_int_set(&packet->u.client_countryinfo_109.protocol, CLIENT_COUNTRYINFO_109_PROTOCOL);
			bn_int_tag_set(&packet->u.client_countryinfo_109.archtag, archtag);
			bn_int_tag_set(&packet->u.client_countryinfo_109.clienttag, clienttag);
			//AARON

			bn_int_set(&packet->u.client_countryinfo_109.versionid, versionid);
			bn_int_tag_set(&packet->u.client_countryinfo_109.gamelang, gamelang);
			bn_int_set(&packet->u.client_countryinfo_109.localip, CLIENT_COUNTRYINFO_109_LOCALIP);
			{
				int bias;

				bias = local_tzbias();

				bn_int_set(&packet->u.client_countryinfo_109.bias, (unsigned int)bias); /* rely on 2's complement */
				dprintf("my tzbias = %d (0x%08hx)\n", bias, (unsigned int)bias);
			}
			/* FIXME: determine from locale */
			bn_int_set(&packet->u.client_countryinfo_109.lcid, CLIENT_COUNTRYINFO_109_LANGID_USENGLISH);
			bn_int_set(&packet->u.client_countryinfo_109.langid, CLIENT_COUNTRYINFO_109_LANGID_USENGLISH);
			packet_append_string(packet, CLIENT_COUNTRYINFO_109_LANGSTR_USENGLISH);
			/* FIXME: determine from locale+timezone... from domain name... nothing really would
			   work.  Maybe add some command-line options */
			packet_append_string(packet, CLIENT_COUNTRYINFO_109_COUNTRYNAME_USA);
			client_blocksend_packet(sd, packet);
			packet_del_ref(packet);

			do
			if (client_blockrecv_packet(sd, rpacket) < 0)
			{
				std::fprintf(stderr, "%s: server closed connection\n", progname);
				goto error_rpacket;
			}
			while (packet_get_type(rpacket) != SERVER_AUTHREQ_109 &&
				packet_get_type(rpacket) != SERVER_AUTHREPLY_109);

			if (packet_get_type(rpacket) == SERVER_AUTHREQ_109) /* hmm... server wants to check the version number */
			{
				dprintf("Got AUTHREQ_109\n");
				*sessionkey = bn_int_get(rpacket->u.server_authreq_109.sessionkey);
				*sessionnum = bn_int_get(rpacket->u.server_authreq_109.sessionnum);
				/* FIXME: also get filename and equation */

				if (!ignoreversion)
				{

					if (!(packet = packet_create(packet_class_bnet)))
					{
						std::fprintf(stderr, "%s: could not create packet\n", progname);
						goto error_rpacket;
					}
					packet_set_size(packet, sizeof(t_client_authreq_109));
					packet_set_type(packet, CLIENT_AUTHREQ_109);
					bn_int_set(&packet->u.client_authreq_109.ticks, std::time(NULL));

					{
						t_cdkey_info cdkey_info;

						bn_int_set(&packet->u.client_authreq_109.gameversion, gameversion);

						bn_int_set(&packet->u.client_authreq_109.cdkey_number, 1); /* only one */
						std::memset(&cdkey_info, '0', sizeof(cdkey_info));
						/* FIXME: put the input cdkey here */
						packet_append_data(packet, &cdkey_info, sizeof(cdkey_info));
						packet_append_string(packet, exeinfo);
						packet_append_string(packet, cdowner);
					}
					bn_int_set(&packet->u.client_authreq_109.spawn, 0x0000);
					bn_int_set(&packet->u.client_authreq_109.checksum, checksum);
					client_blocksend_packet(sd, packet);
					packet_del_ref(packet);

					/* now wait for reply */
					do
					if (client_blockrecv_packet(sd, rpacket) < 0)
					{
						std::fprintf(stderr, "%s: server closed connection\n", progname);
						goto error_rpacket;
					}
					while (packet_get_type(rpacket) != SERVER_AUTHREPLY_109);
					//FIXME: check if AUTHREPLY_109 is success or fail
					if (bn_int_get(rpacket->u.server_authreply_109.message) != SERVER_AUTHREPLY_109_MESSAGE_OK)
					{
						std::fprintf(stderr, "AUTHREPLY_109 failed - closing connection\n");
						goto error_rpacket;
					}
				}
			}
			else
				std::fprintf(stderr, "We didn't get a sessionkey, don't expect login to work!");
			dprintf("Got AUTHREPLY_109\n");

			if (!(packet = packet_create(packet_class_bnet)))
			{
				std::fprintf(stderr, "%s: could not create packet\n", progname);
				goto error_rpacket;
			}
			packet_set_size(packet, sizeof(t_client_iconreq));
			packet_set_type(packet, CLIENT_ICONREQ);
			client_blocksend_packet(sd, packet);
			packet_del_ref(packet);

			do
			if (client_blockrecv_packet(sd, rpacket) < 0)
			{
				std::fprintf(stderr, "%s: server closed connection\n", progname);
				goto error_rpacket;
			}
			while (packet_get_type(rpacket) != SERVER_ICONREPLY);
			dprintf("Got ICONREPLY\n");

			if (std::strcmp(clienttag, CLIENTTAG_STARCRAFT) == 0 ||
				std::strcmp(clienttag, CLIENTTAG_BROODWARS) == 0 ||
				std::strcmp(clienttag, CLIENTTAG_WARCIIBNE) == 0)
			{
				if (!(packet = packet_create(packet_class_bnet)))
				{
					std::fprintf(stderr, "%s: could not create packet\n", progname);
					goto error_rpacket;
				}

				{
					struct
					{
						bn_int sessionkey;
						bn_int ticks;
						bn_int productid;
						bn_int keyvalue1;
						bn_int keyvalue2;
					}            temp;
					t_hash       key_hash;
					unsigned int ticks;
					unsigned int keyvalue1, keyvalue2, productid;

					if (key_interpret(cdkey, &productid, &keyvalue1, &keyvalue2) < 0)
					{
						std::fprintf(stderr, "%s: specified key is not valid, sending junk\n", progname);
						productid = 0;
						keyvalue1 = 0;
						keyvalue2 = 0;
					}

					ticks = 0; /* FIXME: what to use here? */
					bn_int_set(&temp.ticks, ticks);
					bn_int_set(&temp.sessionkey, *sessionkey);
					bn_int_set(&temp.productid, productid);
					bn_int_set(&temp.keyvalue1, keyvalue1);
					bn_int_set(&temp.keyvalue2, keyvalue2);
					bnet_hash(&key_hash, sizeof(temp), &temp);

					packet_set_size(packet, sizeof(t_client_cdkey2));
					packet_set_type(packet, CLIENT_CDKEY2);
					bn_int_set(&packet->u.client_cdkey2.spawn, CLIENT_CDKEY2_SPAWN_FALSE); /* FIXME: add option */
					bn_int_set(&packet->u.client_cdkey2.keylen, std::strlen(cdkey));
					bn_int_set(&packet->u.client_cdkey2.productid, productid);
					bn_int_set(&packet->u.client_cdkey2.keyvalue1, keyvalue1);
					bn_int_set(&packet->u.client_cdkey2.sessionkey, *sessionkey);
					bn_int_set(&packet->u.client_cdkey2.ticks, ticks);
					hash_to_bnhash((t_hash const *)&key_hash, packet->u.client_cdkey2.key_hash); /* avoid warning */
					packet_append_string(packet, cdowner);
					client_blocksend_packet(sd, packet);
					packet_del_ref(packet);
				}

				do
				if (client_blockrecv_packet(sd, rpacket) < 0)
				{
					std::fprintf(stderr, "%s: server closed connection\n", progname);
					goto error_rpacket;
				}
				while (packet_get_type(rpacket) != SERVER_CDKEYREPLY2);
				dprintf("Got CDKEYREPLY2 (%u)\n", bn_int_get(rpacket->u.server_cdkeyreply2.message));
			}

			packet_destroy(rpacket);
			return sd;

			/* error cleanup */

		error_rpacket:

			packet_destroy(rpacket);

		error_lsock:

			if (lsock >= 0)
				psock_close(lsock);

		error_sd:

			psock_close(sd);

			return -1;
		}

	}

}
