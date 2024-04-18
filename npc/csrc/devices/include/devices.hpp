#include <cstddef>
#include <cstdint>
#include <cassert>
#include <initializer_list>
#include <iostream>
#include <array>
#include <iterator>
#include <map>
#include <memory>
#include <unordered_map>
#include <utility>

namespace Devices {
class Device {
  public:
    uint64_t addr;
    size_t len;
    Device(uint64_t addr, size_t len) : addr(addr), len(len) {}
    virtual ~Device() {};
    virtual void io_handler(uint32_t offset, size_t len, bool is_write) = 0;
    virtual void transfer(uint8_t *src, size_t len, bool is_write) = 0;
};

class Serial : public Device {
  std::array<uint8_t, 1> buf;
  public:
    Serial(uint64_t addr, size_t len);
    ~Serial() override { };
    void io_handler(uint32_t offset, size_t len, bool is_write) override;
    void transfer(uint8_t *src, size_t len, bool is_write) override;
};

class DeviceMap {
  std::map<uint64_t, Device *> addr_to_device;
  public:
    DeviceMap(std::initializer_list<Device *> devices) {
      for (auto device : devices) {
        addr_to_device.insert(std::make_pair(device->addr, device));
      }
    }
    void handle(uint64_t addr, uint8_t *data, size_t len, bool is_write) {
      auto it = addr_to_device.upper_bound(addr);
      if(it == addr_to_device.begin() || --it->second->addr + it->second->len <= addr) {
        std::cerr << "Use of a unintialized device at memory addr: 0x" << std::hex << addr << std::dec << std::endl;
        return;
      }
      auto &device = it->second;
      uint32_t offset = addr - device->addr;
      if(is_write) {
        device->transfer(data, len, is_write);
        device->io_handler(offset, len, is_write);
      } else {
        device->io_handler(offset, len, is_write);
        device->transfer(data, len, is_write);
      }
    }
    // ~DeviceMap() {
    //   for (auto &device : this->addr_to_device) {
    //     delete device.second;
    //   }
    // }
};
}