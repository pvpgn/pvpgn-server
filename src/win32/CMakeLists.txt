set(WIN32_SOURCES
  service.cpp service.h console_output.h console_output.cpp dirent.h
)

add_library(win32 ${WIN32_SOURCES})
