#ifndef _SDB_SDB_HEADER_FILE_
#define _SDB_SDB_HEADER_FILE_

#include <components.hpp>
#include <console.hpp>
#include <memory>
#include <stdexcept>
#include <string>
#include <trm_interface.hpp>
#include <types.h>

namespace cr = CppReadline;
using ret = cr::Console::ReturnCode;

namespace SDB {

enum SDBStatus {
  SDB_SUCCESS,
  SDB_EBREAK = 2,
  SDB_WRONG_ARGUMENT,
  SDB_DIFFTEST_FAILED
};

class SDBHandlers;

struct Handler {
  const std::vector<const char *> names;
  // cr::Console::CommandFunction f;
  std::function<int(SDBHandlers *, const cr::Console::Arguments &)> f;
};

class SDBHandlers {
private:
  const TrmInterface &funcs;
  std::vector<Handler> all_handlers = {
      Handler{{"c", "continue"}, &SDBHandlers::cmd_continue},
      Handler{{"si", "step-instruction"}, &SDBHandlers::cmd_step},
      Handler{{"info-r"}, &SDBHandlers::cmd_info_registers},
      Handler{{"p", "print"}, &SDBHandlers::cmd_print},
      Handler{{"disas", "disassemble"}, &SDBHandlers::cmd_disassemble},
  };
  int cmd_continue(const cr::Console::Arguments &input);
  int cmd_step(const std::vector<std::string> &input);
  int cmd_info_registers(const std::vector<std::string> &input);
  int cmd_print(const std::vector<std::string> &input);
  int cmd_disassemble(const std::vector<std::string> &input);
  int exec_catch(uint64_t);

public:
  SDBHandlers(const TrmInterface &funcs) : funcs(funcs){};
  void register_handlers(cr::Console *c);
};

class SDB {
private:
  std::unique_ptr<CppReadline::Console> c;
  const TrmInterface &funcs;
  SDBHandlers handlers;

public:
  SDB(const TrmInterface &funcs,
      std::string const &greeting = "\033[1;34m(npc)\033[0m ")
      : handlers(SDBHandlers{funcs}), funcs(funcs) {
    c = std::make_unique<CppReadline::Console>(greeting);

    handlers.register_handlers(c.get());
  };

  int main_loop() {
    int ret_code;
    funcs.init(0);
    std::vector<std::string> step_commands{"c", "continue", "si",
                                           "step-instruction"};
    do {
      ret_code = c->readLine();
      // We can also change the prompt based on last return value:
      if (ret_code == SDB_SUCCESS || ret_code == SDB_EBREAK)
        c->setGreeting("\033[1;34m(npc)\033[0m ");
      else
        c->setGreeting("\033[1;31m(npc)\033[0m ");

      switch (ret_code) {
      case SDB_EBREAK: {
        std::cout << "\033[1;31m=== ebreak ===\033[0m" << std::endl;
        c->removeCommands(step_commands);
        break;
      }
      case SDB_WRONG_ARGUMENT: {
        std::cout << "Wrong argument(s) is given to command handler\n";
        break;
      }
      }
    } while (ret_code != ret::Quit);
    return 0;
  }
};
} // namespace SDB

#endif
