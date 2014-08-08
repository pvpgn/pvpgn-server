/*
 * Class for simple, arbitrary size unsigned integer math.
 * Note that some method implementations might lack features that weren't required so far!
 *
 * Copyright (C) 2008 - Olaf Freyer
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation; either version 3
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 */

#include "common/setup_before.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "bigint.h"
#include "common/xalloc.h"

#include "compat/uint.h"

#include "common/setup_after.h"
#include <algorithm>

namespace pvpgn
{

	static const char small_digits[] = "0123456789abcdef";

	BigInt::BigInt()
	{
		segment_count = 1;
		segment = (bigint_base*)xmalloc(segment_count * sizeof(bigint_base));
		segment[0] = 0;
	}

	BigInt::BigInt(t_uint8 input)
	{
		segment_count = 1;
		segment = (bigint_base*)xmalloc(segment_count * sizeof(bigint_base));
		segment[0] = input;
	}

	BigInt::BigInt(t_uint16 input)
	{
		segment_count = 1;
		segment = (bigint_base*)xmalloc(segment_count * sizeof(bigint_base));
		segment[0] = input;
	}

	BigInt::BigInt(t_uint32 input)
	{
#ifndef HAVE_UINT64_T
		int i;
#endif
		segment_count = sizeof(t_uint32) / sizeof(bigint_base);
		segment = (bigint_base*)xmalloc(segment_count * sizeof(bigint_base));
#ifdef HAVE_UINT64_T
		segment[0] = input;
#else
		for (i = 0; i < segment_count; i++){
			segment[i] = input & bigint_base_mask;
			input >>= bigint_base_bitcount;
		}
#endif
	}

#ifdef HAVE_UINT64_T
	BigInt::BigInt(t_uint64 input)
	{
		int i;
		segment_count = sizeof(t_uint64) / sizeof(bigint_base);
		segment = (bigint_base*)xmalloc(segment_count * sizeof(bigint_base));
		for (i = 0; i < segment_count; i++){
			segment[i] = input & bigint_base_mask;
			input >>= bigint_base_bitcount;
		}
	}
#endif

	BigInt::BigInt(const BigInt& input)
		:segment_count(input.segment_count)
	{
		segment = (bigint_base*)xmalloc(segment_count * sizeof(bigint_base));
		std::memcpy(segment, input.segment, segment_count * sizeof(bigint_base));
	}

	BigInt&
		BigInt::operator=(const BigInt& input)
	{
			if (&input != this) {
				segment_count = input.segment_count;
				segment = (bigint_base*)xrealloc(segment, segment_count * sizeof(bigint_base));
				std::memcpy(segment, input.segment, segment_count * sizeof(bigint_base));
			}
			return *this;
		}

	BigInt::BigInt(unsigned char const * input, int input_size, int blockSize, bool bigEndian)
	{
		int i, j;
		unsigned char *in;
		unsigned char *inPointer;

		if (bigEndian){
			in = (unsigned char*)input;
			inPointer = (unsigned char*)in;
		}
		else {
			in = (unsigned char*)xmalloc(input_size);
			inPointer = (unsigned char*)in + input_size - 1;
			if (blockSize == 1)
				std::memcpy(in, input, input_size);
			else {
				assert(blockSize % 2 == 0);
				for (i = 0; i < input_size; i += blockSize)
				{
					for (j = 0; j < blockSize / 2; j++)
					{
						in[i + j] = input[i + blockSize - (j + 1)];
						in[i + blockSize - (j + 1)] = input[i + j];
					}
				}
			}
		}

		segment_count = input_size / sizeof(bigint_base);
		if (input_size % sizeof(bigint_base))
			segment_count++;
		segment = (bigint_base*)xmalloc(segment_count * sizeof(bigint_base));
		std::memset(segment, 0, segment_count * sizeof(bigint_base));


		for (i = input_size - 1; i >= 0; i--)
		{
			j = i / sizeof(bigint_base);
			segment[j] <<= 8;
			if (bigEndian)
				segment[j] += *(inPointer++);
			else
				segment[j] += *(inPointer--);
		}

		if (!bigEndian)
			xfree(in);
	}

	BigInt::~BigInt() throw()
	{
		if (segment)
			xfree(segment);
	}

	bool
		BigInt::operator== (const BigInt& right) const
	{
			int i;
			bool result;

			result = (segment_count == right.segment_count);
			for (i = 0; (i < segment_count) && result; i++)
				result = result && (segment[i] == right.segment[i]);

			return result;
		}

	bool
		BigInt::operator< (const BigInt& right) const
	{
			if (segment_count < right.segment_count) {
				return true;
			}
			else if (segment_count > right.segment_count){
				return false;
			}
			else {
				int i;
				for (i = segment_count - 1; i >= 0; i--) {
					if (segment[i]<right.segment[i])
						return true;
					else if (segment[i]>right.segment[i])
						return false;
				}
			}

			return false;
		}
	bool
		BigInt::operator> (const BigInt& right) const
	{
			if (segment_count > right.segment_count) {
				return true;
			}
			else if (segment_count < right.segment_count){
				return false;
			}
			else {
				int i;
				for (i = segment_count - 1; i >= 0; i--) {
					if (segment[i]>right.segment[i])
						return true;
					else if (segment[i] < right.segment[i])
						return false;
				}
			}

			return false;
		}

	BigInt
		BigInt::operator+ (const BigInt& right) const
	{
			int i, max_segment_count;
			bigint_extended sum;
			bigint_extended lhs, rhs;
			bigint_base carry = 0;
			BigInt result;

			max_segment_count = std::max(segment_count, right.segment_count);
			result.segment_count = max_segment_count + 1;
			result.segment = (bigint_base*)xrealloc(result.segment, result.segment_count * sizeof(bigint_base));

			for (i = 0; i < max_segment_count; i++)
			{
				lhs = (i < segment_count) ? segment[i] : 0;
				rhs = (i<right.segment_count) ? right.segment[i] : 0;
				sum = lhs + rhs + carry;
				result.segment[i] = sum & bigint_base_mask;
				carry = sum >> bigint_base_bitcount;
			}

			result.segment[i] = carry;

			for (max_segment_count = result.segment_count - 1; max_segment_count>0; max_segment_count--) {
				if (result.segment[max_segment_count] != 0)
					break;
			}

			if (result.segment_count != max_segment_count + 1)
			{
				result.segment_count = max_segment_count + 1;
				result.segment = (bigint_base*)xrealloc(result.segment, result.segment_count * sizeof(bigint_base));
			}

			return result;
		}

	BigInt
		BigInt::operator- (const BigInt& right) const
	{
			int i, max_segment_count;
			bigint_base diff;
			bigint_base lhs, rhs;
			bigint_base carry = 0;
			BigInt result;

			// Currently we only implement unsigned math
			// so we can either throw exception or simply return 0.
			if (!(this->operator>(right)))
			{
				return result;
			}

			result.segment_count = segment_count;
			result.segment = (bigint_base*)xrealloc(result.segment, result.segment_count * sizeof(bigint_base));
			for (i = 0; i < result.segment_count; i++)
			{
				lhs = (i < segment_count) ? segment[i] : 0;
				rhs = (i < right.segment_count) ? right.segment[i] : 0;
				rhs += carry;
				if (lhs < rhs)
				{
					diff = (bigint_extended_carry + lhs) - rhs;
					carry = 1;
				}
				else
				{
					diff = (lhs)-rhs;
					carry = 0;
				}
				result.segment[i] = diff;
			}

			for (max_segment_count = result.segment_count - 1; max_segment_count > 0; max_segment_count--) {
				if (result.segment[max_segment_count] != 0)
					break;
			}

			if (result.segment_count != max_segment_count + 1)
			{
				result.segment_count = max_segment_count + 1;
				result.segment = (bigint_base*)xrealloc(result.segment, result.segment_count * sizeof(bigint_base));
			}

			return result;
		}

	BigInt
		BigInt::operator* (const BigInt& right) const
	{
			int i, j, index, max_segment_count;
			bigint_extended prod, sum;
			bigint_extended lhs, rhs;
			bigint_base carry = 0;
			BigInt result;

			if ((segment_count == 1 && segment[0] == 0) || (right.segment_count == 1 && right.segment[0] == 0))
				return result;

			result.segment_count = segment_count + right.segment_count;
			result.segment = (bigint_base*)xrealloc(result.segment, result.segment_count * sizeof(bigint_base));
			std::memset(result.segment, 0, result.segment_count * sizeof(bigint_base));
			for (i = 0; i < segment_count; i++)
			{
				for (j = 0; j < right.segment_count; j++)
				{
					lhs = (i < segment_count) ? segment[i] : 0;
					rhs = (j < right.segment_count) ? right.segment[j] : 0;
					prod = lhs * rhs;
					index = i + j;
					sum = result.segment[index] + prod + carry;
					result.segment[index] = sum & bigint_base_mask;
					carry = sum >> bigint_base_bitcount;
				}

				if (carry) {
					index = i + j;
					sum = result.segment[index] + carry;
					result.segment[index] = sum & bigint_base_mask;
					carry = sum >> bigint_base_bitcount;
				}
			}

			result.segment[i + j - 1] += carry;

			for (max_segment_count = result.segment_count - 1; max_segment_count > 0; max_segment_count--) {
				if (result.segment[max_segment_count] != 0)
					break;
			}

			if (result.segment_count != max_segment_count + 1)
			{
				result.segment_count = max_segment_count + 1;
				result.segment = (bigint_base*)xrealloc(result.segment, result.segment_count * sizeof(bigint_base));
			}

			return result;
		}

	BigInt
		BigInt::operator/ (const BigInt& right) const
	{
			int i, j, max_segment_count;
			BigInt quotient;
			BigInt remainder;
			BigInt m;
			BigInt q;
			BigInt p;
			bigint_extended n, d, qest;


			if (this->operator<(right)){
				return quotient;
			}

			quotient.segment_count = (segment_count - right.segment_count) + 1;
			quotient.segment = (bigint_base*)xrealloc(quotient.segment, quotient.segment_count*sizeof(bigint_base));
			std::memset(quotient.segment, 0, quotient.segment_count * sizeof(bigint_base));

			remainder.segment_count = right.segment_count + 1;
			remainder.segment = (bigint_base*)xrealloc(remainder.segment, remainder.segment_count*sizeof(bigint_base));
			std::memset(remainder.segment, 0, remainder.segment_count * sizeof(bigint_base));

			for (j = 0; j < right.segment_count; j++){
				remainder.segment[j] = segment[(segment_count - right.segment_count) + j];
			}

			n = 0;
			for (i = segment_count; i >= right.segment_count; i--){
				//now do some "educated guessing"
				//qest=n/q
				//calculate qest*right
				// adjust by +/-1 until in target range
				n = remainder.segment[right.segment_count];
				n <<= bigint_base_bitcount;
				n += remainder.segment[right.segment_count - 1];
				d = right.segment[right.segment_count - 1];

				qest = n / d;
				p = BigInt(qest) * right;
				while (1) {
					if (p == remainder) {
						break;
					}
					else if (p < remainder) {
						if ((remainder - p) > right) {
							qest++;
							p = p + right;
						}
						else
						{
							break;
						}
					}
					else {
						qest--;
						p = p - right;
					}
				};

				quotient.segment[i - right.segment_count] = qest & bigint_base_mask;
				remainder = remainder - p;

				if (i > right.segment_count) {
					remainder = remainder << sizeof(bigint_base);
					remainder.segment[0] = segment[i - (right.segment_count + 1)];
				}
			}

			for (max_segment_count = quotient.segment_count - 1; max_segment_count > 0; max_segment_count--) {
				if (quotient.segment[max_segment_count] != 0)
					break;
			}

			if (quotient.segment_count != max_segment_count + 1)
			{
				quotient.segment_count = max_segment_count + 1;
				quotient.segment = (bigint_base*)xrealloc(quotient.segment, quotient.segment_count * sizeof(bigint_base));
			}

			return quotient;
		}

	BigInt
		BigInt::operator% (const BigInt& right) const
	{
			int i, j;
			BigInt remainder;
			BigInt m;
			BigInt q;
			BigInt p;
			bigint_extended n, d, qest;


			if (this->operator<(right)){
				return *this;
			}

			remainder.segment_count = right.segment_count + 1;
			remainder.segment = (bigint_base*)xrealloc(remainder.segment, remainder.segment_count*sizeof(bigint_base));
			std::memset(remainder.segment, 0, remainder.segment_count * sizeof(bigint_base));

			for (j = 0; j < right.segment_count; j++){
				remainder.segment[j] = segment[(segment_count - right.segment_count) + j];
			}

			n = 0;
			for (i = segment_count; i >= right.segment_count; i--){
				//now do some "educated guessing"
				//qest=n/q
				//calculate qest*right
				// adjust by +/-1 until in target range
				n = remainder.segment[right.segment_count];
				n <<= bigint_base_bitcount;
				n += remainder.segment[right.segment_count - 1];
				d = right.segment[right.segment_count - 1];

				qest = n / d;
				p = BigInt(qest) * right;
				while (1) {
					if (p == remainder) {
						break;
					}
					else if (p < remainder) {
						if ((remainder - p) > right) {
							qest++;
							p = p + right;
						}
						else
						{
							break;
						}
					}
					else {
						qest--;
						p = p - right;
					}
				};


				remainder = remainder - p;

				if (i > right.segment_count) {
					remainder = remainder << sizeof(bigint_base);
					remainder.segment[0] = segment[i - (right.segment_count + 1)];
				}
			}

			return remainder;
		}

	BigInt
		BigInt::operator<< (int bytesToShift) const {
			int i;
			BigInt result;
			assert(bytesToShift >= 0);
			//currently we only implement (and need) segmentwise shifting
			assert(bytesToShift%sizeof(bigint_base) == 0);

			if (bytesToShift == 0)
				return result;

			int segmentsToShift = bytesToShift / sizeof(bigint_base);
			result.segment_count = segment_count + segmentsToShift;
			result.segment = (bigint_base*)xrealloc(result.segment, result.segment_count * sizeof(bigint_base));

			for (i = segment_count - 1; i >= 0; i--) {
				result.segment[i + segmentsToShift] = segment[i];
			}

			for (i = 0; i < segmentsToShift; i++) {
				result.segment[i] = 0;
			}

			return result;
		}

	BigInt
		BigInt::random(int size)
	{
			BigInt result;
			int i, j;
			unsigned int r;

			assert(size > 0);
			assert(size%sizeof(bigint_base) == 0);

			result.segment_count = size / sizeof(bigint_base);
			result.segment = (bigint_base*)xrealloc(result.segment, result.segment_count * sizeof(bigint_base));

			for (i = 0; i < result.segment_count; i++){
				result.segment[i] = 0;
				for (j = 0; j < sizeof(bigint_base); j += sizeof(unsigned int)){
					r = rand();
					result.segment[i] <<= (sizeof(unsigned int));
					result.segment[i] = result.segment[i] + r;
				}
			}

			return result;
		}

	BigInt
		BigInt::powm(const BigInt& exp, const BigInt& mod) const
	{
			if (exp.segment_count == 1)
			{
				if (exp.segment[0] == 0x02)
				{
					return (*this * *this) % mod;
				}
				else if (exp.segment[0] == 0x01)
				{
					return *this;
				}
				else if (exp.segment[0] == 0x00)
				{
					return BigInt((t_uint8)0x01);
				}
			}

			//trying a divide&conquer approach
			if (exp.segment[0] % 2 == 0)
			{
				// exp is even
				BigInt half = exp / (BigInt((t_uint8)0x02));
				BigInt halfpow = this->powm(half, mod);
				return (halfpow * halfpow) % mod;
			}
			else
			{
				// exp is odd
				BigInt half = exp / (BigInt((t_uint8)0x02));
				BigInt halfpow = this->powm(half, mod);
				return (((halfpow * halfpow) % mod) * *this) % mod;
			}
		}

	unsigned char*
		BigInt::getData(int byteCount, int blockSize, bool bigEndian) const
	{
			unsigned char* result;

			result = (unsigned char*)xmalloc(byteCount);
			getData(result, byteCount, blockSize, bigEndian);

			return result;
		}

	void
		BigInt::getData(unsigned char* out, int byteCount, int blockSize, bool bigEndian) const
	{
			int i, j;
			unsigned char* pos;

			std::memset(out, 0, byteCount);
			pos = out + (byteCount - 1);

			for (i = 0; i < segment_count; i++)
			{
				bigint_base data;
				data = segment[segment_count - (i + 1)];
				for (j = 0; j < sizeof(bigint_base); j++)
				{
					if (pos < out)
						break;
					*pos = data & 0xff;

					pos--;
					data >>= 8;
				}
			}

			if (!bigEndian && blockSize>1) {
				unsigned char val;
				assert(blockSize % 2 == 0);
				for (i = 0; i < byteCount; i += blockSize)
				{
					for (j = 0; j < blockSize / 2; j++)
					{
						val = out[i + j];
						out[i + j] = out[i + blockSize - (j + 1)];
						out[i + blockSize - (j + 1)] = val;
					}
				}
			}
		}

	std::string
		BigInt::toHexString() const
	{
			int i;
			char data;
			std::ostringstream result;
			bigint_base* src;

			// handle most significant segment
			src = &segment[segment_count - 1];
			if (segment_count == 1 && *src == 0)
			{
				result << "00";
			}
			else
			{
				int sum = 0;
				for (i = sizeof(bigint_base)-1; i >= 0; i--)
				{
					data = ((*src) & (0xff << (i * 8))) >> (i * 8);
					sum += data;
					if (sum)
					{
						result << small_digits[(data & 0xf0) >> 4];
						result << small_digits[(data & 0x0f)];
					}
				}

				// handle rest
				for (src = &segment[segment_count - 2]; src >= segment; src--)
				{
					for (i = sizeof(bigint_base)-1; i >= 0; i--)
					{
						data = ((*src) & (0xff << (i * 8))) >> (i * 8);
						result << small_digits[(data & 0xf0) >> 4];
						result << small_digits[(data & 0x0f)];
					}
				}
			}

			return result.str();

		}

}
