
set(SCALING_SOURCES
main.cpp
scaler.cpp
multiplier.cpp
)

add_executable(scaling ${SCALING_SOURCES})
target_link_libraries(scaling fruit)
