#first setup the includes and link paths
include_directories(${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR}/src)

# the following line makes sure all targets will lookup local libraries
# when referenced by short names
link_directories(${CMAKE_BINARY_DIR}/src/common 
	${CMAKE_BINARY_DIR}/src/compat
	${CMAKE_BINARY_DIR}/src/win32
	${CMAKE_BINARY_DIR}/src/tinycdb)

#this is needed to be compatible with existent code
add_definitions("-DHAVE_CONFIG_H")

if(WITH_FLAG_WALL)
    add_definitions("-Wall")
endif(WITH_FLAG_WALL)

if(WITH_FLAG_ANSIPEDANTIC)
    add_definitions("-pedantic -ansi")
endif(WITH_FLAG_ANSIPEDANTIC)

if(LUA_FOUND)
    add_definitions("-DWITH_LUA")
endif(LUA_FOUND)

if(MYSQL_FOUND)
    add_definitions("-DWITH_SQL_MYSQL")
endif(MYSQL_FOUND)
if(SQLITE3_FOUND)
    add_definitions("-DWITH_SQL_SQLITE3")
endif(SQLITE3_FOUND)
if(PGSQL_FOUND)
    add_definitions("-DWITH_SQL_PGSQL")
endif(PGSQL_FOUND)
if(ODBC_FOUND)
    add_definitions("-DWITH_SQL_ODBC")
endif(ODBC_FOUND)

if (WITH_WIN32_GUI)
    add_definitions("-DWIN32_GUI")
endif (WITH_WIN32_GUI)

subdirs(compat common win32 tinycdb bntrackd client bniutils bnpass)

if(WITH_BNETD)
    add_subdirectory(bnetd)
endif(WITH_BNETD)

if(WITH_D2CS)
    add_subdirectory(d2cs)
endif(WITH_D2CS)

if(WITH_D2DBS)
    add_subdirectory(d2dbs)
endif(WITH_D2DBS)

if(HAVE_PCAP_H AND HAVE_LIBPCAP)
    add_subdirectory(bnpcap)
endif(HAVE_PCAP_H AND HAVE_LIBPCAP)

if(CMAKE_TESTING_ENABLED)
    add_subdirectory(test)
endif(CMAKE_TESTING_ENABLED)

