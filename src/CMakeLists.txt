
set(FRUIT_SOURCES
binding_normalization.cpp
demangle_type_name.cpp
component.cpp
component_storage.cpp
fixed_size_allocator.cpp
injector_storage.cpp
normalized_component_storage.cpp
normalized_component_storage_holder.cpp
semistatic_map.cpp
semistatic_graph.cpp)

if("${BUILD_SHARED_LIBS}")
    add_library(fruit SHARED ${FRUIT_SOURCES})

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
        set_target_properties(fruit PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    endif()
else()
    add_library(fruit STATIC ${FRUIT_SOURCES})
endif()

install(TARGETS fruit
        ARCHIVE DESTINATION "${INSTALL_LIBRARY_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIBRARY_DIR}")

if("${UNIX}" AND NOT "${APPLE}")
    target_link_libraries(fruit supc++)
endif()

