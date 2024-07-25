#include "utils.h"
#include <vector>

extern "C" {
#include <cpu/cpu.h>
#include <debug.h>
#include <errno.h>
#include <gdbstub.h>
#include <isa.h>
#include <memory/paddr.h>
#include <stddef.h>
#include <stdlib.h>

typedef struct {
  std::vector<breakpoint_t> *bp;
  bool halt;
} DbgState;

__EXPORT size_t nemu_dbg_state_size = sizeof(DbgState);
__EXPORT bool nemu_do_difftest = true;
__EXPORT arch_info_t nemu_isa_arch_info;

__EXPORT int nemu_read_mem(void *args, size_t addr, size_t len, void *val) {
  if (!in_pmem(addr))
    return EINVAL;
  memcpy(val, guest_to_host(addr), len);
  return 0;
}

__EXPORT int nemu_write_mem(void *args, size_t addr, size_t len, void *val) {
  if (!in_pmem(addr))
    return EINVAL;
  memcpy(guest_to_host(addr), val, len);
  return 0;
}

static void nemu_is_stopped(gdb_action_t *act, breakpoint_t *stopped_at) {
  switch (nemu_state.state) {
  case NEMU_RUNNING:
    nemu_state.state = NEMU_STOP;
    if (stopped_at == NULL) {
      act->reason = gdb_action_t::ACT_NONE;
    } else {
      switch (stopped_at->type) {
      case BP_SOFTWARE:
        act->reason = gdb_action_t::ACT_BREAKPOINT;
        break;
      case BP_ACCESS:
        act->reason = gdb_action_t::ACT_WATCH;
        break;
      case BP_WRITE:
        act->reason = gdb_action_t::ACT_WWATCH;
        break;
      case BP_READ:
        act->reason = gdb_action_t::ACT_RWATCH;
        break;
      }
      act->data = stopped_at->addr;
    }
    break;
  
  case NEMU_GDB_INTERRUPT:
    act->reason = gdb_action_t::ACT_BREAKPOINT;
    act->data = cpu.pc;
    break;

  default:
    act->reason = gdb_action_t::ACT_SHUTDOWN;
    act->data = nemu_state.halt_ret;
  }
}

__EXPORT void nemu_cont(void *args, gdb_action_t *res) {
  DbgState *dbg_state = (DbgState *)args;
  breakpoint_t *stopped_at =
      cpu_exec_with_bp(-1, dbg_state->bp->data(), dbg_state->bp->size());
  nemu_is_stopped(res, stopped_at);
}

__EXPORT void nemu_stepi(void *args, gdb_action_t *res) {
  DbgState *dbg_state = (DbgState *)args;
  breakpoint_t *stopped_at =
      cpu_exec_with_bp(1, dbg_state->bp->data(), dbg_state->bp->size());
  nemu_is_stopped(res, stopped_at);
}

__EXPORT bool nemu_set_bp(void *args, size_t addr, bp_type_t type) {
  DbgState *dbg_state = (DbgState *)args;
  for (const auto &bp : *dbg_state->bp) {
    if (bp.addr == addr && bp.type == type) {
      return true;
    }
  }
  dbg_state->bp->push_back({.addr = addr, .type = type});
  return true;
}

__EXPORT bool nemu_del_bp(void *args, size_t addr, bp_type_t type) {
  DbgState *dbg_state = (DbgState *)args;
  for (auto it = dbg_state->bp->begin(); it != dbg_state->bp->end(); it++) {
    if (it->addr == addr && it->type == type) {
      std::swap(*it, *dbg_state->bp->rbegin());
      dbg_state->bp->pop_back();
      return true;
    }
  }
  return false;
}

__EXPORT void nemu_on_interrupt(void *args) {
  nemu_state.state = NEMU_GDB_INTERRUPT;
}

__EXPORT int nemu_read_reg(void *args, int regno, size_t *data) {
  return isa_read_reg(args, regno, data);
}
__EXPORT int nemu_write_reg(void *args, int regno, size_t data) {
  return isa_write_reg(args, regno, data);
}

static struct target_ops nemu_gdbstub_ops = {.cont = nemu_cont,
                                             .stepi = nemu_stepi,
                                             .read_reg = nemu_read_reg,
                                             .write_reg = nemu_write_reg,
                                             .read_mem = nemu_read_mem,
                                             .write_mem = nemu_write_mem,
                                             .set_bp = nemu_set_bp,
                                             .del_bp = nemu_del_bp,
                                             .on_interrupt = nemu_on_interrupt,
                                             .monitor = NULL};
static DbgState *pdbg;
static gdbstub_t gdbstub_priv;
const char SOCKET_ADDR[] = "/tmp/gdbstub-nemu.sock";

static void init_remote_gdbstub(void *args) {
  DbgState *dbg_state = (DbgState *)args;
  pdbg = (DbgState *)args;
  dbg_state->bp = new std::vector<breakpoint_t>();
  dbg_state->halt = 0;
  Assert(dbg_state->bp != NULL, "Failed to allocate breakpoint");
}

__EXPORT void nemu_init(void *args) {
  init_remote_gdbstub(args);

  void init_rand();
  void init_mem();

  IFDEF(CONFIG_DEVICE, void init_device());

  init_rand();
  init_mem();
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();
}

int gdbstub_loop() {
  if (!gdbstub_init(&gdbstub_priv, &nemu_gdbstub_ops,
                    (arch_info_t)isa_arch_info, NULL, SOCKET_ADDR)) {
    return EINVAL;
  }
  printf("Waiting for gdb connection at %s", SOCKET_ADDR);
  bool success = gdbstub_run(&gdbstub_priv, pdbg);
  // gdbstub_close(&gdbstub_priv);
  return !success;
}
}
