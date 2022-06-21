find_package(PkgConfig)
pkg_search_module(GTEST REQUIRED gtest_main gtest gmock)


include(CTest)
include(GoogleTest)
enable_testing()


add_executable(hello-test hello-test.cpp)
target_link_libraries(hello-test ${GTEST_LDFLAGS})
target_compile_options(hello-test PUBLIC ${GTEST_CFLAGS})
gtest_discover_tests(hello-test)


add_executable(concurrency-test blocking-queue-test.cpp)
target_link_libraries(concurrency-test ${GTEST_LDFLAGS} libconcurrency)
target_compile_options(concurrency-test PUBLIC ${GTEST_CFLAGS})
gtest_discover_tests(concurrency-test)


add_executable(state-machine-test state-machine-test.cpp)
target_link_libraries(state-machine-test ${GTEST_LDFLAGS} libstatemachine)
target_compile_options(state-machine-test PUBLIC ${GTEST_CFLAGS})
gtest_discover_tests(state-machine-test)


add_executable(state-machine-pool-test state-machine-pool-test.cpp)
target_link_libraries(state-machine-pool-test ${GTEST_LDFLAGS} libstatemachine libconcurrency)
target_compile_options(state-machine-pool-test PUBLIC ${GTEST_CFLAGS})
gtest_discover_tests(state-machine-pool-test)
