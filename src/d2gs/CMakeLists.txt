add_executable(d2gs WIN32
	bn_types.h
	callback.cpp
	callback.h
	charlist.cpp
	charlist.h
	config.cpp
	config.h
	connection.h
	d2cs_d2gs_character.h
	d2cs_d2gs_protocol.h
	d2dbs_d2gs_protocol.h
	d2gamelist.cpp
	d2gamelist.h
	d2ge.cpp
	d2ge.h
	d2gs.h
	debug.cpp
	debug.h
	eventlog.cpp
	eventlog.h
	handle_s2s.cpp
	handle_s2s.h
	hexdump.cpp
	hexdump.h
	list.h
	main.cpp
	net.cpp
	net.h
	server.cpp
	server.h
	telnetd.cpp
	telnetd.h
	timer.cpp
	timer.h
	utils.cpp
	utils.h
	vars.cpp
	vars.h
	versioncheck.cpp
	versioncheck.h
	watchdog.cpp
	watchdog.h
	D2Server.ico
	d2server.rc
	resource.h)

target_compile_definitions(d2gs PRIVATE D2GS _USE_32BIT_TIME_T)

target_link_libraries(d2gs PRIVATE common ws2_32 d2gelib)

set_target_properties(d2gs
	PROPERTIES
		CXX_STANDARD 11
		CXX_STANDARD_REQUIRED YES
		CXX_EXTENSIONS NO)

install(TARGETS d2gs DESTINATION ${SBINDIR})

if(WIN32 AND MSVC)
    install(FILES $<TARGET_PDB_FILE:d2gs> DESTINATION ${SBINDIR} OPTIONAL)
endif()