project(injection)
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_definitions(-std=c++11 -W -Wall)
if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  add_definitions(-g -O2 -Werror -DFRUIT_EXTRA_DEBUG -D_GLIBCXX_DEBUG -ftemplate-backtrace-limit=0)
endif()

include_directories(${CMAKE_SOURCE_DIR}/include)

#set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

# Unsafe, only for debugging/benchmarking.
#add_definitions(-DFRUIT_NO_LOOP_CHECK)


set(INSTALL_LIBRARY_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for headers")

set(FRUIT_VERSION "0.9.6")

enable_testing()

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

install(DIRECTORY include
  DESTINATION ${INSTALL_INCLUDE_DIR}/fruit
  FILES_MATCHING PATTERN "*.h")
