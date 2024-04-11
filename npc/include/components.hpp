
#ifndef _NPC_COMPONENTS_H_
#define _NPC_COMPONENTS_H_
#include <array>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>

template <typename T, std::size_t nr> class _RegistersBase {
  std::array<T, nr> regs;
  T pc;
  virtual T fetch_pc();
  virtual T fetch_reg(std::size_t id);

public:
  T operator[](size_t id) { return fetch_reg(id); }
  T get_pc() { return fetch_pc(); }
  void update() {
    for (int i = 0; i < regs.size(); i++) {
      regs[i] = fetch_reg(i);
    }
  }
};

template <typename T, std::size_t n> class Memory {
  std::size_t addr_to_index(std::size_t addr) {
    extern bool g_skip_memcheck;
    if (g_skip_memcheck) {
      g_skip_memcheck = false;
      return 0;
    }
    if (addr < 0x80000000 || addr > 0x8fffffff) {
      std::cerr << "ACCESS " << addr << std::endl;
      throw std::runtime_error("Invalid memory access");
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
  std::array<T, n> mem;
  Memory(std::filesystem::path filepath, bool is_binary = true) {
    if (!std::filesystem::exists(filepath))
      throw std::runtime_error("Memory file not found");
    if (is_binary) {
      std::ifstream file(filepath, std::ios::binary);
      char *pmem = reinterpret_cast<char *>(mem.data());
      file.read(pmem, mem.size() * sizeof(mem[0]));
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
    // printf("raddr: 0x%x\n", raddr);
    return mem[addr_to_index((uint32_t)raddr)];
  }
  /**
   * Always writes to the 4 bytes at the address `waddr` & ~0x3u.
   * Each bit in `wmask` represents a mask for one byte in wdata.
   * For example, wmask = 0x3 means only the lowest 2 bytes are written,
   * and the other bytes in memory remain unchanged.
   */
  void write(int waddr, T wdata, char wmask) {
    // printf("waddr: 0x%x\n", waddr);
    mem[addr_to_index((uint32_t)waddr)] = expand_bits(wmask) & wdata;
  }
  void *guest_to_host(std::size_t addr) {
    return mem.data() + addr_to_index(addr);
  }
};
#endif
