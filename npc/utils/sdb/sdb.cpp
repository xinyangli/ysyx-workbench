#include <disasm.hpp>
#include <components.hpp>
#include <console.hpp>
#include <trm_interface.hpp>
#include <sdb.hpp>
#include <types.h>
extern "C" {
#include <addrexp.h>
#include <addrexp_lex.h>
}

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

word_t parse_expr(const char *arg) {
  if (arg == NULL) {
    puts("Invalid expr argument.");
    return 0;
  } else {
    word_t res;
    yy_scan_string(arg);
    yyparse(&res);
    yylex_destroy();
    return res;
  }
}

int SDBHandlers::cmd_print(const std::vector<std::string> &input) {
  word_t buf[2];
  word_t addr = parse_expr(input[1].c_str());
  this->funcs.memcpy(addr, &buf, sizeof(word_t), TRM_FROM_MACHINE);
  // TODO: Difftest only
  std::cout << std::hex << buf[0] << ' ' << buf[1] << std::dec << std::endl;
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
