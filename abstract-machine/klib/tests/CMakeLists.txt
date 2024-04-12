set(TEST_SOURCES
    stdio
    string
)

foreach(TEST IN LISTS TEST_SOURCES)
    add_executable(${TEST} ${TEST}.c)
    target_link_libraries(${TEST} am-${ARCH} klib m)
    target_include_directories(${TEST}
        # PRIVATE $<TARGET_PROPERTY:am_interface,INTERFACE_INCLUDE_DIRECTORIES>
        PRIVATE $<TARGET_PROPERTY:klib_interface,INTERFACE_INCLUDE_DIRECTORIES>
    )
    # TODO: Run tests in other configurations
    if(__PLATFORM_NATIVE__)
        add_test(NAME ${TEST} COMMAND ${TEST})
    endif()
endforeach()
