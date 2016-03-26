add_library(fruit
demangle_type_name.cpp
component_storage.cpp
injector_storage.cpp
normalized_component_storage.cpp
semistatic_map.cpp
)

if(${BUILD_SHARED_LIBS})
    install(TARGETS fruit
        LIBRARY DESTINATION ${INSTALL_LIBRARY_DIR})
else()
    install(TARGETS fruit
        ARCHIVE DESTINATION ${INSTALL_LIBRARY_DIR})
endif()
