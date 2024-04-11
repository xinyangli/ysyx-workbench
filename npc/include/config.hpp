#ifndef _NPC_CONFIG_H_
#define _NPC_CONFIG_H_
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Validators.hpp>
#include <cstddef>
#include <filesystem>
#include <utility>

struct Config {
  std::filesystem::path memory_file;
  uint64_t max_sim_time = 0;
  bool memory_file_binary = {true};
  bool do_mtrace{false};
  std::vector<std::pair<std::size_t, std::size_t>> mtrace_ranges {
    std::make_pair(0x80000000, 0x8ffffffff)
  };
  std::filesystem::path wavefile;
  std::filesystem::path lib_ref;
  void cli_parse(int argc, char **argv);
};

extern Config config;

#endif