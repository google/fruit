
include(CMakeParseArguments)

if("${WIN32}")
    # This defaults to OFF on Windows, since we don't support PCHs there.
    option(FRUIT_TESTS_USE_PRECOMPILED_HEADERS "Whether to use pre-compiled headers (PCHs) in Fruit tests." OFF)

    if ("${FRUIT_TESTS_USE_PRECOMPILED_HEADERS}")
        # TODO: consider adding support for PCHs on Windows (at least when using MinGW).
        message(FATAL_ERROR "Please rerun CMake without -DFRUIT_TESTS_USE_PRECOMPILED_HEADERS, precompiled headers are not supported on Windows.")
    endif()
else()
    option(FRUIT_TESTS_USE_PRECOMPILED_HEADERS "Whether to use pre-compiled headers (PCHs) in Fruit tests." ON)
endif()

if("${WIN32}")
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

if(NOT "${WIN32}")
  function(check_all_python_tests_listed DIRNAME)
    # Join the list with " ".
    string(REPLACE ";" " " STR "${ARGN}")
    add_test(NAME check-all-python-tests-listed-${DIRNAME}
             WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
             COMMAND bash -c -x "pwd; for f in \$(ls test_*.py); do echo \" ${STR} \" | fgrep -q \" \$f \" || { echo \"\$f not listed.\" && exit 1; }; done")
  endfunction(check_all_python_tests_listed)
endif()

if ("${FRUIT_TESTS_USE_PRECOMPILED_HEADERS}")
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_custom_command(
            OUTPUT test_common-precompiled.h.gch
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            DEPENDS test_common.h fruit
            COMMAND bash -c "${CMAKE_CXX_COMPILER} -x c++-header ${FRUIT_COMPILE_FLAGS} -I${CMAKE_CURRENT_SOURCE_DIR}/../include -I${CMAKE_CURRENT_SOURCE_DIR} -I${CMAKE_CURRENT_BINARY_DIR}/../include ${CMAKE_CURRENT_SOURCE_DIR}/test_common.h -o test_common-precompiled.h.gch")
        add_custom_target(test-common-precompiled-header ALL DEPENDS test_common-precompiled.h.gch)
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
        add_custom_target(test-common-precompiled-header ALL DEPENDS test_common.pch)
    set(FRUIT_TESTONLY_CXXFLAGS "-include-pch ${CMAKE_CURRENT_BINARY_DIR}/test_common.pch")
  else()

    message(ERROR "Using pre-compiled headers in tests is only supported with GCC and Clang. Please add -DFRUIT_TESTS_USE_PRECOMPILED_HEADERS=OFF to your cmake invocation and try again.")
  endif()
else()
    set(FRUIT_TESTONLY_CXXFLAGS "")
endif()

if("${FRUIT_ENABLE_COVERAGE}")
  set(FRUIT_ENABLE_COVERAGE_PYTHON_BOOL "True")
else()
  set(FRUIT_ENABLE_COVERAGE_PYTHON_BOOL "False")
endif()

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(MSVC)$")
  # These warnings are disabled for tests only, since they can only be produced when using fruit as a client. Also, they cannot be disabled via pragma pushes/pops,
  #   so we leave it up to clients to disable them if desired.
  # The warning C4702 is disabled because if MSVC optimizes the call to InvokeLambdaWithInjectedArgVector::operator() when cPtr is null, it will inline
  #   a FRUIT_UNREACHABLE statement, which makes all statements succeeding the operator() call unreachable.
  # The warning C4503 is disabled because some of the test_class_destruction.py tests suchs as "test_injector_creation_and_injection"
  #   produce extremely long decorator names. This has no effect on the actual results of the test.
  set(FRUIT_TESTONLY_CXXFLAGS "${FRUIT_TESTONLY_CXXFLAGS} /wd4702 /wd4503")
endif()

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/fruit_test_config.py"
     CONTENT "
CXX='${CMAKE_CXX_COMPILER}'
CXX_COMPILER_NAME='${CMAKE_CXX_COMPILER_ID}'
CXX_COMPILER_VERSION='${CMAKE_CXX_COMPILER_VERSION}'
FRUIT_COMPILE_FLAGS='${FRUIT_COMPILE_FLAGS} ${FRUIT_TESTONLY_CXXFLAGS}'
ADDITIONAL_INCLUDE_DIRS='${BOOST_DIR}'
ADDITIONAL_LINKER_FLAGS='${CMAKE_EXE_LINKER_FLAGS}'
RUN_TESTS_UNDER_VALGRIND='${RUN_TESTS_UNDER_VALGRIND_FLAG}'
VALGRIND_FLAGS='${VALGRIND_FLAGS_STR}'
CMAKE_BUILD_TYPE='${CMAKE_BUILD_TYPE}'

PATH_TO_COMPILED_FRUIT='$<TARGET_FILE_DIR:fruit>'
PATH_TO_COMPILED_FRUIT_LIB='$<TARGET_FILE:fruit>'
PATH_TO_FRUIT_STATIC_HEADERS='${CMAKE_CURRENT_SOURCE_DIR}/../include'
PATH_TO_FRUIT_GENERATED_HEADERS='${CMAKE_CURRENT_BINARY_DIR}/../include'
PATH_TO_FRUIT_TEST_HEADERS='${CMAKE_CURRENT_SOURCE_DIR}'
ENABLE_COVERAGE=${FRUIT_ENABLE_COVERAGE_PYTHON_BOOL}
")

file(GENERATE OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/pytest.ini"
     CONTENT "
[pytest]
testpaths = \"${CMAKE_CURRENT_SOURCE_DIR}\"
addopts = -r a
")
