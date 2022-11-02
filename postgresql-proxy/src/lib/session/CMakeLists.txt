add_library(libsession session.cpp session-operator.cpp plugin.cpp)

target_include_directories(libsession
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(libsession libcommon libconcurrency libconn libstatemachine libquery libschema)

target_link_libraries(${PROJECT_NAME} libsession)
