add_library(libschema schema-tracker.cpp)

target_include_directories(libschema
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(libschema ${LIBPQXX_LIBRARIES} libconn)

target_link_libraries(${PROJECT_NAME} libschema)
