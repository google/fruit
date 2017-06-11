
def fruit_py_test(filename, data=[]):
    native.py_test(
        name = filename[:-3],
        srcs = [filename],
        srcs_version = "PY3",
        imports = ["."],
        deps = [
            "//third_party/fruit/tests:fruit_test_common",
        ],
        data = data + [
            "//third_party/fruit:fruit_headers",
            "//third_party/fruit/tests:libfruit.so",
            "//third_party/fruit/tests:pytest.ini",
            "//third_party/fruit/tests:test_headers_filegroup",
        ],
        args = [
            "-p",
            "no:cacheprovider",
            # TODO: add these to make Bazel builds run in parallel even within a single target, and make them work.
            # "-c",
            # "third_party/fruit/tests/pytest.ini",
        ],
    )

