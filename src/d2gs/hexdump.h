#ifndef INCLUDED_HEXDUMP_H
#define INCLUDED_HEXDUMP_H

#include <stdio.h>


namespace pvpgn
{

	namespace d2gs
	{

		extern FILE* hexstrm;
		extern void hexdump(void const* data, unsigned int len);

	}

}

#endif
