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
        message(STATUS "Variable: ${VAR}=${VAR_VALUE}, Platform: ${PLATFORM}")
        break()
    endif()
endforeach()

if(NOT PLATFORM)
    message(FATAL_ERROR "Platform not given!")
endif()

if(${PLATFORM} MATCHES "native")
set(ARCH "native")
else()
set(ARCH ${ISA}-${PLATFORM})
endif()
string(TOUPPER ${ARCH} ARCH_UPPER)

set(SUPPORTED_ARCH "riscv-nemu" "native")
set(ARCH ${ISA}-${PLATFORM})
if(NOT ARCH IN_LIST SUPPORTED_ARCH)
    message(FATAL_ERROR "Given ISA-PLATFORM (${ISA}-${PLATFORM}) does not match one of the following: ${SUPPORTED_ARCH}")
endif()

# -- Target specific options
cmake_dependent_option(
    NATIVE_USE_KLIB "Use Klib even if on native"
    ON "NOT __ISA_NATIVE__" OFF)

# -- Add compile definitions based on options
set(AM_COMMON_COMPILE_DEF
    # -- Arch related
    $<MAKE_C_IDENTIFIER:__ARCH_${ARCH_UPPER}__>
    __ISA_${ISA_UPPER}__
    __PLATFORM_${PLATFORM_UPPER}__

    $<$<BOOL:${NATIVE_USE_KLIB}>:__NATIVE_USE_KLIB__>
)

# -- Required compiler flags
# add_compile_options(
#     # -Werror
#     -Wno-main
#     -fno-asynchronous-unwind-tables
#     -fno-builtin
#     -fno-stack-protector
#     -U_FORTIFY_SOURCE
#     $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
#     $<$<COMPILE_LANGUAGE:CXX>:-ffreestanding>
#     $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>)

# add_link_options(
#     -znoexecstack
# )

# -- Include linker script here. Use this linker script at link time if INCLUDE_LINKER_SCRIPT is set to true
set(LINKER_SCRIPT linker.ld)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

add_subdirectory(klib)
add_subdirectory(am)
