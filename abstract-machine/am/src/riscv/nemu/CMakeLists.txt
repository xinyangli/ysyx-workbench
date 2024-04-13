include(nemu-settings)
include(riscv-settings)

add_library(am-riscv-nemu
    cte.c
    start.S
    trap.S
    vme.c
    ${NEMU_SOURCES}
)

target_compile_options(am-riscv-nemu PRIVATE
    ${NEMU_COMPILE_OPTIONS}
    ${RISCV_COMPILE_OPTIONS})
target_link_options(am-riscv-nemu PRIVATE
    ${NEMU_LINK_OPITIONS}
    ${RISCV_LINK_OPTIONS})
target_include_directories(am-riscv-nemu PRIVATE
    ${NEMU_INCLUDE_DIRECTORIES})
target_link_options(am-riscv-nemu INTERFACE
    LINKER:--defsym=_pmem_start=0x80000000
    LINKER:--defsym=_entry_offset=0x0
    LINKER:--gc-sections
    LINKER:-e _start
    -nostartfiles)

target_include_directories(am-riscv-nemu
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/am/include>
      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/abstract-machine>)
target_link_libraries(am-${ARCH}
    PRIVATE klib_interface
    INTERFACE m)

target_compile_definitions(am-riscv-nemu PUBLIC
    ARCH_H="arch/riscv.h")
target_compile_definitions(am-riscv-nemu PRIVATE
    ISA_H="riscv/riscv.h")

set_target_properties(am-${ISA}-nemu PROPERTIES
    POSITION_INDEPENDENT_CODE OFF
    INTERFACE_POSITION_INDEPENDENT_CODE OFF)
