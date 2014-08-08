/*
	Copyright (C) 2000  Marco Ziech (mmz@gmx.net)
	Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
	*/
#include "common/setup_before.h"
#include <cstdlib>
#include <cstring>
#include <cerrno>

#include "common/version.h"
#include "fileio.h"
#include "bni.h"
#include "common/setup_after.h"

#define BUFSIZE 1024

using namespace pvpgn::bni;

namespace
{

	void usage(char const * progname)
	{
		std::fprintf(stderr,
			"usage: %s [<options>] [--] [<BNI file> [<TGA file>]]\n"
			"    -h, --help, --usage  show this information and exit\n"
			"    -v, --version        print version number and exit\n", progname);

		std::exit(EXIT_FAILURE);
	}

}

extern int main(int argc, char * argv[])
{
	char const *  bnifile = NULL;
	char const *  tgafile = NULL;
	std::FILE *        fbni;
	std::FILE *        ftga;
	int           a;
	int           forcefile = 0;
	char          dash[] = "-"; /* unique address used as flag */

	if (argc < 1 || !argv || !argv[0])
	{
		std::fprintf(stderr, "bad arguments\n");
		return EXIT_FAILURE;
	}

	for (a = 1; a < argc; a++)
	if (forcefile && !bnifile)
		bnifile = argv[a];
	else if (std::strcmp(argv[a], "-") == 0 && !bnifile)
		bnifile = dash;
	else if (argv[a][0] != '-' && !bnifile)
		bnifile = argv[a];
	else if (forcefile && !tgafile)
		tgafile = argv[a];
	else if (std::strcmp(argv[a], "-") == 0 && !tgafile)
		tgafile = dash;
	else if (argv[a][0] != '-' && !tgafile)
		tgafile = argv[a];
	else if (forcefile || argv[a][0] != '-' || std::strcmp(argv[a], "-") == 0)
	{
		std::fprintf(stderr, "%s: extra file argument \"%s\"\n", argv[0], argv[a]);
		usage(argv[0]);
	}
	else if (std::strcmp(argv[a], "--") == 0)
		forcefile = 1;
	else if (std::strcmp(argv[a], "-v") == 0 || std::strcmp(argv[a], "--version") == 0)
	{
		std::printf("version "PVPGN_VERSION"\n");
		return EXIT_SUCCESS;
	}
	else if (std::strcmp(argv[a], "-h") == 0 || std::strcmp(argv[a], "--help") == 0 || std::strcmp(argv[a], "--usage")
		== 0)
		usage(argv[0]);
	else
	{
		std::fprintf(stderr, "%s: unknown option \"%s\"\n", argv[0], argv[a]);
		usage(argv[0]);
	}

	if (!bnifile)
		bnifile = dash;
	if (!tgafile)
		tgafile = dash;

	if (bnifile == dash)
		fbni = stdin;
	else
	if (!(fbni = std::fopen(bnifile, "r")))
	{
		std::fprintf(stderr, "%s: could not open BNI file \"%s\" for reading (std::fopen: %s)\n", argv[0], bnifile, std::strerror(errno));
		std::exit(EXIT_FAILURE);
	}
	if (tgafile == dash)
		ftga = stdout;
	else
	if (!(ftga = std::fopen(tgafile, "w")))
	{
		std::fprintf(stderr, "%s: could not open TGA file \"%s\" for reading (std::fopen: %s)\n", argv[0], tgafile, std::strerror(errno));
		std::exit(EXIT_FAILURE);
	}

	{
		unsigned char buf[BUFSIZE];
		std::size_t        rc;
		t_bnifile     bnih;

		file_rpush(fbni);
		bnih.unknown1 = file_readd_le();
		bnih.unknown2 = file_readd_le();
		bnih.numicons = file_readd_le();
		bnih.dataoffset = file_readd_le();
		std::fprintf(stderr, "Info: numicons=%d dataoffset=0x%08x(%d)\n", bnih.numicons, bnih.dataoffset, bnih.dataoffset);
		if (std::fseek(fbni, bnih.dataoffset, SEEK_SET)<0)
		{
			std::fprintf(stderr, "%s: could not seek to offset %u in BNI file \"%s\" (std::fseek: %s)\n", argv[0], bnih.dataoffset, bnifile, std::strerror(errno));
			return EXIT_FAILURE;
		}
		while ((rc = std::fread(buf, 1, sizeof(buf), fbni))>0) {
			if (std::fwrite(buf, rc, 1, ftga) < 1) {
				std::fprintf(stderr, "%s: could not write data to TGA file \"%s\" (std::fwrite: %s)\n", argv[0], tgafile, std::strerror(errno));
				return EXIT_FAILURE;
			}
		}
		file_rpop();
	}

	if (tgafile != dash && std::fclose(ftga) < 0)
		std::fprintf(stderr, "%s: could not close TGA file \"%s\" after writing (std::fclose: %s)\n", argv[0], tgafile, std::strerror(errno));
	if (bnifile != dash && std::fclose(fbni) < 0)
		std::fprintf(stderr, "%s: could not close BNI file \"%s\" after reading (std::fclose: %s)\n", argv[0], bnifile, std::strerror(errno));

	return EXIT_SUCCESS;
}
