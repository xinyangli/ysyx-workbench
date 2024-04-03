#include "components.hpp"
#include "config.hpp"
#include "vl_wrapper.hpp"
#include <VFlow.h>

using VlModule = VlModuleInterfaceCommon<VFlow>;
using Registers = _RegistersVPI<uint32_t, 32>;

extern "C" {
void *pmem_get() {
  static auto pmem = new Memory<int, 128 * 1024>(config.memory_file,
                                                 config.memory_file_binary);
  return pmem;
}

int pmem_read(int raddr) {
  void *pmem = pmem_get();
  auto mem = static_cast<Memory<int, 128 * 1024> *>(pmem);
  return mem->read(raddr);
}

void pmem_write(int waddr, int wdata, char wmask) {
  void *pmem = pmem_get();
  auto mem = static_cast<Memory<int, 128 * 1024> *>(pmem);
  return mem->write((std::size_t)waddr, wdata, wmask);
}
}

int main(int argc, char **argv, char **env) {
  config.cli_parse(argc, argv);
  auto top = std::make_shared<VlModule>(config.do_trace, config.wavefile);
  Registers regs("TOP.Flow.reg_0.regFile_");

  top->reset_eval(10);
  for (int i = 0; i < config.max_sim_time; i++) {
    if (top->is_posedge()) {
      // Posedge
      regs.update();
      regs.print_regs();
    }
    top->eval();
  }
  return 0;
}
