
set(HELLO_WORLD_SOURCES
main.cpp
)

add_executable(hello ${HELLO_WORLD_SOURCES})
target_link_libraries(hello fruit)
