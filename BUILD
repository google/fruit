
package(default_visibility = ["//visibility:public"])
licenses(["notice"])

filegroup(
    name = "fruit_headers",
    srcs = glob([
        "include/**/*.h",         
        "configuration/bazel/**/*.h",
    ]),
)    

cc_library(
    name = "fruit",
    srcs = glob([
        "src/*.cpp",
        "include/fruit/impl/**/*.h",
        "configuration/bazel/**/*.h"]),
    hdrs = glob(["include/fruit/*.h"]),
    includes = ["include", "configuration/bazel"],
    deps = [],
    linkopts = ["-lm"],
)
