project(injection)
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

add_definitions(-std=c++11 -W -Wall -Werror -ftemplate-backtrace-limit=0 -DFRUIT_EXTRA_DEBUG)
include_directories(${CMAKE_SOURCE_DIR}/include)

#set(CMAKE_CXX_COMPILER "/usr/bin/clang++")

set(INSTALL_LIBRARY_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_INCLUDE_DIR include CACHE PATH "Installation directory for headers")

set(FRUIT_VERSION "0.9.5")

enable_testing()

add_subdirectory(src)
add_subdirectory(extras EXCLUDE_FROM_ALL)
add_subdirectory(doc EXCLUDE_FROM_ALL)
add_subdirectory(examples EXCLUDE_FROM_ALL)
add_subdirectory(tests EXCLUDE_FROM_ALL)

install(DIRECTORY include
  DESTINATION ${INSTALL_INCLUDE_DIR}/fruit
  FILES_MATCHING PATTERN "*.h")
