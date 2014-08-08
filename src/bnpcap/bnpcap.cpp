/*
 * Copyright (C) 2001  Marco Ziech (mmz@gmx.net)
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
#include <cerrno>

#include <pcap.h>

#include "compat/pgetopt.h"
#include "compat/psock.h"
#include "common/init_protocol.h"
#include "common/bnet_protocol.h"
#include "common/udp_protocol.h"
#include "common/packet.h"
#include "common/eventlog.h"
#include "common/hexdump.h"
#include "common/list.h"
#include "common/version.h"
#include "common/util.h"
#include "common/xalloc.h"
#include "common/setup_after.h"

using namespace pvpgn;

/* FIXME: everywhere: add checks for NULL pointers */

char *filename = NULL;
pcap_t *pc;
char ebuf[PCAP_ERRBUF_SIZE];


int bnpcap_dodebug = 0;
int bnpcap_beverbose = 0;

unsigned int listen_port = 6112;

/********************* CONNECTIONS ********************/

typedef enum {
	tcp_state_none,
	tcp_state_syn,
	tcp_state_ack,
	tcp_state_ok
} t_tcp_state;

typedef struct {
	/* It's IPV4 */
	unsigned int ip;
	unsigned short port;
} t_bnpcap_addr;

/* To track connections ... */
typedef struct {
	t_bnpcap_addr client;
	t_bnpcap_addr server;
	t_packet_class cclass;
	t_tcp_state tcpstate;
	int incomplete;
	int clientoff;
	t_packet *clientpkt;
	int serveroff;
	t_packet *serverpkt;
	t_list * packets;
} t_bnpcap_conn;

typedef struct {
	t_packet_dir dir;
	struct timeval tv;
	unsigned int id;
	t_packet *p;
} t_bnpcap_packet;

t_list * conns;
t_list * udppackets;

struct timeval packettime;

static unsigned int current_packet_id = 1;

/*********************** HEADERS **********************/

/* FIXME: don't assume that's always true */
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;

/************************** TCP ***********************/

typedef struct {
	u16 sport;
	u16 dport;
	u32 seqno;
	u32 ackno;
	u16 stuff; /* Data offset, various flags */
	u16 window;
	u16 checksum;
	u16 urgp;  /* Urgent Pointer (if URG flag set) */
	/* options */
} t_tcp_header_raw;

typedef struct {
	unsigned short sport;
	unsigned short dport;
	unsigned int seqno;
	unsigned int ackno;
	unsigned char doffset;
	unsigned short flags;
#define TCP_URG 0x20 /* Urgent pointer field significant */
#define TCP_ACK 0x10 /* Acknowlegdement field significant */
#define TCP_PSH 0x08 /* Push function */
#define TCP_RST 0x04 /* Reset connection */
#define TCP_SYN 0x02 /* Synchronize sequence numbers */
#define TCP_FIN 0x01 /* No more data from sender (finish) */
	unsigned short window;
	unsigned short checksum;
	unsigned short urgp;
	/* options */
} t_tcp_header;

/******************************** UDP ************************/

typedef struct {
	u16 sport;
	u16 dport;
	u16 len;
	u16 checksum;
} t_ip_udp_header_raw;

typedef struct {
	unsigned short sport;
	unsigned short dport;
	unsigned short len;
	unsigned short checksum;
} t_ip_udp_header;

/******************************** IP *************************/

typedef struct {
	u8 versionihl;
	u8 tos;
	u16 len;
	u16 id;
	u16 flagsoffset;
	u8 ttl;
	u8 protocol;
	u16 checksum;
	u32 src;
	u32 dst;
	/* options */
} t_ip_header_raw;

typedef struct {
	unsigned char version;
	unsigned char ihl;
	unsigned char tos;
	unsigned short len;
	unsigned short id;
	unsigned char flags;
#define IP_DF 0x02 /* 1 == Don't fragment */
#define IP_MF 0x01 /* 1 == More fragments */
	unsigned short offset;
	unsigned char ttl;
	unsigned char protocol;
	unsigned short checksum;
	unsigned int src;
	unsigned int dst;
	/* options */
} t_ip_header;

/******************************* ETHERNET *****************************/

typedef struct {
	u8 dst[6]; /* Ethernet hardware address */
	u8 src[6]; /* Ethernet hardware address */
	u16 type;  /* Ethernet_II: protocol type */
	/* FIXME: Ethernet [802.2|802.3|SNAP]: maybe something else (eg. length) */
} t_ether_raw;

/************************************************************************/
/************************* CONNECTION FUNCTIONS *************************/

static t_bnpcap_conn * bnpcap_conn_new(t_bnpcap_addr const *s, t_bnpcap_addr const *d)
{
	t_bnpcap_conn * c;

	c = (t_bnpcap_conn *)xmalloc(sizeof(t_bnpcap_conn)); /* avoid warning */
	if (!c) {
		eventlog(eventlog_level_error, __FUNCTION__, "xalloc failed: %s", std::strerror(errno));
		return NULL;
	}
	if (d->port == listen_port || d->port == 6200) { /* FIXME: That's dirty: We assume the server is on port 6112 */
		std::memcpy(&c->client, s, sizeof(t_bnpcap_addr));
		std::memcpy(&c->server, d, sizeof(t_bnpcap_addr));
	}
	else {
		std::memcpy(&c->client, d, sizeof(t_bnpcap_addr));
		std::memcpy(&c->server, s, sizeof(t_bnpcap_addr));
	}
	c->cclass = packet_class_init;
	c->packets = list_create();
	c->incomplete = 0;
	c->tcpstate = tcp_state_none;
	c->clientoff = 0;
	c->clientpkt = NULL;
	c->serveroff = 0;
	c->serverpkt = NULL;
	return c;
}

static void bnpcap_conn_set_class(t_bnpcap_conn *c, t_packet_class cclass)
{
	c->cclass = cclass;
}

static t_packet_class bnpcap_conn_get_class(t_bnpcap_conn *c)
{
	return c->cclass;
}

static t_bnpcap_conn * bnpcap_conn_find(t_bnpcap_addr const *s, t_bnpcap_addr const *d)
{
	t_elem * curr;

	LIST_TRAVERSE(conns, curr) {
		t_bnpcap_conn *c;

		c = (t_bnpcap_conn*)elem_get_data(curr);
		if (((c->client.ip == s->ip) && (c->client.port == s->port)) &&
			((c->server.ip == d->ip) && (c->server.port == d->port))) {
			return c;
		}
		else if (((c->client.ip == d->ip) && (c->client.port == d->port)) &&
			((c->server.ip == s->ip) && (c->server.port == s->port))) {
			return c;
		}
	}
	return NULL;
}

static t_packet_dir bnpcap_conn_get_dir(t_bnpcap_conn const * c, t_bnpcap_addr const *s, t_bnpcap_addr const *d)
{
	if (((c->client.ip == s->ip) && (c->client.port == s->port)) &&
		((c->server.ip == d->ip) && (c->server.port == d->port)))
		return packet_dir_from_client;
	else
		return packet_dir_from_server;
}

static int bnpcap_conn_add_packet(t_bnpcap_conn *c, t_bnpcap_packet *bp) {
	eventlog(eventlog_level_debug, __FUNCTION__, "id=%u ", bp->id);
	list_append_data(c->packets, bp);
	packet_add_ref(bp->p);
	return 0;
}

static int bnpcap_conn_packet(unsigned int sip, unsigned short sport, unsigned int dip, unsigned short dport, unsigned char const * data, unsigned int len)
{
	t_bnpcap_addr s;
	t_bnpcap_addr d;
	t_bnpcap_conn *c;
	t_bnpcap_packet *bp;

	s.ip = sip;
	s.port = sport;
	d.ip = dip;
	d.port = dport;

	if ((c = bnpcap_conn_find(&s, &d))) {
		eventlog(eventlog_level_debug, __FUNCTION__, "adding packet to existing connection");
		if (c->tcpstate == tcp_state_ack) {
			c->tcpstate = tcp_state_ok;
		}
		else if (c->tcpstate == tcp_state_syn) {
			c->incomplete = 1; /* ACK missing */
			c->tcpstate = tcp_state_ok;
		}
	}
	else {
		eventlog(eventlog_level_debug, __FUNCTION__, "adding packet to incomplete connection");
		c = bnpcap_conn_new(&s, &d);
		bnpcap_conn_set_class(c, packet_class_raw); /* we don't know the init sequence */
		c->incomplete = 1;
		c->tcpstate = tcp_state_ok;
		list_append_data(conns, c);
	}
	if (c->tcpstate != tcp_state_ok) {
		eventlog(eventlog_level_warn, __FUNCTION__, "connection got packet in wrong state!");
	}
	if (bnpcap_conn_get_class(c) == packet_class_init) {
		if (len > 1) {
			eventlog(eventlog_level_warn, __FUNCTION__, "init packet larger than 1 byte");
		}
		switch (data[0]) {
		case CLIENT_INITCONN_CLASS_BNET:
			bnpcap_conn_set_class(c, packet_class_bnet);
			break;
		case CLIENT_INITCONN_CLASS_FILE:
			bnpcap_conn_set_class(c, packet_class_file);
			break;
		case 0xf7: // W3 matchmaking hack
			eventlog(eventlog_level_info, __FUNCTION__, "matchmaking packet");
			bnpcap_conn_set_class(c, packet_class_bnet);
			break;
		default:
			bnpcap_conn_set_class(c, packet_class_raw);
		}
	}
	else {
		t_packet *p;
		unsigned int off;
		unsigned char const *datap = data;
		int always_complete = 0;


		if (bnpcap_conn_get_class(c) == packet_class_raw)
			always_complete = 1; /* There is no size field */
		if (bnpcap_conn_get_class(c) == packet_class_file)
			always_complete = 1; /* Size field isn't always there */

		if (always_complete) {
			/* packet is always complete */
			eventlog(eventlog_level_debug, __FUNCTION__, "packet is always complete (class=%d)", bnpcap_conn_get_class(c));
			bp = (t_bnpcap_packet *)xmalloc(sizeof(t_bnpcap_packet)); /* avoid warning */
			if (!bp) {
				eventlog(eventlog_level_error, __FUNCTION__, "xalloc failed: %s", std::strerror(errno));
				return -1;
			}
			bp->dir = bnpcap_conn_get_dir(c, &s, &d);
			bp->p = packet_create(bnpcap_conn_get_class(c));
			bp->id = current_packet_id++;
			if (!bp->p) {
				eventlog(eventlog_level_error, __FUNCTION__, "packet_create failed");
				return -1;
			}
			std::memcpy(&bp->tv, &packettime, sizeof(struct timeval));
			packet_set_size(bp->p, len);
			std::memcpy(packet_get_raw_data(bp->p, 0), data, len);
			bnpcap_conn_add_packet(c, bp);
			if ((packet_get_class(bp->p) == packet_class_file) && (packet_get_type(bp->p) == SERVER_FILE_REPLY)) {
				eventlog(eventlog_level_debug, __FUNCTION__, "file transfer initiated (setting to raw)");
				bnpcap_conn_set_class(c, packet_class_raw);
			}
		}
		else {
			/* read out saved state */
			if (bnpcap_conn_get_dir(c, &s, &d) == packet_dir_from_client) {
				p = c->clientpkt;
				off = c->clientoff;
			}
			else {
				p = c->serverpkt;
				off = c->serveroff;
			}
			while ((datap - data) < (signed)len) {
				if (!p) {
					eventlog(eventlog_level_debug, __FUNCTION__, "creating new packet");
					p = packet_create(bnpcap_conn_get_class(c));
					if (!p) {
						eventlog(eventlog_level_error, __FUNCTION__, "packet_create failed");
						return -1;
					}
					packet_set_size(p, packet_get_header_size(p)); /* set it to the minimum for now */
					off = 0;
				}
				if (off < packet_get_header_size(p)) {
					unsigned int l = (packet_get_header_size(p) - off);
					/* (len-(datap-data)) : remaining bytes in buffer */
					if ((len - (datap - data)) < l)
						l = (len - (datap - data));
					eventlog(eventlog_level_debug, __FUNCTION__, "filling up header (adding %d to %d to get %d)", l, off, packet_get_header_size(p));
					std::memcpy(packet_get_raw_data(p, off), datap, l);
					datap = datap + l;
					off = off + l;
				}
				else {
					unsigned int l = (packet_get_size(p) - off);
					if ((len - (datap - data)) < l)
						l = (len - (datap - data));
					eventlog(eventlog_level_debug, __FUNCTION__, "filling up packet (0x%04x:%s) (adding %d to %d to get %d)", packet_get_type(p), packet_get_type_str(p, bnpcap_conn_get_dir(c, &s, &d)), l, off, packet_get_size(p));
					std::memcpy(packet_get_raw_data(p, off), datap, l);
					datap = datap + l;
					off = off + l;
				}
				if ((off >= packet_get_header_size(p)) && (off >= packet_get_size(p))) {
					/* packet is complete */
					eventlog(eventlog_level_debug, __FUNCTION__, "packet is complete");
					bp = (t_bnpcap_packet *)xmalloc(sizeof(t_bnpcap_packet)); /* avoid warning */
					if (!bp) {
						eventlog(eventlog_level_error, __FUNCTION__, "xalloc failed: %s", std::strerror(errno));
						return -1;
					}
					if ((off != packet_get_size(p)) && (bnpcap_dodebug)) {
						eventlog(eventlog_level_warn, __FUNCTION__, "packet size differs (%d != %d) (offset=0x%04lx)", off, packet_get_size(p), datap - data);
						hexdump(stderr, data, len);
						/*		  std::memcpy(packet_get_raw_data(p,0),data,packet_get_size(p)); */
					}
					bp->dir = bnpcap_conn_get_dir(c, &s, &d);
					bp->p = p;
					bp->id = current_packet_id++;
					std::memcpy(&bp->tv, &packettime, sizeof(struct timeval));
					bnpcap_conn_add_packet(c, bp);
					if (packet_get_size(p) == 0)
						datap = data + len; /* if size is invalid, drop the rest of the stream packet */
					p = NULL;
					off = 0;
				}
			} /* while */
			/* write back saved state */
			if ((off > 0) && (bnpcap_dodebug)) {
				eventlog(eventlog_level_debug, __FUNCTION__, "saving %d bytes in packet buffer (p=0x%08lx)", off, (long)p);
			}
			if (bnpcap_conn_get_dir(c, &s, &d) == packet_dir_from_client) {
				c->clientpkt = p;
				c->clientoff = off;
			}
			else {
				c->serverpkt = p;
				c->serveroff = off;
			}
		} /* !always_complete */
		return 0;
	}
	return 0;
}


/************************************************************************/
/******************************** LAYERS ********************************/

/********************************* TCP **********************************/

static int bnpcap_tcp2tcp(t_tcp_header * d, t_tcp_header_raw const * s)
{
	d->sport = htons(s->sport);
	d->dport = htons(s->dport);
	d->seqno = htonl(s->seqno);
	d->ackno = htonl(s->ackno);
	d->doffset = (htons(s->stuff) & 0xF000) >> 12;
	d->flags = (htons(s->stuff) & 0x0FFF);
	d->window = htons(s->window);
	d->checksum = htons(s->checksum);
	d->urgp = htons(s->urgp);
	return 0;
}

static int bnpcap_process_tcp(t_ip_header const * ip, unsigned char const *data, unsigned int len)
{
	t_tcp_header_raw const *raw;
	t_tcp_header h;

	raw = (t_tcp_header_raw const *)data; /* avoid warning */
	bnpcap_tcp2tcp(&h, raw);
	if (h.doffset < 5) {
		eventlog(eventlog_level_warn, __FUNCTION__, "tcp header too small (%u 32-bit words)", h.doffset);
		return 1;
	}
	else {
		char fstr[32] = "";

		if (h.flags & TCP_URG)
			std::strcat(fstr, "U");
		if (h.flags & TCP_ACK)
			std::strcat(fstr, "A");
		if (h.flags & TCP_PSH)
			std::strcat(fstr, "P");
		if (h.flags & TCP_RST)
			std::strcat(fstr, "R");
		if (h.flags & TCP_SYN)
			std::strcat(fstr, "S");
		if (h.flags & TCP_FIN)
			std::strcat(fstr, "F");
		eventlog(eventlog_level_debug, __FUNCTION__, "tcp: sport=%u dport=%u seqno=0x%08x ackno=0x%08x window=0x%08x len=%d (%s)", h.sport, h.dport, h.seqno, h.ackno, h.window, ((signed)len - (h.doffset * 4)), fstr);
		if (((signed)len - (h.doffset * 4)) <= 0) {
			eventlog(eventlog_level_info, __FUNCTION__, "empty packet (%d)", ((signed)len - (h.doffset * 4)));
			/* handle sync packets */
			if (h.flags & TCP_SYN)  {
				t_bnpcap_addr s, d;

				s.ip = ip->src; s.port = h.sport;
				d.ip = ip->dst; d.port = h.dport;
				if (h.flags & TCP_ACK) {
					t_bnpcap_conn *c = bnpcap_conn_find(&s, &d);

					if (c) {
						if (c->tcpstate == tcp_state_syn)
							c->tcpstate = tcp_state_ack;
					}
				}
				else {
					if (!bnpcap_conn_find(&s, &d)) {
						t_bnpcap_conn *c;

						c = bnpcap_conn_new(&s, &d);
						c->tcpstate = tcp_state_syn;
						list_append_data(conns, c);
						eventlog(eventlog_level_debug, __FUNCTION__, "created new connection with SYN");
					}
					else {
						eventlog(eventlog_level_debug, __FUNCTION__, "got SYN in connection");
					}
				}
			}
		}
		else if (((h.sport != listen_port) && (h.dport != listen_port)) && ((h.sport != 6200) && (h.dport != 6200))) {
			eventlog(eventlog_level_info, __FUNCTION__, "other packet (%d)", ((signed)len - (h.doffset * 4)));
		}
		else {
			eventlog(eventlog_level_info, __FUNCTION__, "valid packet (%d)", ((signed)len - (h.doffset * 4)));
			bnpcap_conn_packet(ip->src, h.sport, ip->dst, h.dport, data + (h.doffset * 4), len - (h.doffset * 4));
		}
		return 0;
	}
}

/************************************ UDP ********************************/

static int bnpcap_udp2udp(t_ip_udp_header *d, t_ip_udp_header_raw const *s)
{
	d->sport = ntohs(s->sport);
	d->dport = ntohs(s->dport);
	d->len = ntohs(s->len);
	d->checksum = ntohs(s->checksum);
	return 0;
}

static int bnpcap_process_udp(unsigned char const *data, unsigned int len)
{
	t_ip_udp_header_raw const *raw;
	t_ip_udp_header h;
	t_bnpcap_packet *bp;

	raw = (t_ip_udp_header_raw const *)data; /* avoid warning */
	bnpcap_udp2udp(&h, raw);

	bp = (t_bnpcap_packet *)xmalloc(sizeof(t_bnpcap_packet)); /* avoid warning */
	if (!bp) {
		eventlog(eventlog_level_error, __FUNCTION__, "xalloc failed: %s", std::strerror(errno));
		return -1;
	}
	if (h.dport == listen_port || h.dport == 6200) {
		bp->dir = packet_dir_from_client;
	}
	else {
		bp->dir = packet_dir_from_server;
	}
	eventlog(eventlog_level_debug, __FUNCTION__, "sport=%u dport=%u len=%u(%d)", h.sport, h.dport, h.len, len);
	bp->id = current_packet_id++;
	std::memcpy(&bp->tv, &packettime, sizeof(struct timeval));
	bp->p = packet_create(packet_class_udp);
	if (!bp->p) {
		eventlog(eventlog_level_error, __FUNCTION__, "packet_create failed");
		return -1;
	}
	packet_set_size(bp->p, h.len - sizeof(t_ip_udp_header_raw));
	std::memcpy(packet_get_raw_data(bp->p, 0), data + sizeof(t_ip_udp_header_raw), h.len - sizeof(t_ip_udp_header_raw));
	eventlog(eventlog_level_error, __FUNCTION__, "id=%u ", bp->id);
	list_append_data(udppackets, bp);
	return 0;
}

/************************************ IP *********************************/

static int bnpcap_ip2ip(t_ip_header * d, t_ip_header_raw const * s)
{
	d->version = ((s->versionihl & 0xf0) >> 4);
	d->ihl = s->versionihl & 0x0f;
	d->tos = s->tos;
	d->len = ntohs(s->len);
	d->id = ntohs(s->id);
	d->offset = ntohl(s->flagsoffset);
	d->flags = ((d->offset & 0xE000) >> 13);
	d->offset = ((d->offset & 0x1FFF));
	d->ttl = s->ttl;
	d->protocol = s->protocol;
	d->checksum = ntohs(s->checksum);
	d->src = ntohl(s->src);
	d->dst = ntohl(s->dst);
	return 0;
}


static int bnpcap_process_ip(unsigned char const *data, unsigned int len)
{
	/* FIXME: handle IP fragmentation */
	/* FIXME: use identification field to pass the datagram in the right order */
	t_ip_header_raw const *raw;
	t_ip_header h;

	raw = (t_ip_header_raw const *)data; /* avoid warning */
	bnpcap_ip2ip(&h, raw);
	if (h.version != 4) {
		eventlog(eventlog_level_warn, __FUNCTION__, "ip version %u not supported (ihl=%u, raw=0x%02x)", h.version, h.ihl, raw->versionihl);
		return 1;
	}
	else if (h.ihl < 5) {
		/* an IP header must be at least 5 words */
		eventlog(eventlog_level_warn, __FUNCTION__, "ip header to small (%u 32-bit words)", h.ihl);
		return 1;
	}
	else if (h.len > len) {
		eventlog(eventlog_level_warn, __FUNCTION__, "ip len larger than packet (%d > %d)", h.len, len);
		return 1;
	}
	else {
		char fstr[32] = "";

		if (h.flags & IP_DF)
			std::strcat(fstr, "D");
		if (h.flags & IP_MF)
			std::strcat(fstr, "M");
		eventlog(eventlog_level_debug, __FUNCTION__, "ip: len=%u(%u) src=%08x dst=%08x protocol=%u offset=0x%08x id=0x%08x (%s)", h.len, len, h.src, h.dst, h.protocol, h.offset, h.id, fstr);
		if (h.protocol == 6) {
			/* This is TCP */
			return bnpcap_process_tcp(&h, data + (h.ihl * 4), h.len - (h.ihl * 4));
		}
		else if (h.protocol == 17) {
			/* This is UDP */
			return bnpcap_process_udp(data + (h.ihl * 4), h.len - (h.ihl * 4));
		}
	}
	return 0;
}

/************************************* ETHERNET ******************************/

static int bnpcap_process_ether(unsigned char const *data, unsigned int len)
{ /* Well, first parse the ethernet header (I hope you use Ethernet_II :) ... */
	t_ether_raw const *raw;

	raw = (t_ether_raw const *)data; /* avoid warning */
	if (ntohs(raw->type) == 0x0800) {
		/* This is IP */
		return bnpcap_process_ip(data + sizeof(t_ether_raw), len - sizeof(t_ether_raw));
	}
	else {
		eventlog(eventlog_level_warn, __FUNCTION__, "unsupported protocol 0x%04x", ntohs(raw->type));
		return 1;
	}
}

/* If you want to use other hardware protocols like PPP add them here ... */

/**************************** PACKET/PCAP *********************************/

static void bnpcap_process_packet(u_char * private_, const struct pcap_pkthdr * p, u_char const * data)
{
	unsigned int pl = p->len;

	if (private_) private_ = NULL; // hack to eliminate compiler warning

	std::memcpy(&packettime, &p->ts, sizeof(struct timeval));
	eventlog(eventlog_level_debug, __FUNCTION__, "packet: len=%d caplen=%d", p->len, p->caplen);
	/* FIXME: check if it's ethernet */
	bnpcap_process_ether(data, pl);
}

/**************************************************************************/

static void bnpcap_usage(void) {
	std::printf("BNPCAP - A tool to convert pcap battle.net dumps to a human-readable format.\n");
	std::printf("Version " PVPGN_VERSION " --- Copyright (c) 2001  Marco Ziech (mmz@gmx.net)\n");
	std::printf("This software makes use of libpcap.\n\n");
	std::printf("Usage: bnpcap [-d] [-v] [-p PORT] <pcap-filename>\n");
	std::printf("   -d          Print out debugging information\n");
	std::printf("   -v          Be more verbose\n");
	std::printf("   -p PORT     Specify port to process (Default: 6112)\n\n");
}

/******************************* MAIN *************************************/

int main(int argc, char **argv) {
	t_elem * currconn;
	t_elem * currudp;
	int c;

	while ((c = getopt(argc, argv, "dvp:")) != -1) {
		switch (c) {
		case 'p':
			str_to_uint(optarg, &listen_port);
			break;
		case 'd':
			bnpcap_dodebug = 1;
			break;
		case 'v':
			bnpcap_beverbose = 1;
			break;
		case '?':
			std::printf("unrecognized option '%c'\n", optopt);
			break;
		default:
			std::printf("getopt returned \'%c\'\n", c);
		}
	}

	if (optind < argc) {
		filename = argv[optind];
	}
	else {
		bnpcap_usage();
		return 1;
	}

	pc = pcap_open_offline(filename, ebuf);
	if (!pc) {
		std::fprintf(stderr, "pcap_open_offline: %s\n", ebuf);
		return -1;
	}

	eventlog_set(stderr);
	eventlog_clear_level();
	if (bnpcap_dodebug)
		eventlog_add_level("debug");
	if (bnpcap_beverbose || bnpcap_dodebug)
		eventlog_add_level("info");
	eventlog_add_level("warn");
	eventlog_add_level("error");
	eventlog_add_level("fatal");

	conns = list_create();
	udppackets = list_create();
	pcap_dispatch(pc, 0, bnpcap_process_packet, NULL);

	std::printf("### This packet dump was created by bnpcap.\n");
	LIST_TRAVERSE(conns, currconn) {
		t_bnpcap_conn *c;
		char cstr[64];
		char sstr[64];
		t_elem * currpacket;

		c = (t_bnpcap_conn*)elem_get_data(currconn);
		snprintf(cstr, 64, "%u.%u.%u.%u:%u", ((c->client.ip & 0xFF000000) >> 24), ((c->client.ip & 0x00FF0000) >> 16), ((c->client.ip & 0x0000FF00) >> 8), ((c->client.ip & 0x000000FF)), c->client.port);
		snprintf(sstr, 64, "%u.%u.%u.%u:%u", ((c->server.ip & 0xFF000000) >> 24), ((c->server.ip & 0x00FF0000) >> 16), ((c->server.ip & 0x0000FF00) >> 8), ((c->server.ip & 0x000000FF)), c->server.port);
		std::printf("## %s connection: client=%s server=%s\n", (c->incomplete ? "incomplete" : "complete"), cstr, sstr);
		LIST_TRAVERSE(c->packets, currpacket) {
			t_bnpcap_packet * bp;

			bp = (t_bnpcap_packet*)elem_get_data(currpacket);
			std::printf("# %u packet from %s: type=0x%04x(%s) length=%d class=%s\n", bp->id/*bp->tv.tv_sec*/, (bp->dir == packet_dir_from_client ? "client" : "server"), packet_get_type(bp->p), packet_get_type_str(bp->p, bp->dir), packet_get_size(bp->p), packet_get_class_str(bp->p));
			hexdump(stdout, packet_get_raw_data(bp->p, 0), packet_get_size(bp->p));
			std::printf("\n");
		}
	}
	std::printf("## udp packets\n");
	LIST_TRAVERSE(udppackets, currudp) {
		t_bnpcap_packet *bp;

		bp = (t_bnpcap_packet*)elem_get_data(currudp);
		std::printf("# %u packet from %s: type=0x%04x(%s) length=%d class=%s\n", bp->id/*bp->tv.tv_sec*/, (bp->dir == packet_dir_from_client ? "client" : "server"), packet_get_type(bp->p), packet_get_type_str(bp->p, bp->dir), packet_get_size(bp->p), packet_get_class_str(bp->p));
		hexdump(stdout, packet_get_raw_data(bp->p, 0), packet_get_size(bp->p));
		std::printf("\n");
	}
	pcap_close(pc);
	return 0;
}
