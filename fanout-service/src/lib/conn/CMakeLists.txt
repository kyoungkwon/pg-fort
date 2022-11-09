add_library(libconn pqxx-conn.cpp)

target_include_directories(libconn
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(libconn ${LIBPQXX_LIBRARIES})

target_link_libraries(${PROJECT_NAME} libconn)
