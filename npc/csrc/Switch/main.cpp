#include <VSwitch.h>
#include <cassert>
#include <cstdlib>
#include <verilated.h>
#include <verilated_vcd_c.h>

const int MAX_SIM_TIME = 100;

int main(int argc, char **argv, char **env) {
  int sim_time = 0;
  Verilated::commandArgs(argc, argv);
  VSwitch *top = new VSwitch;

  Verilated::traceEverOn(true);
  VerilatedVcdC *m_trace = new VerilatedVcdC;
#ifdef VERILATOR_TRACE
  top->trace(m_trace, 5);
  m_trace->open("waveform.vcd");
#endif
  for (sim_time = 0; sim_time < MAX_SIM_TIME; sim_time++) {
    top->io_sw_0 = rand() % 2;
    top->io_sw_1 = rand() % 2;
    top->eval();
    printf("sw0 = %d, sw1 = %d, ledr = %d\n", top->io_sw_0, top->io_sw_1,
           top->io_out);
    assert(top->io_out == (top->io_sw_0 ^ top->io_sw_1));
#ifdef VERILATOR_TRACE
    m_trace->dump(sim_time);
#endif
  }
#ifdef VERILATOR_TRACE
  m_trace->close();
#endif
  delete top;
  exit(EXIT_SUCCESS);
}
