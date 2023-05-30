add_library(libcommon INTERFACE)

target_include_directories(libcommon
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} libcommon)
