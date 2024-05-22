cmake_minimum_required(VERSION 3.2)

project(Fruit VERSION 3.7.1 LANGUAGES CXX)

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

get_property(isMultiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
option(FRUIT_ALLOW_MULTI_CONFIG "Allow multi-configuration generator. If this option is OFF, enforces that a single configuration is used." OFF)
if(FRUIT_ALLOW_MULTI_CONFIG)
  if(isMultiConfig)
    message(STATUS "${CMAKE_PROJECT_NAME} supports the CMake multi-configuration generator,"
                   " but is optimized for the `CMAKE_BUILD_TYPE` specified in the CMake configuration step,"
                   " and tests will only work in this configuration.")
  endif()
else()
  # For backward compatibility, only `CMAKE_BUILD_TYPE` configurations can be
  # used with the multi-configuration generator. This makes it look like
  # single-configuration generator.
  set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}")
endif()

option(BUILD_SHARED_LIBS "Build shared library" ON)

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

option(FRUIT_ADD_WNO_UNKNOWN_WARNING_OPTION "Add -Wno-unknown-warning-option to the compiler options for GCC, Clang, ICC and AppleClang" ON)
if (${FRUIT_ADD_WNO_UNKNOWN_WARNING_OPTION})
  set(FRUIT_NO_UNKNOWN_WARNING_FLAGS -Wno-unknown-warning-option)
endif()

# Get C++ standard flags.
set(FRUIT_CXX_STANDARD_FLAGS ${CMAKE_CXX${CMAKE_CXX_STANDARD}_STANDARD_COMPILE_OPTION})

set(FRUIT_ADDITIONAL_COMPILE_FLAGS_GNU -W -Wall -Wno-missing-braces ${FRUIT_NO_UNKNOWN_WARNING_FLAGS})
set(FRUIT_ADDITIONAL_COMPILE_FLAGS_Clang -W -Wall -Wno-missing-braces ${FRUIT_NO_UNKNOWN_WARNING_FLAGS})
set(FRUIT_ADDITIONAL_COMPILE_FLAGS_Intel -W -Wall -Wno-missing-braces ${FRUIT_NO_UNKNOWN_WARNING_FLAGS})
set(FRUIT_ADDITIONAL_COMPILE_FLAGS_AppleClang -W -Wall -Wno-missing-braces ${FRUIT_NO_UNKNOWN_WARNING_FLAGS})

# TODO: we currently disable the warning C4709 because MSVC emits it even when there is no reason to. Re-enable it when possible.
# TODO: the warning C4141 is disabled, because MSVC emits it ("'inline': used more than once") when a function/method is marked with both __forceinline and inline.
# TODO: the warning C4714 is disabled, MSVC emits it when it decides not to inline a __forceinline function/method.
# The warning C4577 is disabled because we don't need a termination guarantee on exceptions for functions marked with
# 'noexcept'.
# The warning C4530 is disabled because it's triggered by MSVC's STL.
# The warning C5205 is disabled, MSVC emits it for abstract classes in example code with non-virtual destructors, but we never call delete on those (even though std::default_delete<Scaler> is instantiated for those types).
set(FRUIT_ADDITIONAL_COMPILE_FLAGS_MSVC /nologo /FS /W4 /wd4324 /wd4709 /wd4459 /wd4141 /wd4714 /wd4577 /wd4530 /wd5205 /D_SCL_SECURE_NO_WARNINGS)

list(APPEND FRUIT_ADDITIONAL_COMPILE_FLAGS
  $<$<CXX_COMPILER_ID:GNU>:${FRUIT_ADDITIONAL_COMPILE_FLAGS_GNU}>
  $<$<CXX_COMPILER_ID:Clang>:${FRUIT_ADDITIONAL_COMPILE_FLAGS_Clang}>
  $<$<CXX_COMPILER_ID:Intel>:${FRUIT_ADDITIONAL_COMPILE_FLAGS_Intel}>
  $<$<CXX_COMPILER_ID:AppleClang>:${FRUIT_ADDITIONAL_COMPILE_FLAGS_AppleClang}>
  $<$<CXX_COMPILER_ID:MSVC>:${FRUIT_ADDITIONAL_COMPILE_FLAGS_MSVC}>)

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
  if(NOT Boost_FOUND)
    message(FATAL_ERROR "Please re-run CMake, specifying the boost library path as Boost_INCLUDE_DIR, e.g. -DBoost_INCLUDE_DIR=C:\\boost\\boost_1_62_0, or specify -DFRUIT_USES_BOOST=False to not use boost.")
  endif()
endif()

set(RUN_TESTS_UNDER_VALGRIND FALSE CACHE BOOL "Whether to run Fruit tests under valgrind")
if ("${RUN_TESTS_UNDER_VALGRIND}")
  set(RUN_TESTS_UNDER_VALGRIND_FLAG "1")
endif()

# Unsafe, only for debugging/benchmarking.
#set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} -DFRUIT_NO_LOOP_CHECK=1")

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FRUIT_ADDITIONAL_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${FRUIT_ADDITIONAL_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${FRUIT_ADDITIONAL_LINKER_FLAGS}")

set(FRUIT_CLANG_TIDY_CHECKS
    bugprone*,-bugprone-reserved-identifier,-bugprone-exception-escape,clang-analyzer*,performance*,google*,-google-readability*,-google-runtime-references,clang-diagnostic-unused-command-line-argument,misc-macro-parentheses,-clang-diagnostic-dtor-name,-performance-avoid-endl,-performance-enum-size)

set(FRUIT_ENABLE_CLANG_TIDY FALSE CACHE BOOL "Whether to run clang-tidy on the Fruit codebase during the build")
if(${FRUIT_ENABLE_CLANG_TIDY})
  set(CMAKE_CXX_CLANG_TIDY
    clang-tidy;
    -header-filter=fruit;
    -checks=${FRUIT_CLANG_TIDY_CHECKS};
    -warnings-as-errors=*;)
endif()

set(FRUIT_INCLUDE_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_BINARY_DIR}/include")

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
