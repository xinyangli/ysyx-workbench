
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
  virtual T fetch_pc() const;
  virtual T fetch_reg(std::size_t id) const;

public:
  T operator[](size_t id) const { return fetch_reg(id); }
  T get_pc() const { return fetch_pc(); }
  void update() {
    for (int i = 0; i < regs.size(); i++) {
      regs[i] = fetch_reg(i);
    }
  }
};

// class MemoryFile {
//   std::filesystem::path filepath;
//   public:

// };

template <std::size_t n> class Memory {
  paddr_t pmem_start, pmem_end;

public:
  std::array<uint8_t, n> mem;
  // TODO: Read memory file before init and use memcpy to initialize memory.
  Memory(std::filesystem::path filepath, bool is_binary, paddr_t pmem_start,
         paddr_t pmem_end)
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
    if (is_write) {
      // memcpy(guest_to_host(addr), data, len);
      size_t offset = (addr - pmem_start);
      std::copy(data, data + len, &mem[offset]);
    } else {
      // memcpy(data, guest_to_host(addr), len);
      size_t offset = (addr - pmem_start);
      std::copy(&mem[offset], &mem[offset + len], data);
    }
  }
  bool in_pmem(paddr_t addr) const {
    return addr >= pmem_start && addr <= pmem_end;
  }
};

template <typename Mem, typename DevMap> class MemoryMap {
  std::unique_ptr<Mem> ram;
  std::unique_ptr<DevMap> devices;
  const std::vector<std::array<uint64_t, 2>> &trace_ranges;

public:
  MemoryMap(std::unique_ptr<Mem> &&ram, std::unique_ptr<DevMap> &&devices,
            const std::vector<std::array<uint64_t, 2>> &trace_ranges)
      : ram(std::move(ram)), devices(std::move(devices)),
        trace_ranges(trace_ranges) {}
  void write(paddr_t waddr, word_t wdata, char wmask) {
    // printf("waddr: 0x%x\n", waddr);
    size_t len = (wmask & 1) + ((wmask & 2) >> 1) + ((wmask & 4) >> 2) +
                 ((wmask & 8) >> 3);
    if (ram->in_pmem(waddr)) {
      ram->transfer(waddr, (uint8_t *)&wdata, len, true);
    } else if (devices->handle(waddr, (uint8_t *)&wdata, len, true)) {
    }
  }
  word_t read(paddr_t raddr) const {
    word_t res = 0;
    // printf("raddr: 0x%x, in_pmem: %d\n", raddr, ram->in_pmem(raddr));
    if (ram->in_pmem(raddr)) {
      ram->transfer(raddr, (uint8_t *)&res, 4, false);
    } else if (devices->handle(raddr, (uint8_t *)&res, 4, false)) {
    }
    return res;
  }
  int copy_to(paddr_t addr, uint8_t *buf, size_t len) const {
    if (ram->in_pmem(addr)) {
      ram->transfer(addr, buf, len, false);
      return 0;
    } else {
      return EINVAL;
    }
  }
  int copy_from(paddr_t addr, uint8_t *buf, size_t len) {
    if (ram->in_pmem(addr)) {
      ram->transfer(addr, buf, len, true);
      return 0;
    } else {
      return EINVAL;
    }
  }
  void *get_pmem() { return ram->mem.data(); }
  void trace(paddr_t addr, bool is_read, word_t pc = 0, word_t value = 0) {
    for (auto &r : trace_ranges) {
      if (r[0] <= addr && r[1] >= addr) {
        std::stringstream os;
        if (pc != 0)
          os << "0x" << std::hex << pc << " ";
        if (is_read)
          os << "[R] "
             << "0x" << addr << ": 0x" << this->read(addr);
        else
          os << "[W] " << value << " -> "
             << "0x" << addr;
        os << std::dec << std::endl;
        std::cout << os.rdbuf();
        break;
      }
    }
  }
};
#endif
