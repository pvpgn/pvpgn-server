# - Check if the files can be included
#
# CHECK_MKDIR_ARGS(VARIABLE)
#
#  VARIABLE - variable to return if mkdir/_mkdir need a single argument
#
# The following variables may be set before calling this macro to
# modify the way the check is run:
#
#  CMAKE_REQUIRED_FLAGS = string of compile command line flags
#  CMAKE_REQUIRED_DEFINITIONS = list of macros to define (-DFOO=bar)
#  CMAKE_REQUIRED_INCLUDES = list of include directories

MACRO(CHECK_MKDIR_ARGS VARIABLE)
  IF("${VARIABLE}" MATCHES "^${VARIABLE}$")
    SET(CMAKE_CONFIGURABLE_FILE_CONTENT "/* */\n")
    IF(CMAKE_REQUIRED_INCLUDES)
      SET(CHECK_MKDIR_ARGS_INCLUDE_DIRS "-DINCLUDE_DIRECTORIES=${CMAKE_REQUIRED_INCLUDES}")
    ELSE(CMAKE_REQUIRED_INCLUDES)
      SET(CHECK_MKDIR_ARGS_INCLUDE_DIRS)
    ENDIF(CMAKE_REQUIRED_INCLUDES)
    SET(CHECK_MKDIR_ARGS_CONTENT "/* */\n")
    SET(MACRO_CHECK_MKDIR_ARGS_FLAGS ${CMAKE_REQUIRED_FLAGS})
    SET(CMAKE_CONFIGURABLE_FILE_CONTENT
      "${CMAKE_CONFIGURABLE_FILE_CONTENT}\n#include<direct.h>\nint main(){mkdir(\"\");}\n")
    CONFIGURE_FILE("${CMAKE_ROOT}/Modules/CMakeConfigurableFile.in"
      "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckMkdirArgs.cxx" @ONLY IMMEDIATE)

    MESSAGE(STATUS "Checking for single argument mkdir ${VARIABLE}")
    TRY_COMPILE(${VARIABLE}
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CheckMkdirArgs.cxx
      COMPILE_DEFINITIONS ${CMAKE_REQUIRED_DEFINITIONS}
      CMAKE_FLAGS 
      -DCOMPILE_DEFINITIONS:STRING=${MACRO_CHECK_MKDIR_ARGS_FLAGS}
      "${CHECK_MKDIR_ARGS_INCLUDE_DIRS}"
      OUTPUT_VARIABLE OUTPUT)
    IF(${VARIABLE})
      MESSAGE(STATUS "Checking for single argument mkdir ${VARIABLE} - found")
      SET(${VARIABLE} 1 CACHE INTERNAL "Have include ${VARIABLE}")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log 
        "Determining if files ${INCLUDE} "
        "exist passed with the following output:\n"
        "${OUTPUT}\n\n")
    ELSE(${VARIABLE})
      MESSAGE(STATUS "Checking for single argument mkdir ${VARIABLE} - not found.")
      SET(${VARIABLE} "" CACHE INTERNAL "Have includes ${VARIABLE}")
      FILE(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log 
        "Determining if files ${INCLUDE} "
        "exist failed with the following output:\n"
        "${OUTPUT}\nSource:\n${CMAKE_CONFIGURABLE_FILE_CONTENT}\n")
    ENDIF(${VARIABLE})
  ENDIF("${VARIABLE}" MATCHES "^${VARIABLE}$")
ENDMACRO(CHECK_MKDIR_ARGS)
