add_library(libconcurrency INTERFACE)

target_include_directories(libconcurrency
	INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} libconcurrency)
