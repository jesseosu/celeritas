cmake_minimum_required(VERSION 3.20)
project(celeritas LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CompilerWarnings)

# -------------------------
# Common library
# -------------------------
add_library(celeritas_common
  src/common/Time.cpp
  src/common/Log.cpp
)
target_include_directories(celeritas_common PUBLIC include)
set_project_warnings(celeritas_common)

# -------------------------
# Exchange engine library (Phase 1)
# -------------------------
add_library(celeritas_exchange
  src/exchange/OrderBook.cpp
)
target_include_directories(celeritas_exchange PUBLIC include)
target_link_libraries(celeritas_exchange PUBLIC celeritas_common)
set_project_warnings(celeritas_exchange)

# -------------------------
# CLI app
# -------------------------
add_executable(exchange
  src/exchange/main.cpp
)
target_link_libraries(exchange PRIVATE celeritas_exchange)
set_project_warnings(exchange)

# -------------------------
# Tests
# -------------------------
enable_testing()
add_subdirectory(tests)

# -------------------------
# Proto library
# -------------------------
add_library(celeritas_proto
  src/proto/Decoder.cpp
)
target_include_directories(celeritas_proto PUBLIC include)
target_link_libraries(celeritas_proto PUBLIC celeritas_common)
target_compile_features(celeritas_proto PUBLIC cxx_std_20)
add_subdirectory(apps/pipeline_runner)
