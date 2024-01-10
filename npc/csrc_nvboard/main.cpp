#include <VSegHandler.h>
#include <cassert>
#include <cstdlib>
#include <nvboard.h>
#include <verilated.h>
#include <verilated_vcd_c.h>

#ifndef VERILATOR_TOPMODULE
#define VERILATOR_TOPMODULE VSegHandler
#endif

const int MAX_SIM_TIME = 100;
int keycode = 0;

template <class F> void cycle(VERILATOR_TOPMODULE *top, F &&f) {
  top->clock = 0;
  top->eval();
  f();
  top->clock = 1;
  top->eval();
}

void nvboard_bind_all_pins(VERILATOR_TOPMODULE *top);

static void single_cycle(VERILATOR_TOPMODULE *top) {
  top->clock = 0;
  top->eval();
  top->clock = 1;
  top->eval();
}

static void reset(VERILATOR_TOPMODULE *top, int n) {
  top->reset = 1;
  while (n-- > 0)
    single_cycle(top);
  top->reset = 0;
}

int main(int argc, char **argv, char **env) {
  VERILATOR_TOPMODULE *top = new VERILATOR_TOPMODULE;

  nvboard_bind_all_pins(top);
  nvboard_init();
  reset(top, 10);
  while (true) {
    nvboard_update();
    cycle(top, [&] {
      if (keycode != top->io_keycode_bits) {
        printf("%x\n", top->io_keycode_bits);
        keycode = top->io_keycode_bits;
      }
    });
  }
  delete top;
}