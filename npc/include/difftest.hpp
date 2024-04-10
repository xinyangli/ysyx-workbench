#ifndef _DIFFTEST_DIFFTEST_H_
#define _DIFFTEST_DIFFTEST_H_
#include <cassert>
#include <components.hpp>
#include <cstdint>
#include <cstdlib>
#include <dlfcn.h>
#include <filesystem>
#include <functional>
#include <iostream>
#include <map>
#include <memory>

using paddr_t = uint32_t;
enum { TRM_FROM_MACHINE, TRM_TO_MACHINE };

struct TrmInterface {
  using memcpy_t = void (*)(paddr_t, void *, size_t, bool);
  using regcpy_t = void (*)(void *, bool);
  using exec_t = void (*)(uint64_t);
  using init_t = void (*)(int);
  std::function<void(paddr_t, void *, size_t, bool)> memcpy;
  std::function<void(void *, bool)> regcpy;
  std::function<void(uint64_t)> exec;
  std::function<void(int)> init;
};

template <typename S> class Difftest {
  const TrmInterface &ref;
  std::unique_ptr<S> ref_state;
  const TrmInterface &dut;
  std::unique_ptr<S> dut_state;
  void *mem;
  size_t memsize;

public:
  Difftest(const TrmInterface &dut, const TrmInterface &ref, void *mem,
           size_t n, std::unique_ptr<S> ref_state = nullptr,
           std::unique_ptr<S> dut_state = nullptr)
      : ref(ref), dut(dut), ref_state(std::move(ref_state)),
        dut_state(std::move(dut_state)), mem(mem), memsize(n) {
    if (ref_state == nullptr)
      this->ref_state = std::make_unique<S>();
    if (dut_state == nullptr)
      this->dut_state = std::make_unique<S>();
  };

  void init(int n) {
    ref.init(n);
    dut.init(n);
    paddr_t reset_vector = 0x80000000;
    ref.memcpy(reset_vector, mem, memsize, TRM_TO_MACHINE);
    dut.memcpy(reset_vector, mem, memsize, TRM_TO_MACHINE);
    fetch_state();
  }

  void fetch_state(void *pref = nullptr, void *pdut = nullptr) {
    ref.regcpy(pref ? pref : ref_state.get(), TRM_FROM_MACHINE);
    dut.regcpy(pdut ? pdut : dut_state.get(), TRM_FROM_MACHINE);
  }

  void push_state(void *pref = nullptr, void *pdut = nullptr) {
    ref.regcpy(pref ? pref : ref_state.get(), TRM_TO_MACHINE);
    dut.regcpy(pdut ? pdut : dut_state.get(), TRM_TO_MACHINE);
  }

  void push_mem(paddr_t paddr, void * p, size_t n) {
    ref.memcpy(paddr, p, n, TRM_TO_MACHINE);
    dut.memcpy(paddr, p, n, TRM_TO_MACHINE);
  }

  bool step(uint64_t n) {
    std::cout << "REF state:\n"
       << *ref_state << "DUT state:\n"
       << *dut_state << std::endl;
    while(n--) {
      ref.exec(1);
      dut.exec(1);
      fetch_state();
      // uint32_t inst = 0;
      // ref.memcpy(&inst, ref_state->get_pc(), 4, TRM_FROM_MACHINE);
      if(*ref_state == *dut_state) return false;
    }
    return true;
  }

  friend std::ostream &operator<<(std::ostream &os, const Difftest<S> &d) {
    os << "REF state:\n"
       << *d.ref_state << "DUT state:\n"
       << *d.dut_state << std::endl;
    return os;
  }
};

template <typename S> struct DifftestTrmInterface : TrmInterface{
  Difftest<S> diff;
  S cpu_state;

  DifftestTrmInterface<S>() {}
  DifftestTrmInterface<S>(const TrmInterface &dut, const TrmInterface &ref,
                          void *mem, size_t n, S cpu_state,
                          std::unique_ptr<S> ref_state = nullptr,
                          std::unique_ptr<S> dut_state = nullptr)
      : diff(Difftest<S>{dut, ref, mem, n, std::move(ref_state),
                         std::move(dut_state)}),
        cpu_state(cpu_state) {
    init = [this](int n) { diff.init(n); };
    exec = [this](uint64_t n) { diff.step(n); };

    // NOTE: Different from normal Trm, we copy 2 * sizeof(CPUState) to/from p,
    // which represents ref_state and dut state
    regcpy = [this](void *p, bool direction) {
          if (direction == TRM_FROM_MACHINE) {
            diff.push_state(p, (uint8_t *)p + sizeof(S));
          } else if (direction == TRM_TO_MACHINE) {
            diff.fetch_state(p, (uint8_t *)p + sizeof(S));
          }
        };

    memcpy = [this](paddr_t paddr, void * p, size_t n, bool direction) {
          if (direction == TRM_FROM_MACHINE) {
            diff.push_mem(paddr, p, n);
          } else if (direction == TRM_TO_MACHINE) {
            throw std::runtime_error("Not implemented");
          }
        };
  }
  
};

template <typename R, size_t nr_reg> struct CPUStateBase {
  R reg[nr_reg] = {0};
  paddr_t pc = 0x80000000;
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
  R at(std::string name) { return reg[regs_by_name.at(name)]; }

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

#endif