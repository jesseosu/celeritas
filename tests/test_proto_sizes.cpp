#include "celeritas/proto/Messages.hpp"
#include <iostream>

int main() {
  using namespace celeritas::proto;
  std::cout << "MsgHeader=" << sizeof(MsgHeader) << "\n";
  std::cout << "NewOrder=" << sizeof(NewOrder) << "\n";
  std::cout << "Cancel=" << sizeof(Cancel) << "\n";
  std::cout << "Replace=" << sizeof(Replace) << "\n";
  return 0;
}
