add_library(npcgcc ashldi3.c div.S muldi3.S multi3.c unused.c)

target_link_libraries(npcgcc PRIVATE klib_interface am_interface)
target_link_options(npcgcc INTERFACE -nolibc -nostdlib)

install(
  TARGETS npcgcc
  EXPORT amTargets-riscv-npc
  LIBRARY DESTINATION lib)
