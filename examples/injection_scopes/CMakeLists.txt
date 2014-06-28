
set(INJECTION_SCOPES_SOURCES
main.cpp
parsed_request.cpp
request_handler.cpp
server.cpp
)

add_executable(injection_scopes ${INJECTION_SCOPES_SOURCES})
target_link_libraries(injection_scopes fruit)
