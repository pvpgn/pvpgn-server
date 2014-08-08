AM_CPPFLAGS=-I$(top_srcdir)/src

bin_PROGRAMS = bnpass sha1hash

bnpass_SOURCES = bnpass.cpp

bnpass_LDADD = $(top_builddir)/src/common/libcommon.a \
	$(top_builddir)/src/compat/libcompat.a

sha1hash_SOURCES = sha1hash.cpp

sha1hash_LDADD = $(top_builddir)/src/common/libcommon.a \
	$(top_builddir)/src/compat/libcompat.a
