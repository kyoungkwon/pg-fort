enable_testing()
find_package(GTest REQUIRED)


add_executable(hello-test hello-test.cpp)
target_link_libraries(hello-test GTest::gtest GTest::gtest_main)
gtest_discover_tests(hello-test)


add_executable(blocking-queue-test blocking-queue-test.cpp)
target_link_libraries(blocking-queue-test GTest::gtest GTest::gtest_main libconcurrency)
gtest_discover_tests(blocking-queue-test)


add_executable(state-machine-test state-machine-test.cpp)
target_link_libraries(state-machine-test GTest::gtest GTest::gtest_main libstatemachine)
gtest_discover_tests(state-machine-test)


add_executable(state-machine-pool-test state-machine-pool-test.cpp)
target_link_libraries(state-machine-pool-test GTest::gtest GTest::gtest_main libstatemachine libconcurrency)
gtest_discover_tests(state-machine-pool-test)


add_executable(query-test query-test.cpp)
target_link_libraries(query-test GTest::gtest GTest::gtest_main libquery ${LIBPG_QUERY})
gtest_discover_tests(query-test)


add_executable(pqxx-test pqxx-test.cpp)
target_link_libraries(pqxx-test GTest::gtest GTest::gtest_main libconn)
gtest_discover_tests(pqxx-test)
