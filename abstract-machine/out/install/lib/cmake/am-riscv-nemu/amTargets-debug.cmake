#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "am-riscv-nemu" for configuration "Debug"
set_property(TARGET am-riscv-nemu APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(am-riscv-nemu PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "ASM;C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libam-riscv-nemu.a"
  )

list(APPEND _cmake_import_check_targets am-riscv-nemu )
list(APPEND _cmake_import_check_files_for_am-riscv-nemu "${_IMPORT_PREFIX}/lib/libam-riscv-nemu.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
