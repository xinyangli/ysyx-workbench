#ifndef _SDB_SDB_HEADER_FILE_
#define _SDB_SDB_HEADER_FILE_

#include <components.hpp>
#include <console.hpp>
#include <memory>
#include <stdexcept>
#include <trm_interface.hpp>
#include <types.h>

namespace cr = CppReadline;
using ret = cr::Console::ReturnCode;

namespace SDB {

enum SDBStatus { SDB_SUCCESS, SDB_WRONG_ARGUMENT, SDB_DIFFTEST_FAILED };

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
  };
  int cmd_continue(const cr::Console::Arguments &input);
  int cmd_step(const std::vector<std::string> &input);
  int cmd_info_registers(const std::vector<std::string> &input);
  int cmd_print(const std::vector<std::string> &input);

public:
  SDBHandlers(const TrmInterface &funcs) : funcs(funcs){};
  void registerHandlers(cr::Console *c);
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

    handlers.registerHandlers(c.get());
  };

  int main_loop() {
    int retCode;
    funcs.init(0);
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