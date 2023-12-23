#include "Vexample.h"
#include "verilated.h"

int main(int argc, char **argv, char **env) {
    Verilated::commandArgs(argc, argv);
    Vexample *top = new Vexample;
    while (!Verilated::gotFinish()) {
        top->eval();
    }
    exit(0);
}
