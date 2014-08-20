
set(BENCHMARK_SOURCES
benchmark.cpp
)

add_definitions("-ftemplate-depth=1000")

add_executable(benchmark ${BENCHMARK_SOURCES})
target_link_libraries(benchmark fruit)
