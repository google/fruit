load("@rules_cc//cc:cc_binary.bzl", "cc_binary")

licenses(["notice"])

cc_binary(
    name = "annotated_injection",
    srcs = glob([
        "*.cpp", 
        "*.h",
    ]),
    deps = ["//third_party/fruit"],
)
