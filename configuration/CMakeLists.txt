
include(CheckCXXSourceCompiles)

set(CMAKE_REQUIRED_FLAGS "${FRUIT_COMPILE_FLAGS}")

CHECK_CXX_SOURCE_COMPILES("
int main() {}
"
FRUIT_TRIVIAL_SOURCE_COMPILES)

if (NOT "${FRUIT_TRIVIAL_SOURCE_COMPILES}")
    message(FATAL_ERROR "A trivial program with an empty main doesn't compile, something is wrong with your compiler and/or with your compiler flags.")
endif()

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
#include <type_traits>
int main() {
  bool b = std::is_trivially_copy_constructible<int>::value;
  (void) b;
  return 0;
}
"
FRUIT_HAS_STD_IS_TRIVIALLY_COPY_CONSTRUCTIBLE)

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
#include <typeinfo>
int main() {
  constexpr static const std::type_info& x = typeid(int);
  (void) x;
  return 0;
}
"
FRUIT_HAS_CONSTEXPR_TYPEID)

CHECK_CXX_SOURCE_COMPILES("
#include <cxxabi.h>
int main() {
  auto* p = abi::__cxa_demangle;
  (void) p;
  return 0;
}
"
FRUIT_HAS_CXA_DEMANGLE)

if("${FRUIT_ENABLE_COVERAGE}")
    set(FRUIT_HAS_ALWAYS_INLINE_ATTRIBUTE OFF)
    set(FRUIT_HAS_FORCEINLINE OFF)
else()
CHECK_CXX_SOURCE_COMPILES("
__attribute__((always_inline))
void f() {
}

int main() {
  return 0;
}
"
FRUIT_HAS_ALWAYS_INLINE_ATTRIBUTE)

CHECK_CXX_SOURCE_COMPILES("
__forceinline
void f() {
}

int main() {
  return 0;
}
"
FRUIT_HAS_FORCEINLINE)

endif()

CHECK_CXX_SOURCE_COMPILES("
[[deprecated]] void f();

int main() {
  return 0;
}
"
FRUIT_HAS_ATTRIBUTE_DEPRECATED)

CHECK_CXX_SOURCE_COMPILES("
void f() __attribute__((deprecated));

int main() {
  return 0;
}
"
FRUIT_HAS_GCC_ATTRIBUTE_DEPRECATED)

CHECK_CXX_SOURCE_COMPILES("
__declspec(deprecated) void f();

int main() {
  return 0;
}
"
FRUIT_HAS_DECLSPEC_DEPRECATED)

CHECK_CXX_SOURCE_COMPILES("
int f() {
  static int x = 1;
  if (x == 1) {
    return 0;
  } else {
    __assume(0);
    // Note: the lack of return here is intentional
  }
}

int main() {
  return f();
}
"
FRUIT_HAS_MSVC_ASSUME)

CHECK_CXX_SOURCE_COMPILES("
int f() {
  static int x = 1;
  if (x == 1) {
    return 0;
  } else {
    __builtin_unreachable();
    // Note: the lack of return here is intentional
  }
}

int main() {
  return f();
}
"
FRUIT_HAS_BUILTIN_UNREACHABLE)


if (NOT "${FRUIT_HAS_STD_MAX_ALIGN_T}" AND NOT "${FRUIT_HAS_MAX_ALIGN_T}")
  message(WARNING "The current C++ standard library doesn't support std::max_align_t nor ::max_align_t. Attempting to use std::max_align_t anyway, but it most likely won't work.")
endif()

if(NOT "${FRUIT_HAS_STD_IS_TRIVIALLY_COPYABLE}" AND NOT "${FRUIT_HAS_IS_TRIVIALLY_COPYABLE}"
   AND NOT "${FRUIT_HAS_HAS_TRIVIAL_COPY}")
  message(WARNING "The current standard library doesn't support std::is_trivially_copyable<T>, and the current compiler doesn't support __is_trivially_copyable(T) nor __has_trivial_copy(T). Attemping to use std::is_trivially_copyable<T> anyway, but it most likely won't work.")
endif()

if (NOT "${FRUIT_HAS_ATTRIBUTE_DEPRECATED}" AND NOT "${FRUIT_HAS_GCC_ATTRIBUTE_DEPRECATED}" AND NOT "${FRUIT_HAS_DECLSPEC_DEPRECATED}")
  message(WARNING "No supported way to mark functions as deprecated was found. Continuing anyway, without the 'deprecated' markers.")
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/fruit-config-base.h.in ${CMAKE_CURRENT_BINARY_DIR}/../include/fruit/impl/fruit-config-base.h)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/../include/fruit/impl/fruit-config-base.h
  DESTINATION ${INSTALL_INCLUDE_DIR}/impl)
