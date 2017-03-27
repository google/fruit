project(Fruit)
cmake_minimum_required(VERSION 2.8)

if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

# CMake on OSX likes to see this set explicitly, otherwise it outputs a warning.
set(CMAKE_MACOSX_RPATH 1)

if(NOT "${CMAKE_BUILD_TYPE}" MATCHES "^(Debug|Release|RelWithDebInfo|MinSizeRel)$")
  message(FATAL_ERROR "Please re-run CMake, specifying -DCMAKE_BUILD_TYPE=Debug , -DCMAKE_BUILD_TYPE=Release , -DCMAKE_BUILD_TYPE=RelWithDebInfo or -DCMAKE_BUILD_TYPE=MinSizeRel .")
endif()

option(BUILD_SHARED_LIBS "Build shared library" ON)

# The Visual Studio CMake generators default to multiple configurations, but Fruit doesn't support multi-configuration build directories.
set(CMAKE_CONFIGURATION_TYPES "${CMAKE_BUILD_TYPE}")

if(NOT "${RUNTIME_OUTPUT_DIRECTORY}")
  set(RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
endif()

if(NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|AppleClang|MSVC)$")
  message(WARNING "Compiler not officially supported: ${CMAKE_CXX_COMPILER_ID}")
  # Full list of possible values at https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_COMPILER_ID.html .
  # Major compilers not currently supported:
  # * "MSVC": not supported ATM due to compiler bugs:
  #   - https://connect.microsoft.com/VisualStudio/feedback/details/2197110
  #   - https://connect.microsoft.com/VisualStudio/feedback/details/2197169
  # * "Intel": not supported ATM due to compiler bugs:
  #   - https://software.intel.com/en-us/forums/intel-c-compiler/topic/606048
  #   - https://software.intel.com/en-us/forums/intel-c-compiler/topic/606049
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|Intel|AppleClang)$")
  set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} -std=c++11 -W -Wall -Wno-unknown-warning-option -Wno-missing-braces")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(MSVC)$")
  # TODO: we currently disable the warning C4709 because MSVC emits it even when there is no reason to. Re-enable it when possible.
  set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} /nologo /FS /W4 /wd4324 /wd4709 /wd4459 /D_SCL_SECURE_NO_WARNINGS")
endif()

set(FRUIT_USES_BOOST TRUE CACHE BOOL
        "Whether to use Boost (specifically, boost::unordered_set and boost::unordered_map).
        If this is false, Fruit will use std::unordered_set and std::unordered_map instead (however this causes injection to be a bit slower).")

if("${WIN32}" AND "${FRUIT_USES_BOOST}")
  set(BOOST_DIR "" CACHE PATH "The directory where the boost library is installed, e.g. C:\\boost\\boost_1_62_0.")
  if("${BOOST_DIR}" STREQUAL "")
    message(FATAL_ERROR "Please re-run CMake, specifying the boost library path as BOOST_DIR, e.g. -DBOOST_DIR=C:\\boost\\boost_1_62_0.")
  endif()
  include_directories("${BOOST_DIR}")
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|Intel|AppleClang)$")
    set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} -g -Werror -DFRUIT_DEBUG -DFRUIT_EXTRA_DEBUG -D_GLIBCXX_DEBUG")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND "${CMAKE_CXX_COMPILER_VERSION}" MATCHES "4.8.*")
      # Don't set -pedantic with GCC 4.8, it's too restrictive (e.g. it doesn't allow casting a function pointer to void*).
    else()
      set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} -pedantic")
    endif()
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(MSVC)$")
    set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} /DEBUG /WX /DFRUIT_DEBUG /DFRUIT_EXTRA_DEBUG /D_ITERATOR_DEBUG_LEVEL=2")
  endif()
endif()

if("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|Intel|AppleClang)$")
    set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} -O2 -DNDEBUG")
  endif()
endif()

set(RUN_TESTS_UNDER_VALGRIND FALSE CACHE BOOL "Whether to run Fruit tests under valgrind")
if ("${RUN_TESTS_UNDER_VALGRIND}")
  set(RUN_TESTS_UNDER_VALGRIND_FLAG "1")
endif()

# Unsafe, only for debugging/benchmarking.
#set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} -DFRUIT_NO_LOOP_CHECK")

add_definitions(${FRUIT_ADDITIONAL_COMPILE_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FRUIT_ADDITIONAL_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${FRUIT_ADDITIONAL_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${FRUIT_ADDITIONAL_LINKER_FLAGS}")

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    set(FRUIT_COMPILE_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} ${FRUIT_ADDITIONAL_COMPILE_FLAGS}")
else()
    set(FRUIT_COMPILE_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} ${FRUIT_ADDITIONAL_COMPILE_FLAGS}")
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

# (debug-only) compile switch to get deep template instantiation stacktraces for errors (instead
# of the user-friendly default that hides Fruit internals).
#add_definitions(-DFRUIT_DEEP_TEMPLATE_INSTANTIATION_STACKTRACES_FOR_ERRORS)

set(INSTALL_INCLUDE_DIR include/fruit CACHE PATH "Installation directory for headers")
set(INSTALL_LIBRARY_DIR lib CACHE PATH "Installation directory for libraries")

set(FRUIT_VERSION "2.0.4")

enable_testing()
set(CTEST_TEST_TIMEOUT 20 CACHE STRING "Maximum test time, before CTest will kill the test." FORCE)

add_subdirectory(configuration)
add_subdirectory(src)

option(BUILD_TESTS_IN_RELEASE_MODE "Build tests as part of the ALL target even in Release mode" OFF)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  # Do not exclude these from "make all" in debug mode, they must build.
  add_subdirectory(examples)
  add_subdirectory(tests)
else()
  add_subdirectory(examples EXCLUDE_FROM_ALL)
  if ("${BUILD_TESTS_IN_RELEASE_MODE}")
    add_subdirectory(tests)
  else()
    add_subdirectory(tests EXCLUDE_FROM_ALL)
  endif()
endif()

add_subdirectory(extras EXCLUDE_FROM_ALL)

install(DIRECTORY include/fruit/
  DESTINATION "${INSTALL_INCLUDE_DIR}"
  FILES_MATCHING PATTERN "*.h")

set(CPACK_PACKAGE_NAME "Fruit")
set(CPACK_PACKAGE_VENDOR "Marco Poletti")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Fruit - Dependency Injection Framework For C++")
string(REGEX REPLACE "([^.]*)\\.([^.]*)\\.([^.]*)" "\\1" CPACK_PACKAGE_VERSION_MAJOR "${FRUIT_VERSION}")
string(REGEX REPLACE "([^.]*)\\.([^.]*)\\.([^.]*)" "\\2" CPACK_PACKAGE_VERSION_MINOR "${FRUIT_VERSION}")
string(REGEX REPLACE "([^.]*)\\.([^.]*)\\.([^.]*)" "\\3" CPACK_PACKAGE_VERSION_PATCH "${FRUIT_VERSION}")
set(CPACK_PACKAGE_VERSION "${FRUIT_VERSION}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Fruit")
