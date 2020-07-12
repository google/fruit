cmake_minimum_required(VERSION 3.2)

project(Fruit VERSION 3.6.0 LANGUAGES CXX)

set(FRUIT_IS_BEING_BUILT_BY_CONAN FALSE CACHE BOOL "This is set in Conan builds.")
if("${FRUIT_IS_BEING_BUILT_BY_CONAN}")
  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup()
endif()

if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

if ("${CMAKE_CXX_STANDARD}" STREQUAL "")
    set(CMAKE_CXX_STANDARD 11)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CMake on OSX likes to see this set explicitly, otherwise it outputs a warning.
set(CMAKE_MACOSX_RPATH 1)

if(NOT "${CMAKE_BUILD_TYPE}" MATCHES "^(Debug|Release|RelWithDebInfo|MinSizeRel)$")
  message(FATAL_ERROR "Please re-run CMake, specifying -DCMAKE_BUILD_TYPE=Debug , -DCMAKE_BUILD_TYPE=Release , -DCMAKE_BUILD_TYPE=RelWithDebInfo or -DCMAKE_BUILD_TYPE=MinSizeRel .")
endif()

option(BUILD_SHARED_LIBS "Build shared library" ON)

# The Visual Studio CMake generators default to multiple configurations, but Fruit doesn't support multi-configuration build directories.
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}")

set(FRUIT_ADDITIONAL_CXX_FLAGS "" CACHE STRING "Additional CXX compiler flags." FORCE)

set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_CXX_FLAGS}")

if(NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|AppleClang|MSVC)$")
  message(WARNING "Compiler not officially supported: ${CMAKE_CXX_COMPILER_ID}")
  # Full list of possible values at https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html .
  # Major compilers not currently supported:
  # * "Intel": not supported ATM due to compiler bugs:
  #   - https://software.intel.com/en-us/forums/intel-c-compiler/topic/606048
  #   - https://software.intel.com/en-us/forums/intel-c-compiler/topic/606049
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|Intel|AppleClang)$")
  set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} -std=c++11 -W -Wall -Wno-unknown-warning-option -Wno-missing-braces")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(MSVC)$")
  # TODO: we currently disable the warning C4709 because MSVC emits it even when there is no reason to. Re-enable it when possible.
  # TODO: the warning C4141 is disabled, because MSVC emits it ("'inline': used more than once") when a function/method is marked with both __forceinline and inline.
  # TODO: the warning C4714 is disabled, MSVC emits it when it decides not to inline a __forceinline function/method.
  # The warning C4577 is disabled because we don't need a termination guarantee on exceptions for functions marked with
  # 'noexcept'.
  # The warning C4530 is disabled because it's triggered by MSVC's STL.
  # The warning C5205 is disabled, MSVC emits it for abstract classes in example code with non-virtual destructors, but we never call delete on those (even though std::default_delete<Scaler> is instantiated for those types).
  set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} /nologo /FS /W4 /wd4324 /wd4709 /wd4459 /wd4141 /wd4714 /wd4577 /wd4530 /wd5205 /D_SCL_SECURE_NO_WARNINGS")
endif()

option(FRUIT_ENABLE_COVERAGE "Enable collection of test coverage. This is meant to be used by Fruit developers. It's only supported when using GCC on Linux." OFF)
if("${FRUIT_ENABLE_COVERAGE}")
  # We also disable exceptions because otherwise GCC considers every function/method call that could throw as an
  # additional branch.
  set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} -fprofile-arcs -ftest-coverage -fno-exceptions -fno-inline -O0")
  set(FRUIT_ADDITIONAL_LINKER_FLAGS "${FRUIT_ADDITIONAL_LINKER_FLAGS} -fprofile-arcs -ftest-coverage -fno-exceptions -fno-inline -O0")
endif()

set(FRUIT_USES_BOOST TRUE CACHE BOOL
        "Whether to use Boost (specifically, boost::unordered_set and boost::unordered_map).
        If this is false, Fruit will use std::unordered_set and std::unordered_map instead (however this causes injection to be a bit slower).")

if(${FRUIT_USES_BOOST})

  if(DEFINED BOOST_DIR)
    message(DEPRECATION "BOOST_DIR is deprecated. Use Boost_INCLUDE_DIR instead.")
    set(Boost_INCLUDE_DIR "${BOOST_DIR}" CACHE PATH "")
  endif()

  find_package(Boost)
  if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
  else()
    message(FATAL_ERROR "Please re-run CMake, specifying the boost library path as Boost_INCLUDE_DIR, e.g. -DBoost_INCLUDE_DIR=C:\\boost\\boost_1_62_0, or specify -DFRUIT_USES_BOOST=False to not use boost.")
  endif()
endif()

set(RUN_TESTS_UNDER_VALGRIND FALSE CACHE BOOL "Whether to run Fruit tests under valgrind")
if ("${RUN_TESTS_UNDER_VALGRIND}")
  set(RUN_TESTS_UNDER_VALGRIND_FLAG "1")
endif()

# Unsafe, only for debugging/benchmarking.
#set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} -DFRUIT_NO_LOOP_CHECK=1")

add_definitions(${FRUIT_ADDITIONAL_COMPILE_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FRUIT_ADDITIONAL_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${FRUIT_ADDITIONAL_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${FRUIT_ADDITIONAL_LINKER_FLAGS}")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(FRUIT_COMPILE_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} ${FRUIT_ADDITIONAL_COMPILE_FLAGS}")
else()
    set(FRUIT_COMPILE_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} ${FRUIT_ADDITIONAL_COMPILE_FLAGS}")
endif()

set(FRUIT_CLANG_TIDY_CHECKS
    bugprone*,-bugprone-reserved-identifier,-bugprone-exception-escape,clang-analyzer*,performance*,google*,-google-readability*,-google-runtime-references,clang-diagnostic-unused-command-line-argument,misc-macro-parentheses,-clang-diagnostic-dtor-name)

set(FRUIT_ENABLE_CLANG_TIDY FALSE CACHE BOOL "Whether to run clang-tidy on the Fruit codebase during the build")
if(${FRUIT_ENABLE_CLANG_TIDY})
  set(CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    -header-filter=fruit;
    -checks=${FRUIT_CLANG_TIDY_CHECKS};
    -warnings-as-errors=*;)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

# (debug-only) compile switch to get deep template instantiation stacktraces for errors (instead
# of the user-friendly default that hides Fruit internals).
#add_definitions(-DFRUIT_DEEP_TEMPLATE_INSTANTIATION_STACKTRACES_FOR_ERRORS=1)

include(GNUInstallDirs)

add_subdirectory(configuration)
add_subdirectory(src)

if(NOT "${FRUIT_IS_BEING_BUILT_BY_CONAN}")
  if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    # Do not exclude these from "make all" in debug mode, they must build.
    add_subdirectory(examples)
    add_subdirectory(tests)
  else()
    add_subdirectory(examples EXCLUDE_FROM_ALL)
    add_subdirectory(tests)
  endif()

  add_subdirectory(extras EXCLUDE_FROM_ALL)
endif()

install(DIRECTORY include/fruit/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/fruit
  FILES_MATCHING PATTERN "*.h")

set(CPACK_PACKAGE_NAME "Fruit")
set(CPACK_PACKAGE_VENDOR "Marco Poletti")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Fruit - Dependency Injection Framework For C++")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Fruit")
