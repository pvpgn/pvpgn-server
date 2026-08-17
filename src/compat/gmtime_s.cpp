/*
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
#include "gmtime_s.h"

#include <cerrno>
#include <ctime>

#include "common/setup_after.h"


namespace pvpgn
{

	struct std::tm* gmtime_s(const std::time_t* timer, struct std::tm* buf)
	{
		if (!timer || !buf)
		{
			errno = EINVAL;
			return nullptr;
		}

#if defined(HAVE_GMTIME_S)
		return ::gmtime_s(timer, buf);
#elif defined(HAVE_MICROSOFT_GMTIME_S)
		errno_t err = ::gmtime_s(buf, timer);
		if (err == 0)
		{
			return buf;
		}
		else
		{
			errno = err;
			return nullptr;
		}
#elif defined(HAVE_GMTIME_R)
		return gmtime_r(timer, buf);
#else
#error "Both gmtime_s() and gmtime_r() are not available"
#endif
	}

}