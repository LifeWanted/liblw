workspace(name = "liblw")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Load the rules_foreign_cc tooling.
git_repository(
  name = "rules_foreign_cc",
  commit = "5a09829838662332171546ab685d494772b51523",
  shallow_since = "1613717471 -0800",
  remote = "https://github.com/bazelbuild/rules_foreign_cc.git",
)
load(
  "@rules_foreign_cc//:workspace_definitions.bzl",
  "rules_foreign_cc_dependencies"
)
rules_foreign_cc_dependencies()

git_repository(
  name = "gtest",
  commit = "1de637fbdd4ab0051229707f855eee76f5a3d5da",
  shallow_since = "1614008411 -0500",
  remote = "https://github.com/google/googletest.git",
)

http_archive(
  name = "openssl",
  strip_prefix = "openssl-openssl-3.0.0-alpha12",
  sha256 = "e092859390b998ab0c43dbe3a6691731088f4dfc76bd7f797ebcf4e5164752ef",
  build_file_content =
    'filegroup(name = "all", srcs = glob(["**"]), ' +
    'visibility = ["//visibility:public"])',
  url = "https://github.com/openssl/openssl/archive/openssl-3.0.0-alpha12.zip",
)
