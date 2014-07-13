
set(INCREMENTER_SOURCES
checked_adder.cpp
checked_incrementer.cpp
incrementer_impl.cpp
incrementer_component.cpp
simple_adder.cpp
simple_incrementer.cpp
)

add_executable(incrementer_v1 ${INCREMENTER_SOURCES} main_v1.cpp)
add_executable(incrementer ${INCREMENTER_SOURCES} main.cpp)
target_link_libraries(incrementer_v1 fruit)
target_link_libraries(incrementer fruit)
