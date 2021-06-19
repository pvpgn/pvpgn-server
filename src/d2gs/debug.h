#ifndef INCLUDED_DEBUG_H
#define INCLUDED_DEBUG_H

#include "d2gs.h"


namespace pvpgn
{

	namespace d2gs
	{

#ifdef DEBUG
		extern void DebugEventCallback(char const*, int, ...);
		extern void DebugNetPacket(D2GSPACKET* lpPacket);
#else
#define	DebugEventCallback
#define DebugNetPacket
#endif

	}

}


#endif
