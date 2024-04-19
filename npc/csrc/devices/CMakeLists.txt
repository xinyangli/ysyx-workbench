add_library(devices
  serial.cpp
  rtc.cpp
)

target_include_directories(devices PUBLIC include)
