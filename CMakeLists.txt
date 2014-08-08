# Required cmake version
cmake_minimum_required(VERSION 2.4.3)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

# Put the include dirs which are in the source or build tree
# before all other include dirs, so the headers in the sources
# are prefered over the already installed ones
# since cmake 2.4.1
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

project(pvpgn)

option(WITH_BNETD "compile the bnetd target" ON)
option(WITH_D2CS "compile the d2cs target" ON)
option(WITH_D2DBS "compile the d2dbs target" ON)
option(WITH_ANSI "enforce strict ISO C++ conforming" ON)
if(WIN32)
  option(WITH_WIN32_GUI "enable GUI building (default on)" ON)
endif(WIN32)

option(WITH_LUA "enable Lua support" OFF)

#storage backends flags
option(WITH_MYSQL "include MySQL user accounts support" OFF)
option(WITH_SQLITE3 "include SQLite3 user accounts support" OFF)
option(WITH_PGSQL "include PostgreSQL user accounts support" OFF)
option(WITH_ODBC "include ODBC user accounts support" OFF)
include(ConfigureChecks.cmake)


if (CMAKE_COMPILER_IS_GNUCXX)
  # Determine GCC version.
  message("Determining GCC version.")
  EXEC_PROGRAM(${CMAKE_CXX_COMPILER}
               ARGS --version
               OUTPUT_VARIABLE _GCC_VERSION)
  STRING(REGEX REPLACE ".* ([0-9])\\.([0-9])\\.([0-9]) .*" "\\1\\2\\3"
         _GCC_VERSION ${_GCC_VERSION})
  message("  GCC version is ${_GCC_VERSION}")
  
  # Add -Wno-longlong if the GCC version is < 4.0.0.  Add -pedantic flag
  # but disable warnings for variadic macros with GCC >= 4.0.0.  Earlier
  # versions warn because of anonymous variadic macros in pedantic mode
  # but do not have a flag to disable these warnings.
  if (400 GREATER _GCC_VERSION)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-long-long")
  else (400 GREATER _GCC_VERSION)
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wno-variadic-macros")
  endif (400 GREATER _GCC_VERSION)
  
  # Pass CXX flags to flags.
  set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DSEQAN_CXX_FLAGS_=\"${CMAKE_CXX_FLAGS}\"")
endif (CMAKE_COMPILER_IS_GNUCXX)


subdirs(src conf man files)
if(WITH_LUA)
    add_subdirectory(lua)
endif(WITH_LUA)

ENABLE_TESTING()
