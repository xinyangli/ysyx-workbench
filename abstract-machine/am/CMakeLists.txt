add_subdirectory(src)

foreach(PLATFORM IN LISTS PLATFORMS)
    if(ISA MATCHES "native")
        set(ARCH "native")
    else()
        set(ARCH ${ISA}-${PLATFORM})
    endif()
    install(TARGETS am-${ARCH}
            EXPORT amTargets-${ARCH}
            LIBRARY DESTINATION lib)

    install(EXPORT amTargets-${ARCH}
            FILE amTargets.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/am-${ARCH})

    configure_package_config_file(
            ${CMAKE_SOURCE_DIR}/cmake/am-config.cmake.in
            ${CMAKE_CURRENT_BINARY_DIR}/am-${ARCH}-config.cmake
            INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/am-${ARCH})

    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/am-${ARCH}-config.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/am-${ARCH})
endforeach()
