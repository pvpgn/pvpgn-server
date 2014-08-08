/*
 * Class that implements the SRP-3 based authentication schema
 * used by Blizzards WarCraft 3. Implementations is based upon
 * public information available under
 * http://www.javaop.com/@ron/documents/SRP.html
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

#ifndef __BNET_SRP3_INCLUDED__
#define __BNET_SRP3_INCLUDED__

#include <string>
#include "common/bigint.h"

namespace pvpgn
{

	class BnetSRP3
	{
	public:
		BnetSRP3(const char* username, BigInt& salt);
		BnetSRP3(const std::string& username, BigInt& salt);
		BnetSRP3(const char* username, const char* password);
		BnetSRP3(const std::string& username, const std::string& password);
		~BnetSRP3();
		BigInt getVerifier() const;
		BigInt getSalt() const;
		void   setSalt(BigInt salt);
		BigInt getClientSessionPublicKey() const;
		BigInt getServerSessionPublicKey(BigInt& v);
		BigInt getHashedClientSecret(BigInt& B) const;
		BigInt getHashedServerSecret(BigInt& A, BigInt& v);
		BigInt getClientPasswordProof(BigInt& A, BigInt& B, BigInt& K) const;
		BigInt getServerPasswordProof(BigInt& A, BigInt& M, BigInt& K) const;

	private:
		int	init(const char* username, const char* password, BigInt* salt);
		BigInt	getClientPrivateKey() const;
		BigInt	getScrambler(BigInt& B) const;
		BigInt	getClientSecret(BigInt& B) const;
		BigInt	getServerSecret(BigInt& A, BigInt& v);
		BigInt  hashSecret(BigInt& secret) const;
		static BigInt	N;	// modulus
		static BigInt	g;	// generator
		static BigInt	I;	// H(g) xor H(N) where H() is standard SHA1
		BigInt	a;	// client session private key
		BigInt	b;	// server session private key
		BigInt	s;	// salt
		BigInt *B;	// server public key cache
		char*	username;
		size_t	username_length;
		char*	password;
		size_t	password_length;
		unsigned char raw_salt[32];
	};

}

#endif /* __BNET_SRP3_INCLUDED__ */
