set(D2DBS_SOURCES
	charlock.cpp charlock.h cmdline.cpp cmdline.h connection.cpp connection.h d2ladder.cpp d2ladder.h 
	dbsdupecheck.cpp dbsdupecheck.h dbserver.cpp dbserver.h handle_d2gs.cpp handle_d2gs.h handle_init.cpp 
	handle_init.h handle_signal.cpp handle_signal.h main.cpp pgsid.cpp pgsid.h prefs.cpp 
	prefs.h setup.h version.h ../win32/d2dbs_winmain.cpp ../win32/d2dbs_resource.h 
	../win32/d2dbs_resource.rc)

if(WITH_WIN32_GUI)
  add_executable(d2dbs WIN32 ${D2DBS_SOURCES})
else(WITH_WIN32_GUI)
  add_executable(d2dbs ${D2DBS_SOURCES})
endif(WITH_WIN32_GUI)

target_include_directories(d2dbs
    PRIVATE
		$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
		$<INSTALL_INTERFACE:include/>
)

target_link_libraries(d2dbs PRIVATE common compat fmt::fmt win32 ${NETWORK_LIBRARIES})
install(TARGETS d2dbs DESTINATION ${SBINDIR})
if(WIN32 AND MSVC)
    install(FILES $<TARGET_PDB_FILE:d2dbs> DESTINATION ${SBINDIR} OPTIONAL)
endif()