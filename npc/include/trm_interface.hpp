#ifndef _NPC_TRM_INTERFACE_HEADER_FILE_
#define _NPC_TRM_INTERFACE_HEADER_FILE_
#include <dlfcn.h>
#include <filesystem>
#include <functional>
#include <stdexcept>
#include <string>
#include <types.h>

template <typename R, size_t nr_reg> struct CPUStateBase {
  R reg[nr_reg] = {0};
  word_t pc = 0x80000000;

  static const std::map<std::string, int> inline regs_by_name =
      riscv32_regs_by_name;
  CPUStateBase() {
    for (int i = 0; i < nr_reg; i++)
      reg[i] = 0;
  }

  bool operator==(const CPUStateBase &other) const {
    if (pc != other.pc)
      return false;
    for (int i = 0; i < nr_reg; ++i) {
      if (reg[i] != other.reg[i])
        return false;
    }
    return true;
  }

  bool operator!=(const CPUStateBase &other) const {
    return !(*this == other); // Reuse the == operator for != implementation
  }

  /* This does not update the register!!! */
  R at(std::string name) {
    return name == "pc" ? pc : reg[regs_by_name.at(name)];
  }

  uint32_t reg_str2val(const char *name, bool *success) {
    try {
      *success = true;
      return this->at(name);
    } catch (std::runtime_error) {
      *success = false;
      return 0;
    }
  }
};

template <typename R, size_t nr_reg>
std::ostream &operator<<(std::ostream &os, const CPUStateBase<R, nr_reg> &cpu) {
  os << "PC: " << std::hex << cpu.pc << std::endl;
  for (int i = 0; i < nr_reg; i++) {
    os << "reg " << std::dec << std::setw(2) << i << ":" << std::hex
       << std::setw(10) << cpu.reg[i];
    if (i % 4 == 3) {
      os << std::endl;
    } else {
      os << " | ";
    }
  }
  return os;
}

using CPUState = CPUStateBase<word_t, REG_COUNT>;

enum { TRM_FROM_MACHINE, TRM_TO_MACHINE };

class TrmInterface {
protected:
  using memcpy_t = void (*)(paddr_t, void *, size_t, bool);
  using regcpy_t = void (*)(void *, bool);
  using exec_t = void (*)(uint64_t);
  using init_t = void (*)(int);
  std::function<void(void *, bool)> regcpy;

public:
  std::function<void(uint64_t)> exec;
  std::function<void(int)> init;
  // TODO: paddr_t can probably changed to (void *)?
  std::function<void(paddr_t, void *, size_t, bool)> memcpy;
  // Managed by callee
  void *cpu_state;

  TrmInterface() {}
  TrmInterface(memcpy_t f_memcpy, regcpy_t f_regcpy, exec_t f_exec,
               init_t f_init, void *cpu_state)
      : memcpy(f_memcpy), regcpy(f_regcpy), exec(f_exec), init(f_init),
        cpu_state(cpu_state) {}

  void fetch_state() { this->regcpy(cpu_state, TRM_FROM_MACHINE); }
  void push_state() { this->regcpy(cpu_state, TRM_TO_MACHINE); }
  virtual word_t at(std::string) const = 0;
  virtual word_t at(word_t addr) const = 0;
  virtual void print(std::ostream &os) const = 0;
};

class TrmRuntimeException : public std::exception {
private:
  const char *msg_;
  int code_;

public:
  enum { EBREAK, DIFFTEST_FAILED };
  TrmRuntimeException(int code, const char *message)
      : code_(code), msg_(message) {}

  virtual const char *what() const throw() { return msg_; }

  int error_code() const { return code_; }
};

struct RefTrmInterface : TrmInterface {
  RefTrmInterface(std::filesystem::path lib_file) {
    void *handle = dlopen(lib_file.c_str(), RTLD_LAZY);
    if (handle == nullptr) {
      throw std::runtime_error("Failed to open diff library file");
    };
    memcpy = (memcpy_t)dlsym(handle, "difftest_memcpy");
    if (handle == nullptr) {
      throw std::runtime_error("Failed to find `difftest_memcpy`");
    };
    regcpy = (regcpy_t)dlsym(handle, "difftest_regcpy");
    if (handle == nullptr) {
      throw std::runtime_error("Failed to find `difftest_regcpy`");
    };
    exec = (exec_t)dlsym(handle, "difftest_exec");
    if (handle == nullptr) {
      throw std::runtime_error("Failed to find `difftest_exec`");
    };
    init = (init_t)dlsym(handle, "difftest_init");
    if (handle == nullptr) {
      throw std::runtime_error("Failed to find `difftest_init`");
    };
    cpu_state = new CPUState{};
  }

  ~RefTrmInterface() { delete (CPUState *)cpu_state; }

  word_t at(std::string name) const override {
    return ((CPUState *)cpu_state)->at(name);
  }

  word_t at(paddr_t addr) const override {
    word_t buf;
    this->memcpy(addr, &buf, sizeof(word_t), TRM_FROM_MACHINE);
    return buf;
  }

  void print(std::ostream &os) const override { os << *(CPUState *)cpu_state; }
};

#endif