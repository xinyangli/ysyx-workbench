#include "VFlow___024root.h"
#include "components.hpp"
#include <VFlow.h>
#include <array>
#include <config.hpp>
#include <cstdint>
#include <cstdlib>
#include <disasm.hpp>
#include <filesystem>
#include <fstream>
#include <memory>
#include <sdb.hpp>
#include <trm_difftest.hpp>
#include <trm_interface.hpp>
#include <types.h>
#include <vl_wrapper.hpp>
#include <vpi_user.h>
#include <vpi_wrapper.hpp>
#include <devices.hpp>

using VlModule = VlModuleInterfaceCommon<VFlow>;
using Registers = _RegistersVPI<uint32_t, 32>;

// SDB::SDB<NPC::npc_interface> sdb_dut;
using CPUState = CPUStateBase<uint32_t, 32>;
bool g_skip_memcheck = false;
CPUState npc_cpu;
VlModule *top;
Registers *regs;
vpiHandle pc = nullptr;

const size_t PMEM_START = 0x80000000; 
const size_t PMEM_END = 0x87ffffff;

extern "C" {
using MMap = MemoryMap<Memory<128 * 1024>, Devices::DeviceMap>;
void *pmem_get() {
  static Devices::DeviceMap devices {
    new Devices::Serial(0x10000000, 0x1000)
  };
  static auto pmem = new MemoryMap<Memory<128 * 1024>, Devices::DeviceMap>(
      std::make_unique<Memory<128 * 1024>>(
          config.memory_file, config.memory_file_binary, PMEM_START, PMEM_END),
      std::make_unique<Devices::DeviceMap>(devices), config.mtrace_ranges);
  return pmem;
}

int pmem_read(int raddr) {
  void *pmem = pmem_get();
  auto mem = static_cast<MMap *>(pmem);
  // TODO: Do memory difftest at memory read and write to diagnose at a finer
  // granularity
  if (config.do_mtrace)
    mem->trace(raddr, true, regs->get_pc());
  if(g_skip_memcheck) return mem->read(PMEM_START);
  return mem->read(raddr);
}

void pmem_write(int waddr, int wdata, char wmask) {
  void *pmem = pmem_get();
  auto mem = static_cast<MMap *>(pmem);
  if (config.do_mtrace)
    mem->trace((std::size_t)waddr, false, regs->get_pc(), wdata);
  return mem->write((std::size_t)waddr, wdata, wmask);
}
}

namespace NPC {
void npc_memcpy(paddr_t addr, void *buf, size_t sz, bool direction) {
  if (direction == TRM_FROM_MACHINE) {
    static_cast<MMap *>(pmem_get())->copy_to(addr, (uint8_t *)buf, sz);
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

  if(config.lib_ref.empty()) {
    NPC::npc_interface.init(0);
    while(true) {
      word_t inst = NPC::npc_interface.at(regs->get_pc());
      std::cout << std::hex << pc << ' '<< inst << std::endl;
      if (inst == 1048691) {
        return 0;
      }
      NPC::npc_interface.exec(1);
    }
  }

  /* -- Difftest -- */
  std::filesystem::path ref{config.lib_ref};
  RefTrmInterface ref_interface{ref};
  DifftestTrmInterface diff_interface{NPC::npc_interface, ref_interface,
                                      static_cast<MMap *>(pmem_get())->get_pmem(), 128 * 1024};
  if(config.interactive) {
    SDB::SDB sdb_diff{diff_interface};
    sdb_diff.main_loop();
    return 0;
  }

  try {
    diff_interface.init(0);
    diff_interface.exec(-1);
  } catch (TrmRuntimeException &e) {
    switch (e.error_code()) {
    case TrmRuntimeException::EBREAK:
      return 0;
    case TrmRuntimeException::DIFFTEST_FAILED:
      std::cout << "Difftest Failed" << std::endl;
      diff_interface.print(std::cout);
      return 1;
    default:
      std::cerr << "Unknown error happened" << std::endl;
      return 1;
    }
  }

  return 0;
}
