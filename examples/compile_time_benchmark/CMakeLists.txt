
add_custom_target(compile_time_benchmark
                  COMMAND echo Release
                  COMMAND time ${CMAKE_CXX_COMPILER} -std=c++11 -O2 -W -Wall -Werror -DNDEBUG -ftemplate-depth=1000 -I${CMAKE_CURRENT_SOURCE_DIR}/../../include -c ${CMAKE_CURRENT_SOURCE_DIR}/module.cpp -o module.o
                  COMMAND echo Debug
                  COMMAND time ${CMAKE_CXX_COMPILER} -std=c++11 -Os -g -W -Wall -Werror -ftemplate-depth=1000 -I${CMAKE_CURRENT_SOURCE_DIR}/../../include -c ${CMAKE_CURRENT_SOURCE_DIR}/module.cpp -o module.o
                  )
