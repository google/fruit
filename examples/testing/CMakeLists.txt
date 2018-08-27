

set(GREETER_SOURCES
cached_greeter.cpp
cached_greeter.h
cached.h
fake_key_value_storage.cpp
fake_key_value_storage.h
greeter.cpp
greeter.h
key_value_storage.cpp
key_value_storage.h
main.cpp
)

add_executable(greeter ${GREETER_SOURCES})
target_link_libraries(greeter fruit)

# TODO: run the tests here under CMake (ATM they only run when using Bazel).
