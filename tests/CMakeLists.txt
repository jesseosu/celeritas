# ── Phase 0: Sanity ──────────────────────────────────────────────────────
add_executable(celeritas_tests test_sanity.cpp)
target_link_libraries(celeritas_tests PRIVATE celeritas_common)
target_include_directories(celeritas_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME celeritas_tests COMMAND celeritas_tests)

# ── Phase 1: OrderBook ───────────────────────────────────────────────────
add_executable(test_orderbook test_orderbook.cpp)
target_link_libraries(test_orderbook PRIVATE celeritas_exchange)
target_include_directories(test_orderbook PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME test_orderbook COMMAND test_orderbook)

# ── Phase 2: SPSC smoke ──────────────────────────────────────────────────
add_executable(test_spsc_smoke test_spsc_smoke.cpp)
target_include_directories(test_spsc_smoke PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME spsc_smoke COMMAND test_spsc_smoke)

# ── Phase 2: OrderBookL2 ─────────────────────────────────────────────────
add_executable(test_orderbook_l2 test_orderbook_l2.cpp)
target_include_directories(test_orderbook_l2 PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME orderbook_l2 COMMAND test_orderbook_l2)

# ── Phase 3: Proto ───────────────────────────────────────────────────────
add_executable(test_proto test_proto.cpp)
target_link_libraries(test_proto PRIVATE celeritas_proto)
target_include_directories(test_proto PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME test_proto COMMAND test_proto)

add_executable(test_proto_sizes test_proto_sizes.cpp)
target_link_libraries(test_proto_sizes PRIVATE celeritas_proto)
target_include_directories(test_proto_sizes PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME test_proto_sizes COMMAND test_proto_sizes)

# ── Phase 4: Matching engine ─────────────────────────────────────────────
add_executable(test_matching_basic test_matching_basic.cpp)
target_link_libraries(test_matching_basic PRIVATE celeritas_exchange)
target_include_directories(test_matching_basic PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME test_matching_basic COMMAND test_matching_basic)

add_executable(test_matching_partial test_matching_partial.cpp)
target_link_libraries(test_matching_partial PRIVATE celeritas_exchange)
target_include_directories(test_matching_partial PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME test_matching_partial COMMAND test_matching_partial)

add_executable(test_matching_cancel test_matching_cancel.cpp)
target_link_libraries(test_matching_cancel PRIVATE celeritas_exchange)
target_include_directories(test_matching_cancel PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME test_matching_cancel COMMAND test_matching_cancel)

add_executable(test_price_time_priority test_price_time_priority.cpp)
target_link_libraries(test_price_time_priority PRIVATE celeritas_exchange)
target_include_directories(test_price_time_priority PRIVATE ${PROJECT_SOURCE_DIR}/include)
add_test(NAME test_price_time_priority COMMAND test_price_time_priority)