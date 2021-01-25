# Required cmake version
cmake_minimum_required(VERSION 3.1.0)

# Put the include dirs which are in the source or build tree
# before all other include dirs, so the headers in the sources
# are prefered over the already installed ones
# since cmake 2.4.1
set(CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE ON)

project(pvpgn CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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


if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
	# using Clang

	set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wno-idiomatic-parentheses -pedantic")

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	# using G++

	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5.1)
		message(FATAL_ERROR "G++ 5.1 or higher required")
	endif()

	set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -pedantic -Wno-variadic-macros" )

elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	# using Visual Studio

	if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 19.0)
		message(FATAL_ERROR "Visual Studio 2015 or higher required")
	endif()

	add_definitions(
		-D_CRT_SECURE_NO_DEPRECATE
		-D_CRT_NONSTDC_NO_DEPRECATE
		-DUNICODE
		-D_UNICODE
	)
	
	# DEBUG compiler flags:
	#	/Zi create debugging information PDB file
	#	/Od disable optimizations
	# 	/Oy- do not suppress frame pointers (recommended for debugging)
	#	/MTd use statically linked, thread-safe, debug CRT libs (Magic Builder set this flag when build)
	#
	# RELEASE compiler flags:
	#	/MT use statically linked, thread-safe CRT libs (Magic Builder set this flag when build)
	# 	/GS- no Buffer Security Check
	#
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi /Od /Oy-")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /Zi")

	# Explaining of linker flags and why enable pdb with debug info for Release build is on:
	#  https://www.wintellect.com/correctly-creating-native-c-release-build-pdbs
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /DEBUG /OPT:REF /OPT:ICF")
endif()

add_subdirectory(conf)
add_subdirectory(files)
add_subdirectory(lib/fmt)
add_subdirectory(man)
add_subdirectory(src)
if(WITH_LUA)
    add_subdirectory(lua)
endif(WITH_LUA)

enable_testing()

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
