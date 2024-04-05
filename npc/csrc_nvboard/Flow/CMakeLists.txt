include(ChiselBuild)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/auto_bind.cpp
  COMMAND auto_pin_bind ${CMAKE_SOURCE_DIR}/constr/${TOPMODULE}.nxdc ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/auto_bind.cpp
  DEPENDS ${CMAKE_SOURCE_DIR}/constr/${TOPMODULE}.nxdc
)

add_executable(V${TOPMODULE}_nvboard
  ${CMAKE_CURRENT_BINARY_DIR}/${TOPMODULE}/auto_bind.cpp
  main.cpp
)

verilate(V${TOPMODULE}_nvboard TRACE THREADS
  TOP_MODULE ${TOPMODULE}
  PREFIX V${TOPMODULE}
  SOURCES ${CHISEL_OUTPUT_TOPMODULE}
  INCLUDE_DIRS ${CHISEL_OUTPUT_DIR})

target_include_directories(V${TOPMODULE}_nvboard PRIVATE ${NVBOARD_INCLUDE_DIR} ${SDL2_INCLUDE_DIRS})
target_link_libraries(V${TOPMODULE}_nvboard PRIVATE ${NVBOARD_LIBRARY} SDL2::SDL2 SDL2_image::SDL2_image)

install(TARGETS V${TOPMODULE}_nvboard)