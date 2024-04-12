# -- Add an always run target to generate verilog files with sbt/bloop,
#    as we don't know if the result files will be different from cmake
# NOTE: Must reconfigure if we add new files in SCALA_CORE directory
file(GLOB_RECURSE SCALA_CORE_SOURCES "${SCALA_CORE}/src/main/scala/*.scala")
file(GLOB_RECURSE SCALA_CORE_RESOURCES "${SCALA_CORE}/src/main/resources/*")
message(STATUS "Found scala source file: ${SCALA_CORE_SOURCES}")
set(CHISEL_DEPENDENCY ${SCALA_CORE_SOURCES} ${SCALA_CORE_RESOURCES} ${SCALA_CORE}/build.sbt)

if(BUILD_USE_BLOOP)
  set(CHISEL_TARGET bloop_${TOPMODULE})
  set(CHISEL_TEST_TARGET bloop_${TOPMODULE}_test)
  # Export sbt build config to bloop
  if(NOT EXISTS ${SCALA_CORE}/.bloop)
    execute_process(
      COMMAND sbt bloopInstall
      WORKING_DIRECTORY ${SCALA_CORE}
    )
  endif()
  string(REPLACE " " ";" CHISEL_EMIT_ARGS_LIST ${CHISEL_EMIT_ARGS})
  list(TRANSFORM CHISEL_EMIT_ARGS_LIST PREPEND "--args;")
  add_custom_command(
    OUTPUT ${CHISEL_OUTPUT_TOPMODULE} ${CHISEL_OUTPUT_VERILATOR_CONF}
    COMMAND bloop run root ${CHISEL_EMIT_ARGS_LIST}
    WORKING_DIRECTORY ${SCALA_CORE}
    DEPENDS ${CHISEL_DEPENDENCY}
    COMMAND_EXPAND_LISTS
    COMMENT "Run bloop from CMake"
  )
#   add_test(
#     NAME bloop_${TOPMODULE}_test
#     COMMAND bloop test
#     WORKING_DIRECTORY ${SCALA_CORE}
#   )
else()
  set(CHISEL_TARGET sbt_${TOPMODULE})
  set(CHISEL_TEST_TARGET sbt_${TOPMODULE}_test)
  add_custom_command(
    OUTPUT ${CHISEL_OUTPUT_TOPMODULE} ${CHISEL_OUTPUT_VERILATOR_CONF}
    COMMAND sbt "run ${CHISEL_EMIT_ARGS}"
    WORKING_DIRECTORY ${SCALA_CORE}
    DEPENDS ${CHISEL_DEPENDENCY}
    VERBATIM
    COMMENT "Run sbt from CMake"
  )
  add_test(
    NAME sbt_${TOPMODULE}_test
    COMMAND sbt test
    WORKING_DIRECTORY ${SCALA_CORE}
  )
endif()

if(NOT EXISTS ${CHISEL_OUTPUT_DIR})
  # Probably cold build, generate verilog at configure time to produce top module file
  execute_process(
    COMMAND sbt "run ${CHISEL_EMIT_ARGS}"
    WORKING_DIRECTORY ${SCALA_CORE}
  )
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CHISEL_OUTPUT_TMP_DIR} ${CHISEL_OUTPUT_DIR}
  )
endif()
