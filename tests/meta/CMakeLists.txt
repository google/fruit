set(METAPROGRAMMING_TEST_SOURCES
basics.cpp
algos.cpp
list.cpp
vector.cpp
set.cpp
map.cpp
graph.cpp
proof_trees.cpp
)

add_fruit_tests(metaprogramming ${METAPROGRAMMING_TEST_SOURCES})
check_all_tests_listed(metaprogramming ${METAPROGRAMMING_TEST_SOURCES})
