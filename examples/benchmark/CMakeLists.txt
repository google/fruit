
# Note that this ignores most CMake flags used during the build, e.g. debug options. 
add_custom_command(OUTPUT new_delete_benchmark-exec
                   COMMAND ${CMAKE_CXX_COMPILER} -std=c++11 -O2 -g -W -Wall -Werror -DNDEBUG -I${CMAKE_CURRENT_SOURCE_DIR}/../../include ${CMAKE_CURRENT_SOURCE_DIR}/new_delete_benchmark.cpp -o new_delete_benchmark)

add_custom_target(new_delete_benchmark ALL
                  DEPENDS new_delete_benchmark-exec)

add_executable(generate_benchmark generate_benchmark.cpp)

add_custom_command(OUTPUT main.cpp build.sh
                   COMMAND $<TARGET_FILE:generate_benchmark> ${CMAKE_CXX_COMPILER} ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} # Generate *.h, *.cpp, build.sh
                   DEPENDS generate_benchmark fruit)

add_custom_command(OUTPUT main
                   COMMAND bash build.sh
                   DEPENDS main.cpp build.sh)

add_custom_target(benchmark DEPENDS main)

# This is just to help IDEs (e.g. CLion) figure out how new_delete_benchmark.cpp is supposed to be built.
add_executable(new_delete_benchmark-dummy-exec EXCLUDE_FROM_ALL new_delete_benchmark.cpp)
target_link_libraries(new_delete_benchmark-dummy-exec fruit)
