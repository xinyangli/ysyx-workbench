if(ISA MATCHES "native")
set(SOURCEDIR "./${PLATFORM}")
else()
set(SOURCEDIR "./${ISA}/${PLATFORM}")
endif()

add_subdirectory(${SOURCEDIR})
# TODO: Check
target_link_options(am-${ARCH} INTERFACE
    $<BUILD_INTERFACE:-T${CMAKE_SOURCE_DIR}/scripts/${LINKER_SCRIPT}>
    $<INSTALL_INTERFACE:-T${CMAKE_INSTALL_FULL_DATADIR}/${LINKER_SCRIPT}>)

# Interface compile flags
target_link_options(am-${ARCH} INTERFACE
        -znoexecstack)

target_compile_options(am-${ARCH} INTERFACE
        -fno-asynchronous-unwind-tables
        -fno-builtin
        -fno-stack-protector
        -U_FORTIFY_SOURCE
        $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
        $<$<COMPILE_LANGUAGE:CXX>:-ffreestanding>
        $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)

install(TARGETS am-${ARCH} klib_interface am_interface
        EXPORT amTargets
        LIBRARY DESTINATION lib)

install(EXPORT amTargets 
        FILE amTargets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/am-${ARCH})

configure_package_config_file(${CMAKE_SOURCE_DIR}/cmake/am-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/am-${ARCH}-config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/am-${ARCH})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/am-${ARCH}-config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/am-${ARCH})

# TODO: check
install(FILES ${CMAKE_SOURCE_DIR}/scripts/${LINKER_SCRIPT}
        DESTINATION ${CMAKE_INSTALL_DATADIR})
