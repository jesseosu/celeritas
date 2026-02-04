#include "celeritas/common/Time.hpp"
#include <cassert>
#include <iostream>

int main() {
  const auto t1 = celeritas::now_ns();
  const auto t2 = celeritas::now_ns();
  assert(t2 >= t1);

  std::cout << "sanity ok\n";
  return 0;
}