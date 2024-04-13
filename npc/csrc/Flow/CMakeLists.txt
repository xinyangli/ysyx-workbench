include(ChiselBuild)
add_executable(V${TOPMODULE} config.cpp main.cpp)
target_link_libraries(V${TOPMODULE} PRIVATE sdb)
target_include_directories(V${TOPMODULE} PRIVATE ${CMAKE_SOURCE_DIR}/include)

verilate(V${TOPMODULE} TRACE THREADS
  TOP_MODULE ${TOPMODULE}
  PREFIX V${TOPMODULE}
  SOURCES ${CHISEL_OUTPUT_TOPMODULE} ${CHISEL_OUTPUT_VERILATOR_CONF}
  INCLUDE_DIRS ${CHISEL_OUTPUT_DIR}
  VERILATOR_ARGS
    "--vpi"   # Enable VPI
    "-Wno-UNOPTFLAT"
)

foreach(DIFFTEST_BINARY_FILE IN LISTS DIFFTEST_BINARY_FILES)
  get_filename_component(FILENAME ${DIFFTEST_BINARY_FILE} NAME_WE)
  add_test(
    NAME V${TOPMODULE}.${FILENAME}
    COMMAND V${TOPMODULE}
      -m ${DIFFTEST_BINARY_FILE}
      --diff-lib ${DIFFTEST_LIB})
  unset(FILENAME)
endforeach()
