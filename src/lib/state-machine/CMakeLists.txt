add_library(libstatemachine SHARED state-machine.cpp)

target_include_directories(libstatemachine
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(libstatemachine libcommon)

target_link_libraries(${PROJECT_NAME} libstatemachine)
