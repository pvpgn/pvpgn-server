/*
 * Copyright (C) 2000  Rob Crittenden (rcrit@greyoak.com)
 * Copyright (C) 2002 Gianluigi Tiesi (sherpya@netfarm.it)
 * Copyright (C) 2004 CreepLord (creeplord@pvpgn.org)
 * Copyright (C) 2008 Pelish (pelish@gmail.com)
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
#ifndef INCLUDED_AUTOUPDATE_TYPES
#define INCLUDED_AUTOUPDATE_TYPES

#include "common/tag.h"

namespace pvpgn
{

	namespace bnetd
	{

#ifdef AUTOUPDATE_INTERNAL_ACCESS
		typedef struct
		{
			t_tag         archtag;
			t_tag         clienttag;
			char const *  versiontag;
			char const *  updatefile;  /* used for bnet *.mpq file or wol *.rtp file */
			char const *  path;        /* Used only for WOL FTP update */
		} t_autoupdate;
#endif

	}

}

#endif


/*****/
#ifndef JUST_NEED_TYPES
#ifndef INCLUDED_AUTOUPDATE_PROTOS
#define INCLUDED_AUTOUPDATE_PROTOS

namespace pvpgn
{

	namespace bnetd
	{

		extern int autoupdate_load(char const * filename);
		extern int autoupdate_unload(void);
		extern char * autoupdate_check(t_tag archtag, t_tag clienttag, t_tag gamelang, char const * versiontag, char const * sku);

	}

}

#endif
#endif
