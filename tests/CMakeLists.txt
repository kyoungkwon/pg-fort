find_package(PkgConfig)
pkg_search_module(GTEST REQUIRED gtest_main gtest gmock)


include(CTest)
include(GoogleTest)
enable_testing()


add_executable(vector-test vector-test.cpp)
target_link_libraries(vector-test ${GTEST_LDFLAGS})
target_compile_options(vector-test PUBLIC ${GTEST_CFLAGS})
gtest_discover_tests(vector-test)


add_executable(concurrency-test blocking-queue-test.cpp)
target_link_libraries(concurrency-test ${GTEST_LDFLAGS} libconcurrency)
target_compile_options(concurrency-test PUBLIC ${GTEST_CFLAGS})
gtest_discover_tests(concurrency-test)
