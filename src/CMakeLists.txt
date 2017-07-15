
set(FRUIT_SOURCES
demangle_type_name.cpp
component_storage.cpp
injector_storage.cpp
normalized_component_storage.cpp
semistatic_map.cpp
)

if("${BUILD_SHARED_LIBS}")
    add_library(fruit SHARED ${FRUIT_SOURCES})
else()
    add_library(fruit STATIC ${FRUIT_SOURCES})
endif()

install(TARGETS fruit
        ARCHIVE DESTINATION "${INSTALL_LIBRARY_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIBRARY_DIR}")

if("${UNIX}" AND NOT "${APPLE}")
    target_link_libraries(fruit supc++)
endif()

