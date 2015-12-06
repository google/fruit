set(DATA_STRUCTURES_TEST_SOURCES
semistatic_map.cpp 
semistatic_graph.cpp 
hybrid_vector.cpp
fixed_size_vector.cpp 
fixed_size_allocator.cpp 
)

add_fruit_tests(data-structures ${DATA_STRUCTURES_TEST_SOURCES})
check_all_tests_listed(data-structures ${DATA_STRUCTURES_TEST_SOURCES})
