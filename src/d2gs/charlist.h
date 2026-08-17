#ifndef INCLUDED_CHARLIST_H
#define INCLUDED_CHARLIST_H

#include "d2gs.h"


namespace pvpgn
{

	namespace d2gs
	{

		/*
		 * from bnet/common/introtate.h
		 * Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)
		 */
		 /* ROTL(x,n,w) rotates "w" bit wide value "x" by "n" bits to the left */
#ifndef ROTL
#define ROTL(x,n,w) (((n)%(w)) ? (((x)<<((n)%(w))) | ((x)>>((w)-((n)%(w))))) : (x))
#endif
#ifndef ROTL32
#define ROTL32(x,n) ROTL(x,n,32)
#endif


/* const */
#define DEFAULT_HASHTBL_LEN		1000

#define CHARLIST_GET_CHARINFO	1
#define CHARLIST_GET_GAMEINFO	2

/* charlist info */
		typedef struct RAW_D2CHARLIST
		{
			char			charname[MAX_CHARNAME_LEN];
			void* pCharInfo;
			void* pGameInfo;
			struct RAW_D2CHARLIST* next;
		} D2CHARLIST, * PD2CHARLIST, * LPD2CHARLIST;


		/* functions */
		unsigned int string_hash(char const* string);
		int charlist_init(unsigned int tbllen);
		int charlist_destroy(void);
		void charlist_flush(void);
		void* charlist_getdata(const char* charname, int type);
		int charlist_insert(const char* charname, void* pCharInfo, void* pGameInfo);
		int charlist_delete(const char* charname);

	}

}


#endif /* INCLUDED_CHARLIST_H */