cmake_minimum_required(VERSION 3.22)

project(am-kernels)
set(CMAKE_C_STANDARD 11)
enable_language(C ASM)

include(CheckPIESupported)
check_pie_supported()

if(${PLATFORM} MATCHES "native")
set(ARCH "native")
else()
set(ARCH ${ISA}-${PLATFORM})
endif()

add_subdirectory(tests/cpu-tests)