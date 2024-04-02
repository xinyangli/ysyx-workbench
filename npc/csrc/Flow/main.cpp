#include <array>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <sys/types.h>
#include <vpi_user.h>
#include <VFlow.h>
#include <cstdlib>
#include <memory>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include <verilated_vpi.h>
#include <string>
#define MAX_SIM_TIME 100
#define VERILATOR_TRACE

template <class T>
class Tracer {
#ifdef VERILATOR_TRACE
  std::shared_ptr<T> top;
  std::unique_ptr<VerilatedVcdC> m_trace;
  uint64_t cycle = 0;
#endif
  public:
    Tracer(T *top, std::filesystem::path wavefile) {
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
      m_trace->dump(cycle++);
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

typedef _RegistersVPI<uint32_t, 32> Registers;
template <typename T, std::size_t n>
class Memory {
  std::array<T, n> mem;
  size_t addr_to_index(size_t addr) {
    // Linear mapping
    return (addr >> 2) - 0x20000000;
  }
  uint32_t expand_bits(uint8_t bits) {
      uint32_t x = bits;
      x = (x | (x << 7) | (x << 14) | (x << 21)) & 0x01010101;
      x = x * 0xFF;
      // printf("expand: %hhx->%x\n", bits, x);
      return x;
  }
  public:
    const T& operator[](size_t addr) {
      return this->read(addr);
    }
    /**
     * Always reads and returns 4 bytes from the address raddr & ~0x3u.
     */
    T read(int raddr) {
      return mem[addr_to_index(raddr)];
    }
    /**
     * Always writes to the 4 bytes at the address `waddr` & ~0x3u.
     * Each bit in `wmask` represents a mask for one byte in wdata.
     * For example, wmask = 0x3 means only the lowest 2 bytes are written,
     * and the other bytes in memory remain unchanged.
     */
    void write(int waddr, T wdata, char wmask) {
      mem[addr_to_index((uint32_t)waddr)] = expand_bits(wmask) & wdata;
    }
};

extern "C" {
  void *pmem_get() {
    static auto pmem = new Memory<int, 128 * 1024>;
    return pmem;
  }
  int pmem_read(int raddr) {
    void *pmem = pmem_get();
    auto mem = static_cast<Memory<int, 128 * 1024>*>(pmem);
    return mem->read(raddr);
  }

  void pmem_write(int waddr, int wdata, char wmask) {
    void *pmem = pmem_get();
    auto mem = static_cast<Memory<int, 128 * 1024>*>(pmem);
    return mem->write((size_t)waddr, wdata, wmask);
  }
} 

template <typename T>
class VlModuleInterfaceCommon : public T {
  uint64_t sim_time = 0;
  uint64_t posedge_cnt = 0;
  std::unique_ptr<Tracer<T>> tracer;
  public:
    VlModuleInterfaceCommon<T>(bool do_trace, std::filesystem::path wavefile = "waveform.vcd") {
      if(do_trace) tracer = std::make_unique<Tracer<T>>(this, wavefile);
    } 
    void eval() {
      if(this->is_posedge()) {
        posedge_cnt++;
      }
      T::clock = !T::clock;
      sim_time++;
      T::eval();
      if(tracer) tracer->update();
    }
    void eval(int n) {
      for(int i = 0; i < n; i++) {
        this->eval();
      }
    }
    void reset_eval(int n) {
      this->reset = 1;
      this->eval(n);
      this->reset = 0;
    }
    bool is_posedge() {
      // Will be posedge when eval is called
      return T::clock == 0;
    }
};

typedef VlModuleInterfaceCommon<VFlow> VlModule;

int main(int argc, char **argv, char **env) {
  Verilated::commandArgs(argc, argv);

  auto top = std::make_shared<VlModule>(false, "waveform.vcd");

  Registers regs("TOP.Flow.reg_0.regFile_");

  top->reset_eval(10);
  for (int i = 0; i < MAX_SIM_TIME; i++) {
    if(top->is_posedge()) {
      // Posedge
      regs.update();
      regs.print_regs();
    }
    top->eval();
  }
  return 0;
}
