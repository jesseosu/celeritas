add_executable(pipeline_runner
  main.cpp
  ../../src/metrics/latency.cpp
)
target_include_directories(pipeline_runner PRIVATE ../../include)
target_link_libraries(pipeline_runner PRIVATE celeritas_common)
