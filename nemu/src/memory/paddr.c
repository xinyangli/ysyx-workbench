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

#include "common.h"
#include "debug.h"
#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>

#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
#endif
#ifdef CONFIG_MTRACE
static word_t mtrace_start[CONFIG_MTRACE_RANGE_MAX] = {0};
static word_t mtrace_end[CONFIG_MTRACE_RANGE_MAX] = {0};
static int range_count = 0;
#endif

uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

#ifdef CONFIG_MTRACE
static void mtrace_print(char type, word_t addr, int len, word_t data) {
  for (int i = 0; i < range_count; i++)
    if (addr <= mtrace_end[i] && addr >= mtrace_start[i] ) {
      Trace("PC=" FMT_PADDR "Mem %c" FMT_PADDR "%d D " FMT_PADDR, cpu.pc, type, addr, len, data);
      break;
    }
}
#endif

void init_mem() {
#if   defined(CONFIG_PMEM_MALLOC)
  pmem = malloc(CONFIG_MSIZE);
  assert(pmem);
#endif
#ifdef CONFIG_MTRACE
  char range[sizeof(CONFIG_MTRACE_RANGE)] = CONFIG_MTRACE_RANGE;
  char *saveptr, *ptr;
  ptr = strtok_r(range, ",", &saveptr);
  for (range_count = 0; range_count < CONFIG_MTRACE_RANGE_MAX; ) {
    word_t start, end;
    Assert(sscanf(ptr, FMT_PADDR "-" FMT_PADDR, &start, &end) == 2, "Config option MTRACE_RANGE has wrong format");
    mtrace_start[range_count] = start;
    mtrace_end[range_count] = end;

    range_count++;
    ptr = strtok_r(NULL, ",", &saveptr);
    if (!ptr) break;
  }
  Trace("MTRACE ranges: ");
  for (int i = 0; i < range_count; i++) {
    Trace("[0x%x, 0x%x]", mtrace_start[i], mtrace_end[i]);
  }
#endif
  IFDEF(CONFIG_MEM_RANDOM, memset(pmem, rand(), CONFIG_MSIZE));
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

word_t paddr_read(paddr_t addr, int len) {
  word_t result = 0;
  if (likely(in_pmem(addr))) { result = pmem_read(addr, len); goto mtrace;}
  IFDEF(CONFIG_DEVICE, result = mmio_read(addr, len); goto mtrace)
  out_of_bound(addr);

mtrace:
  IFDEF(CONFIG_MTRACE, mtrace_print('R', addr, len, result));
  
  return result;
}

void paddr_write(paddr_t addr, int len, word_t data) {
  IFDEF(CONFIG_MTRACE, mtrace_print('W', addr, len, data));
  if (likely(in_pmem(addr))) { pmem_write(addr, len, data); return; }
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}
