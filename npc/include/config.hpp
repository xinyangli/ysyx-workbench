#ifndef _NPC_CONFIG_H_
#define _NPC_CONFIG_H_
#include <CLI/App.hpp>
#include <CLI/CLI.hpp>
#include <CLI/Validators.hpp>
#include <filesystem>

struct Config {
  std::filesystem::path memory_file;
  uint64_t max_sim_time = 1000;
  bool memory_file_binary = {true};
  bool do_trace{false};
  std::filesystem::path wavefile;
  std::filesystem::path lib_ref;
  void cli_parse(int argc, char **argv);
};

extern Config config;

#endif