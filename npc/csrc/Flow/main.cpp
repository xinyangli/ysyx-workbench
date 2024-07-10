#include "VFlow___024root.h"
#include "components.hpp"
#include <VFlow.h>
#include <config.hpp>
#include <cstdint>
#include <cstdlib>
#include <devices.hpp>
#include <gdbstub.h>
#include <memory>
#include <types.h>
#include <vl_wrapper.hpp>
#include <vpi_user.h>
#include <vpi_wrapper.hpp>

using VlModule = VlModuleInterfaceCommon<VFlow>;
using Registers = _RegistersVPI<uint32_t, 32>;

// SDB::SDB<NPC::npc_interface> sdb_dut;
bool g_skip_memcheck = false;
VlModule *top;
Registers *regs;
vpiHandle pc = nullptr;

const size_t PMEM_START = 0x80000000;
const size_t PMEM_END = 0x87ffffff;

extern "C" {
using MMap = MemoryMap<Memory<128 * 1024>, Devices::DeviceMap>;
void *pmem_get() {
  static Devices::DeviceMap devices{
      new Devices::Serial(0x10000000, 0x1000),
      new Devices::RTC(0x10001000, 0x1000),
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
  if (g_skip_memcheck)
    return mem->read(PMEM_START);
  return mem->read(raddr);
}

void pmem_write(int waddr, int wdata, char wmask) {
  void *pmem = pmem_get();
  auto mem = static_cast<MMap *>(pmem);
  if (config.do_mtrace)
    mem->trace((std::size_t)waddr, false, regs->get_pc(), wdata);
  return mem->write((std::size_t)waddr, wdata, wmask);
}

int main(int argc, char **argv, char **env) {
  config.cli_parse(argc, argv);

  return 0;
}
}
