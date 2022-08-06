protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS /usr/include/pg_query/pg_query.proto)

add_library(libquery ${PROTO_SRCS} ${PROTO_HDRS})

target_include_directories(libquery
    PUBLIC
		${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(libquery ${Protobuf_LIBRARIES})

target_link_libraries(${PROJECT_NAME} libquery)
