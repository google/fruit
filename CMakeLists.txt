project(injection)
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_definitions(-std=c++11 -W -Wall -Werror -ftemplate-backtrace-limit=0 -DFRUIT_EXTRA_DEBUG)
include_directories(${CMAKE_SOURCE_DIR}/include)

set(CXX clang++)

enable_testing()

add_subdirectory(src)
add_subdirectory(examples EXCLUDE_FROM_ALL)
add_subdirectory(tests EXCLUDE_FROM_ALL)

install(DIRECTORY include
  DESTINATION include/fruit
  FILES_MATCHING PATTERN "*.h")
