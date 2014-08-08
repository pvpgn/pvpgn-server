/* unpack 32bit integer
 *
 * This file is a part of tinycdb package by Michael Tokarev, mjt@corpit.ru.
 * Public domain.
 */

#include "common/setup_before.h"
#include "cdb.h"
#include "common/setup_after.h"

namespace pvpgn
{

unsigned
cdb_unpack(const unsigned char buf[4])
{
  unsigned n = buf[3];
  n <<= 8; n |= buf[2];
  n <<= 8; n |= buf[1];
  n <<= 8; n |= buf[0];
  return n;
}

}
