
cc_library(
    name = "simple_injection_lib",
    srcs = glob(["*.cpp"], exclude = ["main*.cpp"]),
    hdrs = glob(["*.h"]), 
    deps = ["//:fruit"],
)

cc_binary(
    name = "simple_injection",
    srcs = ["main.cpp"],
    deps = [
        ":simple_injection_lib",
        "//:fruit", 
    ],
)

cc_binary(
    name = "simple_injection_v1",
    srcs = ["main_v1.cpp"],
    deps = [
        ":simple_injection_lib",
        "//:fruit", 
    ],
)
