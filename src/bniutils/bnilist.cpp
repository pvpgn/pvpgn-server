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
#include "tga.h"
#include "fileio.h"
#include "common/setup_after.h"

using namespace pvpgn::bni;

namespace
{

	void usage(char const * progname)
	{
		std::fprintf(stderr,
			"usage: %s [<options>] [--] [<BNI file>]\n"
			"    -h, --help, --usage  show this information and exit\n"
			"    -v, --version        print version number and exit\n", progname);

		std::exit(EXIT_FAILURE);
	}

}

extern int main(int argc, char * argv[])
{
	char const * bnifile = NULL;
	std::FILE *       fp;
	int          a;
	int          forcefile = 0;
	char         dash[] = "-"; /* unique address used as flag */

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

	if (bnifile == dash)
		fp = stdin;
	else
	if (!(fp = std::fopen(bnifile, "r")))
	{
		std::fprintf(stderr, "%s: could not open BNI file \"%s\" for reading (std::fopen: %s)\n", argv[0], bnifile, std::strerror(errno));
		std::exit(EXIT_FAILURE);
	}

	{
		t_tgaimg * tgaimg;
		int        i;
		int        bniid, unknown, icons, datastart;
		int        expected_width, expected_height;

		file_rpush(fp);
		bniid = file_readd_le();
		unknown = file_readd_le();
		icons = file_readd_le();
		datastart = file_readd_le();
		std::fprintf(stderr, "BNIHeader: id=0x%08x unknown=0x%08x icons=0x%08x datastart=0x%08x\n", bniid, unknown, icons, datastart);
		expected_width = 0;
		expected_height = 0;
		for (i = 0; i < icons; i++) {
			int id, x, y, flags, tag;
			id = file_readd_le();
			x = file_readd_le();
			y = file_readd_le();
			if (id == 0) {
				tag = file_readd_le();
			}
			else {
				tag = 0;
			}
			flags = file_readd_le();
			std::fprintf(stderr, "Icon[%d]: id=0x%08x x=%d y=%d tag=0x%08x(\"%c%c%c%c\") flags=0x%08x\n", i, id, x, y, tag,
				((unsigned char)((tag >> 24) & 0xff)),
				((unsigned char)((tag >> 16) & 0xff)),
				((unsigned char)((tag >> 8) & 0xff)),
				((unsigned char)((tag)& 0xff)), flags);
			if (x > expected_width) expected_width = x;
			expected_height += y;
		}
		if (std::ftell(fp) != datastart) {
			std::fprintf(stderr, "Warning: garbage after header (pos=0x%lx-datastart=0x%lx) = %ld bytes of garbage! \n", (unsigned long)std::ftell(fp), (unsigned long)datastart, (long)(std::ftell(fp) - datastart));
		}
		tgaimg = load_tgaheader();
		print_tga_info(tgaimg, stdout);
		std::fprintf(stderr, "\n");
		std::fprintf(stderr, "Check: Expected %dx%d TGA, got %ux%u. %s\n", expected_width, expected_height, tgaimg->width, tgaimg->height, ((tgaimg->width == expected_width) && (tgaimg->height == expected_height)) ? "OK." : "FAIL.");
		std::fprintf(stderr, "Check: Expected 24bit color depth TGA, got %dbit. %s\n", tgaimg->bpp, (tgaimg->bpp == 24) ? "OK." : "FAIL.");
		std::fprintf(stderr, "Check: Expected ImageType 10, got %d. %s\n", tgaimg->imgtype, (tgaimg->imgtype == 10) ? "OK." : "FAIL.");
		file_rpop();
	}

	if (bnifile != dash && std::fclose(fp) < 0)
		std::fprintf(stderr, "%s: could not close BNI file \"%s\" after reading (std::fclose: %s)\n", argv[0], bnifile, std::strerror(errno));
	return EXIT_SUCCESS;
}
