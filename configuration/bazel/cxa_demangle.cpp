#include <cxxabi.h>
int main() {
  auto* p = abi::__cxa_demangle;
  (void) p;
  return 0;
}
