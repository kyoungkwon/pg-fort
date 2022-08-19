add_library(libproxyserver proxy-server.cpp)

target_include_directories(libproxyserver
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(libproxyserver libconn libsession)

target_link_libraries(${PROJECT_NAME} libproxyserver)
