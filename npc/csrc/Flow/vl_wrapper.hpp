#ifndef _NPC_TRACER_H_
#define _NPC_TRACER_H_
#include "components.hpp"
#include <filesystem>
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

template <typename T> class VlModuleInterfaceCommon : public T {
  uint64_t sim_time = 0;
  uint64_t posedge_cnt = 0;
  std::unique_ptr<Tracer<T>> tracer;

public:
  VlModuleInterfaceCommon<T>(bool do_trace,
                             std::filesystem::path wavefile = "waveform.vcd") {
    if (do_trace)
      tracer = std::make_unique<Tracer<T>>(this, wavefile);
  }
  void eval() {
    if (this->is_posedge()) {
      posedge_cnt++;
    }
    T::clock = !T::clock;
    sim_time++;
    T::eval();
    if (tracer)
      tracer->update();
  }
  void eval(int n) {
    for (int i = 0; i < n; i++) {
      this->eval();
    }
  }
  void reset_eval(int n) {
    extern bool g_skip_memcheck;
    g_skip_memcheck = true;
    this->reset = 1;
    this->eval(n);
    this->reset = 0;
    g_skip_memcheck = false;
  }
  bool is_posedge() {
    // Will be posedge when eval is called
    return T::clock == 0;
  }
};

#endif
