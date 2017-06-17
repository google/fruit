
licenses(["notice"])

load("//third_party/fruit/tests:build_defs.bzl", "fruit_py_tests")

filegroup(
    name = "additional_test_headers_filegroup",
    srcs = ["common.h"],
    visibility = ["//third_party/fruit/tests:__subpackages__"],
)

fruit_py_tests(
    srcs = glob(["test_*.py"]),
    data = [":additional_test_headers_filegroup"],
)
