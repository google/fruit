
set(SERVER_SOURCES
main.cpp
foo_handler.cpp
bar_handler.cpp
request_dispatcher.cpp
server.cpp
)

add_definitions("-pthread")

add_executable(server ${SERVER_SOURCES})
target_link_libraries(server fruit pthread)
