load("//third_party/fruit/configuration/bazel:build_defs.bzl", "generate_fruit_config")

package(default_visibility = ["//third_party/fruit:__subpackages__"])

generate_fruit_config(
  name = "fruit-config-base",
  check_sources = glob(["*.cpp"])
)
