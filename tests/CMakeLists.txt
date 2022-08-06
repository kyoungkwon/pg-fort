enable_testing()
find_package(GTest REQUIRED)


add_executable(hello-test hello-test.cpp)
target_link_libraries(hello-test GTest::gtest GTest::gtest_main)
gtest_discover_tests(hello-test)


add_executable(concurrency-test blocking-queue-test.cpp)
target_link_libraries(concurrency-test GTest::gtest GTest::gtest_main libconcurrency)
gtest_discover_tests(concurrency-test)


add_executable(state-machine-test state-machine-test.cpp)
target_link_libraries(state-machine-test GTest::gtest GTest::gtest_main libstatemachine)
gtest_discover_tests(state-machine-test)


add_executable(state-machine-pool-test state-machine-pool-test.cpp)
target_link_libraries(state-machine-pool-test GTest::gtest GTest::gtest_main libstatemachine libconcurrency)
gtest_discover_tests(state-machine-pool-test)


add_executable(pg-query-test pg-query-test.cpp pg-query-manip.cpp)
target_link_libraries(pg-query-test GTest::gtest GTest::gtest_main ${LIBPG_QUERY} libquery)
gtest_discover_tests(pg-query-test)
