#ifndef _DIFFTEST_DIFFTEST_H_
#define _DIFFTEST_DIFFTEST_H_
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <dlfcn.h>
#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>

using paddr_t = uint32_t;
enum { DIFFTEST_FROM_REF, DIFFTEST_TO_REF };

struct DifftestInterface {
  using memcpy_t = void (*)(paddr_t, void *, size_t, bool);
  using regcpy_t = void (*)(void *, bool);
  using exec_t = void (*)(uint64_t);
  using init_t = void (*)(int);
  std::function<void(paddr_t, void *, size_t, bool)> memcpy;
  std::function<void(void *, bool)> regcpy;
  std::function<void(uint64_t)> exec;
  std::function<void(int)> init;

  DifftestInterface(memcpy_t memcpy, regcpy_t regcpy, exec_t exec, init_t init)
      : memcpy(memcpy), regcpy(regcpy), exec(exec), init(init){};

  // using fs = std::filesystem::path;
  DifftestInterface(std::filesystem::path lib_file) {
    void *handle = dlopen(lib_file.c_str(), RTLD_LAZY);
    assert(handle != nullptr);
    memcpy = (memcpy_t)dlsym(handle, "difftest_memcpy");
    assert(memcpy);
    regcpy = (regcpy_t)dlsym(handle, "difftest_regcpy");
    assert(regcpy);
    exec = (exec_t)dlsym(handle, "difftest_exec");
    assert(exec);
    init = (init_t)dlsym(handle, "difftest_init");
    assert(init);
  }
};

template <typename S> class Difftest {
  const DifftestInterface &ref;
  std::unique_ptr<S> ref_state;
  const DifftestInterface &dut;
  std::unique_ptr<S> dut_state;

public:
  Difftest(const DifftestInterface &dut, const DifftestInterface &ref,
           void *mem, size_t n, std::unique_ptr<S> ref_state = nullptr,
           std::unique_ptr<S> dut_state = nullptr)
      : ref(ref), dut(dut), ref_state(std::move(ref_state)),
        dut_state(std::move(dut_state)) {
    if (ref_state == nullptr)
      this->ref_state = std::make_unique<S>();
    if (dut_state == nullptr)
      this->dut_state = std::make_unique<S>();
    ref.init(0);
    dut.init(0);
    fetch_state();
    paddr_t reset_vector = 0x80000000;
    ref.memcpy(reset_vector, mem, n, DIFFTEST_TO_REF);
    dut.memcpy(reset_vector, mem, n, DIFFTEST_TO_REF);
  };

  void fetch_state() {
    ref.regcpy(ref_state.get(), DIFFTEST_FROM_REF);
    dut.regcpy(dut_state.get(), DIFFTEST_FROM_REF);
  }

  void step(uint64_t n) {
    ref.exec(n);
    dut.exec(n);
    fetch_state();
    if (*ref_state != *dut_state) {
      std::cout << *this;
      exit(EXIT_FAILURE);
    }
  }

  friend std::ostream &operator<<(std::ostream &os, const Difftest<S> &d) {
    os << "REF state:\n"
       << *d.ref_state << "DUT state:\n"
       << *d.dut_state << std::endl;
    return os;
  }
};

template <typename R, size_t nr_reg> struct CPUStateBase {
  R reg[nr_reg] = {0};
  paddr_t pc = 0x80000000;
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

#endif