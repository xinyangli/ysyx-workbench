#include "VFlow___024root.h"
#include "config.hpp"
#include "disasm.hpp"
#include "vl_wrapper.hpp"
#include "vpi_user.h"
#include "vpi_wrapper.hpp"
#include <VFlow.h>
#include <cstdint>
#include <cstdlib>
#include <difftest.hpp>
#include <sdb.hpp>
#include <types.h>

using VlModule = VlModuleInterfaceCommon<VFlow>;
using Registers = _RegistersVPI<uint32_t, 32>;

extern "C" {
void *pmem_get() {
  static auto pmem = new Memory<int, 128 * 1024>(config.memory_file,
                                                 config.memory_file_binary);
  return pmem;
}

int pmem_read(int raddr) {
  void *pmem = pmem_get();
  auto mem = static_cast<Memory<int, 128 * 1024> *>(pmem);
  // TODO: Do memory difftest at memory read and write to diagnose at a finer
  // granularity
  return mem->read(raddr);
}

void pmem_write(int waddr, int wdata, char wmask) {
  void *pmem = pmem_get();
  auto mem = static_cast<Memory<int, 128 * 1024> *>(pmem);
  return mem->write((std::size_t)waddr, wdata, wmask);
}
}

Disassembler d{"riscv32-pc-linux-gnu"};

VlModule *top;
Registers *regs;
vpiHandle pc = nullptr;
void difftest_memcpy(paddr_t, void *, size_t, bool){};

void difftest_regcpy(void *p, bool direction) {

  if (direction == DIFFTEST_FROM_REF) {
    ((CPUState *)p)->pc = regs->get_pc();
    for (int i = 0; i < 32; i++) {
      ((CPUState *)p)->reg[i] = (*regs)[i];
    }
  }
}

void difftest_exec(uint64_t n) {
  while (n--) {
    for (int i = 0; i < 2; i++) {
      if (top->is_posedge()) {
        // Posedge
        regs->update();
      }
      top->eval();
    }
  }
}
// std::cout << d.disassemble(top->rootp->Flow__DOT__pc__DOT__pc_reg, (uint8_t *)&top->rootp->Flow__DOT___ram_inst, 4) << std::endl;

void difftest_init(int port) {
  //   top = std::make_unique<VlModule>(config.do_trace, config.wavefile);
  top = new VlModule{config.do_trace, config.wavefile};
  regs = new Registers("TOP.Flow.reg_0.regFile_", "TOP.Flow.pc.out");
  top->reset_eval(10);
}

DifftestInterface dut_interface = DifftestInterface{
    &difftest_memcpy, &difftest_regcpy, &difftest_exec, &difftest_init};

SDB::SDB<dut_interface> sdb_dut;
extern "C" {
word_t reg_str2val(const char *name, bool *success) {
  return sdb_dut.reg_str2val(name, success);
}
}

int main(int argc, char **argv, char **env) {
  config.cli_parse(argc, argv);

  /* -- Difftest -- */
  std::filesystem::path ref{config.lib_ref};
  DifftestInterface ref_interface = DifftestInterface{ref};

  Difftest<CPUStateBase<uint32_t, 32>> diff{dut_interface, ref_interface,
                                            pmem_get(), 128};
  int t = 8;
  sdb_dut.main_loop();
  while (t--) {
    if (!diff.step(1)) {
      uint32_t pc = regs->get_pc();
      uint32_t inst = pmem_read(pc);
      std::cout << diff << d.disassemble(pc, (uint8_t *)&inst, 4) << std::endl;
      return EXIT_FAILURE;
    }
  }

  return 0;
}
