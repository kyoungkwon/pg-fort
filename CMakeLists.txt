cmake_minimum_required(VERSION 3.23)
project(postgresql-proxy VERSION 0.1.0)


# compiler settings
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++11" COMPILER_SUPPORTS_CXX11)

if(NOT COMPILER_SUPPORTS_CXX11)
	message(FATAL_ERROR "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)


# executable
add_executable(${PROJECT_NAME} src/bin/proxy-server/main.cpp)


# libraries
add_subdirectory(src/lib/common)
add_subdirectory(src/lib/concurrency)
add_subdirectory(src/lib/conn)
add_subdirectory(src/lib/session)
add_subdirectory(src/lib/proxy-server)


# tests
add_subdirectory(tests)


# cpack
set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
