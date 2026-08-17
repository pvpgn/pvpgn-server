#ifndef INCLUDED_BN_TYPES_H
#define INCLUDED_BN_TYPES_H


namespace pvpgn
{

	namespace d2gs
	{

		/* basic bn types */
		typedef unsigned char		bn_basic;
		typedef unsigned char		bn_char;
		typedef unsigned char		bn_byte;
		typedef unsigned short		bn_short;
		typedef unsigned int		bn_int;
		typedef unsigned long		bn_long;	/* 4 bytes, not 8 bytes, fix me */


		/* use network order or not? */
#ifdef USE_NBO
#define bn_htons(a)		(htons(a))
#define bn_ntohs(a)		(ntohs(a))
#define bn_htonl(a)		(htonl(a))
#define bn_ntohl(a)		(ntohl(a))
#else
#define bn_htons(a)		(a)
#define bn_ntohs(a)		(a)
#define bn_htonl(a)		(a)
#define bn_ntohl(a)		(a)
#endif

	}

}


#endif /* INCLUDED_BN_TYPES_H */