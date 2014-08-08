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

#ifndef __PEERCHAT_INCLUDED__
#define __PEERCHAT_INCLUDED__

namespace pvpgn
{


	typedef struct {
		unsigned char   gs_peerchat_1;
		unsigned char   gs_peerchat_2;
		unsigned char   gs_peerchat_crypt[256];
	} gs_peerchat_ctx;

	extern gs_peerchat_ctx * gs_peerchat_create();
	extern void gs_peerchat_destroy(gs_peerchat_ctx const * ctx);
	extern void gs_peerchat_init(gs_peerchat_ctx *ctx, unsigned char *chall, unsigned char *gamekey);
	extern void gs_peerchat(gs_peerchat_ctx *ctx, unsigned char *data, int size);

}

#endif /* __PEERCHAT_INCLUDED__ */


