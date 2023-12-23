#include <cstdlib>
#include <cassert>
#include <cstdlib>
#include "Vexample.h"
#include "verilated.h"

int main(int argc, char **argv, char **env) {
    Verilated::commandArgs(argc, argv);
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
}
