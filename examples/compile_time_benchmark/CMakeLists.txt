
add_custom_target(compile_time_benchmark
                  COMMAND time ${CMAKE_CXX_COMPILER} -std=c++11 -O2 -g -W -Wall -Werror -DNDEBUG -ftemplate-depth=1000 -I${CMAKE_CURRENT_SOURCE_DIR}/../../include -c ${CMAKE_CURRENT_SOURCE_DIR}/module.cpp -o module.o)
