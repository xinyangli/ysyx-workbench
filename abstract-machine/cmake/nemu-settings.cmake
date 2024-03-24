set(NEMU_COMPILE_OPTIONS -fdata-sections -ffunction-sections)
set(NEMU_LINK_OPTIONS
    --defsym=_pmem_start=0x80000000
    --defsym=_entry_offset=0x0
    --gc-sections
    -e _start)
set(NEMU_INCLUDE_DIRECTORIES
    ${CMAKE_SOURCE_DIR}/am/src/platform/nemu/include)
file(GLOB_RECURSE NEMU_SOURCES
    ${CMAKE_SOURCE_DIR}/am/src/platform/nemu/*.[cS])
set(INCLUDE_LINKER_SCRIPT true)
