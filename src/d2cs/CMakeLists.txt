set(D2CS_SOURCES
	bit.h bnetd.cpp bnetd.h cmdline.cpp cmdline.h connection.cpp 
	connection.h d2charfile.cpp d2charfile.h d2charlist.cpp d2charlist.h 
	d2gs.cpp d2gs.h d2ladder.cpp d2ladder.h game.cpp game.h gamequeue.cpp 
	gamequeue.h handle_bnetd.cpp handle_bnetd.h handle_d2cs.cpp 
	handle_d2cs.h handle_d2gs.cpp handle_d2gs.h handle_init.cpp 
	handle_init.h handle_signal.cpp handle_signal.h main.cpp net.cpp 
	net.h prefs.cpp prefs.h s2s.cpp s2s.h server.cpp server.h 
	serverqueue.h serverqueue.cpp setup.h version.h ../win32/d2cs_winmain.cpp 
	../win32/d2cs_resource.h ../win32/d2cs_resource.rc)

if(WITH_WIN32_GUI)
  add_executable(d2cs WIN32 ${D2CS_SOURCES})
else(WITH_WIN32_GUI)
  add_executable(d2cs ${D2CS_SOURCES})
endif(WITH_WIN32_GUI)

target_link_libraries(d2cs common compat win32 ${NETWORK_LIBRARIES})
install(TARGETS d2cs DESTINATION ${SBINDIR})
