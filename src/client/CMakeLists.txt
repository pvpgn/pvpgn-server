add_executable(bnchat bnchat.cpp client.cpp client.h client_connect.cpp 
	client_connect.h udptest.cpp udptest.h ansi_term.h)
target_link_libraries(bnchat PRIVATE common compat fmt::fmt ${NETWORK_LIBRARIES})

add_executable(bnftp bnftp.cpp client.cpp client.h)
target_link_libraries(bnftp PRIVATE common compat fmt::fmt ${NETWORK_LIBRARIES})

add_executable(bnbot bnbot.cpp client.cpp client.h)
target_link_libraries(bnbot PRIVATE common compat fmt::fmt ${NETWORK_LIBRARIES})

add_executable(bnstat bnstat.cpp client.cpp client.h client_connect.cpp 
	client_connect.h udptest.cpp udptest.h)
target_link_libraries(bnstat PRIVATE common compat fmt::fmt ${NETWORK_LIBRARIES})

install(TARGETS bnchat bnftp bnbot bnstat DESTINATION ${BINDIR})
