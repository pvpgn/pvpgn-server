AM_CPPFLAGS=-I$(top_srcdir)/src

bin_PROGRAMS = bnilist bni2tga bniextract bnibuild tgainfo

bnilist_SOURCES = bnilist.cpp fileio.cpp tga.cpp
bnilist_LDADD = $(top_builddir)/src/common/libcommon.a \
	$(top_builddir)/src/compat/libcompat.a

bni2tga_SOURCES = bni2tga.cpp fileio.cpp
bni2tga_LDADD = $(top_builddir)/src/common/libcommon.a

bniextract_SOURCES = bniextract.cpp fileio.cpp tga.cpp bni.cpp
bniextract_LDADD = $(top_builddir)/src/common/libcommon.a \
	$(top_builddir)/src/compat/libcompat.a

bnibuild_SOURCES = bnibuild.cpp fileio.cpp bni.cpp tga.cpp
bnibuild_LDADD = $(top_builddir)/src/common/libcommon.a \
	$(top_builddir)/src/compat/libcompat.a

tgainfo_SOURCES = tgainfo.cpp fileio.cpp tga.cpp
tgainfo_LDADD = $(top_builddir)/src/common/libcommon.a \
	$(top_builddir)/src/compat/libcompat.a

noinst_HEADERS = bni.h fileio.h tga.h
