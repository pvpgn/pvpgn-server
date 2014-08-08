/*
 * Copyright (C) 1999  Ross Combs (rocombs@cs.nmsu.edu)
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
#ifndef INCLUDED_VIRTCONN_TYPES
#define INCLUDED_VIRTCONN_TYPES

typedef enum
{
	virtconn_class_bnet,
	virtconn_class_file,
	virtconn_class_bot,
	virtconn_class_none
} t_virtconn_class;

typedef enum
{
	virtconn_state_empty,
	virtconn_state_initial,
	virtconn_state_connected,
	virtconn_state_connecting
} t_virtconn_state;

#ifdef VIRTCONN_INTERNAL_ACCESS
#ifdef JUST_NEED_TYPES
#include "common/queue.h"
#else
#define JUST_NEED_TYPES
#include "common/queue.h"
#undef JUST_NEED_TYPES
#endif
#endif

typedef struct virtconn
#ifdef VIRTCONN_INTERNAL_ACCESS
{
	int              csd;       /* client side socket descriptor */
	int              ssd;       /* server side socket descriptor */
	t_virtconn_class class;     /* normal, file, or bot */
	t_virtconn_state state;     /* initial, connected, etc */
	unsigned short   udpport;   /* real port # to send UDP to */
	unsigned int     udpaddr;   /* real IP # to send UDP to */
	t_queue *        coutqueue; /* client packets waiting to be sent */
	unsigned int     coutsize;  /* client amount sent from the current output packet */
	t_queue *        cinqueue;  /* client packet waiting to be processed */
	unsigned int     cinsize;   /* client amount received into the current input packet */
	t_queue *        soutqueue; /* server packets waiting to be sent */
	unsigned int     soutsize;  /* server amount sent from the current output packet */
	t_queue *        sinqueue;  /* server packet waiting to be processed */
	unsigned int     sinsize;   /* server amount received into the current input packet */
	unsigned int     fileleft;  /* number of bytes in file download from server left */
}
#endif
t_virtconn;

#endif

#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_VIRTCONN_PROTOS
#define INCLUDED_VIRTCONN_PROTOS

#define JUST_NEED_TYPES
#include "common/packet.h"
#include "common/queue.h"
#include "common/list.h"
#undef JUST_NEED_TYPES

extern t_virtconn * virtconn_create(int csd, int ssd, unsigned int udpaddr, unsigned short udpport);
extern void virtconn_destroy(t_virtconn * vc);
extern t_virtconn_class virtconn_get_class(t_virtconn const * vc);
extern void virtconn_set_class(t_virtconn * vc, t_virtconn_class class);
extern t_virtconn_state virtconn_get_state(t_virtconn const * vc);
extern void virtconn_set_state(t_virtconn * vc, t_virtconn_state state);
extern unsigned int virtconn_get_udpaddr(t_virtconn const * vc);
extern unsigned short virtconn_get_udpport(t_virtconn const * vc);
extern t_queue * * virtconn_get_clientin_queue(t_virtconn * vc);
extern int virtconn_get_clientin_size(t_virtconn const * vc);
extern void virtconn_set_clientin_size(t_virtconn * vc, unsigned int size);
extern t_queue * * virtconn_get_clientout_queue(t_virtconn * vc);
extern int virtconn_get_clientout_size(t_virtconn const * vc);
extern void virtconn_set_clientout_size(t_virtconn * vc, unsigned int size);
extern int virtconn_get_client_socket(t_virtconn const * vc);
extern t_queue * * virtconn_get_serverin_queue(t_virtconn * vc);
extern int virtconn_get_serverin_size(t_virtconn const * vc);
extern void virtconn_set_serverin_size(t_virtconn * vc, unsigned int size);
extern t_queue * * virtconn_get_serverout_queue(t_virtconn * vc);
extern int virtconn_get_serverout_size(t_virtconn const * vc);
extern void virtconn_set_serverout_size(t_virtconn * vc, unsigned int size);
extern int virtconn_get_server_socket(t_virtconn const * vc);
extern void virtconn_set_fileleft(t_virtconn * vc, unsigned int size);
extern unsigned int virtconn_get_fileleft(t_virtconn const * vc);

extern int virtconnlist_create(void);
extern int virtconnlist_destroy(void);
extern t_list * virtconnlist(void);

#endif
#endif
