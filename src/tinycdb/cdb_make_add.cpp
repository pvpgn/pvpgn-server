/* basic cdb_make_add routine
 *
 * This file is a part of tinycdb package by Michael Tokarev, mjt@corpit.ru.
 * Public domain.
 */

#include "common/setup_before.h"
#include <cerrno>

#include "common/xalloc.h"
#include "cdb_int.h"
#include "common/setup_after.h"

namespace pvpgn
{

int
cdb_make_add(struct cdb_make *cdbmp,
	     const void *key, unsigned klen,
	     const void *val, unsigned vlen)
{
  unsigned char rlen[8];
  unsigned hval;
  struct cdb_rl *rl;
  if (klen > 0xffffffff - (cdbmp->cdb_dpos + 8) ||
      vlen > 0xffffffff - (cdbmp->cdb_dpos + klen + 8))
    return errno = ENOMEM, -1;
  hval = cdb_hash(key, klen);
  rl = cdbmp->cdb_rec[hval&255];
  if (!rl || rl->cnt >= sizeof(rl->rec)/sizeof(rl->rec[0])) {
    rl = (struct cdb_rl*)xmalloc(sizeof(struct cdb_rl));
    if (!rl)
      return errno = ENOMEM, -1;
    rl->cnt = 0;
    rl->next = cdbmp->cdb_rec[hval&255];
    cdbmp->cdb_rec[hval&255] = rl;
  }
  rl->rec[rl->cnt].hval = hval;
  rl->rec[rl->cnt].rpos = cdbmp->cdb_dpos;
  ++rl->cnt;
  ++cdbmp->cdb_rcnt;
  cdb_pack(klen, rlen);
  cdb_pack(vlen, rlen + 4);
  if (_cdb_make_write(cdbmp, (char*)rlen, 8) < 0 ||
      _cdb_make_write(cdbmp, (char*)key, klen) < 0 ||
      _cdb_make_write(cdbmp, (char*)val, vlen) < 0)
    return -1;
  return 0;
}

}
