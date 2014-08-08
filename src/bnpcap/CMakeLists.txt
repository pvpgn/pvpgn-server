add_executable(bnpcap bnpcap.cpp)
target_link_libraries(bnpcap common pcap)

install(TARGETS bnpcap DESTINATION ${BINDIR})
