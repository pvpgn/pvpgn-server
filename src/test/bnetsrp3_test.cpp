/*
 * Copyright (C) 1999,2001  Ross Combs (rocombs@cs.nmsu.edu)
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
#include <cstdio>
#include <cassert>
#include "common/xalloc.h"
#include "common/util.h"
#include "common/bnetsrp3.h"
#include "common/setup_after.h"

using namespace pvpgn;

int main()
{
	char * output;
	unsigned char data[40];
	const unsigned char salt[] = { 0xB3, 0x46, 0x25, 0x10, 0x1D, 0xEA, 0x80, 0xB9, 0x92, 0xEB, 0x50, 0x4E, 0x84, 0x00, 0x06, 0xA1,
		0x7E, 0x77, 0x58, 0x66, 0x73, 0x89, 0x27, 0xF7, 0x14, 0x90, 0x2D, 0xA6, 0x3F, 0xCC, 0xF8, 0x52 };
	BnetSRP3 nls1("regen", "bogen");
	nls1.setSalt(BigInt(salt, 32));

	BigInt v = nls1.getVerifier();
	BigInt s = nls1.getSalt();
	BnetSRP3 nls2("power", s);

	//output= (char *)xmalloc(40*3+1);

	//s.getData(data,32);
	//str_to_hex(output,(const char*)data,32);
	//std::printf("salt: %s\n",output);

	//v.getData(data,32,4,false);
	//str_to_hex(output,(const char*)data,32);
	//std::printf("verifier: %s\n",output);

	BigInt A = nls1.getClientSessionPublicKey();
	//A.getData(data,32,4,false);
	//str_to_hex(output,(const char*)data,32);
	//std::printf("A: %s\n",output);

	BigInt B = nls2.getServerSessionPublicKey(v);
	//B.getData(data,32,4,false);
	//str_to_hex(output,(const char*)data,32);
	//std::printf("B: %s\n",output);

	BigInt K1 = nls1.getHashedClientSecret(B);
	//K1.getData(data,40,4,false);
	//str_to_hex(output,(const char*)data,40);
	//std::printf("K1: %s\n",output);

	BigInt K2 = nls2.getHashedServerSecret(A, v);
	//K2.getData(data,40,4,false);
	//str_to_hex(output,(const char*)data,40);
	//std::printf("K2: %s\n",output);

	assert(K1 == K2);

	//BigInt M1 = nls2.getClientPasswordProof(A, B, K1);
	//M1.getData(data,20,4,false);
	//str_to_hex(output,(const char*)data,20);
	//std::printf("M1: %s\n",output);

	//BigInt M2 = nls1.getServerPasswordProof(A, M1, K2);
	//M2.getData(data,20,4,false);
	//str_to_hex(output,(const char*)data,20);
	//std::printf("M2: %s\n",output);

	return 0;
}

