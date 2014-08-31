
add_library(fruit SHARED
demangle_type_name.cpp
component_storage.cpp
injector_storage.cpp)

install(TARGETS fruit
   LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR})
