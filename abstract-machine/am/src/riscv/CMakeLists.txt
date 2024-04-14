foreach(PLATFORM IN LISTS PLATFORMS)
    string(TOUPPER ${ARCH} ARCH_UPPER)
    set(AM_COMMON_COMPILE_DEF
        # -- Arch related
        $<MAKE_C_IDENTIFIER:__ARCH_${ARCH_UPPER}__>
        __ISA_${ISA_UPPER}__
        __PLATFORM_${PLATFORM_UPPER}__

        $<$<BOOL:${NATIVE_USE_KLIB}>:__NATIVE_USE_KLIB__>
    )
    add_subdirectory(${PLATFORM})
endforeach()
