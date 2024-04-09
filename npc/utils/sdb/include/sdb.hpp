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

class SDBHandlers;

struct Handler {
  const std::vector<const char *> names;
  // cr::Console::CommandFunction f;
  std::function<int(SDBHandlers*, const cr::Console::Arguments &)> f;
};

class SDBHandlers {
  using CPUState = CPUStateBase<uint32_t, 32>;

private:
  const TrmInterface &funcs;
  std::vector<Handler> all_handlers = {
      Handler{{"c", "continue"}, &SDBHandlers::cmd_continue},
      Handler{{"si", "step-instruction"}, &SDBHandlers::cmd_step},
  };
  int cmd_continue(const cr::Console::Arguments &input);
  int cmd_step(const std::vector<std::string> &input);
  int cmd_info_registers(const std::vector<std::string> &input);
  int cmd_print(const std::vector<std::string> &input);

public:
  CPUState cpu;
  SDBHandlers(const TrmInterface &funcs)
      : funcs(funcs){};
  void registerHandlers(cr::Console *c);
};

class SDB {
private:
  std::unique_ptr<CppReadline::Console> c;
  TrmInterface func;
  SDBHandlers handlers;
public:
  SDB(TrmInterface func, std::string const &greeting = "\033[1;34m(npc)\033[0m ") :
  handlers(SDBHandlers(func)), func(func) {
    c = std::make_unique<CppReadline::Console>(greeting);
    
    handlers.registerHandlers(c.get());
  };

  int main_loop() {
    int retCode;
    func.init(0);
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