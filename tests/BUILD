
licenses(["notice"])

TEST_HEADERS = [
   "test_macros.h",
   "test_common.h",
   "class_construction_tracker.h",
]

filegroup(
    name = "test_headers_filegroup",
    srcs = TEST_HEADERS,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "test_headers",
    srcs = [],
    hdrs = TEST_HEADERS,
    visibility = ["//visibility:public"],
    includes = ["."],
)

[cc_test(
    name = filename[:-4],
    srcs = [filename],
    deps = [
        ":test_headers",
        "//third_party/fruit",
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
        deps = ["//third_party/fruit"]
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

genrule(
    name = "fruit_test_config_genrule",
    srcs = [
        "//third_party/fruit",
        "//third_party/fruit:fruit_headers",
        ":test_headers_filegroup",
    ],
    # Here we copy libfruit.so to work around an issue with py_test where the outputs of a cc_library in the data
    # attribute of a py_test are not taken into account.
    outs = [
        "fruit_test_config.py",
        "libfruit.so"
    ],
    cmd = ""
          + "FRUIT_HEADERS_LOCATION=`for f in $(locations //third_party/fruit:fruit_headers); do echo \"$$f\"; done | fgrep configuration/bazel/ | head -n 1 | sed 's|configuration/bazel/.*|./|'`;"
          + "TEST_HEADERS_LOCATION=`for f in $(locations :test_headers_filegroup); do echo \"$$f\"; done | fgrep test_macros.h | sed 's|test_macros.h|./|'`;"
          + "LIBFRUIT_LOCATION=`for f in $(locations //third_party/fruit); do echo \"$$f\"; done | fgrep libfruit.so | head -n 1 | sed 's|libfruit.so|./|'`;"
          + "cp $${LIBFRUIT_LOCATION}/libfruit.so $(location libfruit.so);"
          # The removal of ".*/genfiles" from the location is a bit of a hack, but that's how the path will look like in the py_tests
          # below.
          + "LIBFRUIT_COPY_LOCATION=`dirname $(location libfruit.so) | sed 's|.*/genfiles/|./|'`;"
          + "echo -e \""
          + "CXX='g++'\n"
          + "CXX_COMPILER_NAME='GNU'\n"
          + "FRUIT_COMPILE_FLAGS='$(CC_FLAGS) -std=c++0x -W -Wall -Wno-missing-braces -g -Werror'\n"
          + "PATH_TO_COMPILED_FRUIT='$${LIBFRUIT_COPY_LOCATION}'\n"
          + "PATH_TO_FRUIT_STATIC_HEADERS='$${FRUIT_HEADERS_LOCATION}/include'\n"
          + "PATH_TO_FRUIT_GENERATED_HEADERS='$${FRUIT_HEADERS_LOCATION}/configuration/bazel'\n"
          + "PATH_TO_FRUIT_TEST_HEADERS='$${TEST_HEADERS_LOCATION}'\n"
          + "ADDITIONAL_LINKER_FLAGS=''\n"
          + "RUN_TESTS_UNDER_VALGRIND='0'\n"
          + "VALGRIND_FLAGS=''\n"
          + "\" > $(location fruit_test_config.py)"
)

py_library(
    name = "fruit_test_common",
    srcs = ["fruit_test_common.py", "fruit_test_config.py"],
    imports = ["."],
    srcs_version = "PY3",
)

[py_test(
    name = filename[:-3],
    srcs = [filename],
    srcs_version = "PY3",
    imports = ["."],
    deps = [
        ":fruit_test_common",
    ],
    data = [
        ":libfruit.so",
        ":test_headers_filegroup",
        "//third_party/fruit:fruit_headers",
    ],
) for filename in glob(["test_*.py"])]
