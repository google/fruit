
add_custom_target(compile_time_benchmark
                  COMMAND echo Release
                  COMMAND time ${CMAKE_CXX_COMPILER} -std=c++11 -O2 -W -Wall -Werror -DNDEBUG -ftemplate-depth=1000 -I${CMAKE_CURRENT_SOURCE_DIR}/../../include -I${CMAKE_BINARY_DIR}/include -c ${CMAKE_CURRENT_SOURCE_DIR}/module.cpp -o module.o
                  COMMAND echo "Release \"(\"no dep check\")\""
                  COMMAND time ${CMAKE_CXX_COMPILER} -std=c++11 -O2 -W -Wall -Werror -DFRUIT_NO_LOOP_CHECK -DNDEBUG -ftemplate-depth=1000 -I${CMAKE_CURRENT_SOURCE_DIR}/../../include -I${CMAKE_BINARY_DIR}/include -c ${CMAKE_CURRENT_SOURCE_DIR}/module.cpp -o module.o
                  COMMAND echo "Metaprogramming instantiation stats \"(\"no dep check\")\""
                  COMMAND ${CMAKE_CXX_COMPILER} -std=c++11 -O2 -W -Wall -Werror -DFRUIT_TRACE_INSTANTIATIONS -DFRUIT_NO_LOOP_CHECK -DNDEBUG -ftemplate-depth=1000 -I${CMAKE_CURRENT_SOURCE_DIR}/../../include -I${CMAKE_BINARY_DIR}/include -c ${CMAKE_CURRENT_SOURCE_DIR}/module.cpp -o module.o 2>&1 | fgrep "Werror=deprecated-declarations" | sed "'s/.*with MetaFun = //;s/ MetaExprs =.*//;s/.*with MetaExpr =.*/constant/;s/.*static constexpr bool fruit::impl::meta::DoEval<fruit::impl::meta::If.*/If1/;s/.*static constexpr bool fruit::impl::meta::EvalIf.*/If2/'" | sed "'s/fruit::impl::meta:://g'" | sed "'s/::apply<.*>/::apply<...>/'" | sort | uniq -c | sort -n
                  #COMMAND echo Debug
                  #COMMAND time ${CMAKE_CXX_COMPILER} -std=c++11 -Os -g -W -Wall -Werror -ftemplate-depth=1000 -I${CMAKE_CURRENT_SOURCE_DIR}/../../include -c ${CMAKE_CURRENT_SOURCE_DIR}/module.cpp -o module.o
                  #COMMAND echo "Debug \"(\"no dep check\")\""
                  #COMMAND time ${CMAKE_CXX_COMPILER} -std=c++11 -Os -g -W -Wall -Werror -DFRUIT_NO_LOOP_CHECK -ftemplate-depth=1000 -I${CMAKE_CURRENT_SOURCE_DIR}/../../include -c ${CMAKE_CURRENT_SOURCE_DIR}/module.cpp -o module.o
                  )

# This is just to help IDEs (e.g. CLion) figure out how module.cpp is supposed to be built.
add_executable(compile_time_benchmark_executable EXCLUDE_FROM_ALL module.cpp)
target_link_libraries(compile_time_benchmark_executable fruit)
