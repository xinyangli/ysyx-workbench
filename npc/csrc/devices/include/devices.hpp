#include <cstdint>
#include <cassert>
#include <iostream>
#include <memory>

namespace Devices {
class Device {
  uint64_t addr;
  public:
    virtual void io_handler(uint32_t offset, int len, bool is_write) = 0;
};

class Serial : public Device {
  std::array<uint8_t, 8> buf;
  
  
  public:
    Serial() {
      buf.fill(0);
    };
    void io_handler(uint32_t offset, int len, bool is_write) override {
      assert(len == 1);
      if(!is_write) {
        // std::cout << ;
      }
    }
} ;
}