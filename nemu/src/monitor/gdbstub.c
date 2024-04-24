#include <stddef.h>
#include <stdbool.h>
#include <errno.h>
#include <isa.h>
#include <memory/paddr.h>

#include "gdbstub.h"

typedef struct {
  size_t *bp;
  size_t bp_len;
  bool halt;
} DbgState;
// static DbgState dbg_state = { .bp = NULL, .halt = false };

    // DbgState *dbg_state = (DbgState *)args;
static int nemu_read_reg(void *args, int regno, size_t *reg_value) {
    if (regno > 32) {
        return EFAULT;
    }

    if (regno == 32) {
        *reg_value = cpu.pc;
    } else {
        *reg_value = cpu.gpr[regno];
    }
    return 0;
}

static int nemu_write_reg(void *args, int regno, size_t data) {
    if (regno > 32) {
        return EFAULT;
    }

    if (regno == 32) {
        cpu.pc = data;
    } else {
        cpu.gpr[regno] = data;
    }
    return 0;
}

static int nemu_read_mem(void *args, int addr, size_t len, void *val) {
    memcpy(val, guest_to_host(addr), len);
    return 0;
}

static int nemu_write_mem(void *args, int addr, size_t len, void *val) {
    memcpy(guest_to_host(addr), val, len);
    return 0;
}

static gdb_action_t nemu_cont(void *args) {
    
}
