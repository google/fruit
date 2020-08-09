#include <typeinfo>
int main() {
  constexpr static const std::type_info& x = typeid(int);
  (void) x;
  return 0;
}
