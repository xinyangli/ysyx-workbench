#include <cstdint>
#include <cassert>
#include <iostream>
#include <array>
#include <memory>

namespace Devices {
class Device {
  uint64_t addr;
  public:
    virtual void io_handler(uint32_t offset, int len, bool is_write) = 0;
};

class Serial : public Device {
  std::array<uint8_t, 1> buf;
  
  
  public:
    Serial() {
      buf[0] = 0;
    };
    void io_handler(uint32_t offset, int len, bool is_write) override {
      if(!is_write) {
        std::cout << (char)buf[0];
      }
    }
} ;
}