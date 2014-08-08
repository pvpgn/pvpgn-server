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
			"usage: %s [<options>] [--] [<TGA file>]\n"
			"    -h, --help, --usage  show this information and exit\n"
			"    -v, --version        print version number and exit\n", progname);

		std::exit(EXIT_FAILURE);
	}

}

extern int main(int argc, char * argv[])
{
	char const * tgafile = NULL;
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
	if (forcefile && !tgafile)
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

	if (!tgafile)
		tgafile = dash;

	if (tgafile == dash)
		fp = stdin;
	else
	if (!(fp = std::fopen(tgafile, "r")))
	{
		std::fprintf(stderr, "%s: could not open TGA file \"%s\" for reading (std::fopen: %s)\n", argv[0], tgafile, std::strerror(errno));
		return EXIT_FAILURE;
	}

	{
		t_tgaimg * tgaimg;

		file_rpush(fp);
		if (!(tgaimg = load_tgaheader()))
		{
			std::fprintf(stderr, "%s: could not load TGA header\n", argv[0]);
			if (tgafile != dash && std::fclose(fp) < 0)
				std::fprintf(stderr, "%s: could not close file \"%s\" after reading (std::fclose: %s)\n", argv[0], tgafile, std::strerror(errno));
			return EXIT_FAILURE;
		}
		print_tga_info(tgaimg, stdout);
		file_rpop();
	}

	if (tgafile != dash && std::fclose(fp) < 0)
		std::fprintf(stderr, "%s: could not close file \"%s\" after reading (std::fclose: %s)\n", argv[0], tgafile, std::strerror(errno));
	return EXIT_SUCCESS;
}
