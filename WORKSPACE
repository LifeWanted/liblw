workspace(name = "liblw")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Load the rules_foreign_cc tooling.
http_archive(
   name = "rules_foreign_cc",
   strip_prefix = "rules_foreign_cc-master",
   sha256 = "3e6b0691fc57db8217d535393dcc2cf7c1d39fc87e9adb6e7d7bab1483915110",
   url = "https://github.com/bazelbuild/rules_foreign_cc/archive/master.zip",
)
load(
  "@rules_foreign_cc//:workspace_definitions.bzl",
  "rules_foreign_cc_dependencies"
)
rules_foreign_cc_dependencies()

git_repository(
  name = "gtest",
  commit = "703bd9caab50b139428cea1aaff9974ebee5742e",
  shallow_since = "1570114335 -0400",
  remote = "https://github.com/google/googletest.git",
)

http_archive(
  name = "openssl",
  strip_prefix = "openssl-openssl-3.0.0-alpha9",
  sha256 = "d02655c3d807dd77d550347f8490ea272ee9c0b21917fa3f5b43e09eb2854306",
  build_file_content =
    'filegroup(name = "all", srcs = glob(["**"]), ' +
    'visibility = ["//visibility:public"])',
  url = "https://github.com/openssl/openssl/archive/openssl-3.0.0-alpha9.zip",
)
