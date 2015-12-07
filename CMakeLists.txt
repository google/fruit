project(injection)
cmake_minimum_required(VERSION 2.8)

# CMake on OSX likes to see this set explicitly, otherwise it outputs a warning.
set(CMAKE_MACOSX_RPATH 1)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

set(FRUIT_COMPILE_FLAGS "-std=c++11 -W -Wall")
if((${CMAKE_BUILD_TYPE} STREQUAL "Debug"))
  set(FRUIT_COMPILE_FLAGS "${FRUIT_COMPILE_FLAGS} -g -O0 -Werror -DFRUIT_EXTRA_DEBUG -D_GLIBCXX_DEBUG")
  if((((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "4.8.99"))
     OR (CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
     AND NOT ${RUN_TESTS_UNDER_VALGRIND})
    set(FRUIT_COMPILE_FLAGS "${FRUIT_COMPILE_FLAGS} -fsanitize=undefined,address")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fsanitize=undefined,address")
  endif()
endif()
add_definitions(${FRUIT_COMPILE_FLAGS})

include_directories(${CMAKE_SOURCE_DIR}/include)

# Unsafe, only for debugging/benchmarking.
#add_definitions(-DFRUIT_NO_LOOP_CHECK)

# (debug-only) compile switch to get deep template instantiation stacktraces for errors (instead
# of the user-friendly default that hides Fruit internals).
#add_definitions(-DFRUIT_DEEP_TEMPLATE_INSTANTIATION_STACKTRACES_FOR_ERRORS)


set(INSTALL_LIBRARY_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_INCLUDE_DIR include/fruit CACHE PATH "Installation directory for headers")

set(FRUIT_VERSION "1.0.2")

enable_testing()
set(CTEST_TEST_TIMEOUT 20 CACHE STRING "Maximum test time, before CTest will kill the test." FORCE)

add_subdirectory(src)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  # Do not exclude these from "make all" in debug mode, they must build.
  add_subdirectory(extras)
  add_subdirectory(examples)
  add_subdirectory(tests)
else()
  add_subdirectory(extras EXCLUDE_FROM_ALL)
  add_subdirectory(examples EXCLUDE_FROM_ALL)
  add_subdirectory(tests EXCLUDE_FROM_ALL)
endif()

add_subdirectory(doc EXCLUDE_FROM_ALL)

install(DIRECTORY include/fruit/
  DESTINATION ${INSTALL_INCLUDE_DIR}
  FILES_MATCHING PATTERN "*.h")
