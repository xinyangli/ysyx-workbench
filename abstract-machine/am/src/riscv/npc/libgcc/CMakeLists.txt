add_library(mingcc
    ashldi3.c
    div.S
    muldi3.S
    multi3.c
    unused.c
)

target_link_libraries(mingcc PRIVATE klib_interface am_interface)
target_compile_definitions(mingcc PRIVATE ARCH_H=<arch/riscv.h>)
target_link_options(mingcc INTERFACE -nolibc -nostdlib)

install(TARGETS mingcc
        EXPORT amTargets
        LIBRARY DESTINATION lib)
