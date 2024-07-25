#ifndef ARCH_H__
#define ARCH_H__
#include <stdint.h>

#ifdef __riscv_e
#define NR_REGS 16
#else
#define NR_REGS 32
#endif

struct Context {
  // TODO: fix the order of these members to match trap.S
  void *pdir;
  uintptr_t gpr[NR_REGS - 1];
  uintptr_t mcause, mstatus, mepc;
};

enum Cause { CauseEnvironmentCallFromMMode = 11 };

#ifdef __riscv_e
#define GPR1 gpr[15] // a5
#else
#define GPR1 gpr[17] // a7
#endif

#define GPR2 gpr[0]
#define GPR3 gpr[0]
#define GPR4 gpr[0]
#define GPRx gpr[0]

#endif
