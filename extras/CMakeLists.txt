
set(EXTRAS_FILES
debian.control
debian.rules
libfruit.dsc
libfruit.install
libfruit.spec
PKGBUILD
)

# This places configured files (build files with @FRUIT_VERSION@ replaced) in build/extras/built

foreach(F ${EXTRAS_FILES})
  configure_file(${F} built/${F} @ONLY)
endforeach(F)
