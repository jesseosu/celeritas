add_executable(celeritas_tests
  test_sanity.cpp
)

target_link_libraries(celeritas_tests PRIVATE celeritas_common)
target_include_directories(celeritas_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)

add_test(NAME celeritas_tests COMMAND celeritas_tests)
