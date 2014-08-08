/*
 * Copyright (C) 2000,2001	Onlyer	(onlyer@263.net)
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
#ifndef INCLUDED_D2CHARFILE_H
#define INCLUDED_D2CHARFILE_H

/* only used in char convert */
#define D2CHARSAVE_VERSION_OFFSET			0x04
#define D2CHARSAVE_CHECKSUM_OFFSET			0x0C

/* newbie.save offset or old version */
#define D2CHARSAVE_STATUS_OFFSET			0x18	
#define D2CHARSAVE_CLASS_OFFSET				0x22
#define D2CHARSAVE_CHARNAME_OFFSET			0x08

/* 1.09 or later version (1.10) */
#define D2CHARSAVE_CLASS_OFFSET_109			0x28
#define D2CHARSAVE_STATUS_OFFSET_109			0x24
#define D2CHARSAVE_CHARNAME_OFFSET_109			0x14


/* unused */
#define D2CHARSAVE_LEVEL_OFFSET				0x24

#define D2CHAR_MAX_CLASS				0x06

#define D2CHARINFO_PORTRAIT_PADBYTE			0xff
#define D2CHARINFO_PORTRAIT_HEADER			0x8084
#define D2CHARINFO_PORTRAIT_MASK			0x80

#define D2CHARINFO_STATUS_FLAG_INIT			0x01
#define D2CHARINFO_STATUS_FLAG_EXPANSION		0x20
#define D2CHARINFO_STATUS_FLAG_LADDER			0x40
#define D2CHARINFO_STATUS_FLAG_HARDCORE			0x04
#define D2CHARINFO_STATUS_FLAG_DEAD			0x08

#define D2CHARINFO_STATUS_FLAG_INIT_MASK		(D2CHARINFO_STATUS_FLAG_INIT | \
	D2CHARINFO_STATUS_FLAG_EXPANSION| D2CHARINFO_STATUS_FLAG_LADDER |D2CHARINFO_STATUS_FLAG_HARDCORE)

#include "bit.h"

#define charstatus_set_init(status,n)		BIT_SET_CLR_FLAG(status, D2CHARINFO_STATUS_FLAG_INIT, n)
#define charstatus_set_expansion(status,n)	BIT_SET_CLR_FLAG(status, D2CHARINFO_STATUS_FLAG_EXPANSION, n)
#define charstatus_set_ladder(status,n)		BIT_SET_CLR_FLAG(status, D2CHARINFO_STATUS_FLAG_LADDER, n)
#define charstatus_set_hardcore(status,n)	BIT_SET_CLR_FLAG(status, D2CHARINFO_STATUS_FLAG_HARDCORE, n)
#define charstatus_set_dead(status,n)		BIT_SET_CLR_FLAG(status, D2CHARINFO_STATUS_FLAG_DEAD, n)


#define charstatus_get_init(status)		BIT_TST_FLAG(status, D2CHARINFO_STATUS_FLAG_INIT)
#define	charstatus_get_expansion(status)	BIT_TST_FLAG(status, D2CHARINFO_STATUS_FLAG_EXPANSION)
#define	charstatus_get_ladder(status)		BIT_TST_FLAG(status, D2CHARINFO_STATUS_FLAG_LADDER)
#define charstatus_get_hardcore(status)		BIT_TST_FLAG(status, D2CHARINFO_STATUS_FLAG_HARDCORE)
#define charstatus_get_dead(status)		BIT_TST_FLAG(status, D2CHARINFO_STATUS_FLAG_DEAD)
#define charstatus_get_difficulty(status)		((( status >> 0x08) & 0x0f )/4)	/* number of act = 4 */
#define charstatus_get_difficulty_expansion(status)	((( status >> 0x08) & 0x0f )/5)	/* number of act = 5 */


#ifndef JUST_NEED_TYPES
#include "common/d2cs_d2gs_character.h"

namespace pvpgn
{

namespace d2cs
{

extern int d2char_create(char const * account, char const * charname, unsigned char chclass,
			unsigned short status);
extern int d2char_delete(char const * account, char const * charname);
extern int d2char_get_summary(char const * account, char const * charname,t_d2charinfo_summary * charinfo);
extern int d2char_get_portrait(char const * account, char const * filename, t_d2charinfo_portrait * portrait);
extern int d2char_portrait_init(t_d2charinfo_portrait * portrait);
extern int d2charinfo_load(char const * account, char const * charname, t_d2charinfo_file * data);
extern int d2charinfo_check(t_d2charinfo_file * data);
extern unsigned int d2charinfo_get_expansion(t_d2charinfo_summary const * charinfo);
extern unsigned int d2charinfo_get_level(t_d2charinfo_summary const * charinfo);
extern unsigned int d2charinfo_get_class(t_d2charinfo_summary const * charinfo);
extern unsigned int d2charinfo_get_hardcore(t_d2charinfo_summary const * charinfo);
extern unsigned int d2charinfo_get_ladder(t_d2charinfo_summary const * charinfo);
extern unsigned int d2charinfo_get_dead(t_d2charinfo_summary const * charinfo);
extern unsigned int d2charinfo_get_difficulty(t_d2charinfo_summary const * charinfo);
extern int d2char_convert(char const * account, char const * charname);
extern int d2char_find(char const * account, char const * charname);
extern int d2char_get_savefile_name(char * filename,char const * charname);
extern int d2char_get_infofile_name(char * filename,char const * account, char const * charname);
extern int d2char_get_bak_savefile_name(char * filename,char const * charname);
extern int d2char_get_bak_infofile_name(char * filename,char const * account, char const * charname);
extern int d2char_get_infodir_name(char * filename,char const * account);
extern int d2char_check_acctname(char const * name);
extern int d2char_check_charname(char const * name);

extern int file_write(char const * filename, void * data, unsigned int size);
extern int file_read(char const * filename, void * data, unsigned int * size);

}

}

#endif


#endif
