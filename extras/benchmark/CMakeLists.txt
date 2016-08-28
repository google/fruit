
# This is just to help IDEs (e.g. CLion) figure out how new_delete_benchmark.cpp is supposed to be built.
add_executable(new_delete_benchmark-dummy-exec EXCLUDE_FROM_ALL new_delete_benchmark.cpp)
target_link_libraries(new_delete_benchmark-dummy-exec fruit)

# This is just to help IDEs (e.g. CLion) figure out how compile_time_benchmark.cpp is supposed to be built.
add_executable(compile_time_benchmark_executable EXCLUDE_FROM_ALL compile_time_benchmark.cpp)
target_link_libraries(compile_time_benchmark_executable fruit)
