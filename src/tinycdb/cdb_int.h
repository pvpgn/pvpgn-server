/* internal cdb library declarations
 *
 * This file is a part of tinycdb package by Michael Tokarev, mjt@corpit.ru.
 * Public domain.
 */

#include "cdb.h"

#ifndef EPROTO
# define EPROTO EINVAL
#endif

namespace pvpgn
{

struct cdb_rec {
  cdbi_t hval;
  cdbi_t rpos;
};

struct cdb_rl {
  struct cdb_rl *next;
  cdbi_t cnt;
  struct cdb_rec rec[254];
};

int _cdb_make_find(struct cdb_make *cdbmp,
		   const void *key, cdbi_t klen, cdbi_t hval,
		   struct cdb_rl **rlp);
int _cdb_make_write(struct cdb_make *cdbmp,
		    const char *ptr, cdbi_t len);

}
