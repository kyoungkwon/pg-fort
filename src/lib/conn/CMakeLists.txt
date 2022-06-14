add_library(libconn dbconn.cpp)

target_include_directories(libconn
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(libconn libcommon)

target_link_libraries(${PROJECT_NAME} libconn)
