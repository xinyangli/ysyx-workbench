#include <vpi_user.h>
#include <VFlow.h>
#include <cstdlib>
#include <vector>
#include <memory>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <verilated_vpi.h>
#include <string>
#define MAX_SIM_TIME 100
#define VERILATOR_TRACE

std::vector<vpiHandle> regsHandle;
int regs[32];

static void init_vpi_regs() {
  std::string regfile = "TOP.Flow.reg_0.regFile_";
  for(int i = 0; i < 32; i++) {
    std::string regname = regfile + std::to_string(i);
    vpiHandle vh = vpi_handle_by_name((PLI_BYTE8 *)regname.c_str(), NULL);
    regsHandle.push_back(vh);
  }
}

static void init_vpi() {
  init_vpi_regs();
}

static int vpi_get_int(vpiHandle vh) {
  s_vpi_value v;
  v.format = vpiIntVal;
  vpi_get_value(vh, &v);
  return v.value.integer;
}

static void update_regs() {
  for(int i = 0; i < 32; i++) {
    regs[i] = vpi_get_int(regsHandle[i]);
  }
}

static void print_regs() {
  for(int i = 0; i < 32; i++) {
    printf("%d: %d\t", i, regs[i]);
    if(i % 8 == 7) putchar('\n');
  }
  putchar('\n');
}

static int sim_time = 0;

int main(int argc, char **argv, char **env) {
  int sim_time = 0;
  int posedge_cnt = 0;
  Verilated::commandArgs(argc, argv);

  std::unique_ptr<VFlow> top{new VFlow};
  Verilated::traceEverOn(true);
  VerilatedVcdC *m_trace = new VerilatedVcdC;
#ifdef VERILATOR_TRACE
  top->trace(m_trace, 5);
  m_trace->open("waveform.vcd");
#endif

  init_vpi();

  top->reset = 0;
  for (sim_time = 10; sim_time < MAX_SIM_TIME; sim_time++) {
    top->eval();
    top->clock = !top->clock;
    if(top->clock == 1) {
      // Posedge
      ++posedge_cnt;
      update_regs();
      print_regs();
    }

#ifdef VERILATOR_TRACE
    m_trace->dump(sim_time);
#endif
  }
#ifdef VERILATOR_TRACE
  m_trace->close();
#endif
  exit(EXIT_SUCCESS);
}
