project(Fruit)
cmake_minimum_required(VERSION 2.8)

# CMake on OSX likes to see this set explicitly, otherwise it outputs a warning.
set(CMAKE_MACOSX_RPATH 1)

# Build shared libraries by default
option(BUILD_SHARED_LIBS "Build shared library" ON)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

if(NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|AppleClang)$")
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
  set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} -std=c++11 -W -Wall")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(MSVC)$")
  set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} /Wall /D_SCL_SECURE_NO_WARNINGS")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  if("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(GNU|Clang|Intel|AppleClang)$")
    set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} -g -Werror -DFRUIT_DEBUG -DFRUIT_EXTRA_DEBUG -D_GLIBCXX_DEBUG")
  elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(MSVC)$")
    set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} /DEBUG /WX /DFRUIT_DEBUG /DFRUIT_EXTRA_DEBUG /D_ITERATOR_DEBUG_LEVEL=2")
  endif()
endif()

set(RUN_TESTS_UNDER_VALGRIND FALSE CACHE BOOL "Whether to run Fruit tests under valgrind")

# Unsafe, only for debugging/benchmarking.
#set(FRUIT_ADDITIONAL_COMPILE_FLAGS "${FRUIT_ADDITIONAL_COMPILE_FLAGS} -DFRUIT_NO_LOOP_CHECK")

add_definitions(${FRUIT_ADDITIONAL_COMPILE_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FRUIT_ADDITIONAL_LINKER_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${FRUIT_ADDITIONAL_LINKER_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} ${FRUIT_ADDITIONAL_LINKER_FLAGS}")
set(FRUIT_COMPILE_FLAGS "${CMAKE_CXX_FLAGS} ${FRUIT_ADDITIONAL_COMPILE_FLAGS}")

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_BINARY_DIR}/include)

# (debug-only) compile switch to get deep template instantiation stacktraces for errors (instead
# of the user-friendly default that hides Fruit internals).
#add_definitions(-DFRUIT_DEEP_TEMPLATE_INSTANTIATION_STACKTRACES_FOR_ERRORS)

if(${WIN32})
  set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for headers")
else()
  set(INSTALL_INCLUDE_DIR include/fruit CACHE PATH "Installation directory for headers")
  set(INSTALL_LIBRARY_DIR lib CACHE PATH "Installation directory for libraries")
endif()

set(FRUIT_VERSION "2.0.4")

enable_testing()
set(CTEST_TEST_TIMEOUT 20 CACHE STRING "Maximum test time, before CTest will kill the test." FORCE)

add_subdirectory(configuration)
add_subdirectory(src)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  # Do not exclude these from "make all" in debug mode, they must build.
  add_subdirectory(examples)
  add_subdirectory(tests)
else()
  add_subdirectory(examples EXCLUDE_FROM_ALL)
  add_subdirectory(tests EXCLUDE_FROM_ALL)
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
