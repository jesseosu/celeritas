#include "celeritas/common/Log.hpp"
#include <string_view>

static void print_help() {
  celeritas::log(celeritas::LogLevel::Info,
                 "exchange: Celeritas exchange simulator\n"
                 "Usage:\n"
                 "  exchange --help\n"
                 "  exchange --version\n");
}

int main(int argc, char** argv) {
  const std::string_view arg = (argc >= 2) ? std::string_view(argv[1]) : "";

  if (arg == "--help" || arg.empty()) {
    print_help();
    return 0;
  }
  if (arg == "--version") {
    celeritas::log(celeritas::LogLevel::Info, "celeritas exchange v0.0.1");
    return 0;
  }

  celeritas::log(celeritas::LogLevel::Error, "Unknown argument. Use --help.");
  return 2;
}