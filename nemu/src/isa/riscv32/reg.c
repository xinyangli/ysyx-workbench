/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "local-include/reg.h"
#include "macro.h"
#include <errno.h>
#include <gdbstub.h>
#include <isa.h>

const char *regs[] = {"$0", "ra", "sp",  "gp",  "tp", "t0", "t1", "t2",
                      "s0", "s1", "a0",  "a1",  "a2", "a3", "a4", "a5",
                      "a6", "a7", "s2",  "s3",  "s4", "s5", "s6", "s7",
                      "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

void isa_reg_display() {
  int colomn_per_row = 4;
  for (int i = 0; i < ARRLEN(regs); i++) {
    printf("\e[1;34m%3s\e[0m: " FMT_PADDR, reg_name(i), gpr(i));
    if (i % colomn_per_row == 3)
      putchar('\n');
    else
      putchar('|');
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  assert(s);
  int i;
  for (i = 0; i < 32 && strcmp(s, regs[i]) != 0; i++)
    ;

  if (i == 32) {
    *success = false;
    return 0;
  }
  *success = true;

  return gpr(i);
}

int isa_read_reg(void *args, int regno, size_t *reg_value) {
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

int isa_write_reg(void *args, int regno, size_t data) {
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

__EXPORT arch_info_t isa_arch_info = {.reg_num = 32,
                                      .reg_byte = MUXDEF(CONFIG_RV64, 8, 4),
                                      .target_desc = TARGET_RV32};
