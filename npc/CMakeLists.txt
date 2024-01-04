cmake_minimum_required(VERSION 3.10)

project(NPC_xin)

find_package(verilator)
if (NOT verilator_FOUND)
  message(FATAL_ERROR "Verilator was not found. Either install it, or set the VERILATOR_ROOT environment variable")
endif()

if(NOT DEFINED NVBOARD_HOME)
  set(NVBOARD_HOME get_filename_component(real_path "../nvboard" REALPATH))
endif()

add_library(nvboard STATIC IMPORTED)
set_target_properties(bar PROPERTIES
  IMPORTED_LOCATION  "${NVBOARD_HOME}/build/nvboard.a"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/include/libbar"
)

add_executable(Main csrc/main.cpp)
add_executable(Main_nvboard csrc_nvboard/main.cpp)

verilate(Main
  COVERAGE TRACE
  SOURCES build/chisel/Main.sv build/chisel/RegisterFile.sv)

verilate(Main_nvboard
  COVERAGE TRACE
  SOURCES build/chisel/Main.sv build/chisel/RegisterFile.sv)
