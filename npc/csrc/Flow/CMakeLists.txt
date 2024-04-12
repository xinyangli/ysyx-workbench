include(ChiselBuild)
add_executable(V${TOPMODULE} config.cpp main.cpp)
target_link_libraries(V${TOPMODULE} PRIVATE sdb)

verilate(V${TOPMODULE} TRACE THREADS
  TOP_MODULE ${TOPMODULE}
  PREFIX V${TOPMODULE}
  SOURCES ${CHISEL_OUTPUT_TOPMODULE} ${CHISEL_OUTPUT_VERILATOR_CONF}
  INCLUDE_DIRS ${CHISEL_OUTPUT_DIR}
  VERILATOR_ARGS
    "--vpi"   # Enable VPI
    "-Wno-UNOPTFLAT"
)

add_test(
  NAME V${TOPMODULE}
  COMMAND V${TOPMODULE}
    --no-bin -m ${PROJECT_SOURCE_DIR}/resource/addi.txt
    --diff-lib ${DIFFTEST_LIB})
