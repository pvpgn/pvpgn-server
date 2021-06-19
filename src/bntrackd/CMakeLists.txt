add_executable(bntrackd bntrackd.cpp)
target_link_libraries(bntrackd PRIVATE common compat fmt::fmt ${NETWORK_LIBRARIES})
install(TARGETS bntrackd DESTINATION ${SBINDIR})
