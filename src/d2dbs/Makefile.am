AM_CPPFLAGS=-I$(top_srcdir)/src -DD2DBS_DEFAULT_CONF_FILE=\"$(sysconfdir)/d2dbs.conf\"

sbin_PROGRAMS = d2dbs

d2dbs_SOURCES = charlock.cpp d2ladder.cpp dbserver.cpp dbspacket.cpp cmdline.cpp \
	prefs.cpp handle_signal.cpp dbsdupecheck.cpp main.cpp
d2dbs_LDADD = $(top_builddir)/src/common/libcommon.a \
	$(top_builddir)/src/compat/libcompat.a \
	$(top_builddir)/src/win32/libwin32.a

noinst_HEADERS = charlock.h cmdline.h d2ladder.h dbsdupecheck.h dbserver.h \
	dbspacket.h handle_signal.h prefs.h setup.h version.h
