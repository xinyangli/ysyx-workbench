#include <cstdlib>
#include <cassert>
#include <cstdlib>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <Vexample.h>
#include <nvboard.h>

const int MAX_SIM_TIME=100;

void nvboard_bind_all_pins(Vexample* top);

int main(int argc, char **argv, char **env) {
    int sim_time = 0;
    Verilated::commandArgs(argc, argv);
    Vexample *top = new Vexample;

    Verilated::traceEverOn(true);
    VerilatedVcdC *m_trace = new VerilatedVcdC; 
    nvboard_bind_all_pins(top);
    nvboard_init();
#ifdef VERILATOR_TRACE
    top->trace(m_trace, 5);
    m_trace->open("waveform.vcd");
#endif
    // for (sim_time = 0; sim_time < MAX_SIM_TIME; sim_time++) {
    while (true) {
        nvboard_update();
        // CData sw = rand() & 0b11;
        // top->sw = sw;
        top->eval();
        // printf("sw0 = %d, sw1 = %d, ledr = %d\n", sw & 0b1, sw >> 1, top->ledr);
        // assert(top->ledr == ((sw >> 1) ^ (sw & 0b1)) );
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
