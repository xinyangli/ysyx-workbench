#include "vpi_user.h"
#include <VFlow.h>
#include <cstdlib>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <verilated_vpi.h>
#define MAX_SIM_TIME 100
#define VERILATOR_TRACE

int vpiGetInt(const char *name) {
  vpiHandle vh1 = vpi_handle_by_name((PLI_BYTE8 *)name, NULL);
  if(!vh1)
    vl_fatal(__FILE__, __LINE__, "sim_main", "No handle found");
  s_vpi_value v;
  v.format = vpiIntVal;
  vpi_get_value(vh1, &v);
  return v.value.integer;
}
  

int main(int argc, char **argv, char **env) {
  int sim_time = 0;
  Verilated::commandArgs(argc, argv);

  VFlow *top = new VFlow;

  Verilated::traceEverOn(true);
  VerilatedVcdC *m_trace = new VerilatedVcdC;
#ifdef VERILATOR_TRACE
  top->trace(m_trace, 5);
  m_trace->open("waveform.vcd");
#endif
  for (sim_time = 0; sim_time < 10; sim_time++) {
    top->eval();
    top->clock = !top->clock;
    top->reset = 1;
#ifdef VERILATOR_TRACE
    m_trace->dump(sim_time);
#endif
  }
  top->reset = 0;
  for (sim_time = 10; sim_time < MAX_SIM_TIME; sim_time++) {
    top->eval();
    top->clock = !top->clock;
    int o = vpiGetInt("TOP.Flow.reg_0.regFile_2");
    printf("%d\n", o);
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
