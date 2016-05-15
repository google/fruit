
set(ANNOTATED_INJECTION_SOURCES
main.cpp
car.cpp
main_brake.cpp
emergency_brake.cpp
)

add_executable(annotated_injection ${ANNOTATED_INJECTION_SOURCES})
target_link_libraries(annotated_injection fruit)
