
# This is a restricted set compared to Fruit's code, the bar for Fruit code is higher.
if(${FRUIT_ENABLE_CLANG_TIDY})
  set(CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    -header-filter=fruit;
    -checks=${FRUIT_CLANG_TIDY_CHECKS},-google-explicit-constructor,-google-build-using-namespace;
    -warnings-as-errors=*;)
endif()

add_subdirectory(simple_injection)
add_subdirectory(hello_world)

# This uses threads and doesn't work on Windows (at least when using MinGW's GCC).
if(NOT "${WIN32}")
    add_subdirectory(server)
endif()

add_subdirectory(multibindings)
add_subdirectory(scaling_doubles)
add_subdirectory(annotated_injection)
add_subdirectory(testing)
