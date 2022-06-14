add_library(libconcurrency thread-pool.cpp)

target_include_directories(libconcurrency
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} libconcurrency)
