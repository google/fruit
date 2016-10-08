
include(CMakeParseArguments)

set(FRUIT_PUBLIC_HEADERS
"component"
"fruit"
"fruit_forward_decls"
"injector"
"macro"
"normalized_component"
"provider"
)

if(${WIN32})
  # No timeout on windows, the `timeout' executable has a different syntax.
  set(TIMEOUT_COMMAND_PREFIX "")
  set(TIMEOUT_COMMAND_PREFIX_STR "")
else()
  set(TIMEOUT_COMMAND_PREFIX "timeout" "30")
  set(TIMEOUT_COMMAND_PREFIX_STR "timeout 30")
endif()

set(VALGRIND_FLAGS
    "--leak-check=full --malloc-fill=AA --track-origins=yes --read-var-info=yes --num-callers=50 --error-exitcode=1 --gen-suppressions=all --suppressions=${CMAKE_SOURCE_DIR}/tests/valgrind_suppressions.supp")

function(add_test_expected_to_pass TEST_SOURCE)
  string(REPLACE ".cpp" "" NAME ${TEST_SOURCE})
  string(REGEX REPLACE ".*/" "" NAME ${NAME})
  if (${RUN_TESTS_UNDER_VALGRIND})
    # Re-add --show-leak-kinds=all here once Travis CI will have a new-enough version of valgrind.
    add_test(NAME ${NAME}
        COMMAND ${TIMEOUT_COMMAND_PREFIX} valgrind ${VALGRIND_FLAGS} $<TARGET_FILE:${NAME}-exec>)
  else()
    add_test(NAME ${NAME}
             COMMAND ${TIMEOUT_COMMAND_PREFIX} $<TARGET_FILE:${NAME}-exec>)
  endif()
  set(BIN ${NAME}-exec)
  set(BUILD ${NAME}-build)
  add_executable(${BIN} ${TEST_SOURCE})
  target_link_libraries(${BIN} fruit)
  add_test(NAME ${BUILD} COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target ${BIN})
  set_tests_properties(${NAME} PROPERTIES DEPENDS ${BUILD})
endfunction(add_test_expected_to_pass)

function(check_all_tests_listed DIRNAME)
  # Join the list with " ".
  string(REPLACE ";" " " STR "${ARGN}")
  message("In check_all_tests_listed, STR=${STR}, command: bash -c for f in \$(ls *.cpp); do [[ \"\$f\" == \"include_test.cpp\" ]] || echo \" ${STR} \" | fgrep -q \" \$f \" || { echo \"\$f not listed.\" && exit 1; }; done")
  add_test(NAME check-all-tests-listed-${DIRNAME}
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
           COMMAND bash -c -x "pwd; for f in \$(ls *.cpp); do [[ \"\$f\" == \"include_test.cpp\" ]] || echo \" ${STR} \" | fgrep -q \" \$f \" || { echo \"\$f not listed.\" && exit 1; }; done")
endfunction(check_all_tests_listed)

function(add_fruit_tests DIRNAME)
  foreach(TEST_SOURCE ${ARGN})
    add_test_expected_to_pass(${TEST_SOURCE})
  endforeach(TEST_SOURCE)
  check_all_tests_listed(${DIRNAME} ${ARGN})
endfunction()

add_fruit_tests("root"
        class_destruction.cpp
        class_destruction_with_annotation.cpp
        eager_injection.cpp
        install_component_swap_optimization.cpp
        semistatic_map_hash_selection.cpp
        test1.cpp
        type_alignment.cpp
        type_alignment_with_annotation.cpp
)

foreach(HEADER ${FRUIT_PUBLIC_HEADERS})
  add_library(test-header-${HEADER}-compiles "include_test.cpp")
  set_target_properties(test-header-${HEADER}-compiles PROPERTIES
    COMPILE_FLAGS "-DHEADER_UNDER_TEST='<fruit/${HEADER}.h>'")
endforeach(HEADER)

# This test doesn't work on OS X.
IF (NOT ${CMAKE_SYSTEM_NAME} MATCHES "Darwin") 
    add_test(NAME check-defn-header-includes
             WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/include
             COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/check_defn_h_includes.sh)
endif()


set(COMMAND "
  '${CMAKE_COMMAND}' --build '${CMAKE_BINARY_DIR}' --target fruit
  export CXX='${CMAKE_CXX_COMPILER}'
  export CXXFLAGS='${FRUIT_COMPILE_FLAGS} -std=c++11 -I${CMAKE_SOURCE_DIR}/include -I${CMAKE_SOURCE_DIR}/tests -I${CMAKE_BINARY_DIR}/include -DFRUIT_DEBUG -DFRUIT_EXTRA_DEBUG -D_GLIBCXX_DEBUG'
  export LDFLAGS='${CMAKE_EXE_LINKER_FLAGS} -L${CMAKE_BINARY_DIR}/src -Wl,-rpath,${CMAKE_BINARY_DIR}/src'
  export RUN_TESTS_UNDER_VALGRIND='${RUN_TESTS_UNDER_VALGRIND}'
  export VALGRIND_FLAGS='${VALGRIND_FLAGS}'
  nose2-3
")
add_test(NAME nose2-tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND bash -c "${COMMAND}")

add_subdirectory(data_structures)
add_subdirectory(meta)
add_subdirectory(util)
