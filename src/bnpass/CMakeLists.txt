add_executable(bnpass bnpass.cpp)
target_link_libraries(bnpass common compat)

add_executable(sha1hash sha1hash.cpp)
target_link_libraries(sha1hash common compat)

install(TARGETS bnpass sha1hash DESTINATION ${BINDIR})
