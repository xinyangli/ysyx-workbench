cmake_minimum_required(VERSION 3.26)

project(flow)
set (CMAKE_CXX_STANDARD 17)
cmake_policy(SET CMP0144 NEW)

include(CMakeDependentOption)
include(CTest)
enable_testing()
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

# -- Build options
option(BUILD_USE_BLOOP "Whether to use bloop to speed up elaborate" ON)
option(BUILD_SIM_TARGET "Whether to build verilator simulation binary" ON)
cmake_dependent_option(BUILD_SIM_NVBOARD_TARGET "Whether to build nvboard target" OFF "BUILD_SIM_TARGET" OFF)
option(ENABLE_YSYX_GIT_TRACKER "Ysyx tracker support" ON)
set(TOPMODULE "Flow" CACHE STRING "Topmodule name in chisel")
set(DIFFTEST_LIB "" CACHE STRING "Dynamic library file used as difftest reference")

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
find_package(CLI11 CONFIG REQUIRED)
# TODO: Not required
find_package(LLVM CONFIG REQUIRED)

option(ENABLE_SDB "Enable simple debugger" ON)

find_library(NVBOARD_LIBRARY NAMES nvboard)
find_path(NVBOARD_INCLUDE_DIR NAMES nvboard.h)

# FIXME: all scala source file are tracked here, cause all files to rebuild
# after a source update.
set(SCALA_CORE "${CMAKE_CURRENT_SOURCE_DIR}/core")
set(CHISEL_MODULE_CLASS "${CMAKE_PROJECT_NAME}.${TOPMODULE}")

# Verilog files are generted in CHISEL_OUTPUT_TMP_DIR and copy to
# CHISEL_OUTPUT_DIR if content changes
set(CHISEL_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/vsrc)
set(CHISEL_OUTPUT_TMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/vsrc_tmp)

set(CHISEL_OUTPUT_VERILATOR_CONF ${CHISEL_OUTPUT_DIR}/conf.vlt)
set(CHISEL_OUTPUT_TOPMODULE ${CHISEL_OUTPUT_DIR}/${TOPMODULE}.sv)
set(CHISEL_EMIT_ARGS "--target-dir ${CHISEL_OUTPUT_TMP_DIR}")

# -- Build NVBoard executable
if(BUILD_SIM_NVBOARD_TARGET)
  add_subdirectory(csrc_nvboard)
endif()

# -- Build Verilator executable and add to test
include_directories(include)
add_subdirectory(utils)

add_subdirectory(csrc)

# -- Add build tracking
if(ENABLE_YSYX_GIT_TRACKER)
  # add_custom_command(
  #     TARGET V${TOPMODULE}_nvboard PRE_BUILD
  #     COMMAND ${CMAKE_SOURCE_DIR}/../git_commit.sh "build_${CMAKE_PROJECT_NAME}_V${TOPMODULE}_nvboard"
  #     WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..
  # )

  add_custom_target(
    ysyx_git_tracer
    COMMAND ${CMAKE_SOURCE_DIR}/../git_commit.sh "build_${CMAKE_PROJECT_NAME}_V${TOPMODULE}"
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..
  )
  add_dependencies(V${TOPMODULE} ysyx_git_tracer)
endif()
