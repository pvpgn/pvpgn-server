/*
 * Copyright (C) 2001  Rob Crittenden (rcrit@greyoak.com)
 * Copyright (C) 2001  Ross Combs (rocombs@cs.nmsu.edu)
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
//#define PREFS_INTERNAL_ACCESS
#include "common/setup_before.h"
#include "token.h"

#include <cctype>

#include "common/eventlog.h"
#include "common/setup_after.h"

namespace pvpgn
{

	/*
	 * Given a string and an integer pointer skip past pos characters and return
	 * the next white-space delimited string, setting pos to the new position.
	 */
	extern char * next_token(char * ptr, unsigned int * pos)
	{
		unsigned int i;
		unsigned int start;
		int          quoted;

		if (!ptr || !pos)
			return NULL;

		/* skip leading whitespace */
		for (i = *pos; std::isspace((int)ptr[i]); i++);

		if (ptr[i] == '\0')
			return NULL; /* if after whitespace, we're done */

		if (ptr[i] == '"')
		{
			quoted = 1;
			i++;
		}
		else
			quoted = 0;

		start = i;
		for (;;)
		{
			if (ptr[i] == '\0')
				break;
			if (quoted) /* FIXME: add handling of escape chars so quotes can be in tokens */
			{
				if (ptr[i] == '"')
					break;
			}
			else
			if (std::isspace((int)ptr[i]))
				break;
			i++;
		}

		if (ptr[i] != '\0')
		{
			ptr[i] = '\0'; /* terminate the string */
			*pos = i + 1; /* remember the position of the next char */
		}
		else
			*pos = i; /* this was the last token, just remember the NUL */

		return &ptr[start];
	}

}
