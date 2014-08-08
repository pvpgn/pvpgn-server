add_executable(bnetsrp3_test bnetsrp3_test.cpp )
target_link_libraries(bnetsrp3_test common)
ADD_TEST(bnetsrp3_test bnetsrp3_test)

add_executable(bigint bigint.cpp )
target_link_libraries(bigint common)
ADD_TEST(bigint bigint)
