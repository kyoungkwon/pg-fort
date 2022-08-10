add_library(libsession SHARED session.cpp session-operator.cpp)

target_include_directories(libsession
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(libsession libcommon libconcurrency libconn libstatemachine)

target_link_libraries(${PROJECT_NAME} libsession)
