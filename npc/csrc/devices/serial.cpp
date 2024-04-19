#include <iostream>
#include <devices.hpp>

namespace Devices {
Serial::Serial(uint64_t addr, size_t len) : Device(addr, len) {
  buf[0] = 0;
};
void Serial::io_handler(uint32_t offset, size_t len, bool is_write)  {
  if(is_write) {
    std::cout << (char)buf[0];
  }
}
void Serial::transfer(uint8_t *src, size_t len, bool is_write) {
  if (is_write) {
    for (size_t i = 0; i < len; i++)
      buf[i] = src[i];
  } else {
    for (size_t i = 0; i < len; i++)
      src[i] = buf[i];
  }
}
}