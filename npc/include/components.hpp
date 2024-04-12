
#ifndef _NPC_COMPONENTS_H_
#define _NPC_COMPONENTS_H_
#include "types.h"
#include <array>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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
  uint32_t expand_bits(uint8_t bits) {
    uint32_t x = bits;
    x = (x | (x << 7) | (x << 14) | (x << 21)) & 0x01010101;
    x = x * 0xFF;
    // printf("expand: %hhx->%x\n", bits, x);
    return x;
  }

public:
  std::array<T, n> mem;
  std::vector<std::array<uint64_t, 2>> trace_ranges;
  Memory(std::filesystem::path filepath, bool is_binary,
         std::vector<std::array<uint64_t, 2>> &&trace_ranges)
      : trace_ranges(std::move(trace_ranges)) {
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
    return *(word_t *)guest_to_host(raddr);
  }
  /**
   * Always writes to the 4 bytes at the address `waddr` & ~0x3u.
   * Each bit in `wmask` represents a mask for one byte in wdata.
   * For example, wmask = 0x3 means only the lowest 2 bytes are written,
   * and the other bytes in memory remain unchanged.
   */
  void write(int waddr, T wdata, char wmask) {
    // printf("waddr: 0x%x\n", waddr);
    uint8_t *p_data = &wdata;
    while(wmask & 0x1) {
      memcpy(guest_to_host(waddr), p_data + 1, 1);
      waddr++;
      wmask >>= 1;
    }
  }
  void *guest_to_host(std::size_t addr) {
    extern bool g_skip_memcheck;
    if (g_skip_memcheck) {
      return 0;
    }
    if (addr < 0x80000000 || addr > 0x87ffffff) {
      std::cerr << std::hex << "ACCESS " << addr << std::dec << std::endl;
      throw std::runtime_error("Invalid memory access");
    }
    // Linear mapping
    return (uint8_t*)(mem.data() + (addr >> 2) - 0x20000000) + (addr & 0x3);
  }
  void trace(paddr_t addr, bool is_read, word_t pc = 0, word_t value = 0) {
    for (auto &r : trace_ranges) {
      if (r[0] <= addr && r[1] >= addr) {
        std::stringstream os;
        os << std::hex;
        if (pc != 0)
          os << "0x" << pc << " ";
        if (is_read)
          os << "[R] ";
        else
          os << "[W] " << value << " -> ";
        os << "0x" << addr << std::dec << std::endl;
        std::cout << os.rdbuf();
        break;
      }
    }
  }
};
#endif
