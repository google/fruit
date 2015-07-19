set(TEST_SOURCES
binding_compression_undone.cpp
binding_error_not_base.cpp
binding_error_not_base_with_annotations.cpp
bind_instance_error_already_bound.cpp
bind_instance_error_already_bound_with_annotation.cpp
bind_instance_error_already_bound_with_different_annotation.cpp
class_destruction.cpp
class_destruction_with_annotation.cpp
component_conversion.cpp
component_conversion_with_annotation.cpp
component_copy.cpp
component_copy_with_annotations.cpp
component_repeated_type.cpp
component_repeated_type_with_annotations.cpp
component_repeated_type_with_different_annotations.cpp
component_type_required_and_provided.cpp
component_type_required_and_provided_with_annotations.cpp
component_type_required_and_provided_with_different_annotations.cpp
dependency_loop.cpp
dependency_loop_with_annotations.cpp
dependency_loop_with_different_annotations.cpp
duplicated_types_normalized_component.cpp
duplicated_types_normalized_component_with_annotations.cpp
duplicated_types_normalized_component_with_different_annotations.cpp
eager_injection.cpp
empty_injector.cpp
explicit_provider_injection.cpp
explicit_provider_injection_with_annotation.cpp
factory_bindings.cpp
factory_bindings_with_annotation.cpp
incompatible_double_injection.cpp
incompatible_double_injection_with_annotations.cpp
incompatible_double_injection_with_different_annotations.cpp
injector_duplicated_type.cpp
injector_duplicated_type_with_annotation.cpp
injector_duplicated_type_with_different_annotation.cpp
injector_non_class_type.cpp
injector_non_class_type_with_annotation.cpp
injector_non_class_type_with_different_annotation.cpp
injector_with_requirements.cpp
injector_with_requirements_with_annotations.cpp
install_component_swap_optimization.cpp
late_binding_duplicate_consistent.cpp
late_binding_duplicate_consistent_with_annotation.cpp
late_binding_duplicate_error.cpp
late_binding_duplicate_error_with_annotations.cpp
late_binding_duplicate_error_with_different_annotations.cpp
late_binding_new.cpp
late_binding_new_with_annotation.cpp
late_multibinding.cpp
late_multibinding_with_annotation.cpp
lazy_injection.cpp
lazy_injection_with_annotations.cpp
lazy_injection_with_annotations2.cpp
malformed_inject_typedef.cpp
multibinding_error_not_base.cpp
multibinding_error_not_base_with_annotations.cpp
multibindings_bind_instance.cpp
multibindings_bind_instances.cpp
multibindings_bind_instances_with_annotation.cpp
multibindings_bind_instance_with_annotation.cpp
multibindings.cpp
multibindings_get_none.cpp
multibindings_value_provider.cpp
multibindings_value_provider_with_annotation.cpp
multibindings_with_annotations.cpp
no_binding_found.cpp
no_binding_found_with_annotation.cpp
no_factory_binding_found.cpp
no_factory_binding_found_with_annotation.cpp
nonclass_type_in_component.cpp
nonclass_type_in_component_with_annotation.cpp
nonclass_type_in_injector.cpp
nonclass_type_in_injector_with_annotation.cpp
no_repeated_types_in_component2.cpp
no_repeated_types_in_component2_with_annotations.cpp
no_repeated_types_in_component3.cpp
no_repeated_types_in_component3_with_annotation.cpp
no_repeated_types_in_component3_with_different_annotation.cpp
no_repeated_types_in_component.cpp
no_repeated_types_in_component_with_annotations.cpp
no_repeated_types_in_component_with_different_annotations.cpp
normalized_component_and_component_with_requirements.cpp
normalized_component_and_component_with_requirements_with_annotation.cpp
normalized_component_repeated_type.cpp
normalized_component_repeated_type_with_annotation.cpp
normalized_component_repeated_type_with_different_annotation.cpp
normalized_component_successful.cpp
normalized_component_successful_with_annotations.cpp
normalized_component_type_in_injector_not_provided.cpp
normalized_component_type_in_injector_not_provided_with_annotation.cpp
normalized_component_type_required_and_provided.cpp
normalized_component_type_required_and_provided_with_annotation.cpp
provider_get_never_provided.cpp
provider_get_not_provided.cpp
provider_get_ok.cpp
provider_get_ok_with_annotation.cpp
provider_non_class_type.cpp
provider_type_with_annotation.cpp
register_constructor.cpp
register_constructor_malformed_signature_autoinject.cpp
register_constructor_malformed_signature.cpp
register_constructor_not_existing_autoinject.cpp
register_constructor_not_existing_autoinject_with_annotation.cpp
register_constructor_not_existing.cpp
register_constructor_not_existing_with_annotation.cpp
register_constructor_with_annotation.cpp
register_factory_autoinject.cpp
register_factory_autoinject_from_provider.cpp
register_factory_autoinject_from_provider_with_annotation.cpp
register_factory_autoinject_with_binding.cpp
register_factory_autoinject_with_binding_with_annotation.cpp
register_factory.cpp
register_factory_error_not_function.cpp
register_factory_for_pointer.cpp
register_factory_for_pointer_with_annotation.cpp
register_factory_for_unique_pointer.cpp
register_factory_for_unique_pointer_with_annotation.cpp
register_factory_inconsistent_signature.cpp
register_factory_inconsistent_signature_with_annotations.cpp
register_factory_not_existing_constructor1.cpp
register_factory_not_existing_constructor1_with_annotation.cpp
register_factory_not_existing_constructor2.cpp
register_factory_not_existing_constructor2_with_annotation.cpp
register_factory_with_annotation.cpp
register_factory_with_different_annotation.cpp
register_factory_with_different_annotation_error.cpp
register_instance.cpp
register_instance_error1.cpp
register_instance_error1_with_annotations.cpp
register_instance_error2.cpp
register_instance_error2_with_annotation.cpp
register_instance_error3.cpp
register_instance_error3_with_annotation.cpp
register_instance_with_annotation.cpp
register_multibinding_provider.cpp
register_multibinding_provider_error_not_function.cpp
register_multibinding_provider_error_not_function_with_annotation.cpp
register_multibinding_provider_error_returned_nullptr.cpp
register_multibinding_provider_error_returned_nullptr_with_annotation.cpp
register_multibinding_provider_with_annotation.cpp
register_provider.cpp
register_provider_error_not_function.cpp
register_provider_error_not_function_with_annotation.cpp
register_provider_error_returned_nullptr.cpp
register_provider_with_annotation.cpp
register_provider_with_binding_compression.cpp
register_provider_with_binding_compression_with_annotation.cpp
semistatic_map_hash_selection.cpp
test1.cpp
type_alignment.cpp
type_alignment_with_annotation.cpp
type_already_bound2.cpp
type_already_bound2_with_annotation.cpp
type_already_bound2_with_different_annotation.cpp
type_already_bound3.cpp
type_already_bound3_with_annotation.cpp
type_already_bound3_with_different_annotation.cpp
type_already_bound.cpp
type_already_bound_during_component_merge.cpp
type_already_bound_during_component_merge_with_annotations.cpp
type_already_bound_during_component_merge_with_different_annotation.cpp
type_already_bound_with_annotation.cpp
type_already_bound_with_different_annotation.cpp
type_not_provided.cpp
type_not_provided_with_annotation.cpp
unsafe_get.cpp
unsafe_get_with_annotations.cpp
unsatisfied_requirements_in_normalized_component.cpp
unsatisfied_requirements_in_normalized_component_with_annotation.cpp
wrong_class_in_inject_typedef.cpp
wrong_class_in_inject_typedef_with_annotation.cpp
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

set(RUN_TESTS_UNDER_VALGRIND TRUE CACHE BOOL "Whether to run Fruit tests under valgrind")

function(add_test_expected_to_pass NAME)
  if (${RUN_TESTS_UNDER_VALGRIND})
    # Re-add --show-leak-kinds=all here once Travis CI will have a new-enough version of valgrind.
    add_test(NAME ${NAME}
        COMMAND valgrind --leak-check=full --malloc-fill=AA --track-origins=yes --read-var-info=yes --num-callers=50 --error-exitcode=1 $<TARGET_FILE:${NAME}-exec>)
  else()
    add_test(NAME ${NAME}
        COMMAND $<TARGET_FILE:${NAME}-exec>)
  endif()
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

set(TEST_COMPILE_FLAGS "-std=c++11 -O2 -W -Wall -Werror -DFRUIT_EXTRA_DEBUG -D_GLIBCXX_DEBUG")

function(add_test_expected_to_fail_at_compile_time NAME MESSAGE)
  set(COMMAND "
    F=`mktemp`
    if ${CMAKE_CXX_COMPILER} ${TEST_COMPILE_FLAGS} -c ${NAME}.cpp -I ../include -o - >/dev/null 2>\${F}; then
      cat \${F}
      rm -f \${F}
      echo 'Expected compile error but the test passed.'
      exit 1
      fi
    ERROR_TYPE=\"\$(cat \${F} | egrep -on 'fruit::impl::(.*Error<.*>)' | head -n 1 | tr -d ' ')\"
    ERROR_TYPE_LINE=\"\$(echo \$ERROR_TYPE | awk -F: '{print $1}')\"
    ERROR_TYPE_MESSAGE=\"\$(echo \$ERROR_TYPE | sed 's/[0-9]*:fruit::impl:://')\"
    STATIC_ASSERT_ERROR=\"\$(cat \${F} | grep -n 'static.assert' | head -n 1)\"
    STATIC_ASSERT_ERROR_LINE=\"\$(echo \$STATIC_ASSERT_ERROR | awk -F: '{print $1}')\"
    STATIC_ASSERT_ERROR_MESSAGE=\"\$(echo \$STATIC_ASSERT_ERROR | sed 's/.*static.assert failed[^A-Za-z0-9]*//;s/\"//')\"
    EXPECTED_ERROR_TYPE=\"\$(echo '${MESSAGE}' | awk -F'|' '{print $1}')\"
    EXPECTED_STATIC_ASSERT_ERROR=\"\$(echo '${MESSAGE}' | awk -F'|' '{print $2}')\"
    if ! (echo \$ERROR_TYPE_MESSAGE | grep -q \"\$EXPECTED_ERROR_TYPE\") || ! (echo \$STATIC_ASSERT_ERROR_MESSAGE | grep -q \"\$EXPECTED_STATIC_ASSERT_ERROR\"); then
      rm -f \${F}
      echo 'The compilation of ${NAME} failed as expected, but with a different message.'
      echo \"Expected error type: \$EXPECTED_ERROR_TYPE\"
      echo \"Error type was:      \$ERROR_TYPE_MESSAGE\"
      echo \"Expected static assert: \$EXPECTED_STATIC_ASSERT_ERROR\"
      echo \"Static assert was:      \$STATIC_ASSERT_ERROR_MESSAGE\"
      exit 1
    fi
    # 6 is just a constant that works for both g++ (<=4.8.3) and clang++ (<=3.5.0). It might need to be changed.
    if [ \$STATIC_ASSERT_ERROR_LINE -gt 6 ] || [ \$ERROR_TYPE_LINE -gt 6 ]; then
      rm -f \${F}
      echo 'The compilation of ${NAME} failed with the expected message, but the error message contained too many lines before the relevant ones.'
      echo \"The error type was reported on line \$ERROR_TYPE_LINE of the message (should be <=6)\"
      echo \"The static assert was reported on line \$STATIC_ASSERT_ERROR_LINE of the message (should be <=6)\"
      exit 1
    fi
    rm -f \${F}
    ")
  add_test(NAME ${NAME}
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      COMMAND bash -c "${COMMAND}")
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

function(add_fruit_tests DIRNAME SOURCES)
  foreach(TEST_SOURCE ${${SOURCES}})
    string(REPLACE ".cpp" "" TEST_NAME ${TEST_SOURCE})
    string(REGEX REPLACE ".*/" "" TEST_NAME ${TEST_NAME})
    add_test_autodetect(${TEST_NAME})
    # Forces CMake to re-run if a test file changes.
    configure_file(${TEST_SOURCE} .${TEST_SOURCE}.unused)
  endforeach(TEST_SOURCE)
  
  # Join the list with " ".
  string(REPLACE ";" " " STR "${${SOURCES}}")
  add_test(NAME check-all-tests-listed-${DIRNAME}
           WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
           COMMAND bash -c "for f in \$(ls *.cpp); do [[ \"\$f\" == \"include_test.cpp\" ]] || echo \" ${STR} \" | fgrep -q \" \$f \" || { echo \"\$f not listed.\" && exit 1; }; done")
endfunction(add_fruit_tests)

add_fruit_tests(root TEST_SOURCES)

foreach(HEADER ${FRUIT_PUBLIC_HEADERS})
  add_library(test-header-${HEADER}-compiles "include_test.cpp")
  set_target_properties(test-header-${HEADER}-compiles PROPERTIES
    COMPILE_FLAGS "-DHEADER_UNDER_TEST='<fruit/${HEADER}.h>'")
endforeach(HEADER)

add_subdirectory(data_structures)
add_subdirectory(meta)
add_subdirectory(util)
