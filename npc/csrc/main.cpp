#include <cstdlib>
#include <cassert>
#include <cstdlib>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vexample.h"

int main(int argc, char **argv, char **env) {
    int round = 100;
    Verilated::commandArgs(argc, argv);
    Vexample *top = new Vexample;

    Verilated::traceEverOn(true);
    VerilatedVcdC *m_trace = new VerilatedVcdC; 
    top->trace(m_trace, 5);
    m_trace->open("waveform.vcd");
    while (round--) {
        int a = rand() & 1;
        int b = rand() & 1;
        top->a = a;
        top->b = b;
        top->eval();
        printf("a = %d, b = %d, f = %d\n", a, b, top->f);
        assert(top->f == (a ^ b));
    }
    m_trace->close();
    delete top;
    exit(EXIT_SUCCESS);
}
