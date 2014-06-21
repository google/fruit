
set(INCREMENTER_SOURCES
main.cpp
checked_adder.cpp
checked_incrementer.cpp
incrementer_impl.cpp
simple_adder.cpp
simple_incrementer.cpp
)

add_executable(incrementer ${INCREMENTER_SOURCES})
target_link_libraries(incrementer fruit)
