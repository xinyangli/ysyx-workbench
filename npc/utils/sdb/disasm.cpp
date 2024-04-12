#include <disasm.hpp>
#include <llvm/MC/MCAsmInfo.h>
#include <llvm/MC/MCContext.h>
#include <llvm/MC/MCDisassembler/MCDisassembler.h>
#include <llvm/MC/MCInstPrinter.h>
#include <llvm/Support/raw_ostream.h>
#if LLVM_VERSION_MAJOR >= 14
#include <llvm/MC/TargetRegistry.h>
#if LLVM_VERSION_MAJOR >= 15
#include <llvm/MC/MCSubtargetInfo.h>
#endif
#else
#include <llvm/Support/TargetRegistry.h>
#endif
#include <iostream>
#include <llvm/Support/TargetSelect.h>
#include <sstream>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

#if LLVM_VERSION_MAJOR < 11
#error Please use LLVM with major version >= 11
#endif

Disassembler::Disassembler(std::string triple) : triple(triple) {
  llvm::InitializeRISCVTargetInfos();
  llvm::InitializeRISCVTargetMCs();
  llvm::InitializeRISCVAsmParsers();
  llvm::InitializeRISCVDisassemblers();

  std::string errstr;

  llvm::MCInstrInfo *gMII = nullptr;
  llvm::MCRegisterInfo *gMRI = nullptr;
  auto target = llvm::TargetRegistry::lookupTarget(triple, errstr);
  if (!target) {
    llvm::errs() << "Can't find target for " << triple << ": " << errstr
                 << "\n";
    assert(0);
  }

  llvm::MCTargetOptions MCOptions;
  gSTI = target->createMCSubtargetInfo(triple, "", "");
  std::string isa = target->getName();
  if (isa == "riscv32" || isa == "riscv64") {
    gSTI->ApplyFeatureFlag("+m");
    gSTI->ApplyFeatureFlag("+a");
    gSTI->ApplyFeatureFlag("+c");
    gSTI->ApplyFeatureFlag("+f");
    gSTI->ApplyFeatureFlag("+d");
  }
  gMII = target->createMCInstrInfo();
  gMRI = target->createMCRegInfo(triple);
  auto AsmInfo = target->createMCAsmInfo(*gMRI, triple, MCOptions);
#if LLVM_VERSION_MAJOR >= 13
  auto llvmTripleTwine = llvm::Twine(triple);
  auto llvmtriple = llvm::Triple(llvmTripleTwine);
  auto Ctx = new llvm::MCContext(llvmtriple, AsmInfo, gMRI, nullptr);
#else
  auto Ctx = new llvm::MCContext(AsmInfo, gMRI, nullptr);
#endif
  gDisassembler = target->createMCDisassembler(*gSTI, *Ctx);
  gIP = target->createMCInstPrinter(llvm::Triple(triple),
                                    AsmInfo->getAssemblerDialect(), *AsmInfo,
                                    *gMII, *gMRI);
  gIP->setPrintImmHex(true);
  gIP->setPrintBranchImmAsAddress(true);
  if (isa == "riscv32" || isa == "riscv64")
    gIP->applyTargetSpecificCLOption("no-aliases");
}

std::string Disassembler::disassemble(uint64_t pc, uint8_t *code, int nbyte) {
  llvm::MCInst inst;
  llvm::ArrayRef<uint8_t> arr(code, nbyte);
  uint64_t dummy_size = 0;
  gDisassembler->getInstruction(inst, dummy_size, arr, pc, llvm::nulls());

  std::stringstream ss;
  ss << "0x" << std::hex << pc << ": ";
  std::string s = ss.str();
  llvm::raw_string_ostream os{s};
  gIP->printInst(&inst, pc, "", *gSTI, os);

  return s;
}
