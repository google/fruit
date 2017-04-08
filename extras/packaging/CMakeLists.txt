
set(PACKAGING_FILES
debian.control
debian.rules
debian.compat
libfruit.dsc
libfruit.install
libfruit.spec
)

# This places configured files (build files with @FRUIT_VERSION@ replaced) in build/extras/packaging/built

foreach(F ${PACKAGING_FILES})
  configure_file(${F} built/${F} @ONLY)
endforeach(F)

configure_file(PKGBUILD PKGBUILD-template @ONLY)

add_custom_target(fruit-${FRUIT_VERSION}.tar.gz ALL
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/PKGBUILD-template
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
                  COMMAND git archive -o ${CMAKE_CURRENT_BINARY_DIR}/built/fruit-${FRUIT_VERSION}.tar.gz --prefix=fruit-${FRUIT_VERSION}/ HEAD
                  COMMAND md5sum ${CMAKE_CURRENT_BINARY_DIR}/built/fruit-${FRUIT_VERSION}.tar.gz | awk '{print $$1}' >${CMAKE_CURRENT_BINARY_DIR}/tarball-md5
                  COMMAND sed "\"s/.*md5sums.*/md5sums=(`cat" "${CMAKE_CURRENT_BINARY_DIR}/tarball-md5`)/\"" <${CMAKE_CURRENT_BINARY_DIR}/PKGBUILD-template >${CMAKE_CURRENT_BINARY_DIR}/built/PKGBUILD)
