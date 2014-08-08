set(D2DBS_SOURCES
	charlock.cpp charlock.h cmdline.cpp cmdline.h d2ladder.cpp d2ladder.h 
	dbsdupecheck.cpp dbsdupecheck.h dbserver.cpp dbserver.h dbspacket.cpp 
	dbspacket.h handle_signal.cpp handle_signal.h main.cpp prefs.cpp 
	prefs.h setup.h version.h ../win32/d2dbs_winmain.cpp ../win32/d2dbs_resource.h 
	../win32/d2dbs_resource.rc)

if(WITH_WIN32_GUI)
  add_executable(d2dbs WIN32 ${D2DBS_SOURCES})
else(WITH_WIN32_GUI)
  add_executable(d2dbs ${D2DBS_SOURCES})
endif(WITH_WIN32_GUI)

target_link_libraries(d2dbs common compat win32 ${NETWORK_LIBRARIES})
install(TARGETS d2dbs DESTINATION ${SBINDIR})
