set(WIN32_SOURCES
  service.cpp service.h console_output.h console_output.cpp dirent.h windump.cpp windump.h
)

add_library(win32 STATIC ${WIN32_SOURCES})

target_compile_definitions(win32 PUBLIC UNICODE _UNICODE)