add_executable(bntrackd bntrackd.cpp)
target_link_libraries(bntrackd common compat ${NETWORK_LIBRARIES})
install(TARGETS bntrackd DESTINATION ${SBINDIR})
