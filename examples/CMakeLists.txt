
add_subdirectory(simple_injection)
add_subdirectory(hello_world)

# This uses threads and doesn't work on Windows (at least when using MinGW's GCC).
if(NOT "${WIN32}")
    add_subdirectory(server)
endif()

add_subdirectory(multibindings)
add_subdirectory(scaling_doubles)
add_subdirectory(annotated_injection)
add_subdirectory(testing)
