#include "config.hpp"
#include "vl_wrapper.hpp"
#include "components.hpp"
#include "vpi_user.h"
#include <VFlow.h>
#include <cstdint>
#include <difftest.hpp>

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
  return mem->read(raddr);
}

void pmem_write(int waddr, int wdata, char wmask) {
  void *pmem = pmem_get();
  auto mem = static_cast<Memory<int, 128 * 1024> *>(pmem);
  return mem->write((std::size_t)waddr, wdata, wmask);
}
}

VlModule *top;
Registers *regs;
using CPUState = CPUStateBase<uint32_t, 32>;
vpiHandle pc = nullptr;
void difftest_memcpy(paddr_t, void*, size_t, bool) { };

void difftest_regcpy(void *p, bool direction) {

  if(direction == DIFFTEST_FROM_REF) {
    ((CPUState *)p)->pc = regs->get_pc();
    for(int i = 0; i < 32; i++) {
      ((CPUState *)p)->reg[i] = (*regs)[i];
    }
  }
}

void difftest_exec(uint64_t n) {
  while(n--) {
    for(int i = 0 ; i < 2; i++) {
      if(top->is_posedge()) {
        // Posedge
        regs->update();
      }
      top->eval();
    }
  }
}
void difftest_init(int port) {
//   top = std::make_unique<VlModule>(config.do_trace, config.wavefile);
  top = new VlModule{config.do_trace, config.wavefile};
  regs = new Registers("TOP.Flow.reg_0.regFile_", "TOP.Flow.pc.out");
  top->reset_eval(10);
}

int main(int argc, char **argv, char **env) {
  config.cli_parse(argc, argv);

  /* -- Difftest -- */
  std::filesystem::path ref{"/home/xin/repo/ysyx-workbench/nemu/build/riscv32-nemu-interpreter-so"};

  DifftestInterface dut_interface = DifftestInterface{
      &difftest_memcpy, &difftest_regcpy, &difftest_exec, &difftest_init};
  DifftestInterface ref_interface = DifftestInterface{ref};

  Difftest<CPUStateBase<uint32_t, 32>> diff{dut_interface, ref_interface, pmem_get(), 128};
  int t = 5;
  while(t--) {
    diff.step(1);
    // std::cout << diff;
  }

  return 0;
}
