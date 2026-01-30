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

add_library(celeritas_common
  src/common/Time.cpp
  src/common/Log.cpp
)
target_include_directories(celeritas_common PUBLIC include)
set_project_warnings(celeritas_common)

add_executable(exchange
  src/exchange/main.cpp
)
target_link_libraries(exchange PRIVATE celeritas_common)
set_project_warnings(exchange)

enable_testing()
add_subdirectory(tests)
