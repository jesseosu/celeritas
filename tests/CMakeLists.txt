# Sanity test (Phase 0)
add_executable(celeritas_tests
  test_sanity.cpp
)
target_link_libraries(celeritas_tests PRIVATE celeritas_common)
target_include_directories(celeritas_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME celeritas_tests COMMAND celeritas_tests)

# OrderBook test (Phase 1)
add_executable(test_orderbook
  test_orderbook.cpp
)
target_link_libraries(test_orderbook PRIVATE celeritas_exchange)
target_include_directories(test_orderbook PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME test_orderbook COMMAND test_orderbook)
