#include <cstdlib>
#include <cassert>
#include <cstdlib>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <nvboard.h>
#include <VSwitch.h>

const int MAX_SIM_TIME=100;

void nvboard_bind_all_pins(VSwitch* top);

int main(int argc, char **argv, char **env) {
    VSwitch *top = new VSwitch;
    nvboard_bind_all_pins(top);
    nvboard_init();
    while (true) {
        nvboard_update();
        top->eval();
    }
    delete top;
}