#include <components.hpp>
#include <console.hpp>
#include <difftest.hpp>
#include <sdb.hpp>
#include <types.h>

namespace cr = CppReadline;
using ret = cr::Console::ReturnCode;

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
  return SDB_SUCCESS;
}

int SDBHandlers::cmd_info_registers(const std::vector<std::string> &input) {
  if (input.size() > 1)
    return SDB_WRONG_ARGUMENT;
  std::cout << cpu << std::endl;
  return SDB_SUCCESS;
}

int SDBHandlers::cmd_print(const std::vector<std::string> &input) {
  exit(1);
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
