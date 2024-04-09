add_library(disasm disasm.cpp)
target_include_directories(disasm PUBLIC include)
llvm_map_components_to_libnames(LLVM_LIBS ${LLVM_TARGETS_TO_BUILD})
target_link_libraries(disasm PUBLIC ${LLVM_LIBS})
