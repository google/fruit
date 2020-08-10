
package(default_visibility = ["//visibility:public"])
licenses(["notice"])

filegroup(
    name = "fruit_headers",
    srcs = glob([
        "include/**/*.h",         
    ]) + [
        "//third_party/fruit/configuration/bazel:fruit-config-base",
    ],
)    

cc_library(
    name = "fruit",
    srcs = glob([
        "src/*.cpp",
        "include/fruit/impl/**/*.h",
        ]),
    hdrs = glob(["include/fruit/*.h"]),
    includes = ["include", "configuration/bazel"],
    deps = [
        "@boost//:unordered",
        "//third_party/fruit/configuration/bazel:fruit-config-base",
    ],
    linkopts = ["-lm"],
)
