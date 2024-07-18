extern "C" {
#include <gdbstub.h>
}
// #include "VFlow___024root.h"
#include "components.hpp"
#include <VFlow.h>
#include <config.hpp>
#include <cstdint>
#include <cstdlib>
#include <devices.hpp>
#include <memory>
#include <types.h>
#include <vector>
#include <vl_wrapper.hpp>
#include <vpi_user.h>
#include <vpi_wrapper.hpp>

using Registers = _RegistersVPI<uint32_t, 32>;
using VlModule = VlModuleInterfaceCommon<VFlow, Registers>;

// SDB::SDB<NPC::npc_interface> sdb_dut;
bool g_skip_memcheck = false;
VlModule *top;
Registers *regs;
vpiHandle pc = nullptr;

const size_t PMEM_START = 0x80000000;
const size_t PMEM_END = 0x87ffffff;

struct DbgState {
  std::vector<Breakpoint> *bp;
};

extern "C" {

/* === Memory Access === */
using MMap = MemoryMap<Memory<128 * 1024>, Devices::DeviceMap>;
void *pmem_get() {
  static Devices::DeviceMap devices{
      new Devices::Serial(0x10000000, 0x1000),
      new Devices::RTC(0x10001000, 0x1000),
  };
  static auto pmem = new MemoryMap<Memory<128 * 1024>, Devices::DeviceMap>(
      std::make_unique<Memory<128 * 1024>>(
          "/nix/store/nv2c00y24qwz1jihfbaip9n1lskbzyb3-am-kernel-riscv32-none-elf-2024-07-10/share/am-kernels/add.bin", config.memory_file_binary, PMEM_START, PMEM_END),
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

/* === For gdbstub === */

int npc_read_mem(void *args, size_t addr, size_t len, void *val) {
  void *pmem = pmem_get();
  auto mmap = static_cast<MMap *>(pmem);
  mmap->copy_to(addr, (uint8_t *)val, len);
  return 0;
}

int npc_write_mem(void *args, size_t addr, size_t len, void *val) {
  void *pmem = pmem_get();
  auto mmap = static_cast<MMap *>(pmem);
  mmap->copy_from(addr, (uint8_t *)val, len);
  return 0;
}

int npc_read_reg(void *args, int regno, size_t *value) {
  if (regno == 32)
    *value = regs->get_pc();
  else
    *value = (*regs)[regno];
  return 0;
}

int npc_write_reg(void *args, int regno, size_t value) { return 1; }

void npc_cont(void *args, gdb_action_t *res) {
  DbgState *dbg = (DbgState *)args;
  *res = top->eval(*dbg->bp);
}

void npc_stepi(void *args, gdb_action_t *res) {
  DbgState *dbg = (DbgState *)args;
  *res = top->eval(*dbg->bp);
}

bool npc_set_bp(void *args, size_t addr, bp_type_t type) {
  DbgState *dbg = (DbgState *)args;
  for (const auto &bp : *dbg->bp) {
    if (bp.addr == addr && bp.type == type) {
      return true;
    }
  }
  dbg->bp->push_back({.addr = addr, .type = type});
  return true;
}

bool npc_del_bp(void *args, size_t addr, bp_type_t type) {
  DbgState *dbg = (DbgState *)args;
  for (auto it = dbg->bp->begin(); it != dbg->bp->end(); it++) {
    if (it->addr == addr && it->type == type) {
      std::swap(*it, *dbg->bp->rbegin());
      dbg->bp->pop_back();
      return true;
    }
  }
  return false;
}

void npc_on_interrupt(void *args) {
  ;
}

void npc_init(void * args) {
  DbgState *dbg = (DbgState *)args;
  dbg->bp = new std::vector<Breakpoint>;

  top = new VlModule;
  regs = new Registers("TOP.Flow.reg_0.regFile_", "TOP.Flow.pc.out");
  top->setup(config.wavefile, regs);
  top->reset_eval(10);
}

static gdbstub_t gdbstub_priv;
arch_info_t isa_arch_info = {
    .target_desc = strdup(TARGET_RV32), .reg_num = 32, .reg_byte = 4};

size_t argsize = sizeof(DbgState);

} // extern "C"

int gdbstub_loop() {
  DbgState dbg;

  target_ops npc_gdbstub_ops = {.cont = npc_cont,
                                .stepi = npc_stepi,
                                .read_reg = npc_read_reg,
                                .write_reg = npc_write_reg,
                                .read_mem = npc_read_mem,
                                .write_mem = npc_write_mem,
                                .set_bp = npc_set_bp,
                                .del_bp = npc_del_bp,
                                .on_interrupt = NULL};

  if (!gdbstub_init(&gdbstub_priv, &npc_gdbstub_ops, (arch_info_t)isa_arch_info,
                    strdup("/tmp/gdbstub-npc.sock"))) {
    return EINVAL;
  }
  npc_init(&dbg);

  bool success = gdbstub_run(&gdbstub_priv, &dbg);
  // gdbstub_close(&gdbstub_priv);
  return !success;
}

int main(int argc, char **argv, char **env) {
  config.cli_parse(argc, argv);

  return gdbstub_loop();
}
