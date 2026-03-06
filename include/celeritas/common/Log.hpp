#pragma once
#include <string_view>

namespace celeritas {

enum class LogLevel { Info, Warn, Error, Debug };

void log(LogLevel lvl, std::string_view msg);

}