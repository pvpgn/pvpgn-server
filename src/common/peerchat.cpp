/*

GS peerchat encryption/decryption algorithm 0.2
by Luigi Auriemma
e-mail: aluigi@autistici.org
web:    aluigi.org


LICENSE
=======
Copyright 2004,2005,2006 Luigi Auriemma

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

http://www.gnu.org/licenses/gpl.txt

*/

#include "common/setup_before.h"
#include "common/peerchat.h"

#include "common/eventlog.h"
#include "common/xalloc.h"

#include "common/setup_after.h"

namespace pvpgn
{

	extern gs_peerchat_ctx * gs_peerchat_create()
	{
		gs_peerchat_ctx * temp;

		temp = (gs_peerchat_ctx*)xmalloc(sizeof(gs_peerchat_ctx));

		return temp;
	}

	extern void  gs_peerchat_destroy(gs_peerchat_ctx const * ctx)
	{
		if (!ctx) {
			ERROR0("got NULL ctx");
			return;
		}

		xfree((void *)ctx); /* avoid warning */
	}

	void gs_peerchat_init(gs_peerchat_ctx *ctx, unsigned char *chall, unsigned char *gamekey) {
		unsigned char   challenge[16],
			*l,
			*l1,
			*p,
			*p1,
			*crypt,
			t,
			t1;

		ctx->gs_peerchat_1 = 0;
		ctx->gs_peerchat_2 = 0;
		crypt = ctx->gs_peerchat_crypt;

		p = challenge;
		l = challenge + 16;
		p1 = gamekey;
		l1 = gamekey + 6;
		do {
			*p++ = *chall++ ^ *p1++;    // avoid a memcpy(challenge, chall, 16);
			if (p1 == l1) p1 = gamekey;
		} while (p != l);

		t1 = 255;
		p1 = crypt;
		l1 = crypt + 256;
		do { *p1++ = t1--; } while (p1 != l1);

		t1++;       // means t1 = 0;
		p = crypt;
		p1 = challenge;
		do {
			t1 += *p1 + *p;
			t = crypt[t1];
			crypt[t1] = *p;
			*p = t;
			p++;
			p1++;
			if (p1 == l) p1 = challenge;
		} while (p != l1);

		//   DEBUG3("initialised: %u/%u/%s",ctx->gs_peerchat_1,ctx->gs_peerchat_2,ctx->gs_peerchat_crypt);
	}

	void gs_peerchat(gs_peerchat_ctx *ctx, unsigned char *data, int size) {
		unsigned char   num1,
			num2,
			t,
			*crypt;

		num1 = ctx->gs_peerchat_1;
		num2 = ctx->gs_peerchat_2;
		crypt = ctx->gs_peerchat_crypt;

		while (size--) {
			t = crypt[++num1];
			num2 += t;
			crypt[num1] = crypt[num2];
			crypt[num2] = t;
			t += crypt[num1];
			*data++ ^= crypt[t];
		}

		ctx->gs_peerchat_1 = num1;
		ctx->gs_peerchat_2 = num2;
	}

}
