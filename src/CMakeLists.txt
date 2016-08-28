add_library(fruit
binding_normalization.cpp
demangle_type_name.cpp
component.cpp
component_storage.cpp
fixed_size_allocator.cpp
greedy_allocator_storage.cpp
injector_storage.cpp
normalized_component_storage.cpp
normalized_component_storage_holder.cpp
semistatic_map.cpp
semistatic_graph.cpp
)
if(UNIX AND NOT APPLE)
    target_link_libraries(fruit supc++)
endif()

if(${BUILD_SHARED_LIBS})
    install(TARGETS fruit
        LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR})
else()
    install(TARGETS fruit
        ARCHIVE DESTINATION ${INSTALL_LIBRARY_DIR})
endif()
