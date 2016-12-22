
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
    --leak-check=full --malloc-fill=AA --track-origins=yes --read-var-info=yes --num-callers=50 --error-exitcode=1 --gen-suppressions=all --suppressions=${CMAKE_SOURCE_DIR}/tests/valgrind_suppressions.supp)
string(REPLACE ";" " " VALGRIND_FLAGS_STR "${VALGRIND_FLAGS}")

function(check_all_tests_listed DIRNAME)
  # Join the list with " ".
  string(REPLACE ";" " " STR "${ARGN}")
  add_test(NAME check-all-tests-listed-${DIRNAME}
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
           COMMAND bash -c -x "pwd; for f in \$(ls *.cpp); do [[ \"\$f\" == \"include_test.cpp\" ]] || echo \" ${STR} \" | fgrep -q \" \$f \" || { echo \"\$f not listed.\" && exit 1; }; done")
endfunction(check_all_tests_listed)

function(check_all_python_tests_listed DIRNAME)
  # Join the list with " ".
  string(REPLACE ";" " " STR "${ARGN}")
  add_test(NAME check-all-python-tests-listed-${DIRNAME}
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
           COMMAND bash -c -x "pwd; for f in \$(ls test_*.py); do echo \" ${STR} \" | fgrep -q \" \$f \" || { echo \"\$f not listed.\" && exit 1; }; done")
endfunction(check_all_python_tests_listed)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  add_custom_command(
        OUTPUT test_common-precompiled.h.gch
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS test_common.h fruit
        COMMAND bash -c "${CMAKE_CXX_COMPILER} -x c++-header ${FRUIT_COMPILE_FLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}/../include -I${CMAKE_CURRENT_SOURCE_DIR} -I${CMAKE_CURRENT_BINARY_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/test_common.h -o test_common-precompiled.h.gch")
  # This is not a real test, it's just a way to make sure the test_common PCH and the Fruit lib are built before all
  # tests that use it.
  add_test(
      NAME fruit-test-deps
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMAND bash -c "
          ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target fruit;
          ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target tests/test_common-precompiled.h.gch
          ")
  add_custom_target(test-common-precompiled-header DEPENDS test_common-precompiled.h.gch)
  # Note that the "test_common-precompiled.h" header doesn't exist, but it's ok because GCC looks for
  # test_common-precompiled.h.gch first. We don't call the precompiled header test_common.h.gch so that if GCC doesn't
  # find it it reports an error instead of using the normal header.
  set(FRUIT_TESTONLY_CXXFLAGS "-include${CMAKE_CURRENT_BINARY_DIR}/test_common-precompiled.h")

elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(Clang|AppleClang)$")
  add_custom_command(
          OUTPUT test_common.pch
          WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
          DEPENDS test_common.h fruit
          COMMAND bash -c "${CMAKE_CXX_COMPILER} -x c++-header ${FRUIT_COMPILE_FLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}/../include -I${CMAKE_CURRENT_SOURCE_DIR} -I${CMAKE_CURRENT_BINARY_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/test_common.h -o test_common.pch")
  # This is not a real test, it's just a way to make sure the test_common PCH and the Fruit lib are built before all
  # tests that use it.
  add_test(
      NAME fruit-test-deps
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      COMMAND bash -c "
          ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target fruit;
          ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target tests/test_common.pch
          ")
  add_custom_target(test-common-precompiled-header DEPENDS test_common.pch)
  set(FRUIT_TESTONLY_CXXFLAGS "-include-pch ${CMAKE_CURRENT_BINARY_DIR}/test_common.pch")
else()

  # This is not a real test, it's just a way to make sure the test_common PCH and the Fruit lib are built before all
  # tests that use it.
  add_test(
          NAME fruit-test-deps
          COMMAND /bin/true)
  set(FRUIT_TESTONLY_CXXFLAGS "")
endif()

configure_file("fruit_test_config.py.in" "fruit_test_config.py")
configure_file("fruit_test_common.py" "fruit_test_common.py" COPYONLY)

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
  add_dependencies(${BIN} test-common-precompiled-header)
  set_target_properties(${BIN} PROPERTIES COMPILE_FLAGS ${FRUIT_TESTONLY_CXXFLAGS})
  add_test(NAME ${BUILD} COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target ${BIN})
  set_tests_properties(${NAME} PROPERTIES DEPENDS ${BUILD})
endfunction(add_test_expected_to_pass)

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

function(add_nose_based_fruit_tests DIRNAME)
  foreach(TEST_SOURCE ${ARGN})
    string(REPLACE ".py" "" NAME ${TEST_SOURCE})
    string(REGEX REPLACE ".*/" "" NAME ${NAME})
    add_test(NAME ${NAME}
             WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
             COMMAND bash -c "
                unset PYTHONPATH
                unset PYTHONHOME
                cp ${CMAKE_CURRENT_SOURCE_DIR}/${TEST_SOURCE} ${CMAKE_CURRENT_BINARY_DIR} && \\
                ./${TEST_SOURCE}")
    set_tests_properties(${NAME} PROPERTIES DEPENDS fruit-test-deps)
  endforeach()
endfunction()

add_nose_based_fruit_tests("root"
        "test_binding_clash.py"
        "test_binding_compression.py"
        "test_bind_instance.py"
        "test_bind_interface.py"
        "test_component.py"
        "test_dependency_loop.py"
        "test_duplicated_types.py"
        "test_injected_provider.py"
        "test_injector.py"
        "test_injector_unsafe_get.py"
        "test_multibindings_bind_instance.py"
        "test_multibindings_bind_interface.py"
        "test_multibindings_bind_provider.py"
        "test_multibindings_misc.py"
        "test_normalized_component.py"
        "test_register_constructor.py"
        "test_register_factory.py"
        "test_register_instance.py"
        "test_register_provider.py"
)

add_subdirectory(data_structures)
add_subdirectory(meta)
add_subdirectory(util)
