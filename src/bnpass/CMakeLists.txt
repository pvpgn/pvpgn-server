add_executable(bnpass bnpass.cpp)
target_link_libraries(bnpass PRIVATE common compat fmt::fmt)

add_executable(sha1hash sha1hash.cpp)
target_link_libraries(sha1hash PRIVATE common compat fmt::fmt)

install(TARGETS bnpass sha1hash DESTINATION ${BINDIR})
