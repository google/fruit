
licenses(["notice"])

[cc_test(
    name = filename[:-4],
    srcs = [filename, "common.h"],
    deps = [
        "//third_party/fruit/tests:test_headers",
        "//third_party/fruit",
    ]
) for filename in glob(["*.cpp"])]
