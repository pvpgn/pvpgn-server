AM_CPPFLAGS=-I$(top_srcdir)/src -DD2CS_DEFAULT_CONF_FILE=\"$(sysconfdir)/d2cs.conf\"

sbin_PROGRAMS = d2cs

d2cs_SOURCES = bnetd.cpp cmdline.cpp connection.cpp d2charfile.cpp d2charlist.cpp \
	d2gs.cpp d2ladder.cpp game.cpp gamequeue.cpp handle_bnetd.cpp handle_d2cs.cpp \
	handle_d2gs.cpp handle_init.cpp handle_signal.cpp main.cpp net.cpp prefs.cpp \
	s2s.cpp server.cpp serverqueue.cpp
d2cs_LDADD = $(top_builddir)/src/common/libcommon.a \
	$(top_builddir)/src/compat/libcompat.a \
	$(top_builddir)/src/win32/libwin32.a

noinst_HEADERS = bit.h bnetd.h cmdline.h connection.h d2charfile.h \
	d2charlist.h d2gs.h d2ladder.h game.h gamequeue.h handle_bnetd.h \
	handle_d2cs.h handle_d2gs.h handle_init.h handle_signal.h net.h \
	prefs.h s2s.h server.h serverqueue.h setup.h version.h
