
add_library(fruit demangle_type_name.cpp unsafe_module.cpp)

install(TARGETS fruit
  ARCHIVE DESTINATION lib/static)
