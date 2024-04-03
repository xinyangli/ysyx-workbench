
#ifndef _NPC_COMPONENTS_H_
#define _NPC_COMPONENTS_H_
#include <array>
#include <filesystem>
#include <fstream>
#include <verilated_vpi.h>

template <typename T, std::size_t nr> class _RegistersBase {
  std::array<T, nr> regs;
  virtual T fetch_reg(std::size_t id);

public:
  void update() {
    for (int i = 0; i < regs.size(); i++) {
      regs[i] = fetch_reg(i);
    }
  }
  void print_regs() {
    for (int i = 0; i < regs.size(); i++) {
      printf("%d: %d\t", i, regs[i]);
      if (i % 8 == 7)
        putchar('\n');
    }
    putchar('\n');
  }
};

template <typename T, std::size_t nr>
class _RegistersVPI : public _RegistersBase<T, nr> {
  std::array<vpiHandle, nr> reg_handles;
  T fetch_reg(std::size_t id) {
    s_vpi_value v;
    v.format = vpiIntVal;
    vpi_get_value(reg_handles[id], &v);
    return v.value.integer;
  }

public:
  _RegistersVPI<T, nr>(const std::string regs_prefix) {
    for (int i = 0; i < nr; i++) {
      std::string regname = regs_prefix + std::to_string(i);
      vpiHandle vh = vpi_handle_by_name((PLI_BYTE8 *)regname.c_str(), NULL);
      reg_handles[i] = vh;
    }
  }
};

template <typename T, std::size_t n> class Memory {
  std::array<T, n> mem;
  std::size_t addr_to_index(std::size_t addr) {
    if (addr < 0x80000000) {
      return 0;
    }
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
  Memory(std::filesystem::path filepath, bool is_binary = true) {
    assert(std::filesystem::exists(filepath));
    if (is_binary) {
      std::ifstream file(filepath, std::ios::binary);
      char *pmem = reinterpret_cast<char *>(mem.data());
      file.read(pmem, mem.size() / sizeof(mem[0]));
    } else {
      std::string line;
      std::ifstream file(filepath);
      int i = 0;
      while (std::getline(file, line)) {
        mem[i++] = std::stoul(line, 0, 16);
      }
    }
  }
  const T &operator[](std::size_t addr) { return this->read(addr); }
  /**
   * Always reads and returns 4 bytes from the address raddr & ~0x3u.
   */
  T read(int raddr) {
    printf("raddr: 0x%x\n", raddr);
    return mem[addr_to_index((uint32_t)raddr)];
  }
  /**
   * Always writes to the 4 bytes at the address `waddr` & ~0x3u.
   * Each bit in `wmask` represents a mask for one byte in wdata.
   * For example, wmask = 0x3 means only the lowest 2 bytes are written,
   * and the other bytes in memory remain unchanged.
   */
  void write(int waddr, T wdata, char wmask) {
    printf("waddr: 0x%x\n", waddr);
    mem[addr_to_index((uint32_t)waddr)] = expand_bits(wmask) & wdata;
  }
};
#endif
