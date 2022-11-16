cmake_minimum_required(VERSION 3.22)
project(fanout-service VERSION 0.1.0)


# compiler settings
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++20" COMPILER_SUPPORTS_CXX20)

if(NOT COMPILER_SUPPORTS_CXX20)
	message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++20 support. Please use a different C++ compiler.")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20")

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)


# path
set(CMAKE_LIBRARY_PATH /usr/lib/x86_64-linux-gnu)


# dependencies
find_package(PkgConfig)

# libpqxx
pkg_check_modules(LIBPQXX REQUIRED libpqxx)


# executable
add_subdirectory(src/bin/fanout-service)


# libraries
add_subdirectory(src/lib/common)
add_subdirectory(src/lib/conn)
add_subdirectory(src/lib/persistent-deque)


# tests
add_subdirectory(tests)


# cpack
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
