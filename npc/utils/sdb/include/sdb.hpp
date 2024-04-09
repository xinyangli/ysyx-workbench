#ifndef _SDB_SDB_HEADER_FILE_
#define _SDB_SDB_HEADER_FILE_

#include <components.hpp>
#include <console.hpp>
#include <difftest.hpp>
#include <memory>
#include <stdexcept>
#include <types.h>

namespace cr = CppReadline;
using ret = cr::Console::ReturnCode;

namespace SDB {

enum SDBStatus {
  SDB_SUCCESS,
  SDB_WRONG_ARGUMENT,
};

struct Handler {
  const std::vector<const char *> names;
  cr::Console::CommandFunction f;
};

template <const DifftestInterface &funcs> class _SDBHandlers {
  using CPUState = CPUStateBase<uint32_t, 32>;

private:
  std::vector<Handler> all_handlers;

public:
  static CPUState cpu;

private:
  static _SDBHandlers<funcs> *instance;
  static int cmd_continue(const cr::Console::Arguments &input);
  static int cmd_step(const std::vector<std::string> &input);
  static int cmd_info_registers(const std::vector<std::string> &input);
  static int cmd_print(const std::vector<std::string> &input);
  _SDBHandlers<funcs>(std::vector<Handler> all_handlers)
      : all_handlers(all_handlers){};

public:
  _SDBHandlers<funcs>(const _SDBHandlers<funcs> &) = delete;
  _SDBHandlers<funcs> operator=(const _SDBHandlers<funcs> &) = delete;

  static _SDBHandlers<funcs> *getInstance() {
    if (instance == nullptr) {
      std::vector<Handler> all_handlers{
          Handler{{"c", "continue"}, &_SDBHandlers::cmd_continue},
          Handler{{"si", "step-instruction"}, &_SDBHandlers::cmd_step},
      };
      instance = new _SDBHandlers<funcs>(all_handlers);
    }
    return instance;
  }

  void registerHandlers(cr::Console *c);
};

template <const DifftestInterface &funcs>
_SDBHandlers<funcs> *_SDBHandlers<funcs>::instance = nullptr;

template <const DifftestInterface &funcs>
CPUState _SDBHandlers<funcs>::cpu = CPUState();

template <const DifftestInterface &funcs>
int _SDBHandlers<funcs>::cmd_continue(const cr::Console::Arguments &input) {
  if (input.size() > 1)
    return SDB_WRONG_ARGUMENT;
  funcs.exec(-1);
  return SDB_SUCCESS;
}

template <const DifftestInterface &funcs>
int _SDBHandlers<funcs>::cmd_step(const std::vector<std::string> &input) {
  if (input.size() > 2) {
    return SDB_WRONG_ARGUMENT;
  }
  uint64_t step_count = input.size() == 2 ? std::stoull(input[1]) : 1;
  funcs.exec(step_count);
  return SDB_SUCCESS;
}

template <const DifftestInterface &funcs>
int _SDBHandlers<funcs>::cmd_info_registers(
    const std::vector<std::string> &input) {
  if (input.size() > 1)
    return SDB_WRONG_ARGUMENT;
  std::cout << _SDBHandlers<funcs>::getInstance()->cpu << std::endl;
  return SDB_SUCCESS;
}

template <const DifftestInterface &funcs>
int _SDBHandlers<funcs>::cmd_print(const std::vector<std::string> &input) {
  exit(1);
}

template <const DifftestInterface &funcs>
void _SDBHandlers<funcs>::registerHandlers(cr::Console *c) {
  for (auto &h : this->all_handlers) {
    for (auto &name : h.names) {
      c->registerCommand(name, h.f);
    }
  }
}

template <const DifftestInterface &funcs> class SDB {
private:
  std::unique_ptr<CppReadline::Console> c;
  using SDBHandlers = _SDBHandlers<funcs>;

public:
  SDB(std::string const &greeting = "\033[1;34m(npc)\033[0m ") {
    c = std::make_unique<CppReadline::Console>(greeting);
    SDBHandlers::getInstance()->registerHandlers(c.get());
  };

  word_t reg_str2val(const char *name, bool *success) {
    try {
      *success = true;
      return SDBHandlers::getInstance()->cpu.at(name);
    } catch (std::runtime_error) {
      *success = false;
      return 0;
    }
  }

  int main_loop() {
    int retCode;
    do {
      retCode = c->readLine();
      // We can also change the prompt based on last return value:
      if (retCode == ret::Ok)
        c->setGreeting("\033[1;34m(npc)\033[0m ");
      else
        c->setGreeting("\033[1;31m(npc)\033[0m ");

      if (retCode == SDB_WRONG_ARGUMENT) {
        std::cout << "Wrong argument give to command\n";
      }
    } while (retCode != ret::Quit);
    return 0;
  }
};
} // namespace SDB

#endif