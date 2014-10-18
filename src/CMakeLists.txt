
add_library(fruit SHARED
demangle_type_name.cpp
component_storage.cpp
injector_storage.cpp
lambda_invoker.cpp
normalized_component_storage.cpp
semistatic_map.cpp
)

install(TARGETS fruit
   LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR})
