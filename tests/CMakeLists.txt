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

# Phase 2: SPSC smoke test
add_executable(test_spsc_smoke
  test_spsc_smoke.cpp
)
target_include_directories(test_spsc_smoke PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME spsc_smoke COMMAND test_spsc_smoke)

# Phase 2: OrderBookL2 test
add_executable(test_orderbook_l2
  test_orderbook_l2.cpp
)
target_include_directories(test_orderbook_l2 PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME orderbook_l2 COMMAND test_orderbook_l2)