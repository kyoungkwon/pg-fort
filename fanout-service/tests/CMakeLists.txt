enable_testing()
find_package(GTest REQUIRED)


add_executable(persistent-deque-test persistent-deque-test.cpp)
target_link_libraries(persistent-deque-test GTest::gtest GTest::gtest_main libperstdq)
gtest_discover_tests(persistent-deque-test)

