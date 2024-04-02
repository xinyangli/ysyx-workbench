#include "VFlow___024root.h"
#include "tracer.h"
#include <array>
#include <cstddef>
#include <filesystem>
#include <vpi_user.h>
#include <VFlow.h>
#include <cstdlib>
#include <vector>
#include <memory>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <verilated_vpi.h>
#include <string>
#define MAX_SIM_TIME 100
#define VERILATOR_TRACE

std::vector<vpiHandle> regsHandle;
int regs[32];

template <class T>
class Tracer {
#ifdef VERILATOR_TRACE
  std::shared_ptr<T> top;
  std::unique_ptr<VerilatedVcdC> m_trace;
  uint64_t time = 0;
#endif
  public:
    Tracer(std::shared_ptr<T> top,  std::filesystem::path wavefile) {
#ifdef VERILATOR_TRACE
      top = top;
      Verilated::traceEverOn(true);
      m_trace = std::make_unique<VerilatedVcdC>();
      top->trace(m_trace.get(), 5);
      m_trace->open(wavefile.c_str());
#endif
    }
    ~Tracer() {
#ifdef VERILATOR_TRACE
      m_trace->close();
#endif
    }
    
    /**
     * Dump signals to waveform file. Must be called once after every top->eval() call.
     */
    void update() {
#ifdef VERILATOR_TRACE
      m_trace->dump(time++);
#endif
    }
};
template <typename T, std::size_t nr>
class _RegistersBase {
  std::array<T, nr> regs;
  virtual T fetch_reg(size_t id);
  public:
    void update() {
      for(int i = 0; i < regs.size(); i++) {
        regs[i] = fetch_reg(i);
      }
    }
    void print_regs() {
      for(int i = 0; i < regs.size(); i++) {
        printf("%d: %d\t", i, regs[i]);
        if(i % 8 == 7) putchar('\n');
      }
      putchar('\n');
    }
};

template <typename T, std::size_t nr>
class _RegistersVPI : public _RegistersBase<T, nr> {
  std::array<vpiHandle, nr> reg_handles;
  T fetch_reg(size_t id) {
    s_vpi_value v;
    v.format = vpiIntVal;
    vpi_get_value(reg_handles[id], &v);
    return v.value.integer;
  }
  public:
    _RegistersVPI<T, nr>(const std::string regs_prefix) {
      for(int i = 0; i < nr; i++) {
        std::string regname = regs_prefix + std::to_string(i);
        vpiHandle vh = vpi_handle_by_name((PLI_BYTE8 *)regname.c_str(), NULL);
        reg_handles[i] = vh;
      }
    }
};

template <typename T, std::size_t n>
class Memory {
  std::array<T, n> mem;
  size_t addr_to_index(size_t addr) {
    // Linear mapping
    return addr - 0x80000000;
  }
  public:
    const T& operator[](size_t addr) {
      return mem[addr_to_index(index)];
    }
};

typedef _RegistersVPI<uint32_t, 32> Registers;
static int sim_time = 0;

int main(int argc, char **argv, char **env) {
  int sim_time = 0;
  int posedge_cnt = 0;
  Verilated::commandArgs(argc, argv);

  auto top = std::make_shared<VFlow>();
  auto top_tracer = std::make_unique<Tracer<VFlow>>(top, "waveform.vcd");

  Registers regs("TOP.Flow.reg_0.regFile_");

  top->reset = 0;
  top->eval();
  for (sim_time = 10; sim_time < MAX_SIM_TIME; sim_time++) {
    top->clock = !top->clock;
    if(top->clock == 1) {
      // Posedge
      ++posedge_cnt;
      regs.update();
      regs.print_regs();
    }
    top->eval();
  }
  exit(EXIT_SUCCESS);
}
