/*
	Copyright (C) 2000  Marco Ziech
	Copyright (C) 2000  Ross Combs (rocombs@cs.nmsu.edu)

	This program is xfree software; you can redistribute it and/or modify
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
#include <cstring>
#include <cerrno>
#include <cstdlib>

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#include "compat/statmacros.h"
#include "common/xalloc.h"
#include "common/version.h"
#include "fileio.h"
#include "tga.h"
#include "bni.h"
#include "common/setup_after.h"

#define BUFSIZE 1024

using namespace pvpgn;
using namespace pvpgn::bni;

namespace
{

	int read_list(char const * progname, t_bnifile * bnifile, char const * name) {
		std::FILE * f;
		char   line[BUFSIZE];

		f = std::fopen(name, "r");
		if (f == NULL) {
			std::fprintf(stderr, "%s: could not open index file \"%s\" for reading (std::fopen: %s)\n", progname, name, std::strerror(errno));
			return -1;
		}
		bnifile->unknown1 = 0x00000010; /* in case they are not set */
		bnifile->unknown2 = 0x00000001;
		bnifile->numicons = 0;
		bnifile->dataoffset = 16; /* size of header */
		bnifile->icons = (struct bni_iconlist_struct*)xmalloc(1); /* some xrealloc()s are broken */
		while (std::fgets(line, sizeof(line), f)) {
			char cmd[BUFSIZE];
			std::sscanf(line, "%s", cmd);
			if (std::strcmp(cmd, "unknown1") == 0) {
				std::sscanf(line, "unknown1 %08x", &bnifile->unknown1);
			}
			else if (std::strcmp(cmd, "unknown2") == 0) {
				std::sscanf(line, "unknown2 %08x", &bnifile->unknown2);
			}
			else if (std::strcmp(cmd, "icon") == 0) {
				char c;
				std::sscanf(line, "icon %c", &c);
				if (c == '!') {
					unsigned char tg[4];
					int tag;
					unsigned int x, y, unknown;
					std::sscanf(line, "icon !%c%c%c%c %u %u %08x", &tg[0], &tg[1], &tg[2], &tg[3], &x, &y, &unknown);
					tag = tg[3] + (tg[2] << 8) + (tg[1] << 16) + (tg[0] << 24);
					std::fprintf(stderr, "Icon[%d]: id=0x%x x=%u y=%u unknown=0x%x tag=\"%c%c%c%c\"\n", bnifile->numicons, 0, x, y, unknown, ((tag >> 24) & 0xff), ((tag >> 16) & 0xff), ((tag >> 8) & 0xff), ((tag >> 0) & 0xff));
					bnifile->icons = (struct bni_iconlist_struct*)xrealloc(bnifile->icons, ((bnifile->numicons + 1)*sizeof(t_bniicon)));
					bnifile->icons->icon[bnifile->numicons].id = 0;
					bnifile->icons->icon[bnifile->numicons].x = x;
					bnifile->icons->icon[bnifile->numicons].y = y;
					bnifile->icons->icon[bnifile->numicons].tag = tag;
					bnifile->icons->icon[bnifile->numicons].unknown = unknown;
					bnifile->numicons++;
					bnifile->dataoffset += 20;
				}
				else if (c == '#') {
					unsigned int id, x, y, unknown;
					std::sscanf(line, "icon #%08x %u %u %08x", &id, &x, &y, &unknown);
					std::fprintf(stderr, "Icon[%d]: id=0x%x x=%u y=%u unknown=0x%x tag=0x00000000\n", bnifile->numicons, id, x, y, unknown);
					bnifile->icons = (struct bni_iconlist_struct*)xrealloc(bnifile->icons, ((bnifile->numicons + 1)*sizeof(t_bniicon)));
					bnifile->icons->icon[bnifile->numicons].id = id;
					bnifile->icons->icon[bnifile->numicons].x = x;
					bnifile->icons->icon[bnifile->numicons].y = y;
					bnifile->icons->icon[bnifile->numicons].tag = 0;
					bnifile->icons->icon[bnifile->numicons].unknown = unknown;
					bnifile->numicons++;
					bnifile->dataoffset += 16;
				}
				else
					std::fprintf(stderr, "Bad character '%c' in icon specifier for icon %u in index file \"%s\"\n", c, bnifile->numicons + 1, name);
			}
			else
				std::fprintf(stderr, "Unknown command \"%s\" in index file \"%s\"\n", cmd, name);
		}
		if (std::fclose(f) < 0)
			std::fprintf(stderr, "%s: could not close index file \"%s\" after reading (std::fclose: %s)\n", progname, name, std::strerror(errno));
		return 0;
	}


	char * geticonfilename(t_bnifile *bnifile, char const * indir, int i) {
		char * name;

		if (bnifile->icons->icon[i].id == 0) {
			unsigned int tag = bnifile->icons->icon[i].tag;
			name = (char*)xmalloc(std::strlen(indir) + 10);
			std::sprintf(name, "%s/%c%c%c%c.tga", indir, ((tag >> 24) & 0xff), ((tag >> 16) & 0xff), ((tag >> 8) & 0xff), ((tag >> 0) & 0xff));
		}
		else {
			name = (char*)xmalloc(std::strlen(indir) + 16);
			std::sprintf(name, "%s/%08x.tga", indir, bnifile->icons->icon[i].id);
		}
		return name;
	}


	int img2area(t_tgaimg *dst, t_tgaimg *src, int x, int y) {
		unsigned char *sdp;
		unsigned char *ddp;
		int pixelsize;
		int i;

		pixelsize = getpixelsize(dst);
		if (getpixelsize(src) != pixelsize) {
			std::fprintf(stderr, "Error: source pixelsize is %d should be %d!\n", getpixelsize(src), pixelsize);
			return -1;
		}
		if (src->width + x > dst->width) return -1;
		if (src->height + y > dst->height) return -1;
		sdp = src->data;
		ddp = dst->data + (y * dst->width * pixelsize);
		for (i = 0; i < src->height; i++) {
			ddp += x*pixelsize;
			std::memcpy(ddp, sdp, src->width*pixelsize);
			sdp += src->width*pixelsize;
			ddp += (dst->width - x)*pixelsize;
		}
		return 0;
	}


	void usage(char const * progname)
	{
		std::fprintf(stderr,
			"usage: %s [<options>] [--] <input directory> [<BNI file>]\n"
			"    -h, --help, --usage  show this information and exit\n"
			"    -v, --version        print version number and exit\n", progname);

		std::exit(EXIT_FAILURE);
	}

}

extern int main(int argc, char * argv[])
{
	char const * indir = NULL;
	char const * bnifile = NULL;
	std::FILE *       fbni;
	struct stat  s;
	int          a;
	int          forcefile = 0;
	char         dash[] = "-"; /* unique address used as flag */

	if (argc < 1 || !argv || !argv[0])
	{
		std::fprintf(stderr, "bad arguments\n");
		return EXIT_FAILURE;
	}

	for (a = 1; a < argc; a++)
	if (forcefile && !indir)
		indir = argv[a];
	else if (std::strcmp(argv[a], "-") == 0 && !indir)
		indir = dash;
	else if (argv[a][0] != '-' && !indir)
		indir = argv[a];
	else if (forcefile && !bnifile)
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

	if (!indir)
	{
		std::fprintf(stderr, "%s: input directory not specified\n", argv[0]);
		usage(argv[0]);
	}
	if (!bnifile)
		bnifile = dash;

	if (indir == dash)
	{
		std::fprintf(stderr, "%s: can not read directory from <stdin>\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (stat(indir, &s) < 0) {
		std::fprintf(stderr, "%s: could not stat input directory \"%s\" (stat: %s)\n", argv[0], indir, std::strerror(errno));
		return EXIT_FAILURE;
	}
	if (!S_ISDIR(s.st_mode)) {
		std::fprintf(stderr, "%s: \"%s\" is not a directory\n", argv[0], indir);
		return -1;
	}

	if (bnifile == dash)
		fbni = stdout;
	else
	if (!(fbni = std::fopen(bnifile, "w")))
	{
		std::fprintf(stderr, "%s: could not open BNI file \"%s\" for writing (std::fopen: %s)\n", argv[0], bnifile, std::strerror(errno));
		return EXIT_FAILURE;
	}

	{
		unsigned int i;
		unsigned int yline;
		t_tgaimg *   img;
		t_bnifile    bni;
		char *       listfilename;

		listfilename = (char*)xmalloc(std::strlen(indir) + 14);
		std::sprintf(listfilename, "%s/bniindex.lst", indir);
		std::fprintf(stderr, "Info: Reading index from file \"%s\"...\n", listfilename);
		if (read_list(argv[0], &bni, listfilename) < 0)
			return EXIT_FAILURE;
		std::fprintf(stderr, "BNIHeader: unknown1=%u unknown2=%u numicons=%u dataoffset=%u\n", bni.unknown1, bni.unknown2, bni.numicons, bni.dataoffset);
		if (write_bni(fbni, &bni) < 0) {
			std::fprintf(stderr, "Error: Failed to write BNI header.\n");
			return EXIT_FAILURE;
		}
		img = new_tgaimg(0, 0, 24, tgaimgtype_rlecompressed_truecolor);
		for (i = 0; i < bni.numicons; i++) {
			if (bni.icons->icon[i].x > img->width) img->width = bni.icons->icon[i].x;
			img->height += bni.icons->icon[i].y;
		}
		std::fprintf(stderr, "Info: Creating TGA with %ux%ux%ubpp.\n", img->width, img->height, img->bpp);
		img->data = (t_uint8*)xmalloc(img->width*img->height*getpixelsize(img));
		yline = 0;
		for (i = 0; i < bni.numicons; i++) {
			t_tgaimg *icon;
			std::FILE *f;
			char *name;
			name = geticonfilename(&bni, indir, i);
			f = std::fopen(name, "r");
			if (f == NULL) {
				std::perror("std::fopen");
				xfree(name);
				return EXIT_FAILURE;
			}
			xfree(name);
			icon = load_tga(f);
			if (std::fclose(f) < 0)
				std::fprintf(stderr, "Error: could not close TGA file \"%s\" after reading (std::fclose: %s)\n", name, std::strerror(errno));
			if (icon == NULL) {
				std::fprintf(stderr, "Error: load_tga failed with data from TGA file \"%s\"\n", name);
				return EXIT_FAILURE;
			}
			if (img2area(img, icon, 0, yline) < 0) {
				std::fprintf(stderr, "Error: inserting icon from TGA file \"%s\" into big TGA failed\n", name);
				return EXIT_FAILURE;
			}
			yline += icon->height;
			destroy_img(icon);
		}
		if (write_tga(fbni, img) < 0) {
			std::fprintf(stderr, "Error: Failed to write TGA to BNI file.\n");
			return EXIT_FAILURE;
		}
		if (bnifile != dash && std::fclose(fbni) < 0) {
			std::fprintf(stderr, "%s: could not close BNI file \"%s\" after writing (std::fclose: %s)\n", argv[0], bnifile, std::strerror(errno));
			return EXIT_FAILURE;
		}
	}
	std::fprintf(stderr, "Info: Writing to \"%s\" finished.\n", bnifile);
	return EXIT_SUCCESS;
}
