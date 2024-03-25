include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(am_interface INTERFACE)
target_include_directories(am_interface INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include/abstract-machine>)

add_subdirectory(src)

install(DIRECTORY include/ DESTINATION include/abstract-machine)
