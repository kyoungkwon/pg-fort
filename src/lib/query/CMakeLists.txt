protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS /usr/include/pg_query/pg_query.proto)

add_library(libquery SHARED ${PROTO_SRCS} ${PROTO_HDRS} query.cpp)

target_include_directories(libquery
    PUBLIC
		${CMAKE_CURRENT_BINARY_DIR}
		${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(libquery ${Protobuf_LIBRARIES} ${LIBCTEMPLATE})

target_link_libraries(${PROJECT_NAME} libquery)
