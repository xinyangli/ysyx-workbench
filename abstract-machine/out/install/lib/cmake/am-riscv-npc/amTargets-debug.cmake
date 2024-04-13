#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "mingcc" for configuration "Debug"
set_property(TARGET mingcc APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(mingcc PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "ASM;C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libmingcc.a"
  )

list(APPEND _cmake_import_check_targets mingcc )
list(APPEND _cmake_import_check_files_for_mingcc "${_IMPORT_PREFIX}/lib/libmingcc.a" )

# Import target "am-riscv-npc" for configuration "Debug"
set_property(TARGET am-riscv-npc APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(am-riscv-npc PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "ASM;C"
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/lib/libam-riscv-npc.a"
  )

list(APPEND _cmake_import_check_targets am-riscv-npc )
list(APPEND _cmake_import_check_files_for_am-riscv-npc "${_IMPORT_PREFIX}/lib/libam-riscv-npc.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
