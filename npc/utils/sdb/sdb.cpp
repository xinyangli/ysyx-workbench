#include <disasm.hpp>
#include <components.hpp>
#include <console.hpp>
#include <trm_interface.hpp>
#include <sdb.hpp>
#include <types.h>

namespace cr = CppReadline;
using ret = cr::Console::ReturnCode;
Disassembler d{"riscv32-pc-linux-gnu"};

std::ostream& operator<<(std::ostream &os, const TrmInterface &d) {
  d.print(os);
  return os;
};

namespace SDB {

int SDBHandlers::cmd_continue(const cr::Console::Arguments &input) {
  if (input.size() > 1)
    return SDB_WRONG_ARGUMENT;
  this->funcs.exec(-1);
  return SDB_SUCCESS;
}

int SDBHandlers::cmd_step(const std::vector<std::string> &input) {
  if (input.size() > 2) {
    return SDB_WRONG_ARGUMENT;
  }
  uint64_t step_count = input.size() == 2 ? std::stoull(input[1]) : 1;
  this->funcs.exec(step_count);
  std::cout << funcs << std::endl;
  return SDB_SUCCESS;
}

int SDBHandlers::cmd_info_registers(const std::vector<std::string> &input) {
  if (input.size() > 1)
    return SDB_WRONG_ARGUMENT;
  std::cout << this->funcs << std::endl;
  return SDB_SUCCESS;
}

int SDBHandlers::cmd_print(const std::vector<std::string> &input) {
  word_t buf[2];
  paddr_t addr = std::stoul(input[1]);
  this->funcs.memcpy(addr, &buf, sizeof(word_t), TRM_FROM_MACHINE);
  // TODO: Difftest only
  std::cout << buf[0] << ' ' << buf[1] << std::endl;
  return SDB_SUCCESS;
}

void SDBHandlers::registerHandlers(cr::Console *c) {
  for (auto &h : this->all_handlers) {
    for (auto &name : h.names) {
      std::function<int(std::vector<std::string>)> f {std::bind(h.f, this, std::placeholders::_1)};
      c->registerCommand(name, f);
    }
  }
}
}
