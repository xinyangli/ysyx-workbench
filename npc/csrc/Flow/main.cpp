#include <cstdlib>
#include <cassert>
#include <cstdlib>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <VFlow.h>
#define MAX_SIM_TIME 100
#define VERILATOR_TRACE

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
    for (sim_time = 0; sim_time < MAX_SIM_TIME; sim_time++) {
        top->eval();
        top->clock = !top->clock;
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
