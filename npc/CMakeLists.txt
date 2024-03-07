cmake_minimum_required(VERSION 3.20)

project(npc)
include(CTest)
set (CMAKE_CXX_STANDARD 11)
cmake_policy(SET CMP0144 NEW)

execute_process(
  COMMAND ${CMAKE_SOURCE_DIR}/../git_commit.sh "configure(npc)"
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/..
)

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)

find_package(verilator REQUIRED)

find_library(NVBOARD_LIBRARY NAMES nvboard)
find_path(NVBOARD_INCLUDE_DIR NAMES nvboard.h)

set(TOPMODULES "Switch" "Keyboard")

foreach(TOPMODULE IN LISTS TOPMODULES)
 
  # FIXME: all scala source file are tracked here, cause all files to rebuild 
  # after a source update. 
  set(SCALA_CORE "${CMAKE_CURRENT_SOURCE_DIR}/core")
  set(CHISEL_MODULE_CLASS "${CMAKE_PROJECT_NAME}.${TOPMODULE}")
  file(GLOB_RECURSE SCALA_CORE_SOURCES "${SCALA_CORE}/src/main/scala/*.scala")
  file(GLOB_RECURSE SCALA_CORE_TEST_SOURCES "${SCALA_CORE}/src/test/scala/*.scala")

  # Configure time verilog source generation for verilator
  execute_process(
    COMMAND sbt "runMain circt.stage.ChiselMain --target-dir ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/vsrc --module ${CHISEL_MODULE_CLASS} --target verilog"
    WORKING_DIRECTORY ${SCALA_CORE}
  )

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/vsrc/${TOPMODULE}.v
    COMMAND sbt "runMain circt.stage.ChiselMain --target-dir ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/vsrc --module ${CHISEL_MODULE_CLASS} --target verilog"
    WORKING_DIRECTORY ${SCALA_CORE}
    DEPENDS ${SCALA_CORE_SOURCES}
  )

  add_custom_target(
      ChiselBuild_${TOPMODULE}
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/vsrc/${TOPMODULE}.v
  )

  # -- Build NVBoard executable

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/auto_bind.cpp
    COMMAND auto_pin_bind ${CMAKE_SOURCE_DIR}/constr/${TOPMODULE}.nxdc ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/auto_bind.cpp
    DEPENDS ${CMAKE_SOURCE_DIR}/constr/${TOPMODULE}.nxdc 
  )

  unset(SOURCES)
  file(GLOB_RECURSE SOURCES csrc_nvboard/${TOPMODULE}/*.cpp)
  add_executable(V${TOPMODULE}_nvboard ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/auto_bind.cpp)

  verilate(V${TOPMODULE}_nvboard TRACE COVERAGE THREADS
    TOP_MODULE ${TOPMODULE}
    PREFIX V${TOPMODULE}
    SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/vsrc/${TOPMODULE}.v)

  add_dependencies(V${TOPMODULE}_nvboard ChiselBuild_${TOPMODULE})
  target_include_directories(V${TOPMODULE}_nvboard PRIVATE ${NVBOARD_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})
  target_link_libraries(V${TOPMODULE}_nvboard PRIVATE ${NVBOARD_LIBRARY} SDL2::SDL2 SDL2_image::SDL2_image)

  install(TARGETS V${TOPMODULE}_nvboard)

  # -- Build Verilator executable and add to test

  unset(SOURCES)
  file(GLOB_RECURSE SOURCES csrc/${TOPMODULE}/*.cpp)
  add_executable(V${TOPMODULE} ${SOURCES})

  verilate(V${TOPMODULE} TRACE COVERAGE THREADS
    TOP_MODULE ${TOPMODULE}
    PREFIX V${TOPMODULE}
    SOURCES ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/vsrc/${TOPMODULE}.v)

  add_dependencies(V${TOPMODULE} ChiselBuild_${TOPMODULE})

  enable_testing()
  add_test(V${TOPMODULE}_build
    "${CMAKE_COMMAND}"
    --build "${CMAKE_BINARY_DIR}"
    --config "$<CONFIG>"
    --target V${TOPMODULE}
  )
  set_tests_properties(V${TOPMODULE}_build PROPERTIES FIXTURES_SETUP V${TOPMODULE}_fixture)
  message(${TOPMODULE})
  add_test(NAME V${TOPMODULE} COMMAND V${TOPMODULE})
  set_tests_properties(V${TOPMODULE} PROPERTIES FIXTURES_REQUIRED V${TOPMODULE}_fixture)

  # -- Add build tracking
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

endforeach()
