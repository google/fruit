
include(CheckCXXSourceCompiles)

set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_CXX11_STANDARD_COMPILE_OPTION})

CHECK_CXX_SOURCE_COMPILES("
template <typename T, typename U>
struct Pair {};

struct Map : public Pair<int, float>, Pair<int, char> {};

template <typename Value>
Value f(Pair<int, Value>*) { return Value(); }

int main() {
  f((Map*)0);
}
"
FRUIT_HAS_CLANG_ARBITRARY_OVERLOAD_RESOLUTION_BUG)

CHECK_CXX_SOURCE_COMPILES("
int main() {
  bool b = __has_trivial_copy(int);
  (void) b;
  return 0;
}
"
FRUIT_HAS_HAS_TRIVIAL_COPY)

CHECK_CXX_SOURCE_COMPILES("
int main() {
  bool b = __is_trivially_copyable(int);
  (void) b;
  return 0;
}
"
FRUIT_HAS_IS_TRIVIALLY_COPYABLE)

CHECK_CXX_SOURCE_COMPILES("
#include <cstddef>
using X = max_align_t;
int main() {
  return 0;
}
"
FRUIT_HAS_MAX_ALIGN_T)

CHECK_CXX_SOURCE_COMPILES("
#include <type_traits>
int main() {
  bool b = std::is_trivially_copyable<int>::value;
  (void) b;
  return 0;
}
"
FRUIT_HAS_STD_IS_TRIVIALLY_COPYABLE)

CHECK_CXX_SOURCE_COMPILES("
#include <cstddef>
using X = std::max_align_t;
int main() {
  return 0;
}
"
FRUIT_HAS_STD_MAX_ALIGN_T)

CHECK_CXX_SOURCE_COMPILES("
#include <typeinfo>
int main() {
  (void) typeid(int);
  return 0;
}
"
FRUIT_HAS_TYPEID)

CHECK_CXX_SOURCE_COMPILES("
#include <cxxabi.h>
int main() {
  auto* p = abi::__cxa_demangle;
  return 0;
}
"
FRUIT_HAS_CXA_DEMANGLE)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fruit-config-base.h.in ${CMAKE_BINARY_DIR}/include/fruit/impl/fruit-config-base.h)

install(FILES ${CMAKE_BINARY_DIR}/include/fruit/impl/fruit-config-base.h
  DESTINATION ${INSTALL_INCLUDE_DIR}/impl)
