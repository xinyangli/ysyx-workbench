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

Disassembler d{"riscv32-pc-linux-gnu"};

struct RefTrmInterface : TrmInterface {
  RefTrmInterface(memcpy_t memcpy, regcpy_t regcpy, exec_t exec, init_t init)
      : TrmInterface(
            {.memcpy = memcpy, .regcpy = regcpy, .exec = exec, .init = init}) {}

  // using fs = std::filesystem::path;
  RefTrmInterface(std::filesystem::path lib_file) {
    void *handle = dlopen(lib_file.c_str(), RTLD_LAZY);
    assert(handle != nullptr);
    memcpy = (memcpy_t)dlsym(handle, "difftest_memcpy");
    assert(memcpy);
    regcpy = (regcpy_t)dlsym(handle, "difftest_regcpy");
    assert(regcpy);
    exec = (exec_t)dlsym(handle, "difftest_exec");
    assert(exec);
    init = (init_t)dlsym(handle, "difftest_init");
    assert(init);
  }
};

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


VlModule *top;
Registers *regs;
vpiHandle pc = nullptr;

namespace NPC {
void npc_memcpy(paddr_t, void *, size_t, bool){};

void npc_regcpy(void *p, bool direction) {

  if (direction == TRM_FROM_MACHINE) {
    ((CPUState *)p)->pc = regs->get_pc();
    for (int i = 0; i < 32; i++) {
      ((CPUState *)p)->reg[i] = (*regs)[i];
    }
  }
}

void npc_exec(uint64_t n) {
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

void npc_init(int port) {
  //   top = std::make_unique<VlModule>(config.do_trace, config.wavefile);
  top = new VlModule{config.do_trace, config.wavefile};
  regs = new Registers("TOP.Flow.reg_0.regFile_", "TOP.Flow.pc.out");
  top->reset_eval(10);
}

TrmInterface npc_interface = TrmInterface{
    &npc_memcpy, &npc_regcpy, &npc_exec, &npc_init};
}

// SDB::SDB<NPC::npc_interface> sdb_dut;
using CPUState = CPUStateBase<uint32_t, 32>;
CPUState cpu[2];
extern "C" {
word_t reg_str2val(const char *name, bool *success) {
  return cpu[0].reg_str2val(name, success);
}
}

int main(int argc, char **argv, char **env) {
  config.cli_parse(argc, argv);

  /* -- Difftest -- */
  std::filesystem::path ref{config.lib_ref};
  RefTrmInterface ref_interface = RefTrmInterface{ref};
  DifftestTrmInterface<CPUState> diff_interface {NPC::npc_interface, ref_interface,
                                      pmem_get(), 128, cpu[0]};
  SDB::SDB sdb_diff{diff_interface};

  int t = 8;
  std::cout << diff_interface.diff << std::endl;
  sdb_diff.main_loop();

  return 0;
}
