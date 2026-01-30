#include "celeritas/common/Log.hpp"
#include <iostream>

namespace celeritas {

static const char* to_string(LogLevel lvl) {
  switch (lvl) {
    case LogLevel::Info:  return "INFO";
    case LogLevel::Warn:  return "WARN";
    case LogLevel::Error: return "ERROR";
    case LogLevel::Debug: return "DEBUG";
  }
  return "UNKNOWN";
}

void log(LogLevel lvl, std::string_view msg) {
  std::cerr << "[" << to_string(lvl) << "] " << msg << "\n";
}

} // namespace celeritas