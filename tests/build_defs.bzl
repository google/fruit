
def fruit_py_tests(srcs, data=[]):
    for filename in srcs:
        native.py_test(
            name = filename[:-3],
            srcs = [filename],
            imports = ["."],
            timeout = "long",
            deps = [
                "//third_party/fruit/tests:fruit_test_common",
            ],
            data = data + [
                "//third_party/fruit:fruit_headers",
                "//third_party/fruit/tests:libfruit.so",
                "//third_party/fruit/tests:test_headers_filegroup",
            ],
            shard_count = 32,
        )
