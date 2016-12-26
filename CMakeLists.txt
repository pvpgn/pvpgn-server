# Required cmake version
cmake_minimum_required(VERSION 3.1.0)

# Put the include dirs which are in the source or build tree
# before all other include dirs, so the headers in the sources
# are prefered over the already installed ones
# since cmake 2.4.1
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

project(pvpgn CXX)

option(WITH_BNETD "compile the bnetd target" ON)
option(WITH_D2CS "compile the d2cs target" ON)
option(WITH_D2DBS "compile the d2dbs target" ON)
option(WITH_LUA "enable Lua support" OFF)
if(WIN32)
	option(WITH_WIN32_GUI "enable GUI building (default on)" ON)
endif(WIN32)

#storage backends flags
option(WITH_MYSQL "include MySQL user accounts support" OFF)
option(WITH_SQLITE3 "include SQLite3 user accounts support" OFF)
option(WITH_PGSQL "include PostgreSQL user accounts support" OFF)
option(WITH_ODBC "include ODBC user accounts support" OFF)
include(ConfigureChecks.cmake)


if (CMAKE_COMPILER_IS_GNUCXX)
    # Minimum G++ version: 5.1
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.1)
        message(FATAL_ERROR "G++ 5.1 or higher required")
    endif()

	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -Wno-variadic-macros")
  
	# Pass CXX flags to flags.
	set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSEQAN_CXX_FLAGS_=\"${CMAKE_CXX_FLAGS}\"")
endif ()

if (MSVC)
	# cl.exe version
	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0)
		message(FATAL_ERROR "Visual Studio 2015 or higher required")
	endif()
	
	add_definitions(-DUNICODE -D_UNICODE)
endif ()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
       set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic -stdlib=libc++")
  
       # Pass CXX flags to flags.
       set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSEQAN_CXX_FLAGS_=\"${CMAKE_CXX_FLAGS}\"")
endif()

subdirs(src conf man files)
if(WITH_LUA)
    add_subdirectory(lua)
endif(WITH_LUA)

ENABLE_TESTING()

# uninstall target
configure_file(
    "${CMAKE_MODULE_PATH}/cmake_uninstall.cmake.in"
    "${CMAKE_MODULE_PATH}/cmake_uninstall.cmake"
    IMMEDIATE @ONLY)

add_custom_target(uninstall
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_MODULE_PATH}/cmake_uninstall.cmake)
	
# purge target
configure_file(
    "${CMAKE_MODULE_PATH}/cmake_purge.cmake.in"
    "${CMAKE_MODULE_PATH}/cmake_purge.cmake"
    IMMEDIATE @ONLY)

add_custom_target(purge
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_MODULE_PATH}/cmake_purge.cmake)
