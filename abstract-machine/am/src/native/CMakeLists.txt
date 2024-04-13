set(SOURCES
    trap.S
    cte.c
    ioe.c
    mpe.c
    platform.c
    trm.c
    vme.c
    ioe/audio.c
    ioe/disk.c
    ioe/gpu.c
    ioe/input.c
    ioe/timer.c
)
add_library(am-native ${SOURCES})

# FIXME: get free(): invalid address when user program compiled without pie
set_target_properties(am-native PROPERTIES
    POSITION_INDEPENDENT_CODE TRUE
    INTERFACE_POSITION_INDEPENDENT_CODE TRUE)

find_package(SDL2 REQUIRED)
target_link_libraries(am-native PUBLIC SDL2::SDL2 PRIVATE klib_interface am_interface)
