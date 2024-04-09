#ifndef _NPC_UTILS_DISASM_
#define _NPC_UTILS_DISASM_
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInstPrinter.h"

class Disassembler {
  llvm::MCDisassembler *gDisassembler = nullptr;
  llvm::MCSubtargetInfo *gSTI = nullptr;
  llvm::MCInstPrinter *gIP = nullptr;
  std::string triple;

public:
  Disassembler(std::string);
  std::string disassemble(uint64_t pc, uint8_t *code, int nbyte);
};

#endif