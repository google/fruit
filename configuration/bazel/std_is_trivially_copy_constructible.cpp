#include <type_traits>
int main() {
  bool b = std::is_trivially_copy_constructible<int>::value;
  (void) b;
  return 0;
}
