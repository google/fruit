
set(FRUIT_SOURCES
    memory_pool.cpp
    binding_normalization.cpp
    demangle_type_name.cpp
    component.cpp
    fixed_size_allocator.cpp
    injector_storage.cpp
    normalized_component_storage.cpp
    normalized_component_storage_holder.cpp
    semistatic_map.cpp
    semistatic_graph.cpp)

if(BUILD_SHARED_LIBS)
    add_library(fruit SHARED ${FRUIT_SOURCES})

    set_target_properties(fruit PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
else()
    add_library(fruit STATIC ${FRUIT_SOURCES})
endif()

target_include_directories(fruit PUBLIC ${FRUIT_INCLUDE_DIRS})
target_compile_options(fruit PUBLIC ${FRUIT_ADDITIONAL_COMPILE_FLAGS})

if(FRUIT_USES_BOOST)
    find_package(Boost REQUIRED)
    target_include_directories(fruit PRIVATE ${Boost_INCLUDE_DIRS})
endif()

install(TARGETS fruit
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
