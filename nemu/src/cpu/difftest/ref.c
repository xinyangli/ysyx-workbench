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

#include "types.h"
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <difftest-def.h>
#include <isa.h>
#include <memory/paddr.h>
#include <stdint.h>

__EXPORT void difftest_memcpy(paddr_t addr, void *buf, size_t n,
                              bool direction) {
  if (direction == DIFFTEST_TO_REF) {
    Log("Copy 0x%08x, %p, %lu, %d\n", addr, buf, n, direction);
    for(int i = 0; i < 32; i++) {
      printf("%04x ", ((uint8_t *)buf)[i]);
      if (i % 4 == 3) putchar('\n');
    }
    memcpy(guest_to_host(addr), buf, n);
  } else {
    assert(0);
  }
}

__EXPORT void difftest_regcpy(void *dut, bool direction) {
  // assert(0);
  if (direction == DIFFTEST_TO_DUT)
    memcpy(dut, &cpu, sizeof(CPU_state));
  else
    memcpy(&cpu, dut, sizeof(CPU_state));
}

__EXPORT void difftest_exec(uint64_t n) { cpu_exec(n); }

__EXPORT void difftest_raise_intr(word_t NO) {
  // assert(0);
}

__EXPORT void difftest_init(int port) {
  void init_mem();
  init_mem();
  /* Perform ISA dependent initialization. */
  init_isa();
}
