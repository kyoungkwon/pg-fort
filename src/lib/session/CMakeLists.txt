add_library(libsession session.cpp)

target_include_directories(libsession
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(libsession libcommon libconn)

target_link_libraries(${PROJECT_NAME} libsession)
