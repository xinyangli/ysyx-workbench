find_package(am-${ARCH})
find_package(klib)

set(SOURCES
    add.c
    add-longlong.c
    bit.c
    bubble-sort.c
    crc32.c
    div.c
    dummy.c
    fact.c
    fib.c
    goldbach.c
    hello-str.c
    if-else.c
    leap-year.c
    load-store.c
    matrix-mul.c
    max.c
    mersenne.c
    min3.c
    mov-c.c
    movsx.c
    mul-longlong.c
    pascal.c
    prime.c
    quick-sort.c
    recursion.c
    select-sort.c
    shift.c
    shuixianhua.c
    string.c
    sub-longlong.c
    sum.c
    switch.c
    to-lower-case.c
    unalign.c
    wanshu.c)

foreach(SOURCE IN LISTS SOURCES)
    get_filename_component(SOURCE_NAME ${SOURCE} NAME_WLE)
    add_executable(${SOURCE_NAME}
        ${SOURCE})
    target_link_libraries(${SOURCE_NAME} PRIVATE am-${ARCH})

    # -- Extract binary file from ELF
    add_custom_command(TARGET ${SOURCE_NAME}
        COMMAND ${CMAKE_OBJCOPY} ARGS  -S --set-section-flags .bss=alloc,contents -O binary ${SOURCE_NAME} ${SOURCE_NAME}.bin)

    install(TARGETS ${SOURCE_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_DATADIR}/elf)
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_NAME}.bin DESTINATION ${CMAKE_INSTALL_DATADIR}/binary)
endforeach()
