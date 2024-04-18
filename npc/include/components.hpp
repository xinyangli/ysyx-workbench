
#ifndef _NPC_COMPONENTS_H_
#define _NPC_COMPONENTS_H_
#include "types.h"
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
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

template <std::size_t n> class Memory {
  static word_t expand_bits(uint8_t bits) {
    word_t x = bits;
    x = (x | (x << 7) | (x << 14) | (x << 21)) & 0x01010101;
    x = x * 0xFF;
    // printf("expand: %hhx->%x\n", bits, x);
    return x;
  }
  paddr_t pmem_start, pmem_end;

public:
  std::array<word_t, n> mem;
  Memory(std::filesystem::path filepath, bool is_binary, paddr_t pmem_start, paddr_t pmem_end)
      : pmem_start(pmem_start), pmem_end(pmem_end) {
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
  const word_t &operator[](std::size_t addr) { return this->read(addr); }
  void transfer(paddr_t addr, uint8_t data[], size_t len, bool is_write) {
    if(!is_write) {
      // memcpy(data, guest_to_host(addr), len);
      size_t offset = (addr - pmem_start);
      std::copy(data, data + len, &mem[offset]);
    } else {
      // memcpy(guest_to_host(addr), data, len);
      size_t offset = (addr - pmem_start);
      std::copy(&mem[offset], &mem[offset + len], data);
    }
  }
  // void *guest_to_host(std::size_t addr) {
  //   extern bool g_skip_memcheck;
  //   if (g_skip_memcheck) {
  //     return mem.data();
  //   }
  //   if (!in_pmem(addr)) {
  //     std::cerr << std::hex << "ACCESS " << addr << std::dec << std::endl;
  //     throw std::runtime_error("Invalid memory access");
  //   }
  //   // Linear mapping
  //   size_t offset = (addr - pmem_start);
  //   return (uint8_t *)mem.data() + offset;
  // }
  bool in_pmem(paddr_t addr) const {
    return addr >= pmem_start && addr <= pmem_end;
  }
};

template <typename Mem, typename DevMap>
class MemoryMap {
  std::unique_ptr <Mem> ram;
  std::unique_ptr <DevMap> devices;
  const std::vector<std::array<uint64_t, 2>> &trace_ranges;
  public:
    MemoryMap(std::unique_ptr<Mem> &&ram, std::unique_ptr<DevMap> &&devices, const std::vector<std::array<uint64_t, 2>> &trace_ranges)
        : ram(std::move(ram)), devices(std::move(devices)), trace_ranges(trace_ranges) {}
    void write(paddr_t waddr, word_t wdata, char wmask) {
      // printf("waddr: 0x%x\n", waddr);
      size_t len = (wmask & 1) + (wmask & 2) + (wmask & 4) + (wmask & 8);
      if (ram->in_pmem(waddr)) { ram->transfer(waddr, (uint8_t *)&wdata, len, true);}
      else if(devices->handle(waddr, (uint8_t *)&wdata, len, true)) {}
    }
    word_t read(paddr_t raddr) {
      word_t res = 0;
      if (ram->in_pmem(raddr)) { ram->transfer(raddr, (uint8_t *)&res, 4, true);}
      else if( devices->handle(raddr, (uint8_t *)&res, 4, true)) {}
      return res;
    }
    void copy_to(paddr_t addr, uint8_t *buf, size_t len) const {
      if (ram->in_pmem(addr)) { ram->transfer(addr, buf, len, false);}
      else { std::cerr << "Not in pmem" << std::endl; }
    }
    void copy_from(paddr_t addr, const uint8_t *buf, size_t len) {
      if (ram->in_pmem(addr)) { ram->transfer(addr, buf, len, true);}
      else { std::cerr << "Not in pmem"; }
    }
    void *get_pmem() {
      return ram->mem.data();
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
