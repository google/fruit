
# This is just to help IDEs (e.g. CLion) figure out how new_delete_benchmark.cpp is supposed to be built.
add_executable(new_delete_benchmark-dummy-exec EXCLUDE_FROM_ALL new_delete_benchmark.cpp)
target_link_libraries(new_delete_benchmark-dummy-exec fruit)
