set(TEST_SOURCES
binding_error_not_base.cpp
component_copy.cpp
dep_canonicalization.cpp
dependency_loop.cpp
empty_injector.cpp
explicit_provider_injection.cpp
factory_bindings.cpp
incompatible_double_injection.cpp
injector_with_requirements.cpp
malformed_inject_typedef.cpp
multibindings.cpp
multibinding_error_not_base.cpp
no_binding_found.cpp
no_repeated_types_in_component.cpp
no_repeated_types_in_component2.cpp
no_repeated_types_in_component3.cpp
nonclass_type_in_injector.cpp
nonclass_type_in_component.cpp
normalized_component_and_component_with_requirements.cpp
normalized_component_successful.cpp
normalized_component_type_in_injector_not_provided.cpp
register_constructor.cpp
register_constructor_malformed_signature.cpp
register_constructor_not_existing.cpp
register_factory.cpp
register_factory_not_existing_constructor1.cpp
register_factory_not_existing_constructor2.cpp
register_multibinding_provider.cpp
register_multibinding_provider_error_not_function.cpp
register_provider.cpp
register_provider_error_not_function.cpp
register_instance.cpp
register_instance_error1.cpp
register_instance_error2.cpp
register_instance_error3.cpp
test1.cpp
type_already_bound.cpp
type_already_bound2.cpp
type_not_provided.cpp
unsatisfied_requirements_in_normalized_component.cpp
wrong_class_in_inject_typedef.cpp
)

set(FRUIT_PUBLIC_HEADERS
"component"
"fruit"
"fruit_forward_decls"
"injector"
"macro"
"normalized_component"
"provider"
)

function(add_successful_compile NAME)
  set(BIN ${NAME}-exec)
  set(BUILD ${NAME}-build)
  add_executable(${BIN} ${NAME}.cpp)
  target_link_libraries(${BIN} fruit)
  add_test(NAME ${BUILD} COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target ${BIN})
  set_tests_properties(${NAME} PROPERTIES DEPENDS ${BUILD})
endfunction(add_successful_compile)

function(add_test_expected_to_pass NAME)
  add_test(NAME ${NAME}
      COMMAND valgrind --leak-check=full --show-leak-kinds=all --malloc-fill=AA --track-origins=yes --read-var-info=yes --num-callers=50 --error-exitcode=1 $<TARGET_FILE:${NAME}-exec>)
  add_successful_compile(${NAME})
endfunction(add_test_expected_to_pass)

function(add_test_expected_to_fail_at_runtime NAME MESSAGE)
  add_test(NAME ${NAME}
      COMMAND bash -c "
      F=`mktemp`
      if $<TARGET_FILE:${NAME}-exec> &>\${F}; then
        cat \$F | fgrep error | head -n 1
        rm -f \${F}
        echo 'Expected runtime error but the test passed.'
        exit 1
      fi
      if ! cat \$F | fgrep error | head -n 1 | grep -q '${MESSAGE}'; then
        cat \$F | fgrep error | head -n 1
        rm -f \${F}
        echo 'The test failed as expected, but with a different message. Expected: ${MESSAGE}'
        exit 1
      fi
      rm -f \${F}
      ")
  add_successful_compile(${NAME})
endfunction(add_test_expected_to_fail_at_runtime)

set(COMPILERS_TO_TEST "g++" "clang++")
set(TEST_COMPILE_FLAGS "-std=c++11 -W -Wall -Werror -DFRUIT_EXTRA_DEBUG")

function(add_test_expected_to_fail_at_compile_time NAME MESSAGE)
  foreach(COMPILER ${COMPILERS_TO_TEST})
    set(COMMAND "
      F=`mktemp`
      if ${COMPILER} ${TEST_COMPILE_FLAGS} -c ${NAME}.cpp -I ../include -o - >/dev/null 2>\${F}; then
        cat \${F}
        rm -f \${F}
        echo 'Expected compile error but the test passed.'
        exit 1
        fi
      if ! cat \${F} | fgrep 'error:' | head -n 1 | grep -q '${MESSAGE}'; then
        cat \${F}
        rm -f \${F}
        echo 'The test failed as expected, but with a different message. Expected: ${MESSAGE}'
        exit 1
      fi
      rm -f \${F}
      ")
    add_test(NAME ${NAME}-${COMPILER}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND bash -c "${COMMAND}")
  endforeach(COMPILER)
endfunction(add_test_expected_to_fail_at_compile_time)

function(add_test_autodetect FILENAME)
  file(STRINGS ${FILENAME}.cpp FIRST_LINE LIMIT_COUNT 1)
  if (FIRST_LINE MATCHES "// expect-success")
    add_test_expected_to_pass(${FILENAME})
  elseif(FIRST_LINE MATCHES "// expect-compile-error *")
    string(REPLACE "// expect-compile-error " "" EXPECTED_ERROR_PATTERN ${FIRST_LINE})
    add_test_expected_to_fail_at_compile_time(${FILENAME} ${EXPECTED_ERROR_PATTERN})
  elseif(FIRST_LINE MATCHES "// expect-runtime-error *")
    string(REPLACE "// expect-runtime-error " "" EXPECTED_ERROR_PATTERN ${FIRST_LINE})
    add_test_expected_to_fail_at_runtime(${FILENAME} ${EXPECTED_ERROR_PATTERN})
  else()
    message(FATAL_ERROR "The test ${FILENAME} doesn't start with a '// expect-{success,compile-error,runtime-error}' comment")
  endif()
endfunction(add_test_autodetect)

foreach(TEST_SOURCE ${TEST_SOURCES})
  string(REPLACE ".cpp" "" TEST_NAME ${TEST_SOURCE})
  string(REGEX REPLACE ".*/" "" TEST_NAME ${TEST_NAME})
  add_test_autodetect(${TEST_NAME})
  # Forces CMake to re-run if a test file changes.
  configure_file(${TEST_SOURCE} .${TEST_SOURCE}.unused)
endforeach(TEST_SOURCE)

foreach(HEADER ${FRUIT_PUBLIC_HEADERS})
  add_library(test-header-${HEADER}-compiles "include_test.cpp")
  set_target_properties(test-header-${HEADER}-compiles PROPERTIES
    COMPILE_FLAGS "-DHEADER_UNDER_TEST='<fruit/${HEADER}.h>'")
endforeach(HEADER)
