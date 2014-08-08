#ifndef BNETD_TRACKER_H_INCLUDED
#define BNETD_TRACKER_H_INCLUDED

#include "common/addr.h"

namespace pvpgn
{

	namespace bnetd
	{

		extern int tracker_set_servers(char const * servers);
		extern int tracker_send_report(t_addrlist const * addrs);

	}

}

#endif
