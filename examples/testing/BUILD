
licenses(["notice"])

cc_library(
    name = "greeter_lib",
    srcs = [
        "cached.h",
        "cached_greeter.h",
        "cached_greeter.cpp",
        "greeter.h",
        "greeter.cpp",
        "key_value_storage.h",
        "key_value_storage.cpp",
    ],
    deps = ["//third_party/fruit"],
)

cc_binary(
    name = "cached_greeter",
    srcs = ["main.cpp"],
    deps = [
        ":greeter_lib",
        "//third_party/fruit",
    ],
)

cc_library(
    name = "greeter_testing_lib",
    srcs = [
        "fake_key_value_storage.h",
        "fake_key_value_storage.cpp",
    ],
    deps = [
        ":greeter_lib",
        "//third_party/fruit",
    ],
)

cc_test(
    name = "cached_greeter_test",
    srcs = ["cached_greeter_test.cpp"],
    deps = [
        ":greeter_lib",
        ":greeter_testing_lib",
        "//third_party/fruit",
        "@com_google_googletest//:gtest",
    ],
)

cc_test(
    name = "cached_greeter_test_with_normalized_component",
    srcs = ["cached_greeter_test_with_normalized_component.cpp"],
    deps = [
        ":greeter_lib",
        ":greeter_testing_lib",
        "//third_party/fruit",
        "@com_google_googletest//:gtest",
    ],
)
