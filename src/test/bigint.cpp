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

#include "common/bigint.h"

#include <string>
#include <iostream>
#include <cassert>

#include "common/xalloc.h"

#include "compat/uint.h"

#include "common/setup_after.h"

using  namespace pvpgn;

const unsigned char data1[] = { 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef };
const unsigned char data2[] = { 0x12, 0xff, 0x34, 0xff, 0x56, 0xff, 0x78, 0xff, 0x90, 0xff, 0xab, 0xff, 0xcd, 0xff, 0xef, 0xff };
const unsigned char data3[] = { 0x12, 0x34, 0x56, 0x78 };
const unsigned char data4[] = { 0xfe, 0xdc, 0xba, 0x09, 0x87, 0x65, 0x43, 0x21, 0x12, 0x34, 0x56, 0x78, 0x90, 0xab, 0xcd, 0xef };

void constructorTests()
{
	// std::cout << __FUNCTION__ << "\n";
	BigInt* number;
	number = new BigInt();
	assert(number->toHexString() == "00");
	delete number;

	number = new BigInt((t_uint8)0xFF);
	assert(number->toHexString() == "ff");
	delete number;

	number = new BigInt((t_uint16)0xFFFF);
	assert(number->toHexString() == "ffff");
	delete number;

	number = new BigInt((t_uint32)0xFFFFFFFF);
	assert(number->toHexString() == "ffffffff");
	delete number;

	number = new BigInt(data1, 8);
	assert(number->toHexString() == "1234567890abcdef");
	delete number;

	number = new BigInt(data1, 7);
	assert(number->toHexString() == "1234567890abcd");
	delete number;

	number = new BigInt(data2, 16);
	assert(number->toHexString() == "12ff34ff56ff78ff90ffabffcdffefff");
	delete number;
}

void getDataTests()
{
	int i;
	unsigned char* data;
	data = BigInt(data4, 16).getData(16);
	for (i = 0; i < 16; i++)
	{
		assert(data[i] = data4[i]);
	}
	xfree(data);
	data = BigInt((t_uint32)0x12345678).getData(3);
	assert(BigInt(data, 3) == BigInt((t_uint32)0x345678));
	xfree(data);
	data = BigInt((t_uint32)0x12345678).getData(2);
	assert(BigInt(data, 2) == BigInt((t_uint16)0x5678));
	xfree(data);
	data = BigInt((t_uint32)0x12345678).getData(1);
	assert(BigInt(data, 1) == BigInt((t_uint8)0x78));
	xfree(data);
}

void compareOperatorsTests()
{
	// std::cout << __FUNCTION__ << "\n";
	assert(BigInt((t_uint32)0x12345678) == BigInt(data3, 4));
	assert(BigInt((t_uint16)0x27a2) < BigInt((t_uint16)0x9876));
	assert(BigInt() < BigInt((t_uint16)0x9876));
	assert(BigInt((t_uint16)0x9876) > BigInt());
}

void addTests()
{
	// std::cout << __FUNCTION__ << "\n";
	assert(BigInt((t_uint32)0x12344321) + BigInt((t_uint32)0x43211234) == BigInt((t_uint32)0x55555555));
	assert(BigInt((t_uint16)0xFFFF) + BigInt((t_uint8)0x01) == BigInt((t_uint32)0x00010000));
}

void subTests()
{
	// std::cout << __FUNCTION__ << "\n";
	assert(BigInt((t_uint32)0x00010000) - BigInt((t_uint8)0x01) == BigInt((t_uint16)0xFFFF));
	assert(BigInt((t_uint32)0x12345678) - BigInt((t_uint16)0x9876) == BigInt((t_uint32)0x1233BE02));
	assert(BigInt((t_uint8)0x10) - BigInt((t_uint8)0xFF) == BigInt((t_uint8)0x00));
}

void mulTests()
{
	// std::cout << __FUNCTION__ << "\n";
	assert(BigInt((t_uint8)0x02) * BigInt((t_uint8)0xff) == BigInt((t_uint16)0x01fe));
	const unsigned char data4[] = { 0x01, 0x4b, 0x66, 0xdc, 0x1d, 0xf4, 0xd8, 0x40 };
	assert(BigInt(data3, 4) * BigInt(data3, 4) == BigInt(data4, 8));
}

void divTests()
{
	// std::cout << __FUNCTION__ << "\n";
	assert(BigInt((t_uint16)0x9876) / BigInt((t_uint32)0x98765432) == BigInt());
	assert(BigInt((t_uint32)0x1e0f7fbc) / BigInt((t_uint16)0x1e2f) == BigInt((t_uint16)0xfef4));
	assert(BigInt((t_uint32)0x01000000) / BigInt((t_uint32)0x00FFFFFF) == BigInt((t_uint8)0x01));
	assert(BigInt(data4, 16) / BigInt((t_uint8)0x02) > BigInt());
}

void modTests()
{
	// std::cout << __FUNCTION__ << "\n";
	assert(BigInt((t_uint32)0x1e0f7fbc) % BigInt((t_uint16)0x1e2f) == BigInt((t_uint16)0x18f0));
	assert(BigInt((t_uint32)0x01000000) % BigInt((t_uint32)0x00FFFFFF) == BigInt((t_uint8)0x01));
	assert(BigInt((t_uint32)0x80000000) % BigInt((t_uint32)0xFFFFFFFF) == BigInt((t_uint32)0x80000000));

}

void randTests()
{
	// std::cout << __FUNCTION__ << "\n";
	assert(BigInt::random(8) > BigInt());
	assert(BigInt::random(16) > BigInt());
	assert(BigInt::random(32) > BigInt());
	assert(BigInt::random(64) > BigInt());
	assert(BigInt::random(128) > BigInt());
}

void powmTests()
{
	// std::cout << __FUNCTION__ << "\n";
	assert(BigInt((t_uint8)0x02).powm(BigInt((t_uint8)0x1F), BigInt((t_uint32)0xFFFFFFFF)) == BigInt((t_uint32)0x80000000));
	BigInt mod = BigInt::random(32);
	assert(BigInt((t_uint8)0x2f).powm(BigInt::random(32), mod) < mod);
}

int main()
{
	constructorTests();
	getDataTests();
	compareOperatorsTests();
	addTests();
	subTests();
	mulTests();
	divTests();
	modTests();
	randTests();
	powmTests();
	return 0;
}
