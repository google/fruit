
add_library(fruit SHARED
demangle_type_name.cpp
unsafe_module.cpp)

install(TARGETS fruit
   LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR})
