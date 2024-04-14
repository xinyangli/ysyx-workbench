set(TEST_SOURCES
    stdio
    string
)

foreach(TEST IN LISTS TEST_SOURCES)
    add_executable(${TEST} ${TEST}.c)
    # TODO: Run tests in other configurations
    if(__PLATFORM_NATIVE__)
        target_link_libraries(${TEST} PRIVATE klib_interface am_interface am-native klib m)
        add_test(NAME ${TEST} COMMAND ${TEST})
    endif()
endforeach()
