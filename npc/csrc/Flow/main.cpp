#include "VFlow___024root.h"
#include <config.hpp>
#include <disasm.hpp>
#include <vl_wrapper.hpp>
#include <vpi_user.h>
#include <vpi_wrapper.hpp>
#include <VFlow.h>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sdb.hpp>
#include <trm_difftest.hpp>
#include <trm_interface.hpp>
#include <types.h>

using VlModule = VlModuleInterfaceCommon<VFlow>;
using Registers = _RegistersVPI<uint32_t, 32>;

// SDB::SDB<NPC::npc_interface> sdb_dut;
using CPUState = CPUStateBase<uint32_t, 32>;
bool g_skip_memcheck = false;
CPUState npc_cpu;
VlModule *top;
Registers *regs;
vpiHandle pc = nullptr;

extern "C" {
void *pmem_get() {
  static auto pmem = new Memory<int, 128 * 1024>(config.memory_file,
                                                 config.memory_file_binary, std::move(config.mtrace_ranges));
  return pmem;
}

int pmem_read(int raddr) {
  void *pmem = pmem_get();
  auto mem = static_cast<Memory<int, 128 * 1024> *>(pmem);
  // TODO: Do memory difftest at memory read and write to diagnose at a finer
  // granularity
  if(config.do_mtrace) {
    std::cout << regs->get_pc() << std::endl;
    mem->trace(raddr, true);
  }
  return mem->read(raddr);
}

void pmem_write(int waddr, int wdata, char wmask) {
  void *pmem = pmem_get();
  auto mem = static_cast<Memory<int, 128 * 1024> *>(pmem);
  if(config.do_mtrace) {
    std::cout << regs->get_pc() << std::endl;
    mem->trace((std::size_t)waddr, false, wdata);
  }
  return mem->write((std::size_t)waddr, wdata, wmask);
}
}

namespace NPC {
void npc_memcpy(paddr_t addr, void *buf, size_t sz, bool direction) {
  if (direction == TRM_FROM_MACHINE) {
    memcpy(
        buf,
        static_cast<Memory<int, 128 * 1024> *>(pmem_get())->guest_to_host(addr),
        sz);
  }
};

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

void npc_atexit(void) {
  delete top;
  delete regs;
}

void npc_init(int port) {
  top = new VlModule{config.wavefile};
  regs = new Registers("TOP.Flow.reg_0.regFile_", "TOP.Flow.pc.out");
  atexit(npc_atexit);
  top->reset_eval(10);
}

class DutTrmInterface : public TrmInterface {
public:
  DutTrmInterface(memcpy_t f_memcpy, regcpy_t f_regcpy, exec_t f_exec,
                  init_t f_init, void *cpu_state)
      : TrmInterface{f_memcpy, f_regcpy, f_exec, f_init, cpu_state} {}
  word_t at(std::string name) const override {
    return ((CPUState *)cpu_state)->at(name);
  }

  word_t at(paddr_t addr) const override {
    word_t buf;
    this->memcpy(addr, &buf, sizeof(word_t), TRM_FROM_MACHINE);
    return buf;
  }
  void print(std::ostream &os) const override {}
};

DutTrmInterface npc_interface =
    DutTrmInterface{&npc_memcpy, &npc_regcpy, &npc_exec, &npc_init, &npc_cpu};
}; // namespace NPC

extern "C" {
word_t reg_str2val(const char *name, bool *success) {
  return npc_cpu.reg_str2val(name, success);
}
}

int main(int argc, char **argv, char **env) {
  config.cli_parse(argc, argv);

  if(config.max_sim_time > 1) {
    NPC::npc_interface.exec(config.max_sim_time / 2);
  } else {
    /* -- Difftest -- */
    std::filesystem::path ref{config.lib_ref};
    RefTrmInterface ref_interface{ref};
    DifftestTrmInterface diff_interface{NPC::npc_interface, ref_interface,
                                        pmem_get(), 1024};
    SDB::SDB sdb_diff{diff_interface};

    int t = 8;
    sdb_diff.main_loop();
  }

  return 0;
}
