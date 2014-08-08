AM_CPPFLAGS=-I$(top_srcdir)/src

noinst_LIBRARIES = libtinycdb.a
libtinycdb_a_SOURCES = cdb_find.cpp cdb_findnext.cpp cdb_hash.cpp cdb_init.cpp \
	cdb_make_add.cpp cdb_make.cpp cdb_make_find.cpp cdb_make_put.cpp cdb_seek.cpp \
	cdb_seq.cpp cdb_unpack.cpp

bin_PROGRAMS = bncdb
bncdb_SOURCES = cdb.cpp
bncdb_LDADD = libtinycdb.a $(top_builddir)/src/common/libcommon.a \
	$(top_builddir)/src/compat/libcompat.a

noinst_HEADERS = cdb.h cdb_int.h
