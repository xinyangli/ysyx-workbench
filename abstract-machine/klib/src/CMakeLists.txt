# find_package(FLEX)
# find_package(BISON)

# FLEX_TARGET(fmt_scanner fmt_scanner.l fmt_scanner.c)

set(SOURCES
    cpp.c
    int64.c
    stdio.c
    stdlib.c
    string.c
    # ${FLEX_fmt_scanner_OUTPUTS}
)

add_library(klib ${SOURCES})
target_link_libraries(klib PRIVATE am_interface klib_interface)

install(TARGETS klib
        EXPORT klibTargets
        LIBRARY DESTINATION lib)

install(EXPORT klibTargets
        FILE klibTargets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/klib)

configure_package_config_file(${CMAKE_SOURCE_DIR}/cmake/klib-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/klib-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/klib)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/klib-config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/klib)

