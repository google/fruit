
[cc_test(
    name = filename[:-4],
    srcs = [filename],
    deps = [
        "//tests:test_macros",
        "//:fruit",
    ]
) for filename in glob(["*.cpp"])]
