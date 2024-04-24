#include "utils.h"
#include <vector>

extern "C" {
#include <gdbstub.h>
#include <stdlib.h>
#include <cpu/cpu.h>
#include <debug.h>
#include <errno.h>
#include <isa.h>
#include <memory/paddr.h>
#include <stddef.h>
}

typedef struct {
  std::vector<breakpoint_t> *bp;
  bool halt;
  gdbstub_t gdbstub;
} DbgState;

static int nemu_read_mem(void *args, size_t addr, size_t len, void *val) {
  if(!in_pmem(addr)) return EINVAL;
  memcpy(val, guest_to_host(addr), len);
  return 0;
}

static int nemu_write_mem(void *args, size_t addr, size_t len, void *val) {
  if(!in_pmem(addr)) return EINVAL;
  memcpy(guest_to_host(addr), val, len);
  return 0;
}

static gdb_action_t nemu_is_stopped() {
  switch (nemu_state.state) {
  case NEMU_RUNNING:
    nemu_state.state = NEMU_STOP;
    return (gdb_action_t) {.reason = gdb_action_t::ACT_BREAKPOINT, .data = 0};

  default:
    return (gdb_action_t) {.reason = gdb_action_t::ACT_SHUTDOWN, .data = nemu_state.halt_ret};
  }
}

static void nemu_cont(void *args, gdb_action_t *res) {
  DbgState *dbg_state = (DbgState *)args;
  cpu_exec_with_bp(-1, dbg_state->bp->data(), dbg_state->bp->size());
  *res = nemu_is_stopped();
}

static void nemu_stepi(void *args, gdb_action_t *res) {
  DbgState *dbg_state = (DbgState *)args;
  cpu_exec_with_bp(1, dbg_state->bp->data(), dbg_state->bp->size());
  *res = nemu_is_stopped();
}

static bool nemu_set_bp(void *args, size_t addr, bp_type_t type) {
  DbgState *dbg_state = (DbgState *)args;
  if (dbg_state->bp == nullptr) {
    dbg_state->bp = new std::vector<breakpoint_t>();
    assert(dbg_state->bp);
  }
  for (const auto &bp : *dbg_state->bp) {
    if (bp.addr == addr && bp.type == type) {
      return true;
    }
  }
  dbg_state->bp->push_back({.addr = addr, .type = type});
  return true;
}

static bool nemu_del_bp(void *args, size_t addr, bp_type_t type) {
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

static void nemu_on_interrupt(void *args) {
  fputs("Not implemented", stderr);
}

extern "C" {

static struct target_ops nemu_gdbstub_ops = {.cont = nemu_cont,
                                             .stepi = nemu_stepi,
                                             .read_reg = isa_read_reg,
                                             .write_reg = isa_write_reg,
                                             .read_mem = nemu_read_mem,
                                             .write_mem = nemu_write_mem,
                                             .set_bp = nemu_set_bp,
                                             .del_bp = nemu_del_bp,
                                             .on_interrupt = nemu_on_interrupt};
static DbgState dbg;
int nemu_gdbstub_init() {
  if (!gdbstub_init(&dbg.gdbstub, &nemu_gdbstub_ops, (arch_info_t)isa_arch_info,
                    (char *)"127.0.0.1:1234")) {
    return EINVAL;
  }
  return 0;
}
int nemu_gdbstub_run() {
  if (!gdbstub_run(&dbg.gdbstub, &dbg)) {
    return EINVAL;
  }
  return 0;
}
}