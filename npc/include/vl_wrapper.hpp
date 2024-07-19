#ifndef _NPC_TRACER_H_
#define _NPC_TRACER_H_
#include "components.hpp"
#include "types.h"
#include "verilated.h"
#include <filesystem>
#include <gdbstub.h>
#include <sys/types.h>
#include <verilated_vcd_c.h>

template <class T> class Tracer {
  std::shared_ptr<T> top;
  std::unique_ptr<VerilatedVcdC> m_trace;
  uint64_t cycle = 0;

public:
  Tracer(T *top, std::filesystem::path wavefile) {
    top = top;
    Verilated::traceEverOn(true);
    m_trace = std::make_unique<VerilatedVcdC>();
    top->trace(m_trace.get(), 5);
    m_trace->open(wavefile.c_str());
  }
  ~Tracer() { m_trace->close(); }

  /**
   * Dump signals to waveform file. Must be called once after every top->eval()
   * call.
   */
  void update() { m_trace->dump(cycle++); }
};

template <typename T, typename R> class VlModuleInterfaceCommon : public T {
  uint64_t sim_time = 0;
  uint64_t posedge_cnt = 0;
  std::unique_ptr<Tracer<T>> tracer;

public:
  const R *registers;
  VlModuleInterfaceCommon<T, R>() {
    tracer = nullptr;
    registers = nullptr;
  }

  void setup(std::filesystem::path wavefile, const R *r) {
    if (!wavefile.empty())
      tracer = std::make_unique<Tracer<T>>(this, wavefile);
    registers = r;
  }

  void eval(void) {
    if (this->is_posedge()) {
      posedge_cnt++;
    }
    T::clock = !T::clock;
    sim_time++;
    T::eval();
    if (tracer)
      tracer->update();
  }

  const Breakpoint *stepi(const std::vector<Breakpoint> &breakpoints) {
    this->eval();
    size_t pc = registers->get_pc();
    for (const auto &bp : breakpoints) {
      if (pc == bp.addr) {
          return &bp;
      }
    }
    return nullptr;
  }

  const Breakpoint *cont(const std::vector<Breakpoint> &breakpoints) {
    const Breakpoint *res = nullptr;
    do {
        res = stepi(breakpoints);
    } while (res == nullptr);
    return res;
  }

  void reset_eval(int n) {
    extern bool g_skip_memcheck;
    g_skip_memcheck = true;
    this->reset = 1;
    do {
      this->eval();
    } while(--n);
    this->reset = 0;
    g_skip_memcheck = false;
  }
  bool is_posedge() {
    // Will be posedge when eval is called
    return T::clock == 0;
  }
};

#endif
