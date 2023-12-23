#include "verilated_vcd_c.h"
#include "Vexample.h"
#include "verilated.h"

int main(int argc, char **argv, char **env) {
}

int main(int argc, char **argv, char **env) {
    Verilated::commandArgs(argc, argv);
    Verilated::traceEverOn(true);
    VerilatedVcdC* tfp = new VerilatedVcdC;
    Vexample *top = new Vexample;
    int round = 100;
    while (round--) {
        int a = rand() & 1;
        int b = rand() & 1;
        top->a = a;
        top->b = b;
        top->eval();
        printf("a = %d, b = %d, f = %d\n", a, b, top->f);
        assert(top->f == (a ^ b));
    }
    exit(0);
    topp->trace (tfp, 99);
    tfp->open ("obj_dir/t_trace_ena_cc/simx.vcd");
    ...
    while (sc_time_stamp() < sim_time && !Verilated::gotFinish()) {
        main_time += #;
        tfp->dump (main_time);
    }
    tfp->close();
}