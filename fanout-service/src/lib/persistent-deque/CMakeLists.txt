add_library(libperstdq persistent-deque.cpp)

target_include_directories(libperstdq
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(libperstdq libcommon libconn)

target_link_libraries(${PROJECT_NAME} libperstdq)
