set(BNETD_SOURCES
	account.cpp account.h account_email_verification.cpp account_email_verification.h
	account_wrap.cpp account_wrap.h adbanner.cpp
	adbanner.h alias_command.cpp alias_command.h anongame.cpp
	anongame_gameresult.cpp anongame_gameresult.h anongame.h 
	anongame_infos.cpp anongame_infos.h anongame_maplists.cpp 
	anongame_maplists.h attrgroup.cpp attrgroup.h attr.h attrlayer.cpp 
	attrlayer.h autoupdate.cpp autoupdate.h channel_conv.cpp channel_conv.h 
	channel.cpp channel.h character.cpp character.h clan.cpp clan.h 
	cmdline.cpp cmdline.h command.cpp command_groups.cpp command_groups.h 
	command.h connection.cpp connection.h file.cpp file.h
	file_plain.cpp file_plain.h friends.cpp friends.h game_conv.cpp 
	game_conv.h game.cpp game.h handle_anongame.cpp handle_anongame.h 
	handle_apireg.cpp handle_apireg.h handle_bnet.cpp handle_bnet.h 
	handle_bot.cpp handle_bot.h handle_d2cs.cpp handle_d2cs.h 
	handle_file.cpp handle_file.h handle_init.cpp handle_init.h 
	handle_irc_common.cpp handle_irc_common.h handle_irc.cpp handle_irc.h 
	handlers.h handle_telnet.cpp handle_telnet.h handle_udp.cpp 
	handle_udp.h handle_wol.cpp handle_wol.h handle_wol_gameres.cpp
    handle_wol_gameres.h helpfile.cpp helpfile.h
	ipban.cpp ipban.h irc.cpp irc.h ladder_calc.cpp ladder_calc.h ladder.cpp 
	ladder.h mail.cpp mail.h main.cpp message.cpp message.h news.cpp news.h
	output.cpp output.h prefs.cpp prefs.h quota.h realm.cpp realm.h 
	runprog.cpp runprog.h server.cpp server.h smtp.cpp smtp.h sql_common.cpp
	sql_common.h sql_dbcreator.cpp sql_dbcreator.h sql_mysql.cpp sql_mysql.h
	sql_odbc.cpp sql_odbc.h sql_pgsql.cpp sql_pgsql.h sql_sqlite3.cpp sql_sqlite3.h 
	storage.cpp storage_file.cpp storage_file.h storage.h
	storage_sql.cpp storage_sql.h support.cpp support.h
	team.cpp team.h tick.cpp tick.h timer.cpp timer.h topic.cpp topic.h 
	tournament.cpp tournament.h icons.cpp icons.h tracker.cpp tracker.h udptest_send.cpp 
	udptest_send.h versioncheck.cpp versioncheck.h watch.cpp watch.h
	anongame_wol.cpp anongame_wol.h handle_wserv.cpp handle_wserv.h
	luafunctions.cpp luafunctions.h luainterface.cpp luainterface.h 
	luaobjects.cpp luaobjects.h luawrapper.cpp luawrapper.h
	i18n.cpp i18n.h userlog.cpp userlog.h
	../win32/winmain.cpp ../win32/winmain.h
	)
	
set(BNETD_CONSOLE_RESOURCES
  ../win32/console_resource.h ../win32/console_resource.rc)

set(BNETD_GUI_RESOURCES
  ../win32/resource.h ../win32/resource.rc)

if(WITH_WIN32_GUI)
  add_executable(bnetd WIN32 ${BNETD_SOURCES} ${BNETD_GUI_RESOURCES})
else(WITH_WIN32_GUI)  
  add_executable(bnetd ${BNETD_SOURCES} ${BNETD_CONSOLE_RESOURCES})
endif(WITH_WIN32_GUI)

target_include_directories(bnetd
    PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
		$<INSTALL_INTERFACE:include/>
		${ZLIB_INCLUDE_DIRS}
		${CURL_INCLUDE_DIRS}
		${MYSQL_INCLUDE_DIR}
		${SQLITE3_INCLUDE_DIR}
		${PGSQL_INCLUDE_DIR}
		${ODBC_INCLUDE_DIR}
)

target_link_libraries(bnetd PRIVATE common compat fmt Threads::Threads win32 ${NETWORK_LIBRARIES} ${ZLIB_LIBRARIES} ${CURL_LIBRARIES} ${MYSQL_LIBRARIES} ${SQLITE3_LIBRARIES} ${PGSQL_LIBRARIES} ${ODBC_LIBRARIES})

if (WITH_LUA)
	target_include_directories(bnetd PRIVATE ${LUA_INCLUDE_DIR})
	target_link_libraries(bnetd PRIVATE ${LUA_LIBRARIES})
endif()

install(TARGETS bnetd DESTINATION ${SBINDIR})

if(WIN32 AND MSVC)
    install(FILES $<TARGET_PDB_FILE:bnetd> DESTINATION ${SBINDIR} OPTIONAL)
endif()

if (WIN32 AND USE_INCLUDED_ZLIB_LIBRARY)
	install(
		FILES
			"${PROJECT_SOURCE_DIR}/lib/zlib/zlib1.dll"
		DESTINATION
			${SBINDIR}
	)
endif()
