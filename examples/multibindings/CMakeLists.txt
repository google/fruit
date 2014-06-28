
set(MULTIBINDINGS_SOURCES
main.cpp
)

add_executable(multibindings ${MULTIBINDINGS_SOURCES})
target_link_libraries(multibindings fruit)
