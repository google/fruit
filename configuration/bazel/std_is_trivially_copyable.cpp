#include <type_traits>
int main() {
  bool b = std::is_trivially_copyable<int>::value;
  (void) b;
  return 0;
}
