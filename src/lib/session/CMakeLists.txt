add_library(libsession session.cpp session-pool.cpp)

target_include_directories(libsession
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(libsession libcommon libconcurrency libconn libstatemachine)

target_link_libraries(${PROJECT_NAME} libsession)
