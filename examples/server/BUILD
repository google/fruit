
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
