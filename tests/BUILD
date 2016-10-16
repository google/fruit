
cc_library(
    name = "test_macros",
    srcs = [],
    hdrs = ["test_macros.h"],
    visibility = ["//visibility:public"],
    includes = ["."],
)

[cc_test(
    name = filename[:-4],
    srcs = [filename],
    deps = [
        ":test_macros",
        "//:fruit",
    ]
) for filename in glob(
    ["*.cpp"],
    exclude = ["include_test.cpp"])]

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
    "true",
]]


# TODO: add a test that runs check_defn_h_includes.sh

# TODO: add tests that run the python-based tests.

