
load(":test_lists.bzl", "TEST_LISTS")

load(":build_defs.bzl", "add_fruit_tests")

TEST_SOURCES_GCC_ONLY = []

EXCLUDED_TESTS = [
    # This only works with Clang because GCC >=4.9 refuses to even mention the type C() when C is an abstract
    # class, while Clang allows to mention the type (but of course there can be no functions with this
    # type).
    "multibinding_error_abstract_class_clang",
]

add_fruit_tests(
    test_lists = TEST_LISTS,
    excluded_tests = EXCLUDED_TESTS
)

FRUIT_PUBLIC_HEADERS = [
    "component",
    "fruit",
    "fruit_forward_decls",
    "injector",
    "macro",
    "normalized_component",
    "provider",
]

# This tests that every public header can be #included on its own.
[(    
    cc_library(
        name = "test_header_" + name + "_compiles_lib",
        srcs = ["include_test.cpp"],
        defines = ["HEADER_UNDER_TEST='<fruit/" + name + ".h>'"],
        deps = ["//:fruit"]
    ),
    sh_test(
        name = "test_header_" + name + "_compiles",
        srcs = ["true_copy.sh"],
        args = [],
        data = [":test_header_" + name + "_compiles_lib"],
        size = "small"
    )

)
for name in FRUIT_PUBLIC_HEADERS]

# This is a workaround to convince Bazel that these script are executable.
# We can't use sh_binary here because these files are used in the srcs of a sh_test.
# We can't use export_files because that would mark the files as non-executable (as far as Bazel knows).
[genrule(
    name = name + "-genrule",
    srcs = [name + ".sh"],
    outs = [name + "_copy.sh"],
    executable = True,
    cmd = "cp $< $@",
    visibility = ["//visibility:public"])
for name in [
    "check_bazel_test_list", 
    "check_compile_error_for_test_expected_to_fail_at_compile_time",
    "run_test_expected_to_fail_at_runtime",
    "compute_bazel_test_lists",
    "true",
]]

cc_library(
    name = "test_macros",
    srcs = [],
    hdrs = ["test_macros.h"],
    visibility = ["//visibility:public"]
)
    

# TODO: add a test that runs check_defn_h_includes.sh
