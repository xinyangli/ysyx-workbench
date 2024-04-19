#include <cstdint>
#include <devices.hpp>

namespace Devices {
uint64_t RTC::get_time_internal() {
#if defined(CONFIG_TARGET_AM)
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
#elif defined(CONFIG_TIMER_GETTIMEOFDAY)
  struct timeval now;
  gettimeofday(&now, NULL);
  uint64_t us = now.tv_sec * 1000000 + now.tv_usec;
#else
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
  uint64_t us = now.tv_sec * 1000000 + now.tv_nsec / 1000;
#endif
  return us;
}

uint64_t RTC::get_time() {
  if (boot_time == 0) boot_time = get_time_internal();
  uint64_t now = get_time_internal();
  return now - boot_time;
}

RTC::RTC(uint64_t addr, size_t len) : Device(addr, len) {
  *(uint64_t *)buf.data() = 0;
};

void RTC::io_handler(uint32_t offset, size_t len, bool is_write) {
  assert(offset == 0 || offset == 4);
  if (!is_write && offset == 4) {
    uint64_t us = get_time();
    buf[0] = (uint32_t)us;
    buf[1] = us >> 32;
  }
}

void RTC::transfer(uint8_t *src, size_t len, bool is_write) {
  if (is_write) {
    for (size_t i = 0; i < len; i++)
      buf[i] = src[i];
  } else {
    for (size_t i = 0; i < len; i++)
      src[i] = buf[i];
  }
}
}