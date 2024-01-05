cmake_minimum_required(VERSION 3.20)

project(npc)
set (CMAKE_CXX_STANDARD 11)
cmake_policy(SET CMP0144 NEW)

execute_process(
  COMMAND ${CMAKE_SOURCE_DIR}/../git_commit.sh "configure\ npc"
  WORKING_DIRECTORY ../
)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

find_package(verilator REQUIRED)

find_library(NVBOARD_LIBRARY NAMES nvboard)
find_path(NVBOARD_INCLUDE_DIR NAMES nvboard.h)

set(TOPMODULE "Switch")
set(SCALA_CORE "${CMAKE_CURRENT_SOURCE_DIR}/core")
set(CHISEL_MODULE_CLASS "${CMAKE_PROJECT_NAME}.${TOPMODULE}")
file(GLOB_RECURSE SCALA_CORE_SOURCES "${SCALA_CORE}/src/main/scala/*.scala")
file(GLOB_RECURSE SCALA_CORE_TEST_SOURCES "${SCALA_CORE}/src/test/scala/*.scala")

# Configure time verilog source generation for verilator
execute_process(
  COMMAND sbt "runMain circt.stage.ChiselMain --target-dir ${CMAKE_CURRENT_BINARY_DIR}/vsrc --module ${CHISEL_MODULE_CLASS} --target verilog"
  WORKING_DIRECTORY ${SCALA_CORE}
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/vsrc/${TOPMODULE}.v
  COMMAND sbt "runMain circt.stage.ChiselMain --target-dir ${CMAKE_CURRENT_BINARY_DIR}/vsrc --module ${CHISEL_MODULE_CLASS} --target verilog"
  WORKING_DIRECTORY ${SCALA_CORE}
  DEPENDS ${SCALA_CORE_SOURCES}
)

add_custom_target(
    ChiselBuild
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/vsrc/${TOPMODULE}.v
)

# -- Build NVBoard executable

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/auto_bind.cpp
  COMMAND auto_pin_bind ${CMAKE_SOURCE_DIR}/constr/${TOPMODULE}.nxdc ${CMAKE_CURRENT_BINARY_DIR}/auto_bind.cpp
  DEPENDS ${CMAKE_SOURCE_DIR}/constr/${TOPMODULE}.nxdc 
)

add_executable(V${TOPMODULE}_nvboard csrc_nvboard/main.cpp auto_bind.cpp)

verilate(V${TOPMODULE}_nvboard TRACE COVERAGE THREADS
  TOP_MODULE ${TOPMODULE}
  PREFIX V${TOPMODULE}
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/vsrc/${TOPMODULE}.v)

add_dependencies(V${TOPMODULE}_nvboard ChiselBuild)
target_include_directories(V${TOPMODULE}_nvboard PRIVATE ${NVBOARD_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})
target_link_libraries(V${TOPMODULE}_nvboard PRIVATE ${NVBOARD_LIBRARY} SDL2::SDL2 SDL2_image::SDL2_image)

install(TARGETS V${TOPMODULE}_nvboard)

# -- Build Verilator executable and add to test

add_executable(V${TOPMODULE} csrc/main.cpp)

verilate(V${TOPMODULE} TRACE COVERAGE THREADS
  TOP_MODULE ${TOPMODULE}
  PREFIX V${TOPMODULE}
  SOURCES ${CMAKE_CURRENT_BINARY_DIR}/vsrc/${TOPMODULE}.v)

add_dependencies(V${TOPMODULE} ChiselBuild)

enable_testing()
add_test(NAME V${TOPMODULE} COMMAND V${TOPMODULE})

# -- Add build tracking
add_custom_command(
    TARGET V${TOPMODULE}_nvboard
    COMMAND ${CMAKE_SOURCE_DIR}/../git_commit.sh "Build V${TOPMODULE}_nvboard"
    WORKING_DIRECTORY ../
)

add_custom_command(
    TARGET V${TOPMODULE}
    COMMAND ${CMAKE_SOURCE_DIR}/../git_commit.sh "Build V${TOPMODULE}"
    WORKING_DIRECTORY ../
)
