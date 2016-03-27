
# Similar to add_successful_compile, but tags the test with "manual" so it will not normally be built.
# This is used for targets whose build is expected to fail.
def add_pretend_compile(name):
    native.cc_test(
        name = name + "-non-buildable",
        srcs = [name + ".cpp"] + native.glob(["*.h"]),
        deps = ["//:fruit", "//tests:test_macros"],
        tags = ["manual"],
        size = "small",
    )

def add_test_expected_to_pass(name):
    native.cc_test(
        name = name,
        srcs = [name + ".cpp"] + native.glob(["*.h"]),
        deps = ["//:fruit", "//tests:test_macros"],
        size = "small",
    )

def add_test_expected_to_fail_at_runtime(name):
    source_name = name + ".cpp"
    binary_name = name + "-binary"
    native.cc_binary(
        name = binary_name,
        srcs = [source_name] + native.glob(["*.h"]),
        deps = ["//:fruit", "//tests:test_macros"],
    )
    native.sh_test(
        name = name,
        srcs = ["//tests:run_test_expected_to_fail_at_runtime_copy.sh"],
        args = ["$(location %s)" % f for f in [binary_name, source_name]],
        data = [binary_name, source_name],
        size = "small",
    )

def cc_compile_expecting_error(name, srcs, deps, includes = [], copts = []):
    """Compiles the given C++ source file expecting a compilation error.
    
    Args:
        name (str): The name of the file that will contain the compiler output.
        srcs (List(str)): A singleton list containing the file to compile.
        deps (List(str)): Headers included by srcs. Note that these must be files or filegroups, cc_library targets are not allowed.
        includes (List(str)): Directory names that should be passed to the compiler with the -I switch, if any.
        copts (List(str)): Additional flags to pass to the compiler.
    """
    src = srcs[0]
    if len(srcs) != 1:
        fail("srcs should contain 1 element", srcs)
    flags = " ".join(["-I" + d for d in includes] + copts)
    native.genrule(
        name = name + "-compile-error-genrule",
        srcs = srcs + deps,
        outs = [name],
        cmd = "if $(CC) $(CC_FLAGS) -std=c++11 " + flags + " $(location " + src + ") -o /dev/null 2> $@; then echo 'Compilation succeeded unexpectedly.'; exit 1; fi"
    )

def add_test_expected_to_fail_at_compile_time(name):
    test_source = name + ".cpp"
    compile_output = name + "-compile-output"
    cc_compile_expecting_error(
        name = compile_output,
        srcs = [test_source],
        deps = ["//:fruit_headers"] + native.glob(["*.h"]),
        includes = ["include", "configuration/bazel"],
        copts = [
            "-W", 
            "-Wall", 
            "-Werror"
        ]
    )
    native.sh_test(
        name = name,
        srcs = ["//tests:check_compile_error_for_test_expected_to_fail_at_compile_time_copy.sh"],
        args = ["$(location %s)" % f for f in [compile_output, test_source]],
        data = [compile_output, test_source],
        size = "small",
    )

def add_fruit_tests(test_lists, excluded_tests = []):
    native.sh_test(
        name = "check_bazel_test_list",
        srcs = ["//tests:check_bazel_test_list_copy.sh"],
        args = ["$(location %s)" % f for f in ["test_lists.bzl"]],
        data = ["test_lists.bzl", "//tests:compute_bazel_test_lists_copy.sh"] + native.glob(["*.cpp"]),
        size = "small",
    )
    [add_test_expected_to_pass(name) for name in test_lists["SUCCESSFUL_TESTS"] if name not in excluded_tests]
    [add_test_expected_to_fail_at_runtime(name) for name in test_lists["TESTS_EXPECTED_TO_FAIL_AT_RUNTIME"] if name not in excluded_tests]
    [add_test_expected_to_fail_at_compile_time(name) for name in test_lists["TESTS_EXPECTED_TO_FAIL_AT_COMPILE_TIME"] if name not in excluded_tests]
