cmake_minimum_required(VERSION 3.22)
project(postgresql-proxy VERSION 0.1.0)


# compiler settings
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++20" COMPILER_SUPPORTS_CXX20)

if(NOT COMPILER_SUPPORTS_CXX20)
	message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. Please use a different C++ compiler.")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)


# dependencies
find_library(LIBPG_QUERY pg_query PATHS /usr/lib/x86_64-linux-gnu)


# executable
add_executable(${PROJECT_NAME} src/bin/proxy-server/main.cpp)


# libraries
add_subdirectory(src/lib/common)
add_subdirectory(src/lib/concurrency)
add_subdirectory(src/lib/conn)
add_subdirectory(src/lib/session)
add_subdirectory(src/lib/state-machine)
add_subdirectory(src/lib/proxy-server)


# tests
add_subdirectory(tests)


# cpack
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
