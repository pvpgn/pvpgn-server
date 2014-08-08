/* routines to search in in-progress cdb file
 *
 * This file is a part of tinycdb package by Michael Tokarev, mjt@corpit.ru.
 * Public domain.
 */

#include "common/setup_before.h"
#include <cstdio>
#include <cstring>

#include "cdb_int.h"
#include "common/setup_after.h"

namespace pvpgn
{

static int
match(std::FILE *fd, unsigned pos, const char *key, unsigned klen)
{
  unsigned char buf[64]; /*XXX cdb_buf may be used here instead */
  if (std::fseek(fd, pos, SEEK_SET) || std::fread(buf, 1, 8, fd) != 8)
    return -1;
  if (cdb_unpack(buf) != klen)
    return 0;

  while(klen > sizeof(buf)) {
    if (std::fread(buf, 1, sizeof(buf), fd) != sizeof(buf))
      return -1;
    if (std::memcmp(buf, key, sizeof(buf)) != 0)
      return 0;
    key += sizeof(buf);
    klen -= sizeof(buf);
  }
  if (klen) {
    if (std::fread(buf, 1, klen, fd) != klen)
      return -1;
    if (std::memcmp(buf, key, klen) != 0)
      return 0;
  }
  return 1;
}

int
_cdb_make_find(struct cdb_make *cdbmp,
	       const void *key, unsigned klen, unsigned hval,
	       struct cdb_rl **rlp)
{
  struct cdb_rl *rl = cdbmp->cdb_rec[hval&255];
  int r, i;
  int seeked = 0;
  while(rl) {
    for(i = rl->cnt - 1; i >= 0; --i) { /* search backward */
      if (rl->rec[i].hval != hval)
	continue;
      /*XXX this explicit flush may be unnecessary having
       * smarter match() that looks to cdb_buf too, but
       * most of a time here spent in finding hash values
       * (above), not keys */
      if (cdbmp->cdb_bpos != cdbmp->cdb_buf) {
        if (std::fwrite(cdbmp->cdb_buf, 1,
	          cdbmp->cdb_bpos - cdbmp->cdb_buf, cdbmp->cdb_fd) < 0)
          return -1;
        cdbmp->cdb_bpos = cdbmp->cdb_buf;
      }
      seeked = 1;
      r = match(cdbmp->cdb_fd, rl->rec[i].rpos, (char*)key, klen);
      if (!r)
	continue;
      if (r < 0)
	return -1;
      if (std::fseek(cdbmp->cdb_fd, cdbmp->cdb_dpos, SEEK_SET))
        return -1;
      if (rlp)
	*rlp = rl;
      return i + 1;
    }
    rl = rl->next;
  }
  if (seeked && std::fseek(cdbmp->cdb_fd, cdbmp->cdb_dpos, SEEK_SET))
    return -1;
  return 0;
}

int
cdb_make_exists(struct cdb_make *cdbmp,
                const void *key, unsigned klen)
{
  return _cdb_make_find(cdbmp, key, klen, cdb_hash(key, klen), NULL);
}

}
