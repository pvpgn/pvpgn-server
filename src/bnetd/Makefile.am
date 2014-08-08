AM_CPPFLAGS=-I$(top_srcdir)/src -DBNETD_DEFAULT_CONF_FILE=\"$(sysconfdir)/bnetd.conf\" ${DEFINES}

sbin_PROGRAMS = bnetd

bnetd_SOURCES = account.cpp account_wrap.cpp adbanner.cpp alias_command.cpp anongame.cpp \
	anongame_gameresult.cpp anongame_infos.cpp anongame_maplists.cpp attrgroup.cpp \
	attrlayer.cpp autoupdate.cpp channel.cpp channel_conv.cpp character.cpp clan.cpp \
	cmdline.cpp command.cpp command_groups.cpp connection.cpp file.cpp file_cdb.cpp \
	file_plain.cpp friends.cpp game.cpp game_conv.cpp handle_anongame.cpp \
	handle_bnet.cpp handle_bot.cpp handle_d2cs.cpp handle_file.cpp handle_init.cpp \
	handle_irc.cpp handle_telnet.cpp handle_udp.cpp helpfile.cpp ipban.cpp irc.cpp \
	ladder.cpp ladder_calc.cpp mail.cpp main.cpp message.cpp news.cpp \
	output.cpp prefs.cpp realm.cpp runprog.cpp server.cpp sql_dbcreator.cpp \
	sql_mysql.cpp sql_odbc.cpp sql_pgsql.cpp sql_sqlite3.cpp storage.cpp \
	storage_file.cpp storage_sql.cpp support.cpp team.cpp tick.cpp timer.cpp topic.cpp \
	tournament.cpp tracker.cpp udptest_send.cpp versioncheck.cpp watch.cpp \
	storage_sql2.cpp sql_common.cpp handle_wol.cpp handle_irc_common.cpp handle_apireg.cpp \
	handle_wserv.cpp luafunctions.cpp luainterface.cpp luaobjects.cpp luawrapper.cpp \
	i18n.cpp icons.cpp

bnetd_LDADD = $(top_builddir)/src/common/libcommon.a \
	$(top_builddir)/src/compat/libcompat.a \
	$(top_builddir)/src/tinycdb/libtinycdb.a \
	$(top_builddir)/src/win32/libwin32.a

noinst_HEADERS = account.h account_wrap.h adbanner.h alias_command.h \
	anongame_gameresult.h anongame.h anongame_infos.h anongame_maplists.h \
	attrgroup.h attr.h attrlayer.h autoupdate.h channel_conv.h channel.h \
	character.h clan.h cmdline.h command_groups.h command.h connection.h \
	file_cdb.h file.h file_plain.h friends.h game_conv.h game.h ipban.h \
	handle_anongame.h handle_bnet.h handle_bot.h handle_d2cs.h helpfile.h \
	handle_file.h handle_init.h handle_irc.h handlers.h handle_telnet.h \
	handle_udp.h irc.h ladder_calc.h ladder.h mail.h \
	message.h news.h output.h prefs.h quota.h realm.h runprog.h server.h \
	sql_dbcreator.h sql_mysql.h sql_odbc.h sql_pgsql.h sql_sqlite3.h \
	storage_file.h storage.h storage_sql.h support.h team.h tick.h \
	timer.h topic.h tournament.h udptest_send.h versioncheck.h watch.h \
	tracker.h storage_sql2.h sql_common.h handle_wol.h handle_irc_common.h handle_apireg.h \
	handle_wserv.h luafunctions.h luainterface.h luaobjects.h luawrapper.h \
	i18n.h icons.h
