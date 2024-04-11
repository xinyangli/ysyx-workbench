#ifndef _DIFFTEST_DIFFTEST_H_
#define _DIFFTEST_DIFFTEST_H_
#include "disasm.hpp"
#include "types.h"
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
#include <ostream>
#include <stdexcept>
#include <trm_interface.hpp>
Disassembler d{"riscv32-linux-pc-gnu"};

using paddr_t = uint32_t;
struct DifftestTrmInterface : public TrmInterface {
  TrmInterface &dut;
  TrmInterface &ref;

  DifftestTrmInterface(TrmInterface &dut, TrmInterface &ref, void *mem,
                       size_t mem_size)
      : dut(dut), ref(ref) {
    init = [this, mem, mem_size](int n) {
      this->ref.init(n);
      this->dut.init(n);
      paddr_t reset_vector = 0x80000000;
      this->ref.memcpy(reset_vector, mem, mem_size, TRM_TO_MACHINE);
      this->dut.memcpy(reset_vector, mem, mem_size, TRM_TO_MACHINE);
      fetch_state();
    };
    exec = [this](uint64_t n) {
      bool enable_disasm = true;
      if (n > 30) {
        enable_disasm = false;
      }

      while (n--) {
        word_t pc = this->ref.at("pc");
        word_t inst = this->ref.at(pc);
        if (enable_disasm)
          std::cout << d.disassemble(pc, (uint8_t *)&inst, WORD_BYTES)
                    << std::endl;
        if (inst == 1048691) {
          // ebreak
          throw TrmRuntimeException(TrmRuntimeException::EBREAK, "ebreak");
        }
        this->ref.exec(1);
        this->dut.exec(1);
        this->ref.fetch_state();
        this->dut.fetch_state();
        if (*(CPUState *)this->ref.cpu_state !=
            *(CPUState *)this->dut.cpu_state) {
          throw TrmRuntimeException(TrmRuntimeException::DIFFTEST_FAILED,
                                    "Difftest failed");
        }
      }
    };

    // NOTE: Different from normal Trm, we copy 2 * sizeof(CPUState) to/from p,
    // which represents ref_state and dut state
    regcpy = [this](void *p, bool direction) {
      // this->ref.regcpy(p, direction);
      // this->dut.regcpy(p, direction);
    };

    memcpy = [this](paddr_t paddr, void *p, size_t n, bool direction) {
      this->dut.memcpy(paddr, p, n, direction);
      this->ref.memcpy(paddr, (uint8_t *)p + n, n, direction);
    };
  }

  word_t at(std::string name) const override {
    if (name.empty()) {
      throw std::runtime_error("Empty register name");
    } else if (name[0] == 'r') {
      std::cout << name.substr(1) << std::endl;
      this->ref.at(name.substr(1));
    } else if (name[0] == 'd') {
      this->dut.at(name.substr(1));
    } else {
      throw std::runtime_error("Register name provided to difftest interface "
                               "must start with r or d.");
    }
    return 0;
  }

  word_t at(paddr_t addr) const override {
    std::cout << ref.at(addr) << "\t" << dut.at(addr) << std::endl;
    return dut.at(addr);
  }

  void print(std::ostream &os) const override {
    os << "REF state:\n"
       << *(CPUState *)ref.cpu_state << "DUT state:\n"
       << *(CPUState *)dut.cpu_state << std::endl;
  }
};
#endif