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
#include <cstring>
#include <utility>

namespace Devices {
class Device {
  public:
    uint8_t *p_buf;
    uint64_t addr;
    size_t len;
    Device(uint64_t addr, size_t len, uint8_t buf[]) : addr(addr), len(len), p_buf(buf) {}
    virtual ~Device() {};
    virtual void io_handler(uint32_t offset, size_t len, bool is_write) = 0;
    void transfer(uint8_t *src, size_t len, bool is_write) {
      if (is_write) {
        memmove(p_buf, src, len);
      } else {
        memmove(src, p_buf, len);
      }
    };
};

class Serial : public Device {
  uint8_t buf[1];
  public:
    Serial(uint64_t addr, size_t len);
    ~Serial() override { };
    void io_handler(uint32_t offset, size_t len, bool is_write) override;
    // void transfer(uint8_t *src, size_t len, bool is_write) override;
};

class RTC : public Device {
  uint8_t buf[8];
  uint64_t boot_time;
  uint64_t get_time_internal();
  uint64_t get_time();
  public:
    RTC(uint64_t addr, size_t len);
    ~RTC() override { };
    void io_handler(uint32_t offset, size_t len, bool is_write) override;
    // void transfer(uint8_t *src, size_t len, bool is_write) override;
};

class DeviceMap {
  std::map<uint64_t, Device *> addr_to_device;
  public:
    DeviceMap(std::initializer_list<Device *> devices) {
      for (auto device : devices) {
        addr_to_device.insert(std::make_pair(device->addr, device));
      }
    }
    bool handle(uint64_t addr, uint8_t *data, size_t len, bool is_write) {
      auto it = addr_to_device.upper_bound(addr);
      if(it == addr_to_device.begin() || (--it)->second->addr + it->second->len <= addr) {
        std::cerr << "Use of a unintialized device at memory addr: 0x" << std::hex << addr << std::dec << std::endl;
        return false;
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
      return true;
    }
    // ~DeviceMap() {
    //   for (auto &device : this->addr_to_device) {
    //     delete device.second;
    //   }
    // }
};
}