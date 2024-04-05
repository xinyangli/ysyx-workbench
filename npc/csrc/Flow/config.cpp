#include "config.hpp"

void Config::cli_parse(int argc, char **argv) {
  CLI::App app;
  app.add_option("-m,--memory", memory_file, "Content of memory")
      ->required()
      ->check(CLI::ExistingFile);
  app.add_flag("!--no-bin", memory_file_binary,
               "Memory file is in text format");
  app.add_flag("--trace", do_trace, "Enable tracing");
  app.add_option("--wav", wavefile, "output .vcd file path")
      ->check([=](const std::string &) {
        if (!do_trace)
          throw CLI::ValidationError(
              "dependency", "You must turn on trace before specify wave file");
        return std::string();
      });
  app.add_option("-t", max_sim_time, "Max simulation timestep");
  app.add_option("--diff-lib", lib_ref,
                 "Dynamic library file of difftest reference")
      ->check(CLI::ExistingFile);

  try {
    app.parse(argc, argv);
  } catch (const CLI::ParseError &e) {
    exit((app).exit(e));
  }
}

Config config;
