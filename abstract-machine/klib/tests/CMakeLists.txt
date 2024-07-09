set(TEST_SOURCES stdio string)

foreach(TEST IN LISTS TEST_SOURCES)
  # TODO: Run tests in other configurations
  if(__PLATFORM_NATIVE__)
    add_executable(${TEST} ${TEST}.c)
    target_link_libraries(${TEST} PRIVATE am_interface klib_interface klib m)
    target_link_libraries(${TEST} PRIVATE am-native)
    add_test(NAME ${TEST} COMMAND ${TEST})
  endif()
endforeach()
