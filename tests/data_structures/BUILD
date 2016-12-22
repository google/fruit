
licenses(["notice"])

[cc_test(
    name = filename[:-4],
    srcs = [filename],
    deps = [
        "//third_party/fruit/tests:test_headers",
        "//third_party/fruit:fruit",
    ]
) for filename in glob(["*.cpp"])]
