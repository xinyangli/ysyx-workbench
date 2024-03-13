#include <verilated.h>
#include <verilated_vcd_c.h>
#include <nvboard.h>
#include <VFlow.h>

const int MAX_SIM_TIME=100;

void nvboard_bind_all_pins(VFLow* top);

int main(int argc, char **argv, char **env) {
    VFlow *top = new VFlow;

    // nvboard_init();
    while (true) {
        // nvboard_update();
        top->eval();
    }
    delete top;
}