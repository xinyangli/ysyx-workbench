#include <components.hpp>
#include <console.hpp>
#include <cstdint>
#include <disasm.hpp>
#include <sdb.hpp>
#include <trm_interface.hpp>
#include <types.h>
extern "C" {
#include <addrexp.h>
#include <addrexp_lex.h>
}

namespace cr = CppReadline;
using ret = cr::Console::ReturnCode;

std::ostream &operator<<(std::ostream &os, const TrmInterface &d) {
  d.print(os);
  return os;
};

Disassembler d{"riscv32-linux-pc-gnu"};

namespace SDB {

int SDBHandlers::exec_catch(uint64_t n) {
  try {
    this->funcs.exec(n);
  } catch (TrmRuntimeException &e) {
    switch (e.error_code()) {
    case TrmRuntimeException::EBREAK:
      return SDB_EBREAK;
    case TrmRuntimeException::DIFFTEST_FAILED:
      std::cout << "Difftest Failed" << std::endl << funcs << std::endl;
      return SDB_DIFFTEST_FAILED;
    default:
      std::cerr << "Unknown error code" << std::endl;
      exit(1);
    }
  }
  return SDB_SUCCESS;
}

int SDBHandlers::cmd_continue(const cr::Console::Arguments &input) {
  if (input.size() > 1)
    return SDB_WRONG_ARGUMENT;
  return exec_catch(-1);
}

int SDBHandlers::cmd_step(const std::vector<std::string> &input) {
  if (input.size() > 2) {
    return SDB_WRONG_ARGUMENT;
  }
  uint64_t step_count = input.size() == 2 ? std::stoull(input[1]) : 1;
  return exec_catch(step_count);
  // std::cout << funcs << std::endl;
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
  std::cout << std::hex << "dut: 0x" << buf[0] << ' ' << "ref: 0x" << buf[1]
            << std::dec << std::endl;
  return SDB_SUCCESS;
}

int SDBHandlers::cmd_disassemble(const std::vector<std::string> &input) {
  word_t buf[2];
  word_t addr = parse_expr(input[1].c_str());
  this->funcs.memcpy(addr, &buf, sizeof(word_t), TRM_FROM_MACHINE);
  // TODO: Difftest only
  std::cout << "dut: \n"
            << d.disassemble(addr, (uint8_t *)&buf[0], sizeof(word_t))
            << std::endl
            << "ref: \n"
            << d.disassemble(addr, (uint8_t *)&buf[0], sizeof(word_t))
            << std::endl;
  ;
  return SDB_SUCCESS;
}

void SDBHandlers::register_handlers(cr::Console *c) {
  for (auto &h : this->all_handlers) {
    for (auto &name : h.names) {
      std::function<int(std::vector<std::string>)> f{
          std::bind(h.f, this, std::placeholders::_1)};
      c->registerCommand(name, f);
    }
  }
}

} // namespace SDB
