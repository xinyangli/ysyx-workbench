cmake_minimum_required(VERSION 3.26)

project(flow)
set (CMAKE_CXX_STANDARD 14)
cmake_policy(SET CMP0144 NEW)

include(CMakeDependentOption)
enable_testing()

# -- Build options
option(BUILD_USE_BLOOP "Whether to use bloop to spped up elaborate" ON)
option(BUILD_SIM_TARGET "Whether to build verilator simulation binary" ON)
cmake_dependent_option(BUILD_SIM_NVBOARD_TARGET "Whether to build nvboard target" OFF "BUILD_SIM_TARGET" OFF)
option(ENABLE_YSYX_GIT_TRACKER "Ysyx tracker support" OFF)
set(TOPMODULE "Flow" CACHE STRING "Topmodule name in chisel")

# -- Ysyx tracker, configure
if(ENABLE_YSYX_GIT_TRACKER)
  execute_process(
    COMMAND ${CMAKE_SOURCE_DIR}/../git_commit.sh "configure(npc)"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..
  )
endif()

# -- Check dependencies
if(BUILD_SIM_TARGET)
  find_package(verilator REQUIRED)
endif()
if(BUILD_SIM_NVBOARD_TARGET)
  find_package(SDL2 REQUIRED)
  find_package(SDL2_image REQUIRED)
endif()

find_library(NVBOARD_LIBRARY NAMES nvboard)
find_path(NVBOARD_INCLUDE_DIR NAMES nvboard.h)

# FIXME: all scala source file are tracked here, cause all files to rebuild 
# after a source update. 
set(SCALA_CORE "${CMAKE_CURRENT_SOURCE_DIR}/core")
set(CHISEL_MODULE_CLASS "${CMAKE_PROJECT_NAME}.${TOPMODULE}")

# Verilog files are generted in CHISEL_OUTPUT_TMP_DIR and copy to 
# CHISEL_OUTPUT_DIR if content changes
set(CHISEL_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/vsrc/)
set(CHISEL_OUTPUT_TMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/vsrc_tmp/)

set(CHISEL_OUTPUT_VERILATOR_CONF ${CHISEL_OUTPUT_DIR}/conf.vlt)
set(CHISEL_OUTPUT_TOPMODULE ${CHISEL_OUTPUT_DIR}/${TOPMODULE}.sv)
set(CHISEL_EMIT_ARGS "--target-dir ${CHISEL_OUTPUT_TMP_DIR}")

# -- Add an always run target to generate verilog files with sbt/bloop,
#    as we don't know if the result files will be different from cmake
# NOTE: Must reconfigure if we add new files in SCALA_CORE directory
file(GLOB_RECURSE SCALA_CORE_SOURCES "${SCALA_CORE}/src/main/scala/*.scala")
file(GLOB_RECURSE SCALA_CORE_RESOURCES "${SCALA_CORE}/src/resource/*")
set(CHISEL_DEPENDENCY ${SCALA_CORE_SOURCES} ${SCALA_CORE_RESOURCE} ${SCALA_CORE}/build.sbt)

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
    OUTPUT ${CHISEL_OUTPUT_TOPMODULE}
    COMMAND bloop run root ${CHISEL_EMIT_ARGS_LIST}
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CHISEL_OUTPUT_TMP_DIR} ${CHISEL_OUTPUT_DIR}
    WORKING_DIRECTORY ${SCALA_CORE}
    DEPENDS ${CHISEL_DEPENDENCY}
    COMMAND_EXPAND_LISTS
  )
  add_test(
    NAME bloop_${TOPMODULE}_test
    COMMAND bloop test
    WORKING_DIRECTORY ${SCALA_CORE}
  )
else()
  set(CHISEL_TARGET sbt_${TOPMODULE})
  set(CHISEL_TEST_TARGET sbt_${TOPMODULE}_test)
  add_custom_command(
    OUTPUT ${CHISEL_OUTPUT_TOPMODULE}
    COMMAND sbt "run ${CHISEL_EMIT_ARGS}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CHISEL_OUTPUT_TMP_DIR} ${CHISEL_OUTPUT_DIR}
    WORKING_DIRECTORY ${SCALA_CORE}
    DEPENDS ${CHISEL_DEPENDENCY}
    VERBATIM
  )
  add_test(
    NAME sbt_${TOPMODULE}_test
    COMMAND sbt test
    WORKING_DIRECTORY ${SCALA_CORE}
  )
endif()

if(NOT EXISTS ${CHISEL_OUTPUT_TOPMODULE})
  # Probably cold build, generate verilog at configure time to produce top module file
  execute_process(
    COMMAND sbt "run ${CHISEL_EMIT_ARGS}"
    WORKING_DIRECTORY ${SCALA_CORE}
  )
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different ${CHISEL_OUTPUT_TMP_DIR} ${CHISEL_OUTPUT_DIR}
  )
endif()

# -- Build NVBoard executable
if(BUILD_SIM_NVBOARD_TARGET)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/auto_bind.cpp
    COMMAND auto_pin_bind ${CMAKE_SOURCE_DIR}/constr/${TOPMODULE}.nxdc ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/auto_bind.cpp
    DEPENDS ${CMAKE_SOURCE_DIR}/constr/${TOPMODULE}.nxdc 
  )

  file(GLOB_RECURSE SOURCES csrc_nvboard/${TOPMODULE}/*.cpp)
  add_executable(V${TOPMODULE}_nvboard ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/auto_bind.cpp)

  verilate(V${TOPMODULE}_nvboard TRACE THREADS
    TOP_MODULE ${TOPMODULE}
    PREFIX V${TOPMODULE}
    SOURCES ${CHISEL_OUTPUT_TOPMODULE}
    INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/vsrc)

  add_dependencies(V${TOPMODULE}_nvboard ChiselBuild_${TOPMODULE})
  target_include_directories(V${TOPMODULE}_nvboard PRIVATE ${NVBOARD_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})
  target_link_libraries(V${TOPMODULE}_nvboard PRIVATE ${NVBOARD_LIBRARY} SDL2::SDL2 SDL2_image::SDL2_image)

  install(TARGETS V${TOPMODULE}_nvboard)
endif()

# -- Build Verilator executable and add to test
file(GLOB_RECURSE SOURCES csrc/${TOPMODULE}/*.cpp)
add_executable(V${TOPMODULE} ${SOURCES})

verilate(V${TOPMODULE} TRACE COVERAGE THREADS
  TOP_MODULE ${TOPMODULE}
  PREFIX V${TOPMODULE}
  SOURCES ${CHISEL_OUTPUT_TOPMODULE} ${CHISEL_OUTPUT_VERILATOR_CONF}
  INCLUDE_DIRS ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/vsrc
  VERILATOR_ARGS
    "--vpi"   # Enable VPI
    
)


add_test(NAME V${TOPMODULE} COMMAND V${TOPMODULE})

# -- Add build tracking
if(ENABLE_YSYX_GIT_TRACKER)
  add_custom_command(
      TARGET V${TOPMODULE}_nvboard PRE_BUILD
      COMMAND ${CMAKE_SOURCE_DIR}/../git_commit.sh "build_${CMAKE_PROJECT_NAME}_V${TOPMODULE}_nvboard"
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..
  )

  add_custom_command(
      TARGET V${TOPMODULE} PRE_BUILD
      COMMAND ${CMAKE_SOURCE_DIR}/../git_commit.sh "build_${CMAKE_PROJECT_NAME}_V${TOPMODULE}"
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..
  )
endif()
