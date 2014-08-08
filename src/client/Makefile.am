AM_CPPFLAGS=-I$(top_srcdir)/src

bin_PROGRAMS = bnchat bnftp bnbot bnstat

bnchat_SOURCES = bnchat.cpp client.cpp client_connect.cpp udptest.cpp
bnchat_LDADD = $(top_builddir)/src/common/libcommon.a \
	$(top_builddir)/src/compat/libcompat.a

bnftp_SOURCES = bnftp.cpp client.cpp
bnftp_LDADD = $(top_builddir)/src/common/libcommon.a \
	$(top_builddir)/src/compat/libcompat.a

bnbot_SOURCES = bnbot.cpp client.cpp
bnbot_LDADD = $(top_builddir)/src/common/libcommon.a \
	$(top_builddir)/src/compat/libcompat.a

bnstat_SOURCES = bnstat.cpp client.cpp client_connect.cpp udptest.cpp
bnstat_LDADD = $(top_builddir)/src/common/libcommon.a \
	$(top_builddir)/src/compat/libcompat.a

noinst_HEADERS = ansi_term.h client_connect.h client.h udptest.h
