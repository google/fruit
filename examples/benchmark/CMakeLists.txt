
add_executable(generate_benchmark generate_benchmark.cpp)

add_custom_command(OUTPUT main.cpp build.sh
                   COMMAND $<TARGET_FILE:generate_benchmark> ${CMAKE_CXX_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/../.. # Generate *.h, *.cpp, build.sh
                   DEPENDS generate_benchmark fruit)

add_custom_command(OUTPUT main
                   COMMAND bash build.sh
                   DEPENDS main.cpp build.sh)

add_custom_target(benchmark DEPENDS main bench.sh)

# For convenience, so that bench.sh ends up in the build directory.
configure_file(bench.sh bench.sh)
