# NOTE: klib include some header files in am
add_library(am_interface INTERFACE)
target_include_directories(am_interface INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include/abstract-machine>)

set(AM_COMMON_COMPILE_DEF
    # -- Arch related
    $<MAKE_C_IDENTIFIER:__ARCH_${ARCH_UPPER}__>
    __ISA_${ISA_UPPER}__
    __PLATFORM_${PLATFORM_UPPER}__

    $<$<BOOL:${NATIVE_USE_KLIB}>:__NATIVE_USE_KLIB__>
)

add_subdirectory(src)

install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/abstract-machine)
