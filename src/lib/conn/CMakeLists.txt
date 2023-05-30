add_library(libconn buffer.cpp client-conn.cpp server-conn.cpp pqxx-conn.cpp)

target_include_directories(libconn
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(libconn libcommon libconcurrency ${LIBPQXX_LIBRARIES})

target_link_libraries(${PROJECT_NAME} libconn)
