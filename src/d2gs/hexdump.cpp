#include <stdio.h>
#include <stdlib.h>

#include "hexdump.h"

namespace pvpgn
{

    namespace d2gs
    {

        FILE* hexstrm = NULL;

        extern void hexdump(void const* data, unsigned int len)
        {
            unsigned int i;
            unsigned int r, c;

            if (!hexstrm)
                return;
            if (!data)
                return;

            for (r = 0, i = 0; r < (len / 16 + (len % 16 != 0)); r++, i += 16)
            {
                fprintf(hexstrm, "%04X:   ", i); /* location of first byte in line */

                for (c = i; c < i + 8; c++) /* left half of hex dump */
                    if (c < len)
                        fprintf(hexstrm, "%02X ", ((unsigned char const*)data)[c]);
                    else
                        fprintf(hexstrm, "   "); /* pad if short line */

                fprintf(hexstrm, "  ");

                for (c = i + 8; c < i + 16; c++) /* right half of hex dump */
                    if (c < len)
                        fprintf(hexstrm, "%02X ", ((unsigned char const*)data)[c]);
                    else
                        fprintf(hexstrm, "   "); /* pad if short line */

                fprintf(hexstrm, "   ");

                for (c = i; c < i + 16; c++) /* ASCII dump */
                    if (c < len)
                        if (((unsigned char const*)data)[c] >= 32 &&
                            ((unsigned char const*)data)[c] < 127)
                            fprintf(hexstrm, "%c", ((char const*)data)[c]);
                        else
                            fprintf(hexstrm, "."); /* put this for non-printables */
                    else
                        fprintf(hexstrm, " "); /* pad if short line */

                fprintf(hexstrm, "\n");
            }
            fflush(hexstrm);
        }

    }

}