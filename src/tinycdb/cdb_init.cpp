/* cdb_init, cdb_free and cdb_read routines
 *
 * This file is a part of tinycdb package by Michael Tokarev, mjt@corpit.ru.
 * Public domain.
 */

#include "common/setup_before.h"
#include <cstdio>
#include <cerrno>
#include <cstring>

#include "compat/mmap.h"
#include "cdb_int.h"
#include "common/setup_after.h"

namespace pvpgn
{

int
cdb_init(struct cdb *cdbp, std::FILE *fd)
{
  unsigned char *mem;
  unsigned fsize, dend;

  /* get file size */
  if (std::fseek(fd, 0, SEEK_END))
    return -1;
  fsize = (unsigned)(std::ftell(fd));
  std::rewind(fd);
  /* trivial sanity check: at least toc should be here */
  if (fsize < 2048)
    return errno = EPROTO, -1;

  /* memory-map file */
  if ((mem = (unsigned char*)pmmap(NULL, fsize, PROT_READ, MAP_SHARED, fileno(fd), 0)) ==
      (unsigned char *)-1)
    return -1;

  cdbp->cdb_fd = fd;
  cdbp->cdb_fsize = fsize;
  cdbp->cdb_mem = mem;

#if 0
  /* XXX don't know well about madvise syscall -- is it legal
     to set different options for parts of one mmap() region?
     There is also posix_madvise() exist, with POSIX_MADV_RANDOM etc...
  */
#ifdef MADV_RANDOM
  /* set madvise() parameters. Ignore errors for now if system
     doesn't support it */
  madvise(mem, 2048, MADV_WILLNEED);
  madvise(mem + 2048, cdbp->cdb_fsize - 2048, MADV_RANDOM);
#endif
#endif

  cdbp->cdb_vpos = cdbp->cdb_vlen = 0;
  cdbp->cdb_kpos = cdbp->cdb_klen = 0;
  dend = cdb_unpack(mem);
  if (dend < 2048) dend = 2048;
  else if (dend >= fsize) dend = fsize;
  cdbp->cdb_dend = dend;

  return 0;
}

void
cdb_free(struct cdb *cdbp)
{
  if (cdbp->cdb_mem) {
    pmunmap((void*)cdbp->cdb_mem, cdbp->cdb_fsize);
    cdbp->cdb_mem = NULL;
  }
  cdbp->cdb_fsize = 0;
}

const void *
cdb_get(const struct cdb *cdbp, unsigned len, unsigned pos)
{
  if (pos > cdbp->cdb_fsize || cdbp->cdb_fsize - pos < len) {
    errno = EPROTO;
    return NULL;
  }
  return cdbp->cdb_mem + pos;
}

int
cdb_read(const struct cdb *cdbp, void *buf, unsigned len, unsigned pos)
{
  const void *data = cdb_get(cdbp, len, pos);
  if (!data) return -1;
  std::memcpy(buf, data, len);
  return 0;
}

}
