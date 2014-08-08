/*
 * Copyright (C) 2001		faster	(lqx@cic.tsinghua.edu.cn)
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
#ifndef INCLUDED_CHARLOCK_H
#define INCLUDED_CHARLOCK_H

#define DEFAULT_HASHTBL_LEN		65000

#include "common/field_sizes.h"

namespace pvpgn
{

	namespace d2dbs
	{

		/* char locking info */
		typedef struct raw_charlockinfo {
			unsigned char	charname[MAX_CHARNAME_LEN];
			unsigned char	realmname[MAX_REALMNAME_LEN];
			unsigned int	gsid;
			struct raw_charlockinfo	*next;
			struct raw_charlockinfo	*gsqnext;
			struct raw_charlockinfo	*gsqprev;
		} t_charlockinfo;


		/* functions */
		int cl_init(unsigned int tbllen, unsigned int maxgs);
		int cl_destroy(void);
		int cl_query_charlock_status(unsigned char *charname,
			unsigned char *realmnamei, unsigned int *gsid);
		int cl_lock_char(unsigned char *charname,
			unsigned char *realmname, unsigned int gsid);
		int cl_unlock_char(unsigned char *charname, unsigned char *realmname, unsigned int gsid);
		int cl_unlock_all_char_by_gsid(unsigned int gsid);

	}

}

#endif  /* INCLUDED_CHARLOCK_H */
