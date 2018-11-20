# You can set your own route by setting FRUIT_INSTALLED_DIR:
# set(ENV{FRUIT_INSTALLED_DIR} "/path/to/fruit/build")

find_path(FRUIT_INCLUDE_DIR fruit.h
    HINTS (
        FRUIT_INSTALLED_DIR
        /usr
        /usr/local
    )
    PATH_SUFFIXES include/fruit include fruit
)

find_library(FRUIT_LIBRARY
  NAMES fruit
  HINTS (
      FRUIT_INSTALLED_DIR
      /usr
      /usr/local
  )
  PATH_SUFFIXES lib ${FRUIT_INSTALLED_DIR}
)

include(${CMAKE_ROOT}/Modules/FindPackageHandleStandardArgs.cmake)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(fruit DEFAULT_MSG FRUIT_LIBRARY FRUIT_INCLUDE_DIR)
