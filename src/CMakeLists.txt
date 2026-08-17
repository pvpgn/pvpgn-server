include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR})


if(MYSQL_FOUND)
    add_definitions("-DWITH_SQL_MYSQL")
endif(MYSQL_FOUND)
if(SQLITE3_FOUND)
    add_definitions("-DWITH_SQL_SQLITE3")
endif(SQLITE3_FOUND)
if(PGSQL_FOUND)
    add_definitions("-DWITH_SQL_PGSQL")
endif(PGSQL_FOUND)


if(LUA_FOUND)
    add_definitions("-DWITH_LUA")
endif(LUA_FOUND)



if (WITH_WIN32_GUI)
    add_definitions("-DWIN32_GUI")
endif (WITH_WIN32_GUI)


add_subdirectory(bniutils)
add_subdirectory(bnpass)
add_subdirectory(bntrackd)
add_subdirectory(client)
add_subdirectory(common)
add_subdirectory(compat)
add_subdirectory(win32)


if(WITH_BNETD)
    add_subdirectory(bnetd)
endif(WITH_BNETD)

if(WITH_D2CS)
    add_subdirectory(d2cs)
endif(WITH_D2CS)

if(WITH_D2DBS)
    add_subdirectory(d2dbs)
endif(WITH_D2DBS)

if(WITH_D2GS)
	add_subdirectory(d2gs)
	add_subdirectory(d2server_patches)
	add_subdirectory(d2warden)
	add_subdirectory(patch_d2server)
endif(WITH_D2GS)


if(CMAKE_TESTING_ENABLED)
    add_subdirectory(test)
endif(CMAKE_TESTING_ENABLED)

