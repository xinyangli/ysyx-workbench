include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(klib_interface INTERFACE)
target_include_directories(klib_interface
    INTERFACE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:include/abstract-machine>)

add_subdirectory(src)
# add_subdirectory(tests)

install(DIRECTORY include/ DESTINATION include/abstract-machine)
