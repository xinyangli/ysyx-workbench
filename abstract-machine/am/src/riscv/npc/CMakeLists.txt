include(riscv-settings)

add_subdirectory(libgcc)

add_library(am-riscv-npc
    cte.c
    input.c
    ioe.c
    mpe.c
    start.S
    timer.c
    trap.S
    trm.c
    vme.c
)

target_link_libraries(am-riscv-npc PRIVATE npcgcc PUBLIC am_interface klib_interface)

target_link_options(am-riscv-npc INTERFACE
    $<BUILD_INTERFACE:-T${CMAKE_SOURCE_DIR}/scripts/linker.ld>
    $<INSTALL_INTERFACE:-T${CMAKE_INSTALL_FULL_DATADIR}/linker.ld>)

target_link_options(am-riscv-npc INTERFACE
    LINKER:--defsym=_pmem_start=0x80000000
    LINKER:--defsym=_entry_offset=0x0
    LINKER:--gc-sections
    LINKER:-e _start
    -nostartfiles)

target_link_options(am-riscv-npc INTERFACE
    $<BUILD_INTERFACE:-T${CMAKE_SOURCE_DIR}/scripts/linker.ld>
    $<INSTALL_INTERFACE:-T${CMAKE_INSTALL_FULL_DATADIR}/linker.ld>)

target_compile_definitions(am-riscv-npc
    PUBLIC ${AM_COMMON_COMPILE_DEF} ARCH_H=<arch/riscv.h>
)

install(FILES ${CMAKE_SOURCE_DIR}/scripts/linker.ld
        DESTINATION ${CMAKE_INSTALL_DATADIR})
