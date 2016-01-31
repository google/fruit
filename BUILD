
cc_library(
    name = "fruit",
    srcs = glob([
        "src/*.cpp",
        "include/fruit/impl/**/*.h",
        "configuration/bazel/**/*.h",
        "files/*.h",]),
    hdrs = glob(["include/fruit/*.h"]),
    includes = ["include", "configuration/bazel", "files"],
    deps = [
      #"@local_stdlibcxx48//:local_stdlibcxx48"
      #"//usr/lib64/gcc/x86_64-suse-linux/4.8/include/stdarg.h",
      #"//usr/lib64/gcc/x86_64-suse-linux/4.8/include/stddef.h",
      #"//usr/lib64/gcc/x86_64-suse-linux/4.8/include/stdint.h",
    ],
)
