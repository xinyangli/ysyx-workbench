cmake_minimum_required(VERSION 3.22)

project(abstract-machine)
enable_language(CXX C ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 11)

include(CMakeDependentOption)
include(CMakePackageConfigHelpers)  # Used to find libcheck
include(CTest)
include(GNUInstallDirs)

# -- General options
set(ISA CACHE STRING "Target ISA")
set_property(CACHE ISA PROPERTY STRINGS "riscv" "x86" "x86_64" "native")
string(TOUPPER ${ISA} ISA_UPPER)

cmake_dependent_option(
    __PLATFORM_NEMU__ "Run on NEMU"
    ON "ISA MATCHES \"(riscv | x86)\"" OFF)
cmake_dependent_option(
    __PLATFORM_NPC__ "Run on NPC"
    ON "ISA MATCHES riscv" OFF)
cmake_dependent_option(
    __PLATFORM_NATIVE__ "Run on native"
    ON "ISA MATCHES native" OFF)

# -- Set PLATFORM according to options
set(MATCH_PLATFORM_PATTERN "^__PLATFORM_([A-Z]*)__$")
get_cmake_property(CACHE_VARS CACHE_VARIABLES)

message(STATUS "ISA: ${ISA}")
foreach(VAR IN LISTS CACHE_VARS)
    if(VAR MATCHES ${MATCH_PLATFORM_PATTERN})
        # Retrieve the value of the cache variable
        get_property(VAR_VALUE CACHE ${VAR} PROPERTY VALUE)
        set(PLATFORM_UPPER ${CMAKE_MATCH_1})
        string(TOLOWER ${PLATFORM_UPPER} PLATFORM)
        list(APPEND PLATFORMS ${PLATFORM})
        message(STATUS "Variable: ${VAR}=${VAR_VALUE}, Platform: ${PLATFORM}")
    endif()
endforeach()

if((NOT PLATFORM) AND (NOT ISA MATCHES native))
    message(FATAL_ERROR "Platform not given!")
endif()

set(SUPPORTED_ARCH "riscv-nemu" "riscv-npc" "native")
foreach(PLATFORM IN LISTS PLATFORMS)
    if(${ISA} MATCHES "native")
        set(ARCH "native")
    else()
        set(ARCH ${ISA}-${PLATFORM})
    endif()

    if(NOT ARCH IN_LIST SUPPORTED_ARCH)
        message(FATAL_ERROR "Given ISA-PLATFORM (${ISA}-${PLATFORM}) does not match one of the following: ${SUPPORTED_ARCH}")
    endif()
endforeach()

# -- Target specific options
cmake_dependent_option(
    NATIVE_USE_KLIB "Use Klib even if on native"
    ON "NOT __ISA_NATIVE__" OFF)

# -- Add compile definitions based on options

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# NOTE: klib and am include header files in each other,
#       so we need to create interface libraries for correct dependency
add_library(am_interface INTERFACE)
target_include_directories(am_interface INTERFACE
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/am/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/am/src>
    $<INSTALL_INTERFACE:include/abstract-machine>)
target_compile_definitions(am_interface INTERFACE ARCH_H=<arch/${ISA}.h>)

add_library(klib_interface INTERFACE)
target_include_directories(klib_interface
    INTERFACE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/klib/include>
      $<INSTALL_INTERFACE:include/abstract-machine>)

install(TARGETS am_interface klib_interface
    EXPORT interfaceTargets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/abstract-machine)

install(EXPORT interfaceTargets
        FILE interfaceTargets.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake)

add_subdirectory(klib)
add_subdirectory(am)

# -- Test depends on klib and am should be added last.
add_subdirectory(klib/tests)
