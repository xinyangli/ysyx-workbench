#ifndef __NEMU_DIFFTEST_H__
#define __NEMU_DIFFTEST_H__

#include <stdbool.h>

extern bool do_difftest;
static void difftest_skip_ref(void) {
  do_difftest = false;
}

#endif // __NEMU_DIFFTEST_H__