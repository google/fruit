
licenses(["notice"])

TEST_HEADERS = [
   "test_macros.h",
   "test_common.h",
   "class_construction_tracker.h",
]

filegroup(
    name = "test_headers_filegroup",
    srcs = TEST_HEADERS,
    visibility = ["//third_party/fruit/tests:__subpackages__"],
)

cc_library(
    name = "test_headers",
    srcs = [],
    hdrs = TEST_HEADERS,
    visibility = ["//third_party/fruit/tests:__subpackages__"],
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
    toolchains = [
        # For $(CC_FLAGS)
        "@bazel_tools//tools/cpp:cc_flags",
        # for $(CC)
        "@bazel_tools//tools/cpp:current_cc_toolchain",
    ],
    visibility = ["//third_party/fruit/tests:__subpackages__"],
    cmd = ""
          + "FRUIT_HEADERS_LOCATION=`for f in $(locations //third_party/fruit:fruit_headers); do echo \"$$f\"; done | fgrep configuration/bazel/ | head -n 1 | sed 's|configuration/bazel/.*|./|'`;"
          + "TEST_HEADERS_LOCATION=`for f in $(locations :test_headers_filegroup); do echo \"$$f\"; done | fgrep test_macros.h | sed 's|test_macros.h|./|'`;"
          + "LIBFRUIT_LOCATION=`for f in $(locations //third_party/fruit); do echo \"$$f\"; done | fgrep libfruit.so | head -n 1 | sed 's|libfruit.so|./|'`;"
          + "cp $${LIBFRUIT_LOCATION}/libfruit.so $(@D)/;"
          + "echo -e \""
          + "CXX='$(CC)'\n"
          + "CXX_COMPILER_NAME='GNU'\n"
          + "CXX_COMPILER_VERSION='5.0.0'\n"
          + "FRUIT_COMPILE_FLAGS='$(CC_FLAGS) -std=c++0x -W -Wall -Wno-missing-braces -g -Werror'\n"
          + "ADDITIONAL_INCLUDE_DIRS=''\n"
          + "CMAKE_BUILD_TYPE=None\n"
          + "PATH_TO_COMPILED_FRUIT='third_party/fruit/tests'\n"
          + "PATH_TO_COMPILED_FRUIT_LIB='third_party/fruit/tests'\n"
          + "PATH_TO_FRUIT_STATIC_HEADERS='$${FRUIT_HEADERS_LOCATION}/include'\n"
          + "PATH_TO_FRUIT_GENERATED_HEADERS='$${FRUIT_HEADERS_LOCATION}/configuration/bazel'\n"
          + "PATH_TO_FRUIT_TEST_HEADERS='$${TEST_HEADERS_LOCATION}'\n"
          + "ADDITIONAL_LINKER_FLAGS='-lstdc++ -lm'\n"
          + "RUN_TESTS_UNDER_VALGRIND='0'\n"
          + "VALGRIND_FLAGS=''\n"
          + "ENABLE_COVERAGE=False\n"
          + "\" > $(location fruit_test_config.py)",
)

py_library(
    name = "fruit_test_common",
    srcs = ["fruit_test_common.py", "fruit_test_config.py"],
    imports = ["."],
    visibility = ["//third_party/fruit/tests:__subpackages__"],
)

load("//third_party/fruit/tests:build_defs.bzl", "fruit_py_tests")

fruit_py_tests(
    srcs = glob(["test_*.py"]),
)
