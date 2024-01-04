#include <cstdlib>
#include <cassert>
#include <cstdlib>
#include <verilated.h>
#include <VMain.h>
#include <nvboard.h>

const int MAX_SIM_TIME=100;

void nvboard_bind_all_pins(VMain* top);

int main(int argc, char **argv, char **env) {
    VMain* top = new VMain;

    nvboard_bind_all_pins(top);
    nvboard_init();
    while (true) {
        nvboard_update();
        top->eval();
    }
    delete top;
}