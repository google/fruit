load("@rules_cc//cc:cc_binary.bzl", "cc_binary")

licenses(["notice"])

cc_binary(
    name = "server",
    srcs = glob([
        "*.cpp", 
        "*.h",
    ]),
    linkopts = ["-pthread"],
    deps = ["//third_party/fruit"],
)
